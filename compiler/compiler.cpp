/*
This is a fiber based framework for compiling assets and building a plugin to be used in the engine.
It will run "generators" on the assets, and run cl.exe on the source.

Todo:
	- Dependencies between assets, if one change compile the asset-chain
	- Adding a new generator won't invalidate the cache and potentially not get file changes (should get the 'added' status)
*/
#include "plugin.h"

#include "engine/utils/profiler.c"
enum ProfilerScopes {
	ProfilerScopes__parse_project,
	ProfilerScopes__setup,
	ProfilerScopes__read_cache,
	ProfilerScopes__run,
	ProfilerScopes__build,
	ProfilerScopes__write_cache,

	ProfilerScopes__count,
};

#include "engine/utils/containers/dynamic_array.h"
#include "engine/utils/containers/hashmap.cpp"
#include "engine/utils/serialize.h"
#include "engine/utils/file_system.cpp"

#include "generation_cache.cpp"
#include "project.cpp"
#include "builder.cpp"

#define FIBER_STACK_GUARD_PAGES
#include "engine/utils/fibers/task_scheduler.cpp"

#define MAX_PLUGINS 64

struct Compiler;

struct Plugin {
	PluginInfo *plugin_info;

	b32 active;

	plugin_on_file_change_t *on_file_change;
	plugin_get_file_endings_t *get_file_endings;
};
typedef Plugin* PluginArray;

struct Compiler {
	MemoryArena arena;

	TaskScheduler scheduler;
	FileSystem file_system;
	Project project;

	CompilerContext context;
	PluginArray plugin_array;
};

void register_file_info_change(Compiler &compiler, u32 handle) {
	for (i32 i = 0; i < array_count(compiler.plugin_array); ++i) {
		Plugin &plugin = compiler.plugin_array[i];

		if (plugin.active) {
			Task *task = plugin.on_file_change(compiler.context, compiler.file_system.file_infos[handle]);
			if (task) {
				scheduler_add_tasks(compiler.scheduler, 1, task);
			}
		}
	}
}

void push_directory(Plugin &plugin, u64 directory_name_id) {
	PluginInfo &info = *plugin.plugin_info;

	for (i32 i = 0; i < array_count(info.excludes); ++i) {
		PathId &ids = info.excludes[i];
		if (ids.path[ids.cursor] == directory_name_id) {
			ids.cursor++;

			if (plugin.active && ids.cursor == array_count(ids.path)) {
				plugin.active = false;
			}
		}
	}
	for (i32 i = 0; i < array_count(info.includes); ++i) {
		PathId &ids = info.includes[i];
		if (ids.path[ids.cursor] == directory_name_id) {
			ids.cursor++;

			if (!plugin.active && ids.cursor == array_count(ids.path)) {
				plugin.active = true;
			}
		}
	}
}

void pop_directory(Plugin &plugin, u64 directory_name_id) {
	PluginInfo &info = *plugin.plugin_info;

	for (i32 i = 0; i < array_count(info.excludes); ++i) {
		PathId &ids = info.excludes[i];
		if (ids.cursor > 0 && ids.path[ids.cursor-1] == directory_name_id) {
			ids.cursor--;
		}
	}
	for (i32 i = 0; i < array_count(info.includes); ++i) {
		PathId &ids = info.includes[i];
		if (ids.cursor > 0 && ids.path[ids.cursor-1] == directory_name_id) {
			ids.cursor--;
		}
	}
}

void iterate(Compiler &compiler, String directory_path) {
	WIN32_FIND_DATA find_data = {};
	char buffer[MAX_PATH];
	sprintf_s(buffer, MAX_PATH, "%s/*.*", *directory_path);
	HANDLE handle = find_first_file(buffer, &find_data);

	if (handle != INVALID_HANDLE_VALUE) {
		do {
			char *filename = find_data.cFileName;
			b32 ignored = filename[0] == '.';
			if (ignored)
				continue;

			String filename_string = make_string(filename);
			u32 filename_id = make_string_id32(filename_string);

			String file_path_string = make_path(compiler.arena, directory_path, filename_string);
			u64 file_path_id = make_string_id64(file_path_string);

			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (file_path_id != compiler.file_system.output_folder_id) {
					b32 any_active = false;
					bool changed_plugin[MAX_PLUGINS];
					for (i32 i = 0; i < array_count(compiler.plugin_array); ++i) {
						Plugin &plugin = compiler.plugin_array[i];
						b32 was_active = plugin.active;
						push_directory(plugin, filename_id);
						any_active |= plugin.active;
						changed_plugin[i] = (was_active != plugin.active);
					}

					if (any_active) {
						iterate(compiler, file_path_string);
					}

					for (i32 i = 0; i < array_count(compiler.plugin_array); ++i) {
						Plugin &plugin = compiler.plugin_array[i];
						pop_directory(plugin, filename_id);
						if (changed_plugin[i]) { // If it changed, change it back
							plugin.active = !plugin.active;
						}
					}
				}
			} else {
				Ending *ending = get_ending(compiler.file_system, filename_string);
				if (ending) {
					u32 cache_handle = read_cached_info(compiler.file_system, file_path_string, file_path_id, find_data.ftLastWriteTime, ending);
					register_file_info_change(compiler, cache_handle);
				} else {
					// printf("Ignoring ending! (filename=%s)\n", *filename_string);
				}
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}
}

////////// ADD NEW GENERATORS HERE //////////
// #include "obj/compile_obj.cpp"

void register_generators(Compiler &compiler) {
	PluginInfo *plugin_infos = compiler.project.plugin_infos;

	for (i32 i = 0; i < array_count(plugin_infos); ++i) {
		Plugin plugin = {};
		plugin.active = false; // Should it start inactive?
		plugin.plugin_info = &plugin_infos[i];

		char plugin_path[MAX_PATH];
		sprintf_s(plugin_path, ARRAY_COUNT(plugin_path), "%s/%s", *compiler.file_system.root, *plugin_infos[i].path);
		HMODULE module = LoadLibrary(plugin_path);
		plugin.on_file_change = (plugin_on_file_change_t*) GetProcAddress(module, "on_file_change");
		plugin.get_file_endings = (plugin_get_file_endings_t*) GetProcAddress(module, "get_file_endings");

		StringIdArray file_endings = {};
		plugin.get_file_endings(compiler.context, file_endings);
		for (i32 j = 0; j < file_endings.count; ++j) {
			register_ending(compiler.file_system, file_endings.ids[j].string, file_endings.ids[j].id);
		}

		array_push(compiler.plugin_array, plugin);
	};
}
//////////////////////////////////////////////

static String _fileinfo_cache_str = MAKE_STRING("fileinfo.cache");

// This runs the main fiber. All other fibers will be waiting for tasks so try to kick jobs as soon as possible!
void run(void *arg) {
	Compiler &compiler = *(Compiler*)arg;

	iterate(compiler, compiler.file_system.source_folder);

	// Go through the cache and check all the entries that weren't found when iterating over the file structure.
	// This will be the entries that corresponds to removed files.
	{ // void prune_removed_files(FileSystem &file_system) {
		Cache &cache = compiler.file_system.cache;
		for (u32 i = 0; i < cache.max_count; ++i) {
			CacheEntry &entry = cache.entries[i];
			if (!cache.touched[i] && entry.key) { // If this cache entry wasn't touched, the file that produced it have been removed.
				// file_infos[i].filepath = entry.filepath;
				String filepath = {};
				compiler.file_system.file_infos[i].filepath = filepath;
				compiler.file_system.file_infos[i].state = CacheState_Removed;
				compiler.file_system.file_infos[i].filetype = entry.filetype;
				compiler.file_system.file_infos[i].key = entry.key;

				register_file_info_change(compiler, i);
				entry.key = 0;
			}
		}
	}
	// At this point, all parsing work has been dealt out. We can't exit until all tasks are done
	// Start helping out.
	for (i32 i = 0; i < compiler.scheduler.thread_count * MAX_JOBS_PER_THREAD; ++i) {
		scheduler_wait_for_job(compiler.scheduler, i, 0);
	}
	// generate_reloader::coalesce(compiler.plugin_array[0], *scheduler);

	// bool changed = true;
	// for (u32 i = 0; i < array_count(plugin_array) && !changed; ++i) {
	// 	changed = generator_work_contexts[i].changed;
	// }
}

int main(int argc, char *argv[]) {
PROFILER_START(parse_project);
	char *project_path = 0;

	for (int i = 1; i < argc; ++i) {
		char *option = argv[i];
		if (strcmp(option, "--project") == 0) {
			project_path = argv[++i];
		}
	}

	ASSERT(project_path, "Need to specify a project file to compile! Use --project path/to/some_project");
	// printf("Running compiler for project '%s'\n", project_path);
	fflush(stdout); // We want to se the status update when running from sublime..

	Compiler compiler = {};

	compiler.context.arena = &compiler.arena;
	compiler.context.file_system = &compiler.file_system;

	parse_project(compiler.arena, compiler.project, project_path);
PROFILER_STOP(parse_project);

PROFILER_START(setup);
	init_filesystem(compiler.file_system, compiler.project.root, compiler.project.output_path, compiler.arena);
	array_init(compiler.plugin_array, 8);
	register_generators(compiler);
PROFILER_STOP(setup);

PROFILER_START(read_cache);
	FILETIME executable_modifed_time = get_current_module_filetime();
	String cache_filepath = make_path(compiler.arena, compiler.file_system.output_folder, _fileinfo_cache_str);
	read_infocache_from_disc(executable_modifed_time, cache_filepath, compiler.file_system.cache);
PROFILER_STOP(read_cache);

PROFILER_START(run);
	scheduler_start(compiler.scheduler, compiler.arena, run, &compiler);
PROFILER_STOP(run);

PROFILER_START(build);
	b32 success = build(compiler.project, compiler.file_system);
PROFILER_STOP(build);

PROFILER_START(write_cache);
	if (success) {
		write_infocache_to_disc(executable_modifed_time, cache_filepath, compiler.file_system.cache);
	}
PROFILER_STOP(write_cache);

	PROFILER_PRINT(parse_project);
	PROFILER_PRINT(setup);
	PROFILER_PRINT(read_cache);
	PROFILER_PRINT(run);
	PROFILER_PRINT(build);
	PROFILER_PRINT(write_cache);

	return !success;
}

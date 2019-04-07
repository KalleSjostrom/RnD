/*
This is a fiber based framework for compiling assets and building a plugin to be used in the engine.
It will run "generators" on the assets, and run cl.exe on the source.

Todo:
	- Dependencies between assets, if one change compile the asset-chain
	- Adding a new generator won't invalidate the cache and potentially not get file changes (should get the 'added' status)
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "common.h"

#include "modules/error.h"
#include "modules/logging.h"

#include "plugin.h"

// #include "core/utils/assert.h"
// #include "core/utils/stopwatch.h"

#include "core/math/math.h"
#include "utils/file_utils.h"
#include "utils/parser.cpp"

#include "core/memory/allocator.h"
#include "core/containers/array.h"
#include "core/containers/hashmap.cpp"
#include "core/containers/hashmap_inplace.cpp"

#include "utils/serialize.h"
#include "utils/file_system.cpp"

#include "generation_cache.cpp"
#include "project.cpp"
#include "builder.cpp"

#include "core/utils/fiber_jobs.h"

#include "core/memory/allocator.cpp"
#include "core/memory/mspace_allocator.cpp"
#include "core/memory/arena_allocator.cpp"
#include "core/utils/murmur_hash.cpp"
#include "core/containers/array.cpp"
#include "core/utils/dynamic_string.cpp"
#include "core/utils/string.cpp"

#include "core/utils/fiber_jobs.cpp"

#include "utils/profiler.cpp"
enum ProfilerScopes {
	ProfilerScopes__parse_project,
	ProfilerScopes__setup,
	ProfilerScopes__read_cache,
	ProfilerScopes__run,
	ProfilerScopes__build,
	ProfilerScopes__write_cache,
	ProfilerScopes__error_deinit,
	ProfilerScopes__log_update,
	ProfilerScopes__total,
};

#define MAX_PLUGINS 64

struct Compiler;

struct Plugin {
	PluginInfo *plugin_info;

	plugin_on_file_change_t *on_file_change;
	plugin_get_file_endings_t *get_file_endings;

	bool active;
};
typedef Plugin* PluginArray;

struct Compiler {
	ArenaAllocator arena;
	FiberJobManager *job_manager;
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
				add_tasks(compiler.job_manager, 1, task);
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

void iterate(Compiler &compiler, DynamicString &directory_path) {
	WIN32_FIND_DATA find_data = {};
	char buffer[MAX_PATH];
	sprintf_s(buffer, MAX_PATH, "%.*s/*.*", STR(directory_path));
	HANDLE handle = find_first_file(buffer, &find_data);

	if (handle != INVALID_HANDLE_VALUE) {
		do {
			char *filename = find_data.cFileName;
			bool ignored = filename[0] == '.';
			if (ignored)
				continue;

			String filename_string = string(filename);
			u32 filename_id = string_id32(filename_string);

			DynamicString &file_path_string = directory_path;
			file_path_string += "/";
			file_path_string += filename_string;
			u64 file_path_id = string_id64(file_path_string);

			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (file_path_id != compiler.file_system.output_folder_id) {
					bool any_active = false;
					bool changed_plugin[MAX_PLUGINS];
					for (i32 i = 0; i < array_count(compiler.plugin_array); ++i) {
						Plugin &plugin = compiler.plugin_array[i];
						bool was_active = plugin.active;
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

			file_path_string -= (int)filename_string.length + 1;
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
		sprintf_s(plugin_path, ARRAY_COUNT(plugin_path), "%.*s/%.*s", STR(compiler.file_system.root), STR(plugin_infos[i].path));
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

static String _fileinfo_cache_str = MAKE_STRING("");

// This runs the main fiber. All other fibers will be waiting for tasks so try to kick jobs as soon as possible!
void do_main_fiber_task(FiberJobManager *job_manager, void *user_data) {
	Compiler &compiler = *(Compiler*)user_data;

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

	// generate_reloader::coalesce(compiler.plugin_array[0], *scheduler);

	// bool changed = true;
	// for (u32 i = 0; i < array_count(plugin_array) && !changed; ++i) {
	// 	changed = generator_work_contexts[i].changed;
	// }
}

// #include "SDL.h"

int run(int argc, char *argv[]) {
PROFILER_START(total);

PROFILER_START(parse_project);
	error_init();
	log_init();

	char *project_path = 0;
	char *command_line = 0;

	for (int i = 0; i < argc; ++i) {
		char *option = argv[i];
		if (strcmp(option, "--project") == 0) {
			project_path = argv[++i];
		} else if (strcmp(option, "--command_line") == 0) {
			command_line = argv[++i];
		}
	}

	ASSERT(project_path, "Need to specify a project file to compile! Use --project path/to/some_project");
	printf("Running compiler for project '%s'\n", project_path);
	fflush(stdout); // We want to se the status update when running from sublime..

	Compiler compiler = {};

	compiler.context.allocator = allocator_arena(&compiler.arena);
	compiler.context.file_system = &compiler.file_system;

	Allocator *allocator = &compiler.context.allocator;
	parse_project(allocator, compiler.project, project_path);
PROFILER_STOP(parse_project);

PROFILER_START(setup);
	init_filesystem(compiler.file_system, compiler.project.root, compiler.project.output_path, compiler.arena);
	array_make(allocator, compiler.plugin_array, 8);
	register_generators(compiler);
PROFILER_STOP(setup);

PROFILER_START(read_cache);
	FILETIME executable_modifed_time = get_current_module_filetime();
	DynamicString cache_filepath = dynamic_stringf(allocator, "%.*s/fileinfo.cache", STR(compiler.file_system.output_folder));
	read_infocache_from_disc(executable_modifed_time, cache_filepath, compiler.file_system.cache);
PROFILER_STOP(read_cache);

PROFILER_START(run);
	FiberJobsResult result = run_fibertask(do_main_fiber_task, &compiler);
	if (result == FiberJobsResult_Failed) {
		ASSERT(false, "blah");
	}
	// scheduler_start(compiler.scheduler, compiler.arena, run, &compiler);
PROFILER_STOP(run);

PROFILER_START(build);
	bool success = build(compiler.project, compiler.file_system, command_line);
PROFILER_STOP(build);

PROFILER_START(write_cache);
	if (success) {
		write_infocache_to_disc(executable_modifed_time, cache_filepath, compiler.file_system.cache);
	}
PROFILER_STOP(write_cache);

PROFILER_START(error_deinit);
	error_deinit();
PROFILER_STOP(error_deinit);

PROFILER_START(log_update);
	log_update();
PROFILER_STOP(log_update);

PROFILER_STOP(total);

	PROFILER_PRINT(parse_project);
	PROFILER_PRINT(setup);
	PROFILER_PRINT(read_cache);
	PROFILER_PRINT(run);
	PROFILER_PRINT(build);
	PROFILER_PRINT(write_cache);
	PROFILER_PRINT(error_deinit);
	PROFILER_PRINT(log_update);
	PROFILER_PRINT(total);

	log_update();
	return !success;
}

int main(int argc, char *argv[]) {
	run(argc, argv);
	return 0;
}

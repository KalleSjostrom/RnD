#include "includes.h"

// static String _folder_generated_code = MAKE_STRING("code/generated");
// #define TYPE_INFO_FILE "typeinfo.generated.cpp"

struct Compiler;

struct Generator {
	u32 *handle_array;
	void *user_data;

	Compiler *compiler;

	b32 active;
	i32 __padding;

	String name;

	void (*register_file_info_change) (Compiler &compiler, Generator &data, FileInfo &file_info);
	bool (*ignored_directory) (String &directory, u64 directory_id);
	// bool (*generate) (Generator &data, Cache &cache, FileInfo *file_info);
};
typedef Generator* GeneratorArray;

// void activate_generators(GeneratorArray &generator_array, Location active_location) {
// 	for (i32 i = 0; i < array_count(generator_array); ++i) {
// 		Generator &generator = generator_array[i];
// 		generator.active = generator.locations & active_location;
// 	}
// }

struct Compiler {
	MemoryArena arena;

	TaskScheduler scheduler;
	FileSystem file_system;

	GeneratorArray generator_array;
};

void register_file_info_change(Compiler &compiler, u32 handle) {
	for (i32 i = 0; i < array_count(compiler.generator_array); ++i) {
		Generator &generator = compiler.generator_array[i];

		if (generator.active) {
			generator.register_file_info_change(compiler, generator, compiler.file_system.file_infos[handle]);
		}
	}
}

void iterate(Compiler &compiler, String directory_path) {
	DIR *dfd = opendir(*directory_path);
	struct dirent *dp = readdir(dfd);

	while ((dp = readdir(dfd)) != 0) {
		char *filename = dp->d_name;
		b32 ignored = filename[0] == '.' && (filename[1] == 0 || (filename[1] == '.' && filename[2] == 0));

		if (ignored)
			continue;

		String filename_string = make_string(filename);
		u32 filename_id = make_string_id32(filename_string);

		String file_path_string = make_path(compiler.arena, directory_path, filename_string);
		u64 file_path_id = make_string_id64(file_path_string);

		struct stat file_attributes;
		stat(*file_path_string, &file_attributes);
		if ((file_attributes.st_mode & S_IFMT) == S_IFDIR) {
			if (!ignored_directory(file_path_string, file_path_id, filename_string, filename_id)) {
				bool any_active = false;
				for (i32 i = 0; i < array_count(compiler.generator_array); ++i) {
					Generator &generator = compiler.generator_array[i];
					if (generator.active) {
						generator.active = !generator.ignored_directory(file_path_string, file_path_id);
						any_active |= generator.active;
					}
				}

				if (any_active) {
					iterate(compiler, file_path_string);
				}

				for (i32 i = 0; i < array_count(compiler.generator_array); ++i) {
					Generator &generator = compiler.generator_array[i];
					if (!generator.active && generator.ignored_directory(file_path_string, file_path_id)) {
						generator.active = true;
					}
				}
			}
		} else {
			Ending *ending = get_ending(compiler.file_system, filename_string);
			if (ending) {
				String cloned_file_path_string = clone_string(compiler.arena, file_path_string);

				u32 handle = read_cached_info(compiler.file_system, cloned_file_path_string, file_path_id, file_attributes.st_mtime, ending);
				register_file_info_change(compiler, handle);
			} else {
				// printf("Ignoring ending! (filename=%s)\n", *filename_string);
			}
		}
	}
}

////////// ADD NEW GENERATORS HERE //////////

// #include "../generate_global/generate_global.cpp"
#include "generate_reloader/generate_reloader.cpp"

void register_generators(Compiler &compiler) {
	for (unsigned i = 0; i < 1; ++i) {
		Generator generator = {};
		generator.compiler = &compiler;
		array_push(compiler.generator_array, generator);
	}

	generate_reloader::register_generator(compiler.generator_array[0]);
}
// ////////////////////////////////////////////

// b32 run_generator(LPVOID param) {
// 	GeneratorContext &context = *(GeneratorContext*) param;
// 	context.changed = context.generator->generate(*context.generator, *context.cache, context.file_infos);
// 	return 0;
// }

static String _fileinfo_cache_str = MAKE_STRING("fileinfo.cache");

void run(TaskScheduler *scheduler, void *arg) {
	printf("run\n");

	Compiler &compiler = *(Compiler*)arg;

	// PROFILER_START(setup_generators)
		register_generators(compiler);
	// PROFILER_STOP(setup_generators)

	// Parse the given folder
	// PROFILER_START(iterate_code)
		// activate_generators(generator_array);
		iterate(compiler, compiler.file_system.source_folder);
	// PROFILER_STOP(iterate_code)

	// PROFILER_START(check_removed_entries)
		// Go through the cache and check all the entries that weren't found when iterating over the file structure.
		// This will be the entries that corresponds to removed files.
		{ // void prune_removed_files(FileSystem &file_system) {
			Cache &cache = compiler.file_system.cache;
			for (u32 i = 0; i < cache.max_count; ++i) {
				CacheEntry &entry = cache.entries[i];
				if (!cache.touched[i] && entry.key) { // If this cache entry wasn't touched, the file that produced it have been removed.
					// file_infos[i].filepath = entry.filepath;
                    compiler.file_system.file_infos[i].filepath = {};
					compiler.file_system.file_infos[i].state = CacheState_Removed;
					compiler.file_system.file_infos[i].filetype = entry.filetype;
					compiler.file_system.file_infos[i].key = entry.key;

					register_file_info_change(compiler, i);
					entry.key = 0;
				}
			}
		}
	// PROFILER_STOP(check_removed_entries)

		// At this point, all parsing work has been dealt out. We can't exit until all tasks are done
		// Start helping out.
		printf("Start wait\n");
		for (i32 i = 0; i < scheduler->thread_count * MAX_JOBS_PER_THREAD; ++i) {
			scheduler_wait_for_job(*scheduler, i, 0);
		}
		printf("End wait\n");

		generate_reloader::coalesce(compiler.generator_array[0], *scheduler);

	// bool changed = true;
	// for (u32 i = 0; i < array_count(generator_array) && !changed; ++i) {
	// 	changed = generator_work_contexts[i].changed;
	// }

	// { // If we have no dll, then 'changed' should always be true, to try and trigger a new build
	// 	char project_output_folder_name[MAX_PATH] = {};
	// 	char project_folder[MAX_PATH] = {};
	// 	DWORD size = GetFullPathName(PROJECT_ROOT, MAX_PATH, project_folder, 0);
	// 	if (size > 0) {
	// 		int at = 0;
	// 		for (int i = size - 2; i >= 0; i--) {
	// 			if (project_folder[i] == '\\' || project_folder[i] == '/') {
	// 				at = i;
	// 				break;
	// 			}
	// 		}

	// 		int count = 0;
	// 		for (int i = at + 1; i < size - 1; i++) {
	// 			project_output_folder_name[count++] = project_folder[i];
	// 		}
	// 	}

	// 	char pattern[MAX_PATH] = {};
	// 	if (strcmp(platform, "ps4") == 0) {
	// 		sprintf(pattern, PROJECT_ROOT "../%s_data/ps4/game/*.prx", project_output_folder_name);
	// 	} else {
	// 		sprintf(pattern, PROJECT_ROOT "../%s_data/win32/game/*.dll", project_output_folder_name);
	// 	}

	// 	WIN32_FIND_DATA find_data;
	// 	HANDLE handle = FindFirstFile(pattern, &find_data);
	// 	if (handle == INVALID_HANDLE_VALUE) {
	// 		changed = true;
	// 	}
	// }

	// Build if _anything_ has changed
	// if (changed) {
	// 	// PROFILER_START(build_game)
	// 		printf("Running build_game.bat\n");
	// 		fflush(stdout); // We want to see the status update when running from sublime..
	// 		bool success = run_batch_script(0, "build_game.bat", 1, &platform);
	// 		ASSERT(success, "Failed to run build_game.bat");
	// 	// PROFILER_STOP(build_game);
	// }
}

int main(int argc, char *argv[]) {
	char *platform = "OSX";
	char *source_folder = 0;
	char *output_folder = 0;
	for (int i = 1; i < argc; ++i) {
		char *option = argv[i];
		if (strcmp(option, "--platform") == 0) {
			platform = argv[++i];
		} else if (strcmp(option, "--source") == 0) {
			source_folder = argv[++i];
		} else if (strcmp(option, "--output") == 0) {
			output_folder = argv[++i];
		}
	}

	ASSERT(source_folder, "Need to specify a source folder to generate from! Use --source path/to/source");
	ASSERT(output_folder, "Need to specify an output folder to generate from! Use --output path/to/output");

	printf("Running compiler for platform '%s'\n", platform);
	fflush(stdout); // We want to se the status update when running from sublime..

	Compiler compiler = {};

	init_filesystem(compiler.file_system, source_folder, output_folder, compiler.arena);
	array_init(compiler.generator_array, 8);

	struct stat executable_attributes;
	stat(argv[0], &executable_attributes);

	String cache_filepath = make_path(compiler.arena, compiler.file_system.output_folder, _fileinfo_cache_str);
	read_infocache_from_disc(executable_attributes.st_mtime, cache_filepath, compiler.file_system.cache);
		scheduler_start(compiler.scheduler, compiler.arena, run, &compiler);
	write_infocache_to_disc(executable_attributes.st_mtime, cache_filepath, compiler.file_system.cache);

	return 0;
}

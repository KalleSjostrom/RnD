#include "../utils/common.cpp"
#include "../utils/serialize.inl"

static const unsigned cache_version = 2;
#include "../utils/generation_cache.cpp"

#define PROFILE 0
#if PROFILE
/*
// Clean run
time: 0.000050          read_cache
time: 0.000052          allocate_memory
time: 0.000664          iterate_foundation
time: 0.001883          output_game_strings
time: 0.007409          iterate_code
time: 0.010359          total

// Consecutive run without anything changed
time: 0.000000          output_game_strings
time: 0.000046          allocate_memory
time: 0.000085          iterate_foundation
time: 0.000086          read_cache
time: 0.000292          iterate_code
time: 0.000768          total
*/
#include "../utils/profiler.c"
enum ProfilerScopes {
	ProfilerScopes__allocate_memory,
	ProfilerScopes__read_cache,
	ProfilerScopes__iterate_code,
	ProfilerScopes__iterate_foundation,
	ProfilerScopes__check_removed_files,
	ProfilerScopes__write_cache,
	ProfilerScopes__output_game_strings,
	ProfilerScopes__total,
	ProfilerScopes__count,
};
#else
#define PROFILER_START(...)
#define PROFILER_STOP(...)
#define PROFILER_PRINT(...)
#endif

enum EntryHeaderType {
	EntryHeaderType_None = 0,

	EntryHeaderType_GameStrings,
};

#include "data_game_strings.cpp"
#include "parse_for_game_strings.cpp"
#include "output_game_strings.cpp"

static const String _abilities          = MAKE_STRING("abilities");
static const String _generated          = MAKE_STRING("generated");
static const String _hg                 = MAKE_STRING(".hg");
static const String _boot               = MAKE_STRING("boot");
static const String _codegen            = MAKE_STRING("codegen");
static const String _debug              = MAKE_STRING("debug");
static const String _flow               = MAKE_STRING("flow");
static const String _game_strings       = MAKE_STRING("game_strings");
static const String _gui                = MAKE_STRING("gui");
static const String _network            = MAKE_STRING("network");
static const String _plugin_environment = MAKE_STRING("plugin_environment");
static const String _precompiled        = MAKE_STRING("precompiled");
static const String _reload             = MAKE_STRING("reload");
static const String _scripts            = MAKE_STRING("scripts");
static const String _utils              = MAKE_STRING("utils");

bool ignored_directory(String &directory) {
			// Code folder
	return  are_strings_equal(directory, _abilities) ||
			// NOTE(bauer): The generated folder is now searched for generated .game_strings files
			// are_strings_equal(directory, _generated) ||

			// Foundation folder
			are_strings_equal(directory, _hg) ||
			are_strings_equal(directory, _boot) ||
			are_strings_equal(directory, _codegen) ||
			are_strings_equal(directory, _flow) ||
			are_strings_equal(directory, _game_strings) ||
			are_strings_equal(directory, _gui) ||
			are_strings_equal(directory, _network) ||
			are_strings_equal(directory, _plugin_environment) ||
			are_strings_equal(directory, _precompiled) ||
			are_strings_equal(directory, _reload) ||
			are_strings_equal(directory, _scripts) ||
			are_strings_equal(directory, _utils);
}

void iterate(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, String &current_folder,
			CacheHashMap &cache_hash_map,
			GameStringArray &game_string_array, GameStringHashMap &game_string_hash_map) {

	WIN32_FIND_DATA find_data = {};
	char buffer[MAX_PATH];
	sprintf(buffer, "%s*.*", filepath);

	HANDLE handle = find_first_file(buffer, &find_data);
	if (handle != INVALID_HANDLE_VALUE) {
		do {
			if (find_data.cFileName[0] == '.') {
				if (find_data.cFileName[1] == '\0')
					continue;

				if (find_data.cFileName[1] == '.' && find_data.cFileName[2] == '\0')
					continue;
			}

			String filename = make_string(find_data.cFileName);
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!ignored_directory(filename)) {
					sprintf(buffer, "%s%s/", filepath, *filename);
					iterate(temp_arena, arena, buffer, filename, cache_hash_map, game_string_array, game_string_hash_map);
				}
			} else {
				MemoryBlockHandle block_handle = begin_block(temp_arena);
				uint64_t key; CacheHashEntry *entry;

				if ((!are_strings_equal(current_folder, _generated) && (string_ends_in(filename, MAKE_STRING(".h")) || string_ends_in(filename, MAKE_STRING(".cpp")))) || string_ends_in(filename, MAKE_STRING(".game_strings"))) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);

					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_GameStrings, &key, &entry)) {
						unsigned count_snapshot = game_string_array.count;
						parse_for_game_strings(temp_arena, arena, buffer, game_string_array, game_string_hash_map);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						game_string::serialize(arena, count_snapshot, entry, game_string_array);
					} else { // We have a valid cache entry, use that instead of reparsing the file!
						game_string::deserialize(arena, buffer, entry, game_string_array, game_string_hash_map);
					}
				}
				end_block(temp_arena, block_handle);
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}
}

int main(int argc, char *argv[]) {
PROFILER_START(total)
	PROFILER_START(allocate_memory)
		MemoryArena temp_arena = init_memory(32*MB);
		MemoryArena arena = init_memory(32*MB);

		unsigned max_string_count = 8192;

		GameStringArray game_string_array = game_string::make_array(arena, max_string_count);
		// Game string hash map
		unsigned hash_count = max_string_count *  1.5;
		unsigned hash_size = sizeof(GameStringHashEntry) * hash_count;
		GameStringHashEntry *entries = (GameStringHashEntry *)allocate_memory(arena, hash_size);
		memset(entries, 0, hash_size);
		GameStringHashMap game_string_hash_map = { entries, hash_count, };
	PROFILER_STOP(allocate_memory)

	PROFILER_START(read_cache)
		CacheHashMap cache_hash_map = {};
		read_cache_from_disc(arena, cache_hash_map, cache_version);
	PROFILER_STOP(read_cache)

	// Parse the "code" folder
	PROFILER_START(iterate_code)
		iterate(temp_arena, arena, "../../"GAME_CODE_DIR"/", MAKE_STRING("code"), cache_hash_map, game_string_array, game_string_hash_map);
	PROFILER_STOP(iterate_code)

	PROFILER_START(iterate_foundation)
		iterate(temp_arena, arena, "../../", MAKE_STRING("foundation"), cache_hash_map, game_string_array, game_string_hash_map);
	PROFILER_STOP(iterate_foundation)

	// Go through the cache and check all the entries that weren't found when iterating over the file structure.
	// This will be the entries that corresponds to removed files. We need to make sure to mark these entries as changed, so that the output is regenerated.
	PROFILER_START(check_removed_entries)
		for (int i = 0; i < ARRAY_COUNT(cache_hash_map.entries); ++i) {
			CacheHashEntry &entry = cache_hash_map.entries[i];
			if (!entry.touched && entry.key) { // If this cache entry wasn't 'touched', the file that produced it have been removed.
				switch (entry.header_type) {
					case EntryHeaderType_GameStrings: {
						game_string_array.changed = true;
					}; break;
					default: {
						ASSERT(false, "Unknown cache head type! %u", entry.header_type);
					};
				};
			}
		}
	PROFILER_STOP(check_removed_entries)

	PROFILER_START(write_cache)
		write_cache_to_disc(cache_hash_map, cache_version);
	PROFILER_STOP(write_cache)

	PROFILER_START(output_game_strings)
		if (game_string_array.has_changed())
			output_game_strings(game_string_array);
	PROFILER_STOP(output_game_strings)
PROFILER_STOP(total)

	PROFILER_PRINT(allocate_memory);
	PROFILER_PRINT(read_cache);
	PROFILER_PRINT(iterate_code);
	PROFILER_PRINT(iterate_foundation);
	PROFILER_PRINT(check_removed_entries);
	PROFILER_PRINT(write_cache);
	PROFILER_PRINT(output_game_strings);
	PROFILER_PRINT(total);

	return 0;
}
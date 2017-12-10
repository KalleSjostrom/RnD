#include "../utils/common.cpp"
#include "../utils/exceptions.h"

#include "../utils/serialize.inl"

static const unsigned cache_version = 2;
#include "../utils/generation_cache.cpp"

#define PROFILE 0
#if PROFILE
/*
// Clean run

// Consecutive run without anything changed
*/
#include "../utils/profiler.c"
enum ProfilerScopes {
	ProfilerScopes__exceptions_setup,
	ProfilerScopes__allocate_memory,
	ProfilerScopes__read_cache,
	ProfilerScopes__iterate_code,
	ProfilerScopes__iterate_foundation,
	ProfilerScopes__check_removed_entries,
	ProfilerScopes__write_cache,
	ProfilerScopes__output_reloader,
	ProfilerScopes__exceptions_shutdown,
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

	EntryHeaderType_Reloadable,
};

#include "data_reloadable.cpp"

static parser::Token include_token         = TOKENIZE("include");
static parser::Token reloadable_token      = TOKENIZE("reloadable");
static parser::Token not_reloadable_token  = TOKENIZE("not_reloadable");
static parser::Token struct_token          = TOKENIZE("struct");
static parser::Token on_added_token        = TOKENIZE("on_added");
static parser::Token zero_token            = TOKENIZE("zero");
static parser::Token construct_token       = TOKENIZE("construct");
static parser::Token class_token           = TOKENIZE("class");
static parser::Token friend_token          = TOKENIZE("friend");
static parser::Token Reloader_token        = TOKENIZE("Reloader");
static parser::Token static_token          = TOKENIZE("static");
static parser::Token const_token           = TOKENIZE("const");
static parser::Token __forceinline_token   = TOKENIZE("__forceinline");
static parser::Token inline_token          = TOKENIZE("inline");
static parser::Token count_token           = TOKENIZE("count");
static parser::Token namespace_token       = TOKENIZE("namespace");
static parser::Token BEGIN_GENERATED_token = TOKENIZE("BEGIN_GENERATED");
static parser::Token volatile_token        = TOKENIZE("volatile");
static parser::Token enum_token            = TOKENIZE("enum");
static parser::Token union_token           = TOKENIZE("union");

#include "parse_hfile.cpp"
#include "output_reloader.cpp"

// Code
static const String _abilities = MAKE_STRING("abilities");

// Foundation
static const String _hg                 = MAKE_STRING(".hg");
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
			// Code
	return  are_strings_equal(directory, _abilities) ||
			// Foundation
			are_strings_equal(directory, _hg) ||
			are_strings_equal(directory, _codegen) ||
			are_strings_equal(directory, _debug) ||
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

void iterate(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, CacheHashMap &cache_hash_map, ReloadableArray &reloadable_array) {
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
					iterate(temp_arena, arena, buffer, cache_hash_map, reloadable_array);
				}
			} else {
				MemoryBlockHandle block_handle = begin_block(temp_arena);
				uint64_t key; CacheHashEntry *entry;

				if (string_ends_in(filename, MAKE_STRING(".h"))) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);

					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_Reloadable, &key, &entry)) {
						unsigned count_snapshot = reloadable_array.count;
						parse_hfile(temp_arena, arena, buffer, reloadable_array);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						reloadable::serialize(arena, count_snapshot, entry, reloadable_array);
					} else { // We have a valid cache entry, use that instead of reparsing the file!
						reloadable::deserialize(arena, buffer, entry, reloadable_array);
					}
				}
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}
}

int main(int argc, char *argv[]) {
PROFILER_START(total)
	PROFILER_START(exceptions_setup)
		exceptions_setup();
	PROFILER_STOP(exceptions_setup)

	PROFILER_START(allocate_memory)
		unsigned reloadable_count = 1024;

		MemoryArena data_arena = init_memory(reloadable::get_size(reloadable_count));

		ReloadableArray reloadable_array = reloadable::make_array(data_arena, reloadable_count);

		MemoryArena temp_arena = init_memory(32*MB);
		MemoryArena arena = init_memory(32*MB);
	PROFILER_STOP(allocate_memory)

	PROFILER_START(read_cache)
		CacheHashMap cache_hash_map = {};
		read_cache_from_disc(arena, cache_hash_map, cache_version);
	PROFILER_STOP(read_cache)

	PROFILER_START(iterate_code)
		iterate(temp_arena, arena, "../../"GAME_CODE_DIR"/", cache_hash_map, reloadable_array);
	PROFILER_STOP(iterate_code)

	PROFILER_START(iterate_foundation)
		iterate(temp_arena, arena, "../../", cache_hash_map, reloadable_array);
	PROFILER_STOP(iterate_foundation)

	// Go through the cache and check all the entries that weren't found when iterating over the file structure.
	// This will be the entries that corresponds to removed files. We need to make sure to mark these entries as changed, so that the output is regenerated.
	PROFILER_START(check_removed_entries)
		for (int i = 0; i < ARRAY_COUNT(cache_hash_map.entries); ++i) {
			CacheHashEntry &entry = cache_hash_map.entries[i];
			if (!entry.touched && entry.key) { // If this cache entry wasn't 'touched', the file that produced it have been removed.
				switch (entry.header_type) {
					case EntryHeaderType_Reloadable: {
						reloadable_array.changed = true;
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

	PROFILER_START(output_reloader)
		if (reloadable_array.has_changed())
			output_reloader(reloadable_array, temp_arena);
	PROFILER_STOP(output_reloader)

	PROFILER_START(exceptions_shutdown)
		exceptions_shutdown();
	PROFILER_STOP(exceptions_shutdown)
PROFILER_STOP(total)

	PROFILER_PRINT(exceptions_setup);
	PROFILER_PRINT(allocate_memory);
	PROFILER_PRINT(read_cache);
	PROFILER_PRINT(iterate_code);
	PROFILER_PRINT(iterate_foundation);
	PROFILER_PRINT(check_removed_entries);
	PROFILER_PRINT(write_cache);
	PROFILER_PRINT(output_reloader);
	PROFILER_PRINT(exceptions_shutdown);
	PROFILER_PRINT(total);
}

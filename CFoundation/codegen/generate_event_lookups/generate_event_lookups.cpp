#include "../utils/common.cpp"
#include "../utils/serialize.inl"

static const unsigned cache_version = 2;
#include "../utils/generation_cache.cpp"

#define PROFILE 0
#if PROFILE
/*
// Clean run
time: 0.000040          read_cache
time: 0.000046          allocate_memory
time: 0.000244          write_cache
time: 0.001106          output_flow
time: 0.001517          iterate_characters
time: 0.002888          output_animation
time: 0.005544          iterate_levels
time: 0.011384          total

// Consecutive run without anything changed
time: 0.000000          output_animation
time: 0.000000          output_flow
time: 0.000042          allocate_memory
time: 0.000078          read_cache
time: 0.000207          write_cache
time: 0.000338          iterate_characters
time: 0.001402          iterate_levels
time: 0.002067          total
*/
#include "../utils/profiler.c"
enum ProfilerScopes {
	ProfilerScopes__allocate_memory,
	ProfilerScopes__read_cache,
	ProfilerScopes__iterate_content,
	ProfilerScopes__check_removed_entries,
	ProfilerScopes__write_cache,
	ProfilerScopes__output_flow,
	ProfilerScopes__output_animation,
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

	EntryHeaderType_Flow,
	EntryHeaderType_Animation,
};

#include "data_event.cpp"

struct EventHashEntry {
	unsigned key;
};
struct EventHashMap {
	EventHashEntry *entries;
	unsigned max_count;
};

void add_event(MemoryArena &arena, String &event, EventArray &event_array, EventHashMap &event_hash_map) {
	unsigned id = make_string_id(event);
	HASH_LOOKUP(entry, event_hash_map.entries, event_hash_map.max_count, id);
	if (entry->key == 0) { // Didn't  exist
		// Add it to the hash map
		entry->key = id;

		// Add it to the array
		ARRAY_CHECK_BOUNDS(event_array);
		Event &animation_event = event_array.entries[event_array.count++];
		animation_event.name = clone_string(event, arena);
		animation_event.name_id = id;
	}
}

void add_cached_events(unsigned count_snapshot, EventArray &event_array, EventHashMap &event_hash_map) {
	for (unsigned i = count_snapshot; i < event_array.count; i++) {
		Event &event = event_array.entries[i];
		HASH_LOOKUP(entry, event_hash_map.entries, event_hash_map.max_count, event.name_id);
		if (entry->key == 0) { // Didn't  exist
			// Add it to the hash map
			entry->key = event.name_id;
		} else {
			// If they were in the hash map, then remove them from the list
			event_array.entries[i--] = event_array.entries[--event_array.count];
		}
	}
}

#include "parse_for_flow_events.cpp"
#include "parse_for_animation_events.cpp"

#include "output_event_lookup.cpp"

static const String flow_ending = MAKE_STRING(".flow");
static const String level_ending = MAKE_STRING(".level");
static const String anim_controller_ending = MAKE_STRING(".anim_controller");

static const String FlowEvent = MAKE_STRING("FlowEvent");
static const String flow_lookup = MAKE_STRING("flow_lookup");
static const String AnimationEvent = MAKE_STRING("AnimationEvent");
static const String animation_lookup = MAKE_STRING("animation_lookup");

// Folders
static const String animations = MAKE_STRING("animations"); // The anim_controller should never be placed in the animations folder
static const String textures = MAKE_STRING("textures");
static const String materials = MAKE_STRING("materials");

bool ignored_directory(String &directory) {
	return  are_strings_equal(directory, animations) ||
			are_strings_equal(directory, textures) ||
			are_strings_equal(directory, materials);
}

void iterate(MemoryArena &temp_arena, MemoryArena &arena, char *filepath,
			CacheHashMap &cache_hash_map,
			EventArray &flow_event_array, EventHashMap &flow_event_hash_map,
			EventArray &animation_event_array, EventHashMap &animation_event_hash_map) {

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
					iterate(temp_arena, arena, buffer, cache_hash_map, flow_event_array, flow_event_hash_map, animation_event_array, animation_event_hash_map);
				}
			} else {
				MemoryBlockHandle block_handle = begin_block(temp_arena);
				uint64_t key; CacheHashEntry *entry;

				if (string_ends_in(filename, flow_ending) || string_ends_in(filename, level_ending)) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);

					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_Flow, &key, &entry)) {
						unsigned count_snapshot = flow_event_array.count;
						parse_for_flow_events(temp_arena, arena, buffer, flow_event_array, flow_event_hash_map);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						event::serialize(arena, count_snapshot, entry, flow_event_array);
					} else { // We have a valid cache entry, use that instead of reparsing the file!
						unsigned count_snapshot = flow_event_array.count;
						event::deserialize(arena, buffer, entry, flow_event_array);
						add_cached_events(count_snapshot, flow_event_array, flow_event_hash_map);
					}
				} else if (string_ends_in(filename, anim_controller_ending)) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);

					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_Animation, &key, &entry)) {
						unsigned count_snapshot = animation_event_array.count;
						parse_for_animation_events(temp_arena, arena, buffer, animation_event_array, animation_event_hash_map);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						event::serialize(arena, count_snapshot, entry, animation_event_array);
					} else { // We have a valid cache entry, use that instead of reparsing the file!
						unsigned count_snapshot = animation_event_array.count;
						event::deserialize(arena, buffer, entry, animation_event_array);
						add_cached_events(count_snapshot, animation_event_array, animation_event_hash_map);
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

		unsigned max_flow_event_count = 1024;
		unsigned flow_hash_count = max_flow_event_count * 1.5;
		unsigned flow_hash_size = sizeof(EventHashEntry) * flow_hash_count;

		unsigned max_animation_event_count = 1024;
		unsigned animation_hash_count = max_animation_event_count * 1.5;
		unsigned animation_hash_size = sizeof(EventHashEntry) * animation_hash_count;

		EventArray flow_event_array = event::make_array(arena, max_flow_event_count);
		// Flow event hash map
		EventHashEntry *flow_entries = (EventHashEntry *)allocate_memory(arena, flow_hash_size);
		memset(flow_entries, 0, flow_hash_size);
		EventHashMap flow_event_hash_map = { flow_entries, flow_hash_count, };

		EventArray animation_event_array = event::make_array(arena, max_animation_event_count);
		// Animation event hash map
		EventHashEntry *animation_entries = (EventHashEntry *)allocate_memory(arena, animation_hash_size);
		memset(animation_entries, 0, animation_hash_size);
		EventHashMap animation_event_hash_map = { animation_entries, animation_hash_count, };
	PROFILER_STOP(allocate_memory)

	PROFILER_START(read_cache)
		CacheHashMap cache_hash_map = {};
		read_cache_from_disc(arena, cache_hash_map, cache_version);
	PROFILER_STOP(read_cache)

	PROFILER_START(iterate_content)
		iterate(temp_arena, arena, "../../../content/", cache_hash_map, flow_event_array, flow_event_hash_map, animation_event_array, animation_event_hash_map);
	PROFILER_STOP(iterate_content)

	// Go through the cache and check all the entries that weren't found when iterating over the file structure.
	// This will be the entries that corresponds to removed files. We need to make sure to mark these entries as changed, so that the output is regenerated.
	PROFILER_START(check_removed_entries)
		for (int i = 0; i < ARRAY_COUNT(cache_hash_map.entries); ++i) {
			CacheHashEntry &entry = cache_hash_map.entries[i];
			if (!entry.touched && entry.key) { // If this cache entry wasn't 'touched', the file that produced it have been removed.
				switch (entry.header_type) {
					case EntryHeaderType_Flow: {
						flow_event_array.changed = true;
					}; break;
					case EntryHeaderType_Animation: {
						animation_event_array.changed = true;
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

	PROFILER_START(output_flow)
		if (flow_event_array.has_changed())
			output_event_lookup(flow_event_array, FlowEvent, flow_lookup);
	PROFILER_STOP(output_flow)

	PROFILER_START(output_animation)
		if (animation_event_array.has_changed())
			output_event_lookup(animation_event_array, AnimationEvent, animation_lookup);
	PROFILER_STOP(output_animation)
PROFILER_STOP(total)

	PROFILER_PRINT(allocate_memory);
	PROFILER_PRINT(read_cache);
	PROFILER_PRINT(iterate_content);
	PROFILER_PRINT(check_removed_entries);
	PROFILER_PRINT(write_cache);
	PROFILER_PRINT(output_flow);
	PROFILER_PRINT(output_animation);
	PROFILER_PRINT(total);

	return 0;
}
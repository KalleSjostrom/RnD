enum StringFormat {
	StringFormat_ID32,
	StringFormat_ID64,
};

namespace game_string {
	struct Data {
		String key;
		unsigned key_id;
		
		String value;
		uint64_t value_id;

		StringFormat format;
	};
	struct HashEntry {
		unsigned key;
		uint64_t value_id;
	};
	struct HashMap {
		HashEntry *entries;
		unsigned max_count;
	};

	#include "../utils/data_generic.inl"

	void write(unsigned count_snapshot, Array &array, char **buffer) {
		unsigned delta;

		delta = array.count - count_snapshot;
		write_unsigned(delta, buffer);
		if (delta > 0) {
			array.changed = true;
			for (int i = count_snapshot; i < array.count; ++i) {
				Data &data = array.entries[i];
				write_string(data.key, buffer);
				write_unsigned(data.key_id, buffer);
				write_string(data.value, buffer);
				write_u64(data.value_id, buffer);
				write_unsigned(*(unsigned*)&data.format, buffer);
			}
		}
	}

	void read(MemoryArena &arena, Array &array, HashMap &hash_map, char **buffer) {
		unsigned num_entries;

		read_unsigned(num_entries, buffer);
		for (int i = 0; i < num_entries; ++i) {
			Data data = {};

			read_string(arena, data.key, buffer);
			read_unsigned(data.key_id, buffer);
			read_string(arena, data.value, buffer);
			read_u64(data.value_id, buffer);
			read_unsigned(*(unsigned*)&data.format, buffer);

			HASH_LOOKUP(entry, hash_map.entries, hash_map.max_count, data.key_id);
			if (entry->key == 0) {
				entry->key = data.key_id;
				entry->value_id = data.value_id;
				array.entries[array.count++] = data;
			}
		}
	}

	void serialize(MemoryArena &arena, unsigned count_snapshot, CacheHashEntry *entry, Array &array) {
		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		write(count_snapshot, array, &buffer);
		
		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Array &array, HashMap &hash_map) {
		char *buffer = (char*)entry->value.buffer;

		read(arena, array, hash_map, &buffer);
	}
}

typedef game_string::Data GameString;
typedef game_string::Array GameStringArray;
typedef game_string::HashEntry GameStringHashEntry;
typedef game_string::HashMap GameStringHashMap;

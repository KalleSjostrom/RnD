enum StringFormat {
	StringFormat_ID32,
	StringFormat_ID64,
};

struct GameString {
	String key;
	unsigned key_id;

	String value;
	uint64_t value_id;

	StringFormat format;
};
typedef GameString* GameStringArray;

struct GameStringHashEntry {
	unsigned key;
	uint64_t value;
};
struct GameStringHashMap {
	GameStringHashEntry *entries;
	unsigned max_count;
};

namespace game_string {
	void write(GameString &data, char **buffer) {
		write_string(data.key, buffer);
		write_unsigned(data.key_id, buffer);
		write_string(data.value, buffer);
		write_u64(data.value_id, buffer);
		write_unsigned(*(unsigned*)&data.format, buffer);
	}

	bool write_array(unsigned count_snapshot, GameStringArray &array, char **buffer) {
		int delta = array_count(array) - count_snapshot;
		write_int(delta, buffer);

		bool changed = delta > 0;
		if (changed) {
			for (int i = count_snapshot; i < array_count(array); ++i) {
				GameString &data = array[i];
				write(data, buffer);
			}
		}
		return changed;
	}

	void read(MemoryArena &arena, GameString &data, char **buffer) {
		read_string(arena, data.key, buffer);
		read_unsigned(data.key_id, buffer);
		read_string(arena, data.value, buffer);
		read_u64(data.value_id, buffer);
		read_unsigned(*(unsigned*)&data.format, buffer);
	}

	void read_array(MemoryArena &arena, GameStringArray &array, char **buffer) {
		int delta;
		read_int(delta, buffer);
		for (int i = 0; i < delta; ++i) {
			array_new_entry(array);
			GameString &data = array_last(array);
			read(arena, data, buffer);
		}
	}

	bool serialize(MemoryArena &arena, unsigned count_snapshot, CacheHashEntry *entry, GameStringArray &array) {
		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		bool changed = write_array(count_snapshot, array, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;

		return changed;
	}

	void deserialize(MemoryArena &arena, CacheHashEntry *entry, GameStringArray &array) {
		char *buffer = (char*)entry->value.buffer;

		read_array(arena, array, &buffer);
	}
}

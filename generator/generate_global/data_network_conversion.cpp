namespace network_conversions {
	struct Data {
		String name;
		unsigned name_id;
		String type;
		String from;
		String to;
	};

	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_string(arena, data.type, buffer);
		read_string(arena, data.from, buffer);
		read_string(arena, data.to, buffer);
	}
	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_string(data.type, buffer);
		write_string(data.from, buffer);
		write_string(data.to, buffer);
	}

	bool serialize(MemoryArena &arena, CacheHashEntry *entry, unsigned count_snapshot, Data* &array) {
		bool changed = false;

		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		int delta = array_count(array) - count_snapshot;
		write_int(delta, &buffer);
		if (delta > 0) {
			changed = true;
			for (int i = count_snapshot; i < array_count(array); ++i) {
				write(array[i], &buffer);
			}
		}

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;
		return changed;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Data* &array) {
		char *buffer = (char*)entry->value.buffer;
		int delta;
		read_int(delta, &buffer);
		for (int i = 0; i < delta; ++i) {
			array_new_entry(array);
			Data &data = array_last(array);
			read(arena, data, &buffer);
		}
	}
}

typedef network_conversions::Data NetworkConversion;
typedef NetworkConversion* NetworkConversionArray;

namespace event {
	struct Data {
		String name;
		unsigned name_id;
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
				write_string(data.name, buffer);
				write_unsigned(data.name_id, buffer);
			}
		}
	}

	void read(MemoryArena &arena, Array &array, char **buffer) {
		unsigned num_entries;

		read_unsigned(num_entries, buffer);
		for (int i = 0; i < num_entries; ++i) {
			Data &data = array.entries[array.count++];
			read_string(arena, data.name, buffer);
			read_unsigned(data.name_id, buffer);
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

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Array &array) {
		char *buffer = (char*)entry->value.buffer;

		read(arena, array, &buffer);
	}
}

typedef event::Data Event;
typedef event::Array EventArray;

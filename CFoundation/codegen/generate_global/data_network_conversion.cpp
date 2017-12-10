namespace network_conversions {
	struct Data {
		String name;
		unsigned name_id;
		String type;
		String from;
		String to;
	};

	#include "../utils/data_generic.inl"

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

	void serialize(MemoryArena &arena, CacheHashEntry *entry, unsigned count_snapshot, Array &array) {
		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		unsigned delta = array.count - count_snapshot;
		write_unsigned(delta, &buffer);
		if (delta > 0) {
			array.changed = true;
			for (int i = count_snapshot; i < array.count; ++i) {
				write(array.entries[i], &buffer);
			}
		}

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Array &array) {
		char *buffer = (char*)entry->value.buffer;

		unsigned num_entries;
		read_unsigned(num_entries, &buffer);
		for (int i = 0; i < num_entries; ++i) {
			Data &data = array.entries[array.count++];
			read(arena, data, &buffer);
		}
	}
}

typedef network_conversions::Data NetworkConversion;
typedef network_conversions::Array NetworkConversionArray;

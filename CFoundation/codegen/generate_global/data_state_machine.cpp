namespace state_machine {
	struct Data {
		String name;
		unsigned name_id;

		unsigned function_count;
		Function functions[32];

		unsigned state_count;
		State *states[64];
	};

	#include "../utils/data_generic.inl"

	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_unsigned(data.function_count, buffer);
		for (int i = 0; i < data.function_count; ++i) {
			Function &function = data.functions[i];
			read_function(arena, function, buffer);
		}
	}
	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_unsigned(data.function_count, buffer);
		for (int i = 0; i < data.function_count; ++i) {
			Function &function = data.functions[i];
			write_function(function, buffer);
		}
	}

	void serialize(MemoryArena &arena, CacheHashEntry *entry, Array &array) {
		// Get the entry where the parsed data was placed 
		Data &data = array.entries[array.count - 1];
		CachedData &value = entry->value;

		array.changed = true;

		value.buffer = arena.memory + arena.offset;
		char *buffer = (char*)value.buffer;

		intptr_t start = (intptr_t)buffer;
		write(data, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		value.size = size;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Array &array) {
		Data &data = array.entries[array.count++];
		// data.path = make_project_path(filepath, arena);
		char *buffer = (char*)entry->value.buffer;
		read(arena, data, &buffer);
	}
}

typedef state_machine::Data StateMachine;
typedef state_machine::Array StateMachineArray;

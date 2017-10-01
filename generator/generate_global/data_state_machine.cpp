typedef State** StatePointerArray;

struct StateMachine {
	String name;
	unsigned name_id;

	FunctionArray functions;
	StatePointerArray states;
};
typedef StateMachine* StateMachineArray;

namespace state_machine {
	void read(MemoryArena &arena, StateMachine &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_serialized_array(data.functions, buffer);
		for (int i = 0; i < array_count(data.functions); ++i) {
			Function &function = data.functions[i];
			function::read(arena, function, buffer);
		}
	}
	void write(StateMachine &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_int(array_count(data.functions), buffer);
		for (int i = 0; i < array_count(data.functions); ++i) {
			Function &function = data.functions[i];
			function::write(function, buffer);
		}
	}

	bool serialize(MemoryArena &arena, CacheHashEntry *entry, StateMachineArray &array) {
		// Get the entry where the parsed data was placed
		StateMachine &data = array_last(array);
		CachedData &value = entry->value;

		value.buffer = arena.memory + arena.offset;
		char *buffer = (char*)value.buffer;

		intptr_t start = (intptr_t)buffer;
		write(data, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		value.size = size;

		return true;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, StateMachineArray &array) {
		array_new_entry(array);
		StateMachine &data = array_last(array);
		// data.path = make_project_path(filepath, arena);
		char *buffer = (char*)entry->value.buffer;
		read(arena, data, &buffer);
		array_init(data.states, 32);
	}
}

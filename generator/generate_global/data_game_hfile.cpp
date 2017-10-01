enum DefaultSettingsType {
	DefaultSettingsType_None = 0, // No default value

	DefaultSettingsType_Value, // Using //! default_value
	DefaultSettingsType_Exported, // Using //! export
	DefaultSettingsType_SettingsStruct, // Defaults is gotten from the settings struct
};

enum MemberFlag {
	MemberFlag_IsPointer = 1 << 0,
	MemberFlag_IsArray = 1 << 1,
	MemberFlag_IsBehaviorNode = 1 << 2,
	MemberFlag_BehaviorNodeReset = 1 << 3,
	MemberFlag_BehaviorNodePrefix = 1 << 4,
};
#define FLAG_SET(member, flag) ((member).flags & flag)
#define SET_FLAG(member, flag) ((member).flags |= flag)
#define CLEAR_FLAG(member, flag) ((member).flags &= ~flag)

//// MEMBER ////
struct Member {
	String name;
	unsigned name_id;

	String type;
	unsigned type_id;

	unsigned flags;
	DefaultSettingsType default_settings_type;

	unsigned max_count; // If is_array, this is what is written inbetween the brackets, [].

	unsigned exported_line_counter;
	String exported_lines[8];

	String default_value;
};
void write_member(Member &member, char **buffer) {
	write_string(member.name, buffer);
	write_unsigned(member.name_id, buffer);

	write_string(member.type, buffer);
	write_unsigned(member.type_id, buffer);

	write_unsigned(member.flags, buffer);

	write_unsigned((unsigned)member.default_settings_type, buffer);
	write_unsigned(member.max_count, buffer);

	write_unsigned(member.exported_line_counter, buffer);
	for (unsigned i = 0; i < member.exported_line_counter; ++i) {
		write_string(member.exported_lines[i], buffer);
	}
	write_string(member.default_value, buffer);
}
void read_member(MemoryArena &arena, Member &member, char **buffer) {
	read_string(arena, member.name, buffer);
	read_unsigned(member.name_id, buffer);

	read_string(arena, member.type, buffer);
	read_unsigned(member.type_id, buffer);

	read_unsigned(member.flags, buffer);

	read_unsigned(*(unsigned*)&member.default_settings_type, buffer);
	read_unsigned(member.max_count, buffer);

	read_unsigned(member.exported_line_counter, buffer);
	for (unsigned i = 0; i < member.exported_line_counter; ++i) {
		read_string(arena, member.exported_lines[i], buffer);
	}
	read_string(arena, member.default_value, buffer);
}

typedef Member* MemberArray;

#include "../script/data_script.cpp"

//// RECEIVER ////
namespace receiver {
	struct Data {
		String name;
		unsigned name_id;
		String name_underscored;
		unsigned num_events;
	};
	__forceinline Data make_data(String &name, MemoryArena &arena) {
		Data data = { };
		data.name = name;
		data.name_id = to_id32(name.length, *name);
		data.name_underscored = make_underscore_case(name, arena);
		return data;
	}
	__forceinline Data make_empty() {
		Data data = {};
		return data;
	}

	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);
		write_string(data.name_underscored, buffer);
		write_unsigned(data.num_events, buffer);
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);
		read_string(arena, data.name_underscored, buffer);
		read_unsigned(data.num_events, buffer);
	}
}
typedef receiver::Data Receiver;
typedef Receiver* ReceiverArray;

//// FUNCTION ////
namespace function {
	struct Data {
		ParameterArray parameters;
		unsigned receiver_name_id;

		String name;
		unsigned name_id;
	};
	void write(Data &data, char **buffer) {
		write_int(array_count(data.parameters), buffer);
		for (unsigned i = 0; i < array_count(data.parameters); ++i) {
			write_parameter(data.parameters[i], buffer);
		}
		write_unsigned(data.receiver_name_id, buffer);

		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		int parameter_count;
		read_int(parameter_count, buffer);
		array_init(data.parameters, parameter_count);
		array_set_count(data.parameters, parameter_count);
		for (unsigned i = 0; i < parameter_count; ++i) {
			Parameter &parameter = data.parameters[i];
			read_parameter(arena, parameter, buffer);
		}
		read_unsigned(data.receiver_name_id, buffer);

		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);
	}
}

typedef function::Data Function;
typedef Function* FunctionArray;

//// RPC FUNCTION ////
namespace rpc {
	struct Data {
		Function function;
		bool session_bound;
	};
	void read(MemoryArena &arena, Data &data, char **buffer) {
		function::read(arena, data.function, buffer);
		read_bool(data.session_bound, buffer);
	}
	void write(Data &data, char **buffer) {
		function::write(data.function, buffer);
		write_bool(data.session_bound, buffer);
	}
}
typedef rpc::Data RPCFunction;
typedef RPCFunction* RPCArray;

//// EVENTS ////
namespace event {
	struct Data {
		Function function;

		unsigned max_number_of_entities; // Only used if this is an entity event
		unsigned component_count; // If this is > 0 then this is an entity event
		unsigned component_ids[8];
	};

	void write(Data &data, char **buffer) {
		function::write(data.function, buffer);

		write_unsigned(data.component_count, buffer);
		for (unsigned i = 0; i < data.component_count; ++i) {
			write_unsigned(data.component_ids[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		function::read(arena, data.function, buffer);

		read_unsigned(data.component_count, buffer);
		for (unsigned i = 0; i < data.component_count; ++i) {
			read_unsigned(data.component_ids[i], buffer);
		}
	}
}
typedef event::Data Event;
typedef Event* EventArray;

#define ARRAY_NAME FunctionMap
#define ARRAY_TYPE String
#define ARRAY_MAX_SIZE 16
#include "../utils/array_static.cpp"

//// FLOW FUNCTION ////
namespace flow {
	struct Data {
		Function function;

		bool suppress_default_output_events;

		FunctionMap function_map;
		ParameterArray return_list;
	};
	void write(Data &data, char **buffer) {
		function::write(data.function, buffer);

		write_bool(data.suppress_default_output_events, buffer);

		write_unsigned(data.function_map.count, buffer);
		for (unsigned i = 0; i < data.function_map.count; ++i) {
			write_string(data.function_map[i], buffer);
		}

		write_int(array_count(data.return_list), buffer);
		for (unsigned i = 0; i < array_count(data.return_list); ++i) {
			write_parameter(data.return_list[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		function::read(arena, data.function, buffer);

		read_bool(data.suppress_default_output_events, buffer);

		read_unsigned(data.function_map.count, buffer);
		for (unsigned i = 0; i < data.function_map.count; ++i) {
			read_string(arena, data.function_map[i], buffer);
		}

		int return_list_count;
		read_int(return_list_count, buffer);
		array_init(data.return_list, return_list_count);
		array_set_count(data.return_list, return_list_count);
		for (unsigned i = 0; i < return_list_count; ++i) {
			read_parameter(arena, data.return_list[i], buffer);
		}
	}
}
typedef flow::Data FlowFunction;
typedef FlowFunction* FlowArray;

//// NETWORK TYPE ////
namespace network_type {
	struct Data {
		String name;
		unsigned name_id;
		String name_lower_case;
		String type;

		bool is_array;

		// only if not is_array
		unsigned count;

		// only if is_array
		Member member;
		String max_size;
	};

	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_string(data.name_lower_case, buffer);
		write_string(data.type, buffer);

		write_bool(data.is_array, buffer);

		// only if not is_array
		write_unsigned(data.count, buffer);

		// only if is_array
		write_member(data.member, buffer);
		write_string(data.max_size, buffer);
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_string(arena, data.name_lower_case, buffer);
		read_string(arena, data.type, buffer);

		read_bool(data.is_array, buffer);

		// only if not is_array
		read_unsigned(data.count, buffer);

		// only if is_array
		read_member(arena, data.member, buffer);
		read_string(arena, data.max_size, buffer);
	}
}
typedef network_type::Data NetworkType;
typedef NetworkType* NetworkTypeArray;



//// EXPORTED ENUM ////
namespace settings_enum {
	struct Data {
		String name;
		unsigned name_id;
		String *entry_array;
	};

	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_int(array_count(data.entry_array), buffer);
		for (unsigned i = 0; i < array_count(data.entry_array); ++i) {
			write_string(data.entry_array[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_serialized_array(data.entry_array, buffer);
		for (unsigned i = 0; i < array_count(data.entry_array); ++i) {
			read_string(arena, data.entry_array[i], buffer);
		}
	}
}
typedef settings_enum::Data SettingsEnum;
typedef SettingsEnum* SettingsEnumArray;


//// EXPORTED STRUCT ////
namespace settings_struct {
	struct Data {
		String name;
		unsigned name_id;
		MemberArray member_array;
	};

	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_int(array_count(data.member_array), buffer);
		for (unsigned i = 0; i < array_count(data.member_array); ++i) {
			write_member(data.member_array[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_serialized_array(data.member_array, buffer);
		for (unsigned i = 0; i < array_count(data.member_array); ++i) {
			read_member(arena, data.member_array[i], buffer);
		}
	}
}
typedef settings_struct::Data SettingsStruct;
typedef SettingsStruct* SettingsStructArray;


//// STATE ////
namespace state {
	struct Data {
		String machine_name;
		unsigned machine_name_id;

		String path;

		String name;
	};

	void write(Data &data, char **buffer) {
		write_string(data.machine_name, buffer);
		write_unsigned(data.machine_name_id, buffer);

		write_string(data.path, buffer);
		write_string(data.name, buffer);
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.machine_name, buffer);
		read_unsigned(data.machine_name_id, buffer);

		read_string(arena, data.path, buffer);
		read_string(arena, data.name, buffer);
	}
}
typedef state::Data State;
typedef State* StateArray;


//// COLLECTION ////
struct HFileCollection {
	RPCArray rpc_array;
	EventArray event_array;
	FlowArray flow_array;
	NetworkTypeArray network_type_array;
	ReceiverArray receiver_array;
	StateArray state_array;
	SettingsEnumArray settings_enum_array;
	SettingsStructArray settings_struct_array;
	AbilityNodeArray ability_node_array;
	BehaviorNodeArray behavior_node_array;

	bool rpc_array_changed;
	bool event_array_changed;
	bool flow_array_changed;
	bool network_type_array_changed;
	bool receiver_array_changed;
	bool state_array_changed;
	bool settings_enum_array_changed;
	bool settings_struct_array_changed;
	bool ability_node_array_changed;
	bool behavior_node_array_changed;
};

HFileCollection make_hfile_collection() {
	HFileCollection c = {};

	array_init(c.rpc_array, RPC_COUNT);
	array_init(c.event_array, EVENT_COUNT);
	array_init(c.flow_array, FLOW_COUNT);
	array_init(c.network_type_array, NETWORK_TYPE_COUNT);
	array_init(c.receiver_array, RECEIVER_COUNT);
	array_init(c.state_array, STATE_COUNT);
	array_init(c.settings_enum_array, SETTINGS_ENUM_COUNT);
	array_init(c.settings_struct_array, SETTINGS_STRUCT_COUNT);
	array_init(c.ability_node_array, ABILITY_NODE_COUNT);
	array_init(c.behavior_node_array, BEHAVIOR_NODE_COUNT);

	return c;
}


//// SERIALIZE ////
struct CountSnapshot {
	unsigned rpc;
	unsigned event;
	unsigned flow;
	unsigned network_type;
	unsigned receiver;
	unsigned state;
	unsigned settings_enum;
	unsigned settings_struct;
	unsigned ability_node;
	unsigned behavior_node;
};
CountSnapshot take_count_snapshot(HFileCollection &c) {
	CountSnapshot count_snapshot = {};

	count_snapshot.rpc = array_count(c.rpc_array);
	count_snapshot.event = array_count(c.event_array);
	count_snapshot.flow = array_count(c.flow_array);
	count_snapshot.network_type = array_count(c.network_type_array);
	count_snapshot.receiver = array_count(c.receiver_array);
	count_snapshot.state = array_count(c.state_array);
	count_snapshot.settings_enum = array_count(c.settings_enum_array);
	count_snapshot.settings_struct = array_count(c.settings_struct_array);
	count_snapshot.ability_node = array_count(c.ability_node_array);
	count_snapshot.behavior_node = array_count(c.behavior_node_array);

	return count_snapshot;
}

void write_hfile(CountSnapshot &count_snapshot, HFileCollection &c, char **buffer) {
	unsigned rpc_array_delta = array_count(c.rpc_array) - count_snapshot.rpc;
	unsigned event_array_delta = array_count(c.event_array) - count_snapshot.event;
	unsigned flow_array_delta = array_count(c.flow_array) - count_snapshot.flow;
	unsigned network_type_array_delta = array_count(c.network_type_array) - count_snapshot.network_type;
	unsigned receiver_array_delta = array_count(c.receiver_array) - count_snapshot.receiver;
	unsigned state_array_delta = array_count(c.state_array) - count_snapshot.state;
	unsigned settings_enum_array_delta = array_count(c.settings_enum_array) - count_snapshot.settings_enum;
	unsigned settings_struct_array_delta = array_count(c.settings_struct_array) - count_snapshot.settings_struct;
	unsigned ability_node_array_delta = array_count(c.ability_node_array) - count_snapshot.ability_node;
	unsigned behavior_node_array_delta = array_count(c.behavior_node_array) - count_snapshot.behavior_node;

	write_unsigned(rpc_array_delta, buffer);
	write_unsigned(event_array_delta, buffer);
	write_unsigned(flow_array_delta, buffer);
	write_unsigned(network_type_array_delta, buffer);
	write_unsigned(receiver_array_delta, buffer);
	write_unsigned(state_array_delta, buffer);
	write_unsigned(settings_enum_array_delta, buffer);
	write_unsigned(settings_struct_array_delta, buffer);
	write_unsigned(ability_node_array_delta, buffer);
	write_unsigned(behavior_node_array_delta, buffer);

	if (rpc_array_delta > 0) {
		c.rpc_array_changed = true;
		for (unsigned i = count_snapshot.rpc; i < array_count(c.rpc_array); ++i) {
			rpc::write(c.rpc_array[i], buffer);
		}
	}

	if (event_array_delta > 0) {
		c.event_array_changed = true;
		for (unsigned i = count_snapshot.event; i < array_count(c.event_array); ++i) {
			event::write(c.event_array[i], buffer);
		}
	}

	if (flow_array_delta > 0) {
		c.flow_array_changed = true;
		for (unsigned i = count_snapshot.flow; i < array_count(c.flow_array); ++i) {
			flow::write(c.flow_array[i], buffer);
		}
	}

	if (network_type_array_delta > 0) {
		c.network_type_array_changed = true;
		for (unsigned i = count_snapshot.network_type; i < array_count(c.network_type_array); ++i) {
			network_type::write(c.network_type_array[i], buffer);
		}
	}

	if (receiver_array_delta > 0) {
		c.receiver_array_changed = true;
		for (unsigned i = count_snapshot.receiver; i < array_count(c.receiver_array); ++i) {
			receiver::write(c.receiver_array[i], buffer);
		}
	}

	if (state_array_delta > 0) {
		c.state_array_changed = true;
		for (unsigned i = count_snapshot.state; i < array_count(c.state_array); ++i) {
			state::write(c.state_array[i], buffer);
		}
	}

	if (settings_enum_array_delta > 0) {
		c.settings_enum_array_changed = true;
		for (unsigned i = count_snapshot.settings_enum; i < array_count(c.settings_enum_array); ++i) {
			settings_enum::write(c.settings_enum_array[i], buffer);
		}
	}

	if (settings_struct_array_delta > 0) {
		c.settings_struct_array_changed = true;
		for (unsigned i = count_snapshot.settings_struct; i < array_count(c.settings_struct_array); ++i) {
			settings_struct::write(c.settings_struct_array[i], buffer);
		}
	}

	if (ability_node_array_delta > 0) {
		c.ability_node_array_changed = true;
		for (unsigned i = count_snapshot.ability_node; i < array_count(c.ability_node_array); ++i) {
			ability_node::write(c.ability_node_array[i], buffer);
		}
	}

	if (behavior_node_array_delta > 0) {
		c.behavior_node_array_changed = true;
		for (unsigned i = count_snapshot.behavior_node; i < array_count(c.behavior_node_array); ++i) {
			behavior_node::write(c.behavior_node_array[i], buffer);
		}
	}
}

void update_changed_arrays(CountSnapshot &count_snapshot, HFileCollection &c) {
	c.rpc_array_changed = c.rpc_array_changed || count_snapshot.rpc > 0;
	c.event_array_changed = c.event_array_changed || count_snapshot.event > 0;
	c.flow_array_changed = c.flow_array_changed || count_snapshot.flow > 0;
	c.network_type_array_changed = c.network_type_array_changed || count_snapshot.network_type > 0;
	c.receiver_array_changed = c.receiver_array_changed || count_snapshot.receiver > 0;
	c.state_array_changed = c.state_array_changed || count_snapshot.state > 0;
	c.settings_enum_array_changed = c.settings_enum_array_changed || count_snapshot.settings_enum > 0;
	c.settings_struct_array_changed = c.settings_struct_array_changed || count_snapshot.settings_struct > 0;
	c.ability_node_array_changed = c.ability_node_array_changed || count_snapshot.ability_node > 0;
	c.behavior_node_array_changed = c.behavior_node_array_changed || count_snapshot.behavior_node > 0;
}

CountSnapshot read_hfile_snapshot(char **buffer) {
	CountSnapshot count_snapshot = {};

	read_unsigned(count_snapshot.rpc, buffer);
	read_unsigned(count_snapshot.event, buffer);
	read_unsigned(count_snapshot.flow, buffer);
	read_unsigned(count_snapshot.network_type, buffer);
	read_unsigned(count_snapshot.receiver, buffer);
	read_unsigned(count_snapshot.state, buffer);
	read_unsigned(count_snapshot.settings_enum, buffer);
	read_unsigned(count_snapshot.settings_struct, buffer);
	read_unsigned(count_snapshot.ability_node, buffer);
	read_unsigned(count_snapshot.behavior_node, buffer);

	return count_snapshot;
}

void read_hfile(MemoryArena &arena, HFileCollection &c, char **buffer) {
	CountSnapshot count_snapshot = read_hfile_snapshot(buffer);

	for (unsigned i = 0; i < count_snapshot.rpc; ++i) {
		array_new_entry(c.rpc_array);
		rpc::read(arena, array_last(c.rpc_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.event; ++i) {
		array_new_entry(c.event_array);
		event::read(arena, array_last(c.event_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.flow; ++i) {
		array_new_entry(c.flow_array);
		flow::read(arena, array_last(c.flow_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.network_type; ++i) {
		array_new_entry(c.network_type_array);
		network_type::read(arena, array_last(c.network_type_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.receiver; ++i) {
		array_new_entry(c.receiver_array);
		receiver::read(arena, array_last(c.receiver_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.state; ++i) {
		array_new_entry(c.state_array);
		state::read(arena, array_last(c.state_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.settings_enum; ++i) {
		array_new_entry(c.settings_enum_array);
		settings_enum::read(arena, array_last(c.settings_enum_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.settings_struct; ++i) {
		array_new_entry(c.settings_struct_array);
		settings_struct::read(arena, array_last(c.settings_struct_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.ability_node; ++i) {
		array_new_entry(c.ability_node_array);
		ability_node::read(arena, array_last(c.ability_node_array), buffer);
	}
	for (unsigned i = 0; i < count_snapshot.behavior_node; ++i) {
		array_new_entry(c.behavior_node_array);
		behavior_node::read(arena, array_last(c.behavior_node_array), buffer);
	}
}

void serialize_hfile(MemoryArena &arena, CacheHashEntry *entry, CountSnapshot &count_snapshot, HFileCollection &collection) {
	char *buffer = arena.memory + arena.offset;
	entry->value.buffer = buffer;
	intptr_t start = (intptr_t)buffer;

	write_hfile(count_snapshot, collection, &buffer);

	size_t size = (size_t)((intptr_t)buffer - start);
	arena.offset += size;
	entry->value.size = size;
}

void deserialize_hfile(MemoryArena &arena, char *filepath, CacheHashEntry *entry, HFileCollection &collection) {
	char *buffer = (char*)entry->value.buffer;

	read_hfile(arena, collection, &buffer);
}

CountSnapshot deserialize_snapshot(MemoryArena &arena, CacheHashEntry *entry, HFileCollection &collection) {
	if (entry->key != 0) { // we have no entry in the cache for this
		char *buffer = (char*)entry->value.buffer;
		return read_hfile_snapshot(&buffer);
	} else {
		CountSnapshot snapshot = {};
		return snapshot;
	}
}
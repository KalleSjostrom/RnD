//// MEMBER ////
struct Member {
	String name;
	unsigned name_id;

	String type;
	unsigned type_id;

	bool is_pointer;
	bool has_default_settings;

	unsigned exported_line_counter;
	String exported_lines[8];

	String default_value;
};
void write_member(Member &member, char **buffer) {
	write_string(member.name, buffer);
	write_unsigned(member.name_id, buffer);

	write_string(member.type, buffer);
	write_unsigned(member.type_id, buffer);

	write_bool(member.is_pointer, buffer);
	write_bool(member.has_default_settings, buffer);

	write_unsigned(member.exported_line_counter, buffer);
	for (int i = 0; i < member.exported_line_counter; ++i) {
		write_string(member.exported_lines[i], buffer);
	}
	write_string(member.default_value, buffer);
}
void read_member(MemoryArena &arena, Member &member, char **buffer) {
	read_string(arena, member.name, buffer);
	read_unsigned(member.name_id, buffer);

	read_string(arena, member.type, buffer);
	read_unsigned(member.type_id, buffer);

	read_bool(member.is_pointer, buffer);
	read_bool(member.has_default_settings, buffer);

	read_unsigned(member.exported_line_counter, buffer);
	for (int i = 0; i < member.exported_line_counter; ++i) {
		read_string(arena, member.exported_lines[i], buffer);
	}
	read_string(arena, member.default_value, buffer);
}

//// PARAMETER ////
struct Parameter {
	String type;
	unsigned type_id;
	String name;
	unsigned name_id;
	bool is_const;
	bool is_pointer;
	bool is_ref;
};
void write_parameter(Parameter &parameter, char **buffer) {
	write_string(parameter.type, buffer);
	write_unsigned(parameter.type_id, buffer);

	write_string(parameter.name, buffer);
	write_unsigned(parameter.name_id, buffer);

	write_bool(parameter.is_const, buffer);
	write_bool(parameter.is_pointer, buffer);
	write_bool(parameter.is_ref, buffer);
}
void read_parameter(MemoryArena &arena, Parameter &parameter, char **buffer) {
	read_string(arena, parameter.type, buffer);
	read_unsigned(parameter.type_id, buffer);

	read_string(arena, parameter.name, buffer);
	read_unsigned(parameter.name_id, buffer);

	read_bool(parameter.is_const, buffer);
	read_bool(parameter.is_pointer, buffer);
	read_bool(parameter.is_ref, buffer);
}

//// RECEIVER ////
namespace receiver {
	struct Data {
		String name;
		unsigned name_id;
		String name_underscored;
		unsigned num_events;
	};
	__forceinline Data make_data(String &string, MemoryArena &arena) {
		Data data = { string, to_id32(string.length, *string), make_underscore_case(string, arena) };
		return data;
	}
	__forceinline Data make_empty() {
		Data data = {};
		return data;
	}
	#include "../utils/data_generic.inl"

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
typedef receiver::Array ReceiverArray;

//// FUNCTION ////
enum FunctionType {
	FunctionType_GlobalReceiver,
	FunctionType_EntityReceiver,
	FunctionType_EntitySender,
};
struct Function {
	unsigned parameter_count;
	Parameter parameters[32];

	unsigned receiver_name_id;
	String max_entities; // Only valid if FunctionType is FunctionType_EntitySender
	FunctionType function_type;

	String name;
	unsigned name_id;
};
void write_function(Function &function, char **buffer) {
	write_unsigned(function.parameter_count, buffer);
	for (int i = 0; i < function.parameter_count; ++i) {
		write_parameter(function.parameters[i], buffer);
	}

	write_unsigned(function.receiver_name_id, buffer);
	write_unsigned(function.function_type, buffer);

	write_string(function.name, buffer);
	write_unsigned(function.name_id, buffer);
}
void read_function(MemoryArena &arena, Function &function, char **buffer) {
	read_unsigned(function.parameter_count, buffer);
	for (int i = 0; i < function.parameter_count; ++i) {
		Parameter &parameter = function.parameters[i];
		read_parameter(arena, parameter, buffer);
	}

	read_unsigned(function.receiver_name_id, buffer);
	read_unsigned(*(unsigned*)&function.function_type, buffer);

	read_string(arena, function.name, buffer);
	read_unsigned(function.name_id, buffer);
}

//// RPC FUNCTION ////
namespace rpc {
	struct Data {
		Function function;
		bool session_bound;
	};
	#include "../utils/data_generic.inl"
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_function(arena, data.function, buffer);
		read_bool(data.session_bound, buffer);
	}
	void write(Data &data, char **buffer) {
		write_function(data.function, buffer);
		write_bool(data.session_bound, buffer);
	}
}
typedef rpc::Data RPCFunction;
typedef rpc::Array RPCArray;

//// EVENTS ////
namespace event {
	typedef Function Data;
	#include "../utils/data_generic.inl"
}
typedef event::Data Event;
typedef event::Array EventArray;

//// FLOW FUNCTION ////
namespace flow {
	struct Data {
		Function function;

		bool suppress_default_output_events;

		unsigned function_map_count;
		String function_map[16];

		unsigned return_list_count;
		Parameter return_list[16];
	};
	#include "../utils/data_generic.inl"
	void write(Data &data, char **buffer) {
		write_function(data.function, buffer);

		write_bool(data.suppress_default_output_events, buffer);

		write_unsigned(data.function_map_count, buffer);
		for (int i = 0; i < data.function_map_count; ++i) {
			write_string(data.function_map[i], buffer);
		}

		write_unsigned(data.return_list_count, buffer);
		for (int i = 0; i < data.return_list_count; ++i) {
			write_parameter(data.return_list[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_function(arena, data.function, buffer);

		read_bool(data.suppress_default_output_events, buffer);

		read_unsigned(data.function_map_count, buffer);
		for (int i = 0; i < data.function_map_count; ++i) {
			read_string(arena, data.function_map[i], buffer);
		}

		read_unsigned(data.return_list_count, buffer);
		for (int i = 0; i < data.return_list_count; ++i) {
			read_parameter(arena, data.return_list[i], buffer);
		}
	}
}
typedef flow::Data FlowFunction;
typedef flow::Array FlowArray;

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
	#include "../utils/data_generic.inl"

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
typedef network_type::Array NetworkTypeArray;



//// EXPORTED ENUM ////
namespace settings_enum {
	struct Data {
		String name;
		unsigned name_id;

		unsigned count;
		String entries[64];
	};
	#include "../utils/data_generic.inl"

	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_unsigned(data.count, buffer);
		for (unsigned i = 0; i < data.count; ++i) {
			write_string(data.entries[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_unsigned(data.count, buffer);
		for (unsigned i = 0; i < data.count; ++i) {
			read_string(arena, data.entries[i], buffer);
		}
	}
}
typedef settings_enum::Data SettingsEnum;
typedef settings_enum::Array SettingsEnumArray;


//// EXPORTED STRUCT ////
namespace settings_struct {
	struct Data {
		String name;
		unsigned name_id;

		Member members[64];
		unsigned member_count;
	};
	#include "../utils/data_generic.inl"

	void write(Data &data, char **buffer) {
		write_string(data.name, buffer);
		write_unsigned(data.name_id, buffer);

		write_unsigned(data.member_count, buffer);
		for (unsigned i = 0; i < data.member_count; ++i) {
			write_member(data.members[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_string(arena, data.name, buffer);
		read_unsigned(data.name_id, buffer);

		read_unsigned(data.member_count, buffer);
		for (unsigned i = 0; i < data.member_count; ++i) {
			read_member(arena, data.members[i], buffer);
		}
	}
}
typedef settings_struct::Data SettingsStruct;
typedef settings_struct::Array SettingsStructArray;


//// STATE ////
namespace state {
	struct Data {
		String machine_name;
		unsigned machine_name_id;

		String path;

		String name;
	};
	#include "../utils/data_generic.inl"

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
typedef state::Array StateArray;

//// SERIALIZE ////
struct CountSnapshot {
	unsigned rpc;
	unsigned event;
	unsigned flow;
	unsigned network_type;
	unsigned event_receiver;
	unsigned state;
	unsigned settings_enum;
	unsigned settings_struct;
};
CountSnapshot take_count_snapshot(RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
	CountSnapshot count_snapshot = {};

	count_snapshot.rpc = rpc_array.count;
	count_snapshot.event = event_array.count;
	count_snapshot.flow = flow_array.count;
	count_snapshot.network_type = network_type_array.count;
	count_snapshot.event_receiver = event_receiver_array.count;
	count_snapshot.state = state_array.count;
	count_snapshot.settings_enum = settings_enum_array.count;
	count_snapshot.settings_struct = settings_struct_array.count;

	return count_snapshot;
}

void write_hfile(CountSnapshot &count_snapshot, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array, char **buffer) {
	unsigned rpc_array_delta = rpc_array.count - count_snapshot.rpc;
	unsigned event_array_delta = event_array.count - count_snapshot.event;
	unsigned flow_array_delta = flow_array.count - count_snapshot.flow;
	unsigned network_type_array_delta = network_type_array.count - count_snapshot.network_type;
	unsigned event_receiver_array_delta = event_receiver_array.count - count_snapshot.event_receiver;
	unsigned state_array_delta = state_array.count - count_snapshot.state;
	unsigned settings_enum_array_delta = settings_enum_array.count - count_snapshot.settings_enum;
	unsigned settings_struct_array_delta = settings_struct_array.count - count_snapshot.settings_struct;

	write_unsigned(rpc_array_delta, buffer);
	write_unsigned(event_array_delta, buffer);
	write_unsigned(flow_array_delta, buffer);
	write_unsigned(network_type_array_delta, buffer);
	write_unsigned(event_receiver_array_delta, buffer);
	write_unsigned(state_array_delta, buffer);
	write_unsigned(settings_enum_array_delta, buffer);
	write_unsigned(settings_struct_array_delta, buffer);

	if (rpc_array_delta > 0) {
		rpc_array.changed = true;
		for (int i = count_snapshot.rpc; i < rpc_array.count; ++i) {
			rpc::write(rpc_array.entries[i], buffer);
		}
	}

	if (event_array_delta > 0) {
		event_array.changed = true;
		for (int i = count_snapshot.event; i < event_array.count; ++i) {
			write_function(event_array.entries[i], buffer);
		}
	}

	if (flow_array_delta > 0) {
		flow_array.changed = true;
		for (int i = count_snapshot.flow; i < flow_array.count; ++i) {
			flow::write(flow_array.entries[i], buffer);
		}
	}

	if (network_type_array_delta > 0) {
		network_type_array.changed = true;
		for (int i = count_snapshot.network_type; i < network_type_array.count; ++i) {
			network_type::write(network_type_array.entries[i], buffer);
		}
	}

	if (event_receiver_array_delta > 0) {
		event_receiver_array.changed = true;
		for (int i = count_snapshot.event_receiver; i < event_receiver_array.count; ++i) {
			receiver::write(event_receiver_array.entries[i], buffer);
		}
	}

	if (state_array_delta > 0) {
		state_array.changed = true;
		for (int i = count_snapshot.state; i < state_array.count; ++i) {
			state::write(state_array.entries[i], buffer);
		}
	}

	if (settings_enum_array_delta > 0) {
		settings_enum_array.changed = true;
		for (int i = count_snapshot.settings_enum; i < settings_enum_array.count; ++i) {
			settings_enum::write(settings_enum_array.entries[i], buffer);
		}
	}

	if (settings_struct_array_delta > 0) {
		settings_struct_array.changed = true;
		for (int i = count_snapshot.settings_struct; i < settings_struct_array.count; ++i) {
			settings_struct::write(settings_struct_array.entries[i], buffer);
		}
	}
}

void update_changed_arrays(CountSnapshot &count_snapshot, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
	rpc_array.changed = rpc_array.changed || count_snapshot.rpc > 0;
	event_array.changed = event_array.changed || count_snapshot.event > 0;
	flow_array.changed = flow_array.changed || count_snapshot.flow > 0;
	network_type_array.changed = network_type_array.changed || count_snapshot.network_type > 0;
	event_receiver_array.changed = event_receiver_array.changed || count_snapshot.event_receiver > 0;
	state_array.changed = state_array.changed || count_snapshot.state > 0;
	settings_enum_array.changed = settings_enum_array.changed || count_snapshot.settings_enum > 0;
	settings_struct_array.changed = settings_struct_array.changed || count_snapshot.settings_struct > 0;
}

CountSnapshot read_hfile_snapshot(char **buffer) {
	CountSnapshot count_snapshot = {};

	read_unsigned(count_snapshot.rpc, buffer);
	read_unsigned(count_snapshot.event, buffer);
	read_unsigned(count_snapshot.flow, buffer);
	read_unsigned(count_snapshot.network_type, buffer);
	read_unsigned(count_snapshot.event_receiver, buffer);
	read_unsigned(count_snapshot.state, buffer);
	read_unsigned(count_snapshot.settings_enum, buffer);
	read_unsigned(count_snapshot.settings_struct, buffer);

	return count_snapshot;
}

void read_hfile(MemoryArena &arena, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array, char **buffer) {
	CountSnapshot count_snapshot = read_hfile_snapshot(buffer);

	for (int i = 0; i < count_snapshot.rpc; ++i) {
		RPCFunction &rpc_function = rpc_array.entries[rpc_array.count++];
		rpc::read(arena, rpc_function, buffer);
	}
	for (int i = 0; i < count_snapshot.event; ++i) {
		Event &event_function = event_array.entries[event_array.count++];
		read_function(arena, event_function, buffer);
	}
	for (int i = 0; i < count_snapshot.flow; ++i) {
		FlowFunction &flow_function = flow_array.entries[flow_array.count++];
		flow::read(arena, flow_function, buffer);
	}
	for (int i = 0; i < count_snapshot.network_type; ++i) {
		NetworkType &network_type = network_type_array.entries[network_type_array.count++];
		network_type::read(arena, network_type, buffer);
	}
	for (int i = 0; i < count_snapshot.event_receiver; ++i) {
		Receiver &receiver = event_receiver_array.entries[event_receiver_array.count++];
		receiver::read(arena, receiver, buffer);
	}
	for (int i = 0; i < count_snapshot.state; ++i) {
		State &state = state_array.entries[state_array.count++];
		state::read(arena, state, buffer);
	}
	for (int i = 0; i < count_snapshot.settings_enum; ++i) {
		SettingsEnum &settings_enum = settings_enum_array.entries[settings_enum_array.count++];
		settings_enum::read(arena, settings_enum, buffer);
	}
	for (int i = 0; i < count_snapshot.settings_struct; ++i) {
		SettingsStruct &settings_struct = settings_struct_array.entries[settings_struct_array.count++];
		settings_struct::read(arena, settings_struct, buffer);
	}
}

void serialize_hfile(MemoryArena &arena, CacheHashEntry *entry, CountSnapshot &count_snapshot, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
	char *buffer = arena.memory + arena.offset;
	entry->value.buffer = buffer;
	intptr_t start = (intptr_t)buffer;

	write_hfile(count_snapshot, rpc_array, event_array, flow_array, network_type_array, event_receiver_array, state_array, settings_enum_array, settings_struct_array, &buffer);

	size_t size = (size_t)((intptr_t)buffer - start);
	arena.offset += size;
	entry->value.size = size;
}

void deserialize_hfile(MemoryArena &arena, char *filepath, CacheHashEntry *entry, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
	char *buffer = (char*)entry->value.buffer;

	read_hfile(arena, rpc_array, event_array, flow_array, network_type_array, event_receiver_array, state_array, settings_enum_array, settings_struct_array, &buffer);
}

CountSnapshot deserialize_snapshot(MemoryArena &arena, CacheHashEntry *entry, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
	if (entry->key != 0) { // we have no entry in the cache for this
		char *buffer = (char*)entry->value.buffer;
		return read_hfile_snapshot(&buffer);
	} else {
		CountSnapshot snapshot = {};
		return snapshot;
	}
}

enum SubCompType {
	EMPTY,

	INSTANCE,
	MASTER_INPUT,
	MASTER,
	SLAVE_INPUT,
	SLAVE,
	NETWORK,
	STATIC,

	SUB_COMP_TYPE_MAX,
};
struct SubCompStruct {
	SubCompType type;
	Member members[64];
	unsigned member_count;
};
struct ComponentReference {
	String stem;
	unsigned stem_id;
};
void write_component_reference(ComponentReference &component_reference, char **buffer) {
	write_string(component_reference.stem, buffer);
	write_unsigned(component_reference.stem_id, buffer);
}
void read_component_reference(MemoryArena &arena, ComponentReference &component_reference, char **buffer) {
	read_string(arena, component_reference.stem, buffer);
	read_unsigned(component_reference.stem_id, buffer);
}
struct PriorComponentArray {
	ComponentReference entries[16];
	unsigned count;
};
struct DependencyArray {
	ComponentReference entries[16];
	unsigned count;
};

namespace component {
	struct Data {
		SubCompStruct sub_comps[SUB_COMP_TYPE_MAX];

		String max_instances;

		String icon;		// cloud

		String stem;		// avatar
		unsigned stem_id;
		String stem_upper; 	// AVATAR
		String name;		// AvatarComponent

		unsigned num_received_events;

		PriorComponentArray prior_component_array;
		DependencyArray dependency_array;
	};
	struct Array {
		Data *entries;
		bool *reparsed;
		unsigned count;
		unsigned debug_max_count; // Used for range checks
		bool changed;
		bool has_changed() { return changed || count == 0; }
	};

	__forceinline size_t get_size(unsigned count) {
		return count*sizeof(Data) + count*sizeof(bool);
	}
	__forceinline Array make_array(MemoryArena &arena, unsigned count) {
		Data *entries = (Data*)allocate_memory(arena, count * sizeof(Data));
		bool *reparsed = (bool*)allocate_memory(arena, count * sizeof(bool));
		Array array = { entries, reparsed, 0, count, false };
		return array;
	}

	void write(Data &data, char **buffer) {
		for (unsigned i = 1; i < SUB_COMP_TYPE_MAX; ++i) {
			SubCompStruct &scs = data.sub_comps[i];
			write_unsigned(*(unsigned*)&scs.type, buffer);
			if (scs.type == i) {
				write_unsigned(scs.member_count, buffer);
				for (unsigned j = 0; j < scs.member_count; ++j) {
					write_member(scs.members[j], buffer);
				}
			}
		}

		write_string(data.max_instances, buffer);
		write_string(data.icon, buffer);
		write_string(data.stem, buffer);
		write_unsigned(data.stem_id, buffer);
		write_string(data.stem_upper, buffer);
		write_string(data.name, buffer);

		write_unsigned(data.num_received_events, buffer);

		PriorComponentArray &pca = data.prior_component_array;
		write_unsigned(pca.count, buffer);
		for (unsigned i = 0; i < pca.count; ++i) {
			write_component_reference(pca.entries[i], buffer);
		}

		DependencyArray &dc = data.dependency_array;
		write_unsigned(dc.count, buffer);
		for (unsigned i = 0; i < dc.count; ++i) {
			write_component_reference(dc.entries[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		for (unsigned i = 1; i < SUB_COMP_TYPE_MAX; ++i) {
			SubCompStruct &scs = data.sub_comps[i];
			read_unsigned(*(unsigned*)&scs.type, buffer);
			if (scs.type == i) {
				read_unsigned(scs.member_count, buffer);
				for (unsigned j = 0; j < scs.member_count; ++j) {
					read_member(arena, scs.members[j], buffer);
				}
			}
		}

		read_string(arena, data.max_instances, buffer);
		read_string(arena, data.icon, buffer);
		read_string(arena, data.stem, buffer);
		read_unsigned(data.stem_id, buffer);
		read_string(arena, data.stem_upper, buffer);
		read_string(arena, data.name, buffer);

		read_unsigned(data.num_received_events, buffer);

		PriorComponentArray &pca = data.prior_component_array;
		read_unsigned(pca.count, buffer);
		for (unsigned i = 0; i < pca.count; ++i) {
			read_component_reference(arena, pca.entries[i], buffer);
		}

		DependencyArray &dc = data.dependency_array;
		read_unsigned(dc.count, buffer);
		for (unsigned i = 0; i < dc.count; ++i) {
			read_component_reference(arena, dc.entries[i], buffer);
		}
	}

	void serialize(MemoryArena &arena, CacheHashEntry *entry, Array &array, CountSnapshot &count_snapshot, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		Data &data = array.entries[array.count - 1];
		array.reparsed[array.count - 1] = true;
		component::write(data, &buffer);
		write_hfile(count_snapshot, rpc_array, event_array, flow_array, network_type_array, event_receiver_array, state_array, settings_enum_array, settings_struct_array, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Array &array, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
		char *buffer = (char*)entry->value.buffer;

		array.reparsed[array.count] = false;
		Data &data = array.entries[array.count++];
		component::read(arena, data, &buffer);
		read_hfile(arena, rpc_array, event_array, flow_array, network_type_array, event_receiver_array, state_array, settings_enum_array, settings_struct_array, &buffer);
	}
}
typedef component::Data Component;
typedef component::Array ComponentArray;

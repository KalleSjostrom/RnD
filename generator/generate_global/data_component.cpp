enum SubCompType {
	SubCompType_Empty,

	SubCompType_Instance,
	SubCompType_MasterInput,
	SubCompType_Master,
	SubCompType_SlaveInput,
	SubCompType_Slave,
	SubCompType_Network,
	SubCompType_Static,

	SubCompType_Count,
};

String get_sub_comp_string(SubCompType sub_component_type, bool lower_case) {
	switch (sub_component_type) {
		case SubCompType_MasterInput : {
			return lower_case ? master_input_string : MasterInput_string;
		} break;
		case SubCompType_Master : {
			return lower_case ? master_string : Master_string;
		} break;
		case SubCompType_SlaveInput : {
			return lower_case ? slave_input_string : SlaveInput_string;
		} break;
		case SubCompType_Slave : {
			return lower_case ? slave_string : Slave_string;
		} break;
		case SubCompType_Network : {
			return lower_case ? network_string : Network_string;
		} break;
		case SubCompType_Static : {
			return lower_case ? static_string : Static_string;
		} break;
	};
	ASSERT(false, "Unrecognized sub component type! (%d)", sub_component_type);
	return MAKE_STRING("");
}

enum ComponentMask {
	ComponentMask_Update          = 1<<0,
	ComponentMask_Slave           = 1<<1,
	ComponentMask_Master          = 1<<2,
	ComponentMask_Network         = 1<<3,
	ComponentMask_ScriptReload    = 1<<4,
	ComponentMask_NoUpdate        = 1<<5,
	ComponentMask_MigrationEvents = 1<<6,
	ComponentMask_Init            = 1<<7,
	ComponentMask_Deinit          = 1<<8,
	ComponentMask_OnAdded         = 1<<9,
	ComponentMask_OnRemoved       = 1<<10,
};

#define HAS_SUB_COMP(comp_type) (sub_comps[comp_type].type == comp_type)
#define HAS_SUB_COMP_AND_NON_ZERO_COUNT(comp_type) (sub_comps[comp_type].type == comp_type && array_count(sub_comps[comp_type].member_array) > 0)

#define IS_SET(component, mask) ((component).sub_component_mask & (mask))

struct SubCompStruct {
	SubCompType type;
	MemberArray member_array;
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

//// SCRIPT NODE ////
namespace script_node {
	struct Data {
		unsigned component_stem_id;
		SubCompType sub_component_type;
		unsigned member_index;

		// Transient data
		void *component;
		Member *member;
	};

	void write(Data &data, char **buffer) {
		write_unsigned(data.component_stem_id, buffer);
		write_unsigned(*(unsigned*)&data.sub_component_type, buffer);
		write_unsigned(data.member_index, buffer);
	}

	void read(MemoryArena &arena, Data &data, char **buffer) {
		read_unsigned(data.component_stem_id, buffer);
		read_unsigned(*(unsigned*)&data.sub_component_type, buffer);
		read_unsigned(data.member_index, buffer);
	}

	///// GENERIC STUFF /////
	bool write_array(unsigned count_snapshot, Data* &array, char **buffer) {
		int delta;
		bool changed = false;

		delta = array_count(array) - count_snapshot;
		write_int(delta, buffer);
		if (delta > 0) {
			changed = true;
			for (int i = count_snapshot; i < array_count(array); ++i) {
				Data &data = array[i];
				write(data, buffer);
			}
		}
		return changed;
	}

	void read_array(MemoryArena &arena, Data* &array, char **buffer) {
		int delta;
		read_int(delta, buffer);
		for (int i = 0; i < delta; ++i) {
			array_new_entry(array);
			Data &data = array_last(array);
			read(arena, data, buffer);
		}
	}
}
typedef script_node::Data ScriptNode;
typedef ScriptNode* ScriptNodeArray;


#define ARRAY_NAME ComponentReferenceArray
#define ARRAY_TYPE ComponentReference
#define ARRAY_MAX_SIZE 16
#include "../utils/array_static.cpp"

#define ARRAY_NAME DependencyArray
#define ARRAY_TYPE ComponentReference
#define ARRAY_MAX_SIZE 16
#include "../utils/array_static.cpp"

namespace component {
	struct Data {
		SubCompStruct sub_comps[SubCompType_Count];
		unsigned sub_component_mask;

		String max_instances;
		String category;
		String icon;		// cloud

		String stem;		// avatar
		unsigned stem_id;
		String stem_upper; 	// AVATAR
		String name;		// AvatarComponent

		unsigned num_received_events;

		// This component needs to be update _after_ the components referenced by this array.
		ComponentReferenceArray after_component_array;
		// This component needs to be update _before_ the components referenced by this array.
		ComponentReferenceArray before_component_array;
		DependencyArray dependency_array;
	};
	
	void write(Data &data, char **buffer) {
		for (unsigned i = 1; i < SubCompType_Count; ++i) {
			SubCompStruct &scs = data.sub_comps[i];
			write_unsigned(*(unsigned*)&scs.type, buffer);
			if (scs.type == i) {
				write_int(array_count(scs.member_array), buffer);
				for (unsigned j = 0; j < array_count(scs.member_array); ++j) {
					write_member(scs.member_array[j], buffer);
				}
			}
		}
		write_unsigned(data.sub_component_mask, buffer);

		write_string(data.max_instances, buffer);
		write_string(data.icon, buffer);
		write_string(data.stem, buffer);
		write_unsigned(data.stem_id, buffer);
		write_string(data.stem_upper, buffer);
		write_string(data.name, buffer);

		write_unsigned(data.num_received_events, buffer);

		{ // Write after component array
			ComponentReferenceArray &pca = data.after_component_array;
			write_unsigned(pca.count, buffer);
			for (unsigned i = 0; i < pca.count; ++i) {
				write_component_reference(pca[i], buffer);
			}
		}

		{ // Write before component array
			ComponentReferenceArray &pca = data.before_component_array;
			write_unsigned(pca.count, buffer);
			for (unsigned i = 0; i < pca.count; ++i) {
				write_component_reference(pca[i], buffer);
			}
		}

		DependencyArray &dc = data.dependency_array;
		write_unsigned(dc.count, buffer);
		for (unsigned i = 0; i < dc.count; ++i) {
			write_component_reference(dc[i], buffer);
		}
	}
	void read(MemoryArena &arena, Data &data, char **buffer) {
		for (unsigned i = 1; i < SubCompType_Count; ++i) {
			SubCompStruct &scs = data.sub_comps[i];
			read_unsigned(*(unsigned*)&scs.type, buffer);
			if (scs.type == i) {
				read_serialized_array(scs.member_array, buffer);
				for (unsigned j = 0; j < array_count(scs.member_array); ++j) {
					read_member(arena, scs.member_array[j], buffer);
				}
			}
		}
		read_unsigned(data.sub_component_mask, buffer);

		read_string(arena, data.max_instances, buffer);
		read_string(arena, data.icon, buffer);
		read_string(arena, data.stem, buffer);
		read_unsigned(data.stem_id, buffer);
		read_string(arena, data.stem_upper, buffer);
		read_string(arena, data.name, buffer);

		read_unsigned(data.num_received_events, buffer);

		{ // Read after component array
			ComponentReferenceArray &pca = data.after_component_array;
			read_unsigned(pca.count, buffer);
			for (unsigned i = 0; i < pca.count; ++i) {
				read_component_reference(arena, pca[i], buffer);
			}
		}

		{ // Read before component array
			ComponentReferenceArray &pca = data.before_component_array;
			read_unsigned(pca.count, buffer);
			for (unsigned i = 0; i < pca.count; ++i) {
				read_component_reference(arena, pca[i], buffer);
			}
		}

		DependencyArray &dc = data.dependency_array;
		read_unsigned(dc.count, buffer);
		for (unsigned i = 0; i < dc.count; ++i) {
			read_component_reference(arena, dc[i], buffer);
		}
	}

	void serialize(MemoryArena &arena, CacheHashEntry *entry, Data* &array, bool* &reparsed_components, GameStringArray &game_string_array, bool &game_strings_changed, unsigned game_string_snapshot, ScriptNodeArray &script_node_array, bool &script_nodes_changed, unsigned script_node_snapshot, HFileCollection &collection, CountSnapshot &count_snapshot) {
		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		Data &data = array_last(array);
		array_push(reparsed_components, true);
		write_hfile(count_snapshot, collection, &buffer);
		component::write(data, &buffer);

		game_strings_changed = game_string::write_array(game_string_snapshot, game_string_array, &buffer);
		script_nodes_changed = script_node::write_array(script_node_snapshot, script_node_array, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Data* &array, bool *reparsed_components, GameStringArray &game_string_array, ScriptNodeArray &script_node_array, HFileCollection &collection) {
		char *buffer = (char*)entry->value.buffer;

		array_new_entry(array);
		Data &data = array_last(array);
		array_push(reparsed_components, false);
		read_hfile(arena, collection, &buffer);
		component::read(arena, data, &buffer);
		game_string::read_array(arena, game_string_array, &buffer);
		script_node::read_array(arena, script_node_array, &buffer);
	}
}
typedef component::Data Component;
typedef Component* ComponentArray;

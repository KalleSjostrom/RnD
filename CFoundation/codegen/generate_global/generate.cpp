#include "../utils/common.cpp"
#include "../utils/exceptions.h"
#include "../utils/serialize.inl"
#include "../../utils/linked_list_defines.inl"

static const unsigned cache_version = 5;
#include "../utils/generation_cache.cpp"

#define PROFILE 0
#if PROFILE
/*
// Clean run
time: 0.000007          coalesce_parsed_data
time: 0.000032          read_cache
time: 0.000033          make_sure_directories_exists
time: 0.000048          exceptions_shutdown
time: 0.000071          allocate_memory
time: 0.000139          exceptions_setup
time: 0.000311          write_cache
time: 0.000599          output_network_config
time: 0.000607          output_entity_lookup
time: 0.001036          iterate_content
time: 0.002776          output_component_group
time: 0.002971          output_network_router
time: 0.003173          output_event_delegate
time: 0.003299          output_entity_settings
time: 0.003623          output_flow
time: 0.009879          output_components
time: 0.013431          output_data_components
time: 0.031403          iterate_code
time: 0.073440          total

// Consecutive run without anything changed
time: 0.000000          output_component_group
time: 0.000000          output_components
time: 0.000000          output_data_components
time: 0.000000          output_entity_lookup
time: 0.000000          output_entity_settings
time: 0.000000          output_event_delegate
time: 0.000000          output_flow
time: 0.000000          output_network_config
time: 0.000000          output_network_router
time: 0.000015          coalesce_parsed_data
time: 0.000032          exceptions_shutdown
time: 0.000039          make_sure_directories_exists
time: 0.000059          allocate_memory
time: 0.000100          read_cache
time: 0.000150          exceptions_setup
time: 0.000306          write_cache
time: 0.000393          iterate_code
time: 0.000696          iterate_content
time: 0.001793          total
*/
#include "../utils/profiler.c"
enum ProfilerScopes {
	ProfilerScopes__allocate_memory,
	ProfilerScopes__read_cache,
	ProfilerScopes__iterate_code,
	ProfilerScopes__make_sure_directories_exists,
	ProfilerScopes__iterate_content,
	ProfilerScopes__check_removed_entries,
	ProfilerScopes__write_cache,
	ProfilerScopes__coalesce_parsed_data,
	ProfilerScopes__output_network_config,
	ProfilerScopes__output_flow,
	ProfilerScopes__output_network_router,
	ProfilerScopes__output_event_delegate,
	ProfilerScopes__output_entity_lookup,
	ProfilerScopes__output_entity_settings,
	ProfilerScopes__output_components,
	ProfilerScopes__output_data_components,
	ProfilerScopes__output_component_group,
	ProfilerScopes__output_environment_settings,
	ProfilerScopes__exceptions_setup,
	ProfilerScopes__exceptions_shutdown,
	ProfilerScopes__total,
	ProfilerScopes__count,
};
#else
#define PROFILER_START(...)
#define PROFILER_STOP(...)
#define PROFILER_PRINT(...)
#endif

static const String _abilities            = MAKE_STRING("abilities");
static const String _generated            = MAKE_STRING("generated");
static const String _hg                   = MAKE_STRING(".hg");
static const String _boot                 = MAKE_STRING("boot");
static const String _codegen              = MAKE_STRING("codegen");
static const String _debug                = MAKE_STRING("debug");
static const String _flow                 = MAKE_STRING("flow");
static const String _game_strings         = MAKE_STRING("game_strings");
static const String _gui                  = MAKE_STRING("gui");
static const String _network              = MAKE_STRING("network");
static const String _plugin_environment   = MAKE_STRING("plugin_environment");
static const String _precompiled          = MAKE_STRING("precompiled");
static const String _reload               = MAKE_STRING("reload");
static const String _scripts              = MAKE_STRING("scripts");
static const String _utils                = MAKE_STRING("utils");
static const String _animations           = MAKE_STRING("animations");
static const String _textures             = MAKE_STRING("textures");
static const String _materials            = MAKE_STRING("materials");
static const String _shading_environments = MAKE_STRING("shading_environments");

bool ignored_directory(String &directory) {
			// Code folder
	return  are_strings_equal(directory, _abilities) ||
			are_strings_equal(directory, _generated) ||

			// Foundation folder
			are_strings_equal(directory, _hg) ||
			are_strings_equal(directory, _boot) ||
			are_strings_equal(directory, _codegen) ||
			are_strings_equal(directory, _debug) ||
			are_strings_equal(directory, _flow) ||
			are_strings_equal(directory, _game_strings) ||
			are_strings_equal(directory, _gui) ||
			are_strings_equal(directory, _network) ||
			are_strings_equal(directory, _plugin_environment) ||
			are_strings_equal(directory, _precompiled) ||
			are_strings_equal(directory, _reload) ||
			are_strings_equal(directory, _scripts) ||
			are_strings_equal(directory, _utils) ||

			// Content folders
			are_strings_equal(directory, _animations) ||
			are_strings_equal(directory, _materials) ||
			are_strings_equal(directory, _textures) ||
			are_strings_equal(directory, _shading_environments)
			;
}

static String ROOT_FOLDER = MAKE_STRING("../..");
static String COMPONENT_H_ENDING = MAKE_STRING("_component.h");
static String COMPONENTS_FOLDER = MAKE_STRING(GAME_CODE_DIR"/components");

static String GENERATED_FOLDER = MAKE_STRING(GAME_CODE_DIR"/generated");
static String GENERATED_CODE_FOLDER = MAKE_STRING(GAME_CODE_DIR"/generated/components");
static String GENERATED_CPP_ENDING = MAKE_STRING("_component.generated.cpp");
static String CPP_ENDING = MAKE_STRING("_component.cpp");

static String GENERATED_COMPONENT_FOLDER = MAKE_STRING("../content/generated/components");
static String GENERATED_COMPONENT_ENDING = MAKE_STRING(".component");

static String GENERATED_TYPES_FOLDER = MAKE_STRING("../content/generated/types");
static String GENERATED_TYPE_ENDING = MAKE_STRING(".type");

static String CODE_STRING = MAKE_STRING("code");
static String CONTENT_STRING = MAKE_STRING("content");
static unsigned entity_ending_length = MAKE_STRING(".entity").length;

static String HFILE_GENERATED_BEGIN_MARKER = MAKE_STRING("BEGIN_GENERATED");

void make_sure_directories_exists(MemoryArena &arena) {
	make_sure_directory_exists(arena, ROOT_FOLDER, GENERATED_FOLDER);
	make_sure_directory_exists(arena, ROOT_FOLDER, GENERATED_CODE_FOLDER);
	make_sure_directory_exists(arena, ROOT_FOLDER, MAKE_STRING("../content"));
	make_sure_directory_exists(arena, ROOT_FOLDER, MAKE_STRING("../content/generated"));
	make_sure_directory_exists(arena, ROOT_FOLDER, GENERATED_COMPONENT_FOLDER);
	make_sure_directory_exists(arena, ROOT_FOLDER, GENERATED_TYPES_FOLDER);
}

String make_project_path(char *filepath, MemoryArena &arena, String separator = CONTENT_STRING, unsigned strip_from_end = entity_ending_length, bool include_separator = true) {
	String source_filepath = make_string(filepath);
	int start, stop;
	bool found = string_split(source_filepath, separator, &start, &stop);
	PARSER_ASSERT(found, "Couldn't find 'content' in %s", filepath);

	int split = include_separator ? start : stop;

	source_filepath.length -= strip_from_end;
	unsigned length = source_filepath.length - split;
	char *buffer = allocate_memory(arena, length + 1);
	buffer[length] = '\0';
	String newstring = make_string(buffer, length);
	int index = 0;
	for (int i = split; i < source_filepath.length; ++i) {
		newstring.text[index++] = source_filepath.text[i];
	}
	return newstring;
}

enum EntryHeaderType {
	EntryHeaderType_None = 0,

	EntryHeaderType_Component,
	EntryHeaderType_Settings,
	EntryHeaderType_HFile,
	EntryHeaderType_GameStateMachine,
	EntryHeaderType_NetworkConversions,
};

#include "data_game_hfile.cpp"
#include "data_component.cpp"
#include "data_game_object.cpp"
#include "data_entity_settings.cpp"
#include "data_state_machine.cpp"
#include "data_network_conversion.cpp"

struct AllSettings {
	SettingsArray entities;
	SettingsArray environments;
	SettingsArray tiles;
	SettingsArray level_generation;
};

Component *get_component(unsigned stem_id, ComponentArray &component_array) {
	for (int i = 0; i < component_array.count; ++i) {
		Component &component = component_array.entries[i];
		if (component.stem_id == stem_id)
			return &component;
	}
	return 0;
}

#define HAS_SUB_COMP(comp_type) (sub_comps[comp_type].type == comp_type)
#define HAS_SUB_COMP_AND_NON_ZERO_COUNT(comp_type) (sub_comps[comp_type].type == comp_type && sub_comps[comp_type].member_count > 0)

#include "common_strings.cpp"
#include "common_tokens.cpp"
#include "type_conversions.cpp"
#include "output_components.cpp" // This is before parsed component since parse_component actually inserts stuff in the h-file

#include "parse_network_conversions.cpp"
#include "parse_game_hfile.cpp"
#include "parse_component.cpp"
#include "parse_settings_entity.cpp"
#include "parse_state_machine.cpp"

#include "coalesce_parsed_data.cpp"

#include "output_data_components.cpp"
#include "output_component_group.cpp"
#include "output_flow.cpp"
#include "output_network_router.cpp"
#include "output_event_delegate.cpp"
#include "output_network_config.cpp"
#include "output_entity_lookup.cpp"
#include "output_entity_settings.cpp"
#include "output_state_machines.cpp"
#include "output_environment_settings.cpp"
#include "output_level_generation_settings.cpp"
#include "output_settings_enums.cpp"

void iterate(MemoryArena &temp_arena, MemoryArena &arena, char *filepath,
			CacheHashMap &cache_hash_map,
			ComponentArray &component_array,
			RPCArray &rpc_array,
			EventArray &event_array,
			FlowArray &flow_array,
			ReceiverArray &receiver_array,
			AllSettings &all_settings,
			NetworkTypeArray &network_type_array,
			StateMachineArray &state_machine_array,
			StateArray &state_array,
			SettingsEnumArray &settings_enum_array,
			SettingsStructArray &settings_struct_array) {

	WIN32_FIND_DATA find_data = {};
	char buffer[MAX_PATH];
	sprintf(buffer, "%s*.*", filepath);

	HANDLE handle = find_first_file(buffer, &find_data);
	if (handle != INVALID_HANDLE_VALUE) {
		do {
			if (find_data.cFileName[0] == '.') {
				if (find_data.cFileName[1] == '\0')
					continue;

				if (find_data.cFileName[1] == '.' && find_data.cFileName[2] == '\0')
					continue;
			}

			String filename = make_string(find_data.cFileName);
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!ignored_directory(filename)) {
					sprintf(buffer, "%s%s/", filepath, *filename);
					iterate(temp_arena, arena, buffer, cache_hash_map, component_array, rpc_array, event_array, flow_array, receiver_array, all_settings, network_type_array, state_machine_array, state_array, settings_enum_array, settings_struct_array);
				}
			} else {
				MemoryBlockHandle block_handle = begin_block(temp_arena);
				uint64_t key; CacheHashEntry *entry;

				if (string_ends_in(filename, COMPONENT_H_ENDING)) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);
					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_Component, &key, &entry)) {
						// Check the cache for what we wrote last, if we wrote an rpc for example, we set the rpc_array as changed.
						// This because, we now reparse it so it could have changed name or some such.
						CountSnapshot cached_snapshot = deserialize_snapshot(arena, entry, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						update_changed_arrays(cached_snapshot, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);

						// Parse the component
						CountSnapshot snapshot = take_count_snapshot(rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						parse_component(temp_arena, arena, component_array, buffer, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);

						// Modify the write time of the file, so we trigger a re-read after we add stuff to this component h-file.
						HANDLE handle = CreateFile(buffer, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
						SetFileTime(handle, 0, 0, &find_data.ftLastWriteTime);
						CloseHandle(handle);

						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						component::serialize(arena, entry, component_array, snapshot, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
					} else {
						component::deserialize(arena, buffer, entry, component_array, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
					}
				} else if (string_ends_in(filename, MAKE_STRING(".entity"))) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);
					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_Settings, &key, &entry)) {

						unsigned count_entities = all_settings.entities.count;
						unsigned count_environments = all_settings.environments.count;
						unsigned count_tiles = all_settings.tiles.count;
						unsigned count_level_generation = all_settings.level_generation.count;

						parse_settings_entity(temp_arena, arena, buffer, component_array, all_settings);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);

						if (all_settings.entities.count > count_entities)
							settings::serialize(arena, entry, all_settings.entities);
						else if (all_settings.environments.count > count_environments)
							settings::serialize(arena, entry, all_settings.environments);
						else if (all_settings.level_generation.count > count_level_generation)
							settings::serialize(arena, entry, all_settings.level_generation);
						else if (all_settings.tiles.count > count_tiles)
							settings::serialize(arena, entry, all_settings.tiles);
					} else {
						if (entry->value.size > 0) {
							// We have a valid cache entry, use that instead of reparsing the file!
							Settings settings = {};
							settings::deserialize(arena, buffer, entry, settings);
							insert_settings(all_settings, settings);
						}
					}
				} else if (string_ends_in(filename, MAKE_STRING(".game_state_machine"))) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);
					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_GameStateMachine, &key, &entry)) {
						// Parse the state machine
						parse_state_machine(temp_arena, arena, buffer, state_machine_array);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						state_machine::serialize(arena, entry, state_machine_array);
					} else { // We have a valid cache entry, use that instead of reparsing the file!
						state_machine::deserialize(arena, buffer, entry, state_machine_array);
					}
				} else if (string_ends_in(filename, MAKE_STRING(".h"))) {
					sprintf(buffer, "%s%s", filepath, find_data.cFileName);
					if (pop_cache_entry_for(cache_hash_map, buffer, find_data, EntryHeaderType_HFile, &key, &entry)) {
						// Check the cache for what we wrote last, if we wrote an rpc for example, we set the rpc_array as changed.
						// This because, we now reparse it so it could have changed name or some such.
						CountSnapshot cached_snapshot = deserialize_snapshot(arena, entry, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						update_changed_arrays(cached_snapshot, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);

						// Parse the hfile
						CountSnapshot snapshot = take_count_snapshot(rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						parse_game_hfile(temp_arena, arena, buffer, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						fill_hash_entry(entry, key, find_data.ftLastWriteTime);
						serialize_hfile(arena, entry, snapshot, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
					} else {
						deserialize_hfile(arena, buffer, entry, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
					}
				}
				end_block(temp_arena, block_handle);
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}
}

int main(int argc, char *argv[]) {
PROFILER_START(total)
	PROFILER_START(exceptions_setup)
		exceptions_setup();
	PROFILER_STOP(exceptions_setup)

	PROFILER_START(allocate_memory)
		unsigned rpc_count = 256;
		unsigned event_count = 256;
		unsigned flow_count = 256;
		unsigned network_type_count = 32;
		unsigned receiver_count = 256;
		unsigned component_count = 128;
		unsigned game_object_count = 128;
		unsigned settings_count = 512;
		unsigned environment_settings_count = 8;
		unsigned tile_settings_count = 64;
		unsigned level_generation_count = 64;
		unsigned state_count = 128;
		unsigned state_machine_count = 16;
		unsigned settings_enum_count = 64;
		unsigned settings_struct_count = 64;
		unsigned network_conversion_count = 64;

		MemoryArena data_arena = init_memory(rpc::get_size(rpc_count) + event::get_size(event_count) + flow::get_size(flow_count) + receiver::get_size(receiver_count) + component::get_size(component_count) + game_object::get_size(component_count) + settings::get_size(settings_count) + settings::get_size(environment_settings_count) + settings::get_size(tile_settings_count) + settings::get_size(level_generation_count) + network_type::get_size(network_type_count) + state_machine::get_size(state_machine_count) + state::get_size(state_count) + settings_enum::get_size(settings_enum_count) + settings_struct::get_size(settings_struct_count) + network_conversions::get_size(network_conversion_count));

		AllSettings all_settings;

		RPCArray rpc_array = rpc::make_array(data_arena, rpc_count);
		EventArray event_array = event::make_array(data_arena, event_count);
		FlowArray flow_array = flow::make_array(data_arena, flow_count);
		NetworkTypeArray network_type_array = network_type::make_array(data_arena, network_type_count);
		ReceiverArray receiver_array = receiver::make_array(data_arena, receiver_count);
		ComponentArray component_array = component::make_array(data_arena, component_count);
		all_settings.entities = settings::make_array(data_arena, settings_count);
		all_settings.environments = settings::make_array(data_arena, environment_settings_count);
		all_settings.tiles = settings::make_array(data_arena, tile_settings_count);
		all_settings.level_generation = settings::make_array(data_arena, level_generation_count);
		StateArray state_array = state::make_array(data_arena, state_count);
		StateMachineArray state_machine_array = state_machine::make_array(data_arena, state_machine_count);
		GameObjectArray go_array = game_object::make_array(data_arena, game_object_count);
		SettingsEnumArray settings_enum_array = settings_enum::make_array(data_arena, settings_enum_count);
		SettingsStructArray settings_struct_array = settings_struct::make_array(data_arena, settings_struct_count);
		NetworkConversionArray network_conversion_array = network_conversions::make_array(data_arena, network_conversion_count);

		MemoryArena temp_arena = init_memory(32*MB);
		MemoryArena arena = init_memory(32*MB);

	PROFILER_STOP(allocate_memory)

	PROFILER_START(read_cache)
		CacheHashMap cache_hash_map = {};
		read_cache_from_disc(arena, cache_hash_map, cache_version);
	PROFILER_STOP(read_cache)

	// Parse the "code" folder
	PROFILER_START(iterate_code)
		iterate(temp_arena, arena, "../../", cache_hash_map, component_array, rpc_array, event_array, flow_array, receiver_array, all_settings, network_type_array, state_machine_array, state_array, settings_enum_array, settings_struct_array);
		iterate(temp_arena, arena, "../../"GAME_CODE_DIR"/", cache_hash_map, component_array, rpc_array, event_array, flow_array, receiver_array, all_settings, network_type_array, state_machine_array, state_array, settings_enum_array, settings_struct_array);
	PROFILER_STOP(iterate_code)

	PROFILER_START(make_sure_directories_exists)
		make_sure_directories_exists(temp_arena);
	PROFILER_STOP(make_sure_directories_exists)

	// Parse the "content" folder
	PROFILER_START(iterate_content)
		iterate(temp_arena, arena, "../../../content/", cache_hash_map, component_array, rpc_array, event_array, flow_array, receiver_array, all_settings, network_type_array, state_machine_array, state_array, settings_enum_array, settings_struct_array);
	PROFILER_STOP(iterate_content)

	// Parse the "network conversions"
	WIN32_FIND_DATA find_data = {};
	char *filepath = "../../network/network.conversions";
	HANDLE handle = find_first_file(filepath, &find_data);
	uint64_t key; CacheHashEntry *entry;
	if (pop_cache_entry_for(cache_hash_map, filepath, find_data, EntryHeaderType_NetworkConversions, &key, &entry)) {
		unsigned count_snapshot = network_conversion_array.count;
		parse_network_conversions(temp_arena, arena, filepath, network_conversion_array);
		fill_hash_entry(entry, key, find_data.ftLastWriteTime);
		network_conversions::serialize(arena, entry, count_snapshot, network_conversion_array);
	} else { // We have a valid cache entry, use that instead of reparsing the file!
		network_conversions::deserialize(arena, filepath, entry, network_conversion_array);
	}

	// Go through the cache and check all the entries that weren't found when iterating over the file structure.
	// This will be the entries that corresponds to removed files. We need to make sure to mark these entries as changed, so that the output is regenerated.
	PROFILER_START(check_removed_entries)
		for (int i = 0; i < ARRAY_COUNT(cache_hash_map.entries); ++i) {
			CacheHashEntry &entry = cache_hash_map.entries[i];
			if (!entry.touched && entry.key) { // If this cache entry wasn't touched, the file that produced it have been removed.
				switch (entry.header_type) {
					case EntryHeaderType_Component: {
						component_array.changed = true;

						CountSnapshot cached_snapshot = deserialize_snapshot(arena, &entry, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						update_changed_arrays(cached_snapshot, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
					}; break;
					case EntryHeaderType_Settings: {
						all_settings.entities.changed = true;
						all_settings.environments.changed = true;
						all_settings.tiles.changed = true;
					}; break;
					case EntryHeaderType_HFile: {
						CountSnapshot cached_snapshot = deserialize_snapshot(arena, &entry, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
						update_changed_arrays(cached_snapshot, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
					}; break;
					case EntryHeaderType_GameStateMachine: {
						state_machine_array.changed = true;
					}; break;
					case EntryHeaderType_NetworkConversions: {
						network_conversion_array.changed = true;
					}; break;
					default: {
						ASSERT(false, "Unknown cache head type! %u", entry.header_type);
					};
				};
			}
		}
	PROFILER_STOP(check_removed_entries)

	PROFILER_START(write_cache)
		write_cache_to_disc(cache_hash_map, cache_version);
	PROFILER_STOP(write_cache)

	PROFILER_START(coalesce_parsed_data)
		SortedComponents sorted_components = coalesce_parsed_data(arena, component_array, all_settings.entities, go_array, state_machine_array, state_array);
	PROFILER_STOP(coalesce_parsed_data)

	// global.network_config
	PROFILER_START(output_network_config)
		if (component_array.has_changed() || rpc_array.has_changed() || network_type_array.has_changed() || all_settings.entities.has_changed())
			output_network_config(component_array, rpc_array, network_type_array, go_array, temp_arena, arena);
	PROFILER_STOP(output_network_config)

	// flow_router.generated.cpp, global.script_flow_nodes, flow_messages.generated.cpp
	PROFILER_START(output_flow)
		if (flow_array.has_changed())
			output_flow(flow_array, arena);
	PROFILER_STOP(output_flow)

	PROFILER_START(output_network_router)
		if (rpc_array.has_changed() || event_array.has_changed() || go_array.has_changed() || all_settings.entities.has_changed() || component_array.has_changed() || network_conversion_array.has_changed() || network_type_array.has_changed())
			output_network_router(rpc_array, event_array, go_array, all_settings.entities, network_conversion_array, network_type_array, arena);
	PROFILER_STOP(output_network_router)

	PROFILER_START(output_event_delegate)
		if (rpc_array.has_changed() || event_array.has_changed() || flow_array.has_changed() || receiver_array.has_changed())
			output_event_delegate(rpc_array, event_array, flow_array, receiver_array, arena);
	PROFILER_STOP(output_event_delegate)

	PROFILER_START(output_entity_lookup)
		if (component_array.has_changed() || all_settings.entities.has_changed())
			output_entity_lookup(component_array, all_settings.entities, go_array, arena);
	PROFILER_STOP(output_entity_lookup)

	PROFILER_START(output_entity_settings)
		if (component_array.has_changed() || all_settings.entities.has_changed() || settings_struct_array.has_changed())
			output_entity_settings(component_array, all_settings.entities, settings_struct_array, settings_enum_array, arena);
	PROFILER_STOP(output_entity_settings)

	PROFILER_START(output_components)
		if (component_array.has_changed())
			output_components(component_array, temp_arena);
	PROFILER_STOP(output_components)

	PROFILER_START(output_data_components)
		if (component_array.has_changed() || settings_struct_array.has_changed() || settings_enum_array.has_changed())
			output_data_components(settings_struct_array, settings_enum_array, component_array, temp_arena);
	PROFILER_STOP(output_data_components)

	PROFILER_START(output_settings_enum)
		if (settings_enum_array.has_changed())
			output_settings_enums(settings_enum_array, temp_arena);
	PROFILER_STOP(output_settings_enum)

	PROFILER_START(output_component_group)
		if (all_settings.entities.has_changed() || component_array.has_changed())
			output_component_group(sorted_components, component_array, all_settings.entities, temp_arena);
	PROFILER_STOP(output_component_group)

	PROFILER_START(output_state_machines)
		if (state_machine_array.has_changed() || state_array.has_changed())
			output_state_machines(state_machine_array, temp_arena);
	PROFILER_STOP(output_state_machines)

	PROFILER_START(output_environment_settings)
		if (all_settings.environments.has_changed() || all_settings.tiles.has_changed())
			output_environment_settings(all_settings.environments, all_settings.tiles, temp_arena);
	PROFILER_STOP(output_environment_settings)

	// PROFILER_START(output_level_generation_settings)
	// 	if (all_settings.level_generation.has_changed())
	// 		output_level_generation_settings(all_settings.level_generation, temp_arena);
	// PROFILER_STOP(output_level_generation_settings)

	PROFILER_START(exceptions_shutdown)
		exceptions_shutdown();
	PROFILER_STOP(exceptions_shutdown)

PROFILER_STOP(total)

	PROFILER_PRINT(allocate_memory);
	PROFILER_PRINT(read_cache);
	PROFILER_PRINT(iterate_code);
	PROFILER_PRINT(make_sure_directories_exists);
	PROFILER_PRINT(iterate_content);
	PROFILER_PRINT(check_removed_entries);
	PROFILER_PRINT(write_cache);
	PROFILER_PRINT(coalesce_parsed_data);
	PROFILER_PRINT(output_network_config);
	PROFILER_PRINT(output_flow);
	PROFILER_PRINT(output_network_router);
	PROFILER_PRINT(output_event_delegate);
	PROFILER_PRINT(output_entity_lookup);
	PROFILER_PRINT(output_entity_settings);
	PROFILER_PRINT(output_components);
	PROFILER_PRINT(output_data_components);
	PROFILER_PRINT(output_component_group);
	PROFILER_PRINT(exceptions_setup);
	PROFILER_PRINT(exceptions_shutdown);
	PROFILER_PRINT(total);

	return 0;
}

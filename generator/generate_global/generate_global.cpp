static const unsigned COMPONENT_COUNT = 256;
static const unsigned GAME_STRINGS_COUNT = 8192;
static const unsigned SCRIPT_NODE_COUNT = 512;
static const unsigned GAME_OBJECT_COUNT = 512;
static const unsigned STATE_MACHINE_COUNT = 16;
static const unsigned NETWORK_CONVERSION_COUNT = 64;

static const unsigned SETTINGS_COUNT = 512;
static const unsigned LEVEL_GENERATION_SETUP_COUNT = 16;
static const unsigned LEVEL_ZONES_COUNT = 64;
static const unsigned LEVEL_GENERATION_COUNT = 64;
static const unsigned LEVEL_REGIONS_COUNT = 64;
static const unsigned LEVEL_LOCATIONS_COUNT = 64;
static const unsigned LEVEL_PLANETS_COUNT = 32;
static const unsigned STAMP_COUNT = 256;
static const unsigned WEIGHT_RULE_COUNT = 64;

static const unsigned RPC_COUNT = 256;
static const unsigned EVENT_COUNT = 256;
static const unsigned FLOW_COUNT = 256;
static const unsigned NETWORK_TYPE_COUNT = 32;
static const unsigned RECEIVER_COUNT = 256;
static const unsigned STATE_COUNT = 128;
static const unsigned SETTINGS_ENUM_COUNT = 64;
static const unsigned SETTINGS_STRUCT_COUNT = 64;
static const unsigned ABILITY_NODE_COUNT = 512;
static const unsigned BEHAVIOR_NODE_COUNT = 512;

namespace generate_global {
	bool ignored_directory(String &directory) {
				// Code folder
		return  are_strings_equal(directory, _abilities) ||
				are_strings_equal(directory, _generated) ||

				// Foundation folder
				are_strings_equal(directory, _hg) ||
				are_strings_equal(directory, _codegen) ||
				are_strings_equal(directory, _gui) ||
				are_strings_equal(directory, _plugin_environment) ||
				are_strings_equal(directory, _precompiled) ||
				are_strings_equal(directory, _reload) ||
				are_strings_equal(directory, _scripts) ||
				are_strings_equal(directory, _tools) ||

				// Content folders
				are_strings_equal(directory, _animations) ||
				are_strings_equal(directory, _materials) ||
				are_strings_equal(directory, _textures) ||
				are_strings_equal(directory, _shading_environments)
				;
	}

	String make_project_path(char *filepath, MemoryArena &arena, String separator = _content, unsigned strip_from_end = _ending_entity.length, bool include_separator = true) {
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

	#include "../utils/sorting.cpp"
	#include "data_game_strings.cpp"
	#include "data_game_hfile.cpp"
	#include "data_component.cpp"
	#include "data_game_object.cpp"
	#include "data_entity_settings.cpp"
	#include "data_state_machine.cpp"
	#include "data_network_conversion.cpp"

	inline bool is_array_whitespace(char p) {
		return p == '\n' || p == '\r' || p == '\t' || p == ' ' || p == ',';
	}
	int trim_stingray_array_inplace(String &string) {
		int source_cursor = 0;
		int dest_cursor = 0;
		int element_counter = 0;
		bool found_element = false;

		for (; source_cursor < string.length; source_cursor++) {
			if (!found_element && !is_array_whitespace(string[source_cursor])) {
				if (element_counter > 0) {
					string.text[dest_cursor++] = ',';
					// string.text[dest_cursor++] = ' ';
				}
				found_element = true;
			}

			if (found_element) {
				if (is_array_whitespace(string[source_cursor])) {
					found_element = false;
				} else {
					string.text[dest_cursor++] = string.text[source_cursor];
					++element_counter;
				}
			}
		}
		string.length = dest_cursor;
		null_terminate(string);
		return element_counter;
	}

	String remove_quotation_marks(String string) {
		string.text++;
		string.length -= 2;
		return string;
	}

	struct AllSettings {
		SettingsArray entities;
		SettingsArray level_generation_setup;
		SettingsArray level_zones;
		SettingsArray level_generation;
		SettingsArray level_regions;
		SettingsArray level_planets;
		SettingsArray level_locations;
		SettingsArray stamps;
		SettingsArray weight_rules;

		bool entities_changed;
		bool level_generation_setup_changed;
		bool level_zones_changed;
		bool level_generation_changed;
		bool level_regions_changed;
		bool level_planets_changed;
		bool level_locations_changed;
		bool stamps_changed;
		bool weight_rules_changed;
	};

	Component *get_component(unsigned stem_id, ComponentArray &component_array) {
		for (int i = 0; i < array_count(component_array); ++i) {
			Component &component = component_array[i];
			if (component.stem_id == stem_id)
				return &component;
		}
		return 0;
	}

	#define STATIC_ID32(str) to_id32(strlen(str), str)

	bool find_and_parse_float(SettingsData *settings, unsigned id_to_find, float &result ) {
		HASH_LOOKUP(entry, settings->next->map, ARRAY_COUNT(settings->next->map), id_to_find);
		if(!entry->value)
			return false;
		String text = entry->value->value;
		clean_string_from_quotations(text);
		if(sscanf(text.text, "%f", &result) != 1) {
			return false;
		}
		return true;
	}

	bool find_and_parse_bool(SettingsData *settings, unsigned id_to_find, bool &result) {
		HASH_LOOKUP(entry, settings->next->map, ARRAY_COUNT(settings->next->map), id_to_find);
		if(!entry->value)
			return false;
		String text = entry->value->value;
		clean_string_from_quotations(text);
		if(are_strings_equal(text, make_string("true", strlen("true"))))
			result = true;
		else
			result = false;

		return true;
	}

	bool find_and_parse_idstring(SettingsData *settings, unsigned id_to_find, String &result) {
		HASH_LOOKUP(entry, settings->next->map, ARRAY_COUNT(settings->next->map), id_to_find);
		if(!entry->value)
			return false;
		String text = entry->value->value;
		clean_string_from_quotations(text);
		result = text;
		return true;
	}

	bool find_and_parse_unsigned(SettingsData *settings, unsigned id_to_find, unsigned &result) {
		HASH_LOOKUP(entry, settings->next->map, ARRAY_COUNT(settings->next->map), id_to_find);
		if(!entry->value)
			return false;
		String text = entry->value->value;
		clean_string_from_quotations(text);
		if(sscanf(text.text, "%d", &result) != 1) {
			return false;
		}
		return true;
	}

	static String _folder_generated_code_components = MAKE_STRING(GAME_CODE_DIR "/generated/components");

	static String _folder_generated = MAKE_STRING("../content/generated");
	static String _folder_generated_data_components = MAKE_STRING("../content/generated/components");
	static String _folder_generated_types = MAKE_STRING("../content/generated/types");

	#include "type_conversions.cpp"
	#include "output_components.cpp" // This is before parsed component since parse_component actually inserts stuff in the h-file

	#include "parse_for_game_strings.cpp"
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
	#include "output_level_generation_setup.cpp"
	#include "output_level_zones_settings.cpp"
	#include "output_level_generation_settings.cpp"
	#include "output_stamp_settings.cpp"
	#include "output_settings_enums.cpp"
	#include "output_game_strings.cpp"
	#include "output_ability_nodes.cpp"
	#include "output_behavior_nodes.cpp"
	#include "output_script_nodes.cpp"

	enum OutputType {
		OutputType_NetworkConfig,
		OutputType_Flow,
		OutputType_NetworkRouter,
		OutputType_EventDelegate,
		OutputType_EntityLookup,
		OutputType_EntitySettings,
		OutputType_Components,
		OutputType_DataComponents,
		OutputType_SettingsEnums,
		OutputType_ComponentGroup,
		OutputType_StateMachines,
		OutputType_LevelZoneSettings,
		OutputType_LevelGenerationSettings,
		OutputType_StampSettings,
		OutputType_GameStrings,
		OutputType_AbilityNode,
		OutputType_BehaviorNode,
		OutputType_ScriptNode,
	};

	struct OutputContext {
		OutputType type;
		SortedComponents *sorted_components;

		HFileCollection *hfile_collection;

		ComponentArray *component_array;
		bool *reparsed_component_array;

		GameStringArray *game_string_array;
		ScriptNodeArray *script_node_array;
		GameObjectArray *go_array;
		StateMachineArray *state_machine_array;
		NetworkConversionArray *network_conversion_array;

		AllSettings *all_settings;
	};

	static OutputContext global_output_context = {};

	DWORD output_parsed_data(LPVOID param) {
		OutputType type = *(OutputType*) param;
		MemoryArena arena = init_memory(MB);
		OutputContext &c = global_output_context;
		HFileCollection &hc = *c.hfile_collection;
		switch (type) {
			case OutputType_NetworkConfig: {
				output_network_config(*c.component_array, hc.rpc_array, hc.network_type_array, *c.go_array, arena); // global.network_config
			} break;
			case OutputType_Flow: {
				output_flow(hc.flow_array, arena); // flow_router.generated.cpp, global.script_flow_nodes, flow_messages.generated.cpp
			} break;
			case OutputType_NetworkRouter: {
				output_network_router(hc.rpc_array, hc.event_array, *c.go_array, c.all_settings->entities, *c.network_conversion_array, hc.network_type_array, arena);
			} break;
			case OutputType_EventDelegate: {
				output_event_delegate(hc.rpc_array, hc.event_array, hc.flow_array, hc.receiver_array, arena);
			} break;
			case OutputType_EntityLookup: {
				output_entity_lookup(*c.component_array, c.all_settings->entities, *c.go_array, arena);
			} break;
			case OutputType_EntitySettings: {
				output_entity_settings(*c.component_array, c.all_settings->entities, hc.settings_struct_array, hc.settings_enum_array, arena);
			} break;
			case OutputType_Components: {
				output_components(*c.component_array, c.reparsed_component_array, hc.network_type_array, arena);
			} break;
			case OutputType_DataComponents: {
				output_data_components(hc.settings_struct_array, hc.settings_enum_array, *c.component_array, c.reparsed_component_array, arena);
			} break;
			case OutputType_SettingsEnums: {
				output_settings_enums(hc.settings_enum_array, arena);
			} break;
			case OutputType_ComponentGroup: {
				output_component_group(*c.sorted_components, *c.component_array, c.all_settings->entities, arena);
			} break;
			case OutputType_StateMachines: {
				output_state_machines(*c.state_machine_array, arena);
			} break;
			case OutputType_LevelZoneSettings: {
				output_level_zones_settings(c.all_settings->level_zones, arena);
			} break;
			case OutputType_LevelGenerationSettings: {
				output_level_generation_setup(c.all_settings->level_generation_setup, arena);
				output_level_generation_settings(c.all_settings->level_generation, c.all_settings->level_regions, c.all_settings->level_locations, c.all_settings->level_planets, arena);
			} break;
			case OutputType_StampSettings: {
				output_stamp_settings(c.all_settings->stamps, arena);
			} break;
			case OutputType_GameStrings: {
				output_game_strings(*c.game_string_array, arena);
			} break;
			case OutputType_AbilityNode: {
				output_ability_nodes(hc.ability_node_array, hc.settings_enum_array, arena);
			} break;
			case OutputType_BehaviorNode: {
				output_behavior_nodes(hc.behavior_node_array, hc.settings_enum_array, arena);
			} break;
			case OutputType_ScriptNode: {
				output_script_nodes(*c.script_node_array, arena);
			} break;
		}
		return 0;
	}

	bool generate(Generator &generator, Cache &cache, FileInfo *file_infos) {
		// Allocate memory
		MemoryArena arena = init_memory(64*MB);
		MemoryArena temp_arena = init_memory(32*MB);

		// Setup containers
		HFileCollection hfile_collection = make_hfile_collection();

		ComponentArray component_array; array_init(component_array, COMPONENT_COUNT);
		bool *reparsed_component_array; array_init(reparsed_component_array, COMPONENT_COUNT);
		GameStringArray game_string_array; array_init(game_string_array, GAME_STRINGS_COUNT);
		ScriptNodeArray script_node_array; array_init(script_node_array, SCRIPT_NODE_COUNT);
		GameObjectArray go_array; array_init(go_array, GAME_OBJECT_COUNT);
		StateMachineArray state_machine_array; array_init(state_machine_array, STATE_MACHINE_COUNT);
		NetworkConversionArray network_conversion_array; array_init(network_conversion_array, NETWORK_CONVERSION_COUNT);

		bool component_array_changed = false;
		bool game_string_array_changed = false;
		bool script_node_array_changed = false;
		bool state_machine_array_changed = false;
		bool network_conversion_array_changed = false;

		AllSettings all_settings;
		array_init(all_settings.entities, SETTINGS_COUNT);
		array_init(all_settings.level_generation_setup, LEVEL_GENERATION_SETUP_COUNT);
		array_init(all_settings.level_zones, LEVEL_ZONES_COUNT);
		array_init(all_settings.level_generation, LEVEL_GENERATION_COUNT);
		array_init(all_settings.level_regions, LEVEL_REGIONS_COUNT);
		array_init(all_settings.level_locations, LEVEL_LOCATIONS_COUNT);
		array_init(all_settings.level_planets, LEVEL_PLANETS_COUNT);
		array_init(all_settings.stamps, STAMP_COUNT);
		array_init(all_settings.weight_rules, WEIGHT_RULE_COUNT);

		// Read cache
		CacheHashMap cache_hash_map = {};
		if (cache.valid)
			read_cache_from_disc("global.cache", arena, cache_hash_map);

		// Go through all files
		bool changed = false;
		for (unsigned i = 0; i < array_count(generator.handle_array); ++i) {
			unsigned handle = generator.handle_array[i];
			FileInfo &info = file_infos[handle];

			CacheHashEntry *entry = 0;
			bool valid_entry = get_cache_entry_for(cache_hash_map, info.key, &entry);

			if (info.state == CacheState_Removed) {
				switch (info.filetype) {
					case FileType_H: {
						bool is_component = string_ends_in(info.filepath, _ending_component_h);
						if (is_component) {
							component_array_changed = true;
						}

						CountSnapshot cached_snapshot = deserialize_snapshot(arena, entry, hfile_collection);
						update_changed_arrays(cached_snapshot, hfile_collection);
					} break;
					case FileType_Cpp: {
						game_string_array_changed = true;
					} break;
					case FileType_Entity: {
						all_settings.entities_changed = true;
						all_settings.level_generation_setup_changed = true;
						all_settings.level_zones_changed = true;
						all_settings.level_generation_changed = true;
						all_settings.level_regions_changed = true;
						all_settings.level_planets_changed = true;
						all_settings.level_locations_changed = true;
						all_settings.stamps_changed = true;
						all_settings.weight_rules_changed = true;
					} break;
					case FileType_GameStateMachine: {
						state_machine_array_changed = true;
					} break;
					case FileType_Conversions: {
						network_conversion_array_changed = true;
					} break;
				}
				continue;
			}

			bool needs_to_parse = !valid_entry || info.state == CacheState_Modified || info.state == CacheState_Added;

			if (needs_to_parse) {
				MemoryBlockHandle block_handle = begin_block(temp_arena);
					switch (info.filetype) {
						case FileType_H: {
							// Check the cache for what we wrote last, if we wrote an rpc for example, we set the rpc_array as changed.
							// This because, we now reparse it so it could have changed name or some such.
							CountSnapshot cached_snapshot = {};
							if (valid_entry) {
								cached_snapshot = deserialize_snapshot(arena, entry, hfile_collection);
								update_changed_arrays(cached_snapshot, hfile_collection);
							}

							CountSnapshot snapshot = take_count_snapshot(hfile_collection);

							bool is_component = string_ends_in(info.filepath, _ending_component_h);
							if (is_component) {
								unsigned game_strings_snapshot = array_count(game_string_array);
								unsigned script_node_snapshot = array_count(script_node_array);
								parse_component(temp_arena, arena, *info.filepath, component_array, game_string_array, script_node_array, hfile_collection);

								// FULKOD(kalle): Modify the write time of the file, so we trigger a re-read after we add stuff to this component h-file.
								HANDLE file_handle = CreateFile(*info.filepath, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
								SetFileTime(file_handle, 0, 0, &cache.entries[handle].file_time);
								CloseHandle(file_handle);

								component::serialize(arena, entry, component_array, reparsed_component_array, game_string_array, game_string_array_changed, game_strings_snapshot, script_node_array, script_node_array_changed, script_node_snapshot, hfile_collection, snapshot);
							} else {
								parse_game_hfile(temp_arena, arena, *info.filepath, hfile_collection);
								serialize_hfile(arena, entry, snapshot, hfile_collection);
							}
						} break;
						case FileType_Cpp: {
							unsigned count_snapshot = array_count(game_string_array);
							parse_for_game_strings(temp_arena, arena, *info.filepath, game_string_array);
							game_string_array_changed = game_string::serialize(arena, count_snapshot, entry, game_string_array) || game_string_array_changed;
						} break;
						case FileType_Entity: {
							unsigned count_entities = array_count(all_settings.entities);
							unsigned count_level_generation_setup = array_count(all_settings.level_generation_setup);
							unsigned count_level_zones = array_count(all_settings.level_zones);
							unsigned count_level_generation = array_count(all_settings.level_generation);
							unsigned count_regions = array_count(all_settings.level_regions);
							unsigned count_locations = array_count(all_settings.level_locations);
							unsigned count_planets = array_count(all_settings.level_planets);
							unsigned count_stamps = array_count(all_settings.stamps);
							unsigned count_weight_rules = array_count(all_settings.weight_rules);

							parse_settings_entity(temp_arena, arena, *info.filepath, component_array, all_settings);

							if (array_count(all_settings.entities) > count_entities)
								all_settings.entities_changed = settings::serialize(arena, entry, all_settings.entities) || all_settings.entities_changed;
							else if (array_count(all_settings.level_generation_setup) > count_level_generation_setup)
								all_settings.level_generation_setup_changed = settings::serialize(arena, entry, all_settings.level_generation_setup) || all_settings.level_generation_setup_changed;
							else if (array_count(all_settings.level_zones) > count_level_zones)
								all_settings.level_zones_changed = settings::serialize(arena, entry, all_settings.level_zones) || all_settings.level_zones_changed;
							else if (array_count(all_settings.level_generation) > count_level_generation)
								all_settings.level_generation_changed = settings::serialize(arena, entry, all_settings.level_generation) || all_settings.level_generation_changed;
							else if (array_count(all_settings.level_regions) > count_regions)
								all_settings.level_regions_changed = settings::serialize(arena, entry, all_settings.level_regions) || all_settings.level_regions_changed;
							else if (array_count(all_settings.level_locations) > count_locations)
								all_settings.level_locations_changed = settings::serialize(arena, entry, all_settings.level_locations) || all_settings.level_locations_changed;
							else if(array_count(all_settings.level_planets) > count_planets)
								all_settings.level_planets_changed = settings::serialize(arena, entry, all_settings.level_planets) || all_settings.level_planets_changed;
							else if (array_count(all_settings.stamps) > count_stamps)
								all_settings.stamps_changed = settings::serialize(arena, entry, all_settings.stamps) || all_settings.stamps_changed;
							else if (array_count(all_settings.weight_rules) > count_weight_rules)
								all_settings.weight_rules_changed = settings::serialize(arena, entry, all_settings.weight_rules) || all_settings.weight_rules_changed;
						} break;
						case FileType_GameStateMachine: {
							// Parse the state machine
							parse_state_machine(temp_arena, arena, *info.filepath, state_machine_array);
							state_machine_array_changed = state_machine::serialize(arena, entry, state_machine_array) || state_machine_array_changed;
						} break;
						case FileType_Conversions: {
							unsigned count_snapshot = array_count(network_conversion_array);
							parse_network_conversions(temp_arena, arena, *info.filepath, network_conversion_array);
							network_conversion_array_changed = network_conversions::serialize(arena, entry, count_snapshot, network_conversion_array) || network_conversion_array_changed;
						} break;
					}
				end_block(temp_arena, block_handle);
			} else {
				switch (info.filetype) {
					case FileType_H: {
						bool is_component = string_ends_in(info.filepath, _ending_component_h);
						if (is_component) {
							component::deserialize(arena, *info.filepath, entry, component_array, reparsed_component_array, game_string_array, script_node_array, hfile_collection);
						} else {
							deserialize_hfile(arena, *info.filepath, entry, hfile_collection);
						}
					} break;
					case FileType_Cpp: {
						game_string::deserialize(arena, entry, game_string_array);
					} break;
					case FileType_Entity: {
						if (entry->value.size > 0) {
							// We have a valid cache entry, use that instead of reparsing the file!
							Settings settings = {};
							array_init(settings.component_settings, 32);
							settings::deserialize(arena, *info.filepath, entry, settings);
							insert_settings(all_settings, settings);
						}
					} break;
					case FileType_GameStateMachine: {
						state_machine::deserialize(arena, *info.filepath, entry, state_machine_array);
					} break;
					case FileType_Conversions: {
						network_conversions::deserialize(arena, *info.filepath, entry, network_conversion_array);
					} break;
				}
			}

			if (!changed) {
				if (info.state == CacheState_Modified || info.state == CacheState_Added) {
					if (info.filetype == FileType_H || info.filetype == FileType_Cpp) {
						changed = true;
					}
				}
			}
		}

		// Coalesce data
		SortedComponents sorted_components = coalesce_parsed_data(arena, component_array, reparsed_component_array, component_array_changed, script_node_array, hfile_collection.behavior_node_array, all_settings.entities, go_array, state_machine_array, hfile_collection.state_array, hfile_collection.event_array);

		// Prepare output
		make_sure_directory_exists(arena, _folder_root, _folder_generated_code);
		make_sure_directory_exists(arena, _folder_root, _folder_generated_code_components);

		make_sure_directory_exists(arena, _folder_root, _folder_generated);
		make_sure_directory_exists(arena, _folder_root, _folder_generated_data_components);
		make_sure_directory_exists(arena, _folder_root, _folder_generated_types);

		global_output_context.sorted_components = &sorted_components;

		global_output_context.hfile_collection = &hfile_collection;

		global_output_context.component_array = &component_array;
		global_output_context.reparsed_component_array = reparsed_component_array;
		global_output_context.game_string_array = &game_string_array;
		global_output_context.script_node_array = &script_node_array;
		global_output_context.go_array = &go_array;
		global_output_context.state_machine_array = &state_machine_array;
		global_output_context.network_conversion_array = &network_conversion_array;

		global_output_context.all_settings = &all_settings;

		OutputType work[32] = {};
		unsigned work_counter = 0;

		//// These could probably be run in parallel
		// Write output
		HFileCollection &hc = hfile_collection; // Shortcut
		if (component_array_changed || hc.rpc_array_changed || hc.network_type_array_changed || all_settings.entities_changed) {
			work[work_counter++] = OutputType_NetworkConfig;
			changed = true;
		}

		if (hc.flow_array_changed) {
			work[work_counter++] = OutputType_Flow;
			changed = true;
		}

		if (hc.rpc_array_changed || hc.event_array_changed || all_settings.entities_changed || network_conversion_array_changed || hc.network_type_array_changed) {
			work[work_counter++] = OutputType_NetworkRouter;
			changed = true;
		}

		if (hc.rpc_array_changed || hc.event_array_changed || hc.flow_array_changed || hc.receiver_array_changed || all_settings.entities_changed || component_array_changed) {
			work[work_counter++] = OutputType_EventDelegate;
			changed = true;
		}

		if (component_array_changed || all_settings.entities_changed) {
			work[work_counter++] = OutputType_EntityLookup;
			changed = true;
		}

		if (component_array_changed || all_settings.entities_changed || hc.settings_struct_array_changed) {
			work[work_counter++] = OutputType_EntitySettings;
			changed = true;
		}

		if (component_array_changed) {
			work[work_counter++] = OutputType_Components;
			changed = true;
		}

		if (component_array_changed || hc.settings_struct_array_changed || hc.settings_enum_array_changed) {
			work[work_counter++] = OutputType_DataComponents;
			changed = true;
		}

		if (hc.settings_enum_array_changed) {
			work[work_counter++] = OutputType_SettingsEnums;
			changed = true;
		}

		if (all_settings.entities_changed || component_array_changed) {
			work[work_counter++] = OutputType_ComponentGroup;
			changed = true;
		}

		if (state_machine_array_changed || hc.state_array_changed) {
			work[work_counter++] = OutputType_StateMachines;
			changed = true;
		}

		if (all_settings.level_zones_changed) {
			work[work_counter++] = OutputType_LevelZoneSettings;
			changed = true;
		}

		if (all_settings.level_generation_changed || all_settings.level_generation_setup_changed
			|| all_settings.level_regions_changed || all_settings.level_locations_changed
			|| all_settings.level_planets_changed ) {
			work[work_counter++] = OutputType_LevelGenerationSettings;
			changed = true;
		}

		if (all_settings.stamps_changed) {
			work[work_counter++] = OutputType_StampSettings;
			changed = true;
		}

		if (game_string_array_changed) {
			work[work_counter++] = OutputType_GameStrings;
			changed = true;
		}

		if (hc.ability_node_array_changed || hc.settings_enum_array_changed) {
			work[work_counter++] = OutputType_AbilityNode;
			changed = true;
		}

		if (hc.behavior_node_array_changed || hc.settings_enum_array_changed) {
			work[work_counter++] = OutputType_BehaviorNode;
			changed = true;
		}

		if (component_array_changed) {
			work[work_counter++] = OutputType_ScriptNode;
			changed = true;
		}

#define THREADS 1
#if THREADS
		HANDLE threads[32];
		unsigned thread_count = 0;
		for (unsigned i = 0; i < work_counter; ++i) {
			threads[thread_count++] = CreateThread(NULL, 0, output_parsed_data, work + i, 0, 0);
		}
		WaitForMultipleObjects(thread_count, threads, TRUE, INFINITE);
#else
		for (unsigned i = 0; i < work_counter; ++i) {
			output_parsed_data(work + i);
		}
#endif

		// Write cache
		write_cache_to_disc("global.cache", cache_hash_map);

		return changed;
	}

	void register_generator(Generator &generator) {
		generator.name = MAKE_STRING("global");

		static const unsigned start_handle_count = 1024*8;
		array_init(generator.handle_array, start_handle_count);

		generator.active = true;

		generator.locations = Location_Code | Location_Foundation | Location_Content;
		generator.filetypes = FileType_H | FileType_Cpp | FileType_Entity | FileType_Conversions | FileType_GameStateMachine;
		generator.states = CacheState_All;

		generator.ignored_directory = &ignored_directory;
		generator.generate = &generate;
	}
}

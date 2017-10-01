struct ComponentHashEntry {
	unsigned key;
	Component *value;
};

struct SortedComponents {
	Component **entries;
	unsigned count;
	inline Component *operator[](unsigned index) { return entries[index]; }
	inline void push_back(Component *component) {
		entries[count++] = component;
	}
};

bool does_exists(ComponentArray &component_array, unsigned component_stem_id) {
	for (int j = 0; j < array_count(component_array); ++j) {
		Component &component = component_array[j];
		if (component.stem_id == component_stem_id) {
			return true;
		}
	}
	return false;
}

Settings *get_settings_for(SettingsArray &settings_array, uint64_t path_id) {
	for (int j = 0; j < array_count(settings_array); ++j) {
		Settings &settings = settings_array[j];
		if (settings.path_id == path_id) {
			return &settings;
		}
	}
	return 0;
}

int get_component_settings_index_for(Settings &settings, unsigned component_stem_id) {
	for (unsigned i = 0; i < array_count(settings.component_settings); ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.component_stem_id == component_stem_id) {
			return i;
		}
	}
	return -1;
}

ComponentSettings *find_component_in_prefab(unsigned guid_id, Settings &settings) {
	for (unsigned i = 0; i < array_count(settings.component_settings); ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.guid_id == guid_id) {
			return &component_settings;
		}
	}
	return 0;
}

void ensure_dependencies(Settings &settings, Component &component) {
	for (unsigned i = 0; i < component.dependency_array.count; ++i) {
		unsigned stem_id = component.dependency_array[i].stem_id;

		bool found_dependent_component = false;
		int index = get_component_settings_index_for(settings, stem_id);
		ASSERT(index >= 0, "Entity is missing a dependency! (component=%s, dependency=%s, entity=%s)", *component.name, *component.dependency_array[i].stem, *settings.path);
	}
}

void resolve_data_store_inheritance(SettingsDataStore *data_store, SettingsDataStore &prefab_data_store) {
	for (unsigned i = 0; i < prefab_data_store.count; ++i) {
		SettingsData &prefab_settings_data = prefab_data_store[i];

		HASH_LOOKUP(entry, data_store->map, ARRAY_COUNT(prefab_data_store.map), prefab_settings_data.name_id);
		if (entry->key == 0) { // We found a setting for the prefab, but no override. Copy the prefab settings.
			add_settings_data(*data_store, prefab_settings_data);
		} else { // We found a setting for the prefab but an override. If the value is simple (e.g. a number), we do nothing but if it is table we need to continue recursively.
			if (prefab_settings_data.next) {
				SettingsData *settings_data = entry->value;
				ASSERT(settings_data->next, "Prefab is a table but override is not! (name=%s)", *prefab_settings_data.name);
				resolve_data_store_inheritance(settings_data->next, *prefab_settings_data.next);
			}
		}
	}
}

void resolve_inheritance(SettingsArray &settings_array, Settings &settings) {
	if (settings.inherit_path.length == 0)
		return;

	Settings *inherit_settings = get_settings_for(settings_array, settings.inherit_path_id);
	ASSERT(inherit_settings, "Could not find the inherit path! (settings='%s', inherit_path='%s')", *settings.path, *settings.inherit_path);
	resolve_inheritance(settings_array, *inherit_settings);

	for (unsigned i = 0; i < array_count(settings.component_settings); ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.component_stem_id == 0) { // a modified component
			ComponentSettings &inherit_component_settings = *find_component_in_prefab(component_settings.guid_id, *inherit_settings);

			component_settings.component_stem_id = inherit_component_settings.component_stem_id;

			SettingsDataStore *settings_data_store = component_settings.settings_data_store;
			SettingsDataStore *prefab_settings_data_store = inherit_component_settings.settings_data_store;
			resolve_data_store_inheritance(settings_data_store, *prefab_settings_data_store);
		}
	}

	unsigned count = array_count(settings.component_settings);
	for (unsigned i = 0; i < array_count(inherit_settings->component_settings); ++i) {
		ComponentSettings &inherit_component_settings = inherit_settings->component_settings[i];

		bool found = false;
		for (int j = 0; j < count && !found; ++j) {
			ComponentSettings &component_settings = settings.component_settings[j];
			found = component_settings.component_stem_id == inherit_component_settings.component_stem_id;
		}

		if (!found) {
			array_push(settings.component_settings, inherit_component_settings);
		}
	}
}

SortedComponents coalesce_parsed_data(MemoryArena &arena, ComponentArray &component_array, bool *reparsed_components, bool &component_array_changed, ScriptNodeArray &script_node_array, BehaviorNodeArray &behavior_node_array, SettingsArray &settings_array, GameObjectArray &go_array, StateMachineArray &state_machine_array, StateArray &state_array, EventArray &event_array) {
	// Sort components on dependencies
	unsigned hash_count = array_count(component_array) * 2;
	unsigned hash_size = sizeof(ComponentHashEntry) * hash_count;
	ComponentHashEntry *component_hash_map = (ComponentHashEntry *)allocate_memory(arena, hash_size);
	memset(component_hash_map, 0, hash_size);

	unsigned left_count = 0;
	unsigned left_size = sizeof(Component*) * array_count(component_array);
	Component **left = (Component **)allocate_memory(arena, left_size);
	memset(left, 0, left_size);

	SortedComponents sorted_components = {};

	sorted_components.count = 0;
	unsigned sorted_components_size = sizeof(Component*) * array_count(component_array);
	sorted_components.entries = (Component **)allocate_memory(arena, sorted_components_size);
	memset(sorted_components.entries, 0, sorted_components_size);

	for (unsigned i = 0; i < array_count(component_array) && !component_array_changed; ++i) {
		component_array_changed = reparsed_components[i];
	}

	// We currently have an after_component_array and a before_component_array. It's easier to handle the depency chain if we have a single directed chain instead.
	// Therefore, convert the entries in the componets before_component_array to after_component_array.
	for (unsigned i = 0; i < array_count(component_array); ++i) {
		Component &component = component_array[i];

		for (unsigned j = 0; j < component.before_component_array.count; ++j) {
			ComponentReference &ref = component.before_component_array[j];
			
			// This component must be before ref. Therefore, find the referenced component and insert this into it's after.
			Component *other_component = get_component(ref.stem_id, component_array);
			ComponentReference &after_ref = other_component->after_component_array.new_entry();
			after_ref.stem = component.stem;
			after_ref.stem_id = component.stem_id;
		}
	}

	for (unsigned i = 0; i < array_count(component_array); ++i) {
		Component &component = component_array[i];

		if (component.after_component_array.count == 0) { // It's not dependent on anything being prior to this
			HASH_LOOKUP(entry, component_hash_map, hash_count, component.stem_id);
			entry->key = component.stem_id;
			entry->value = &component;
			sorted_components.push_back(&component);
		} else {
			left[left_count++] = &component;
			for (int j = 0; j < component.after_component_array.count; ++j) {
				ComponentReference &prior = component.after_component_array[j];
				ASSERT(does_exists(component_array, prior.stem_id), "Could not find dependency component '%s' in '%s'!", *prior.stem, *component.stem);
			}
		}
	}

	for (unsigned i = 0; i < array_count(settings_array); ++i) {
		Settings &settings = settings_array[i];
		resolve_inheritance(settings_array, settings);
	}

	unsigned previous_left_count = left_count;
	while (left_count > 0) {
		for (unsigned i = 0; i < left_count; ++i) {
			Component *component = left[i];
			unsigned count = component->after_component_array.count;
			ASSERT(count > 0, "Component dependency error");

			bool all_prior_done = true;
			for (int j = 0; j < count; ++j) {
				ComponentReference &prior = component->after_component_array[j];
				HASH_LOOKUP(entry, component_hash_map, hash_count, prior.stem_id);

				bool in_hash = entry->key == prior.stem_id;
				if (!in_hash) {
					all_prior_done = false;
					break;
				}
			}

			if (all_prior_done) {
				HASH_LOOKUP(entry, component_hash_map, hash_count, component->stem_id);
				entry->key = component->stem_id;
				entry->value = component;
				sorted_components.push_back(component);
				left[i] = left[--left_count];
			}
		}
		ASSERT(left_count < previous_left_count, "Circular component dependencies detected!");
		previous_left_count = left_count;
	}

	ASSERT(left_count == 0 && sorted_components.count == array_count(component_array), "Component dependency error");

	GameObjectArray temp_go_object_array;
	array_init(temp_go_object_array, array_count(settings_array));

	// Collapse settings_path to settings_index
	for (unsigned i = 0; i < array_count(settings_array); ++i) {
		Settings &settings = settings_array[i];

		// Create game_object reference
		bool create_game_object = false;
		// ComponentSettings *settings = entity.settings_list;
		for (int j = 0; j < array_count(settings.component_settings); ++j) {
			ComponentSettings &component_settings = settings.component_settings[j];

			Component *component = get_component(component_settings.component_stem_id, component_array);
			ASSERT(component, "Invalid game component found in (settings='%s', stem_id=0x%x, index=%d)", *settings.path, component_settings.component_stem_id, j);
			component_settings.component = component;
			SubCompStruct *sub_comps = component->sub_comps;
			if (!create_game_object && HAS_SUB_COMP(SubCompType_Network)) {
				create_game_object = true;
			}
		}

		// Sort the component settings
		unsigned target_index = 0;
		for (int j = 0; j < sorted_components.count; ++j) {
			Component *component = sorted_components[j];
			int source_index = get_component_settings_index_for(settings, component->stem_id);
			if (source_index >= 0) {
				ComponentSettings temp = settings.component_settings[target_index]; // Save whatever is in the target
				settings.component_settings[target_index++] = settings.component_settings[source_index]; // Overwrite target
				settings.component_settings[source_index] = temp;

				ensure_dependencies(settings, *component);
			}
		}

		if (create_game_object) {
			String game_object_name = clone_string(settings.path, arena);
			for (unsigned i = 0; i < game_object_name.length; i++) {
				char c = game_object_name[i];
				if (c == '\\' || c == '/')
					game_object_name.text[i] = '_';
			}

			array_new_entry(temp_go_object_array);
			GameObject &game_object = array_last(temp_go_object_array);
			game_object.name = game_object_name;
			game_object.name_id = make_string_id(game_object_name);

			// Bidirectional lookup
			game_object.settings = &settings;
			settings.game_object = &game_object;
		}
	}

	// We need to sort our game objects because stingray does it.
	SortElement *sorted_entities = (SortElement*)allocate_memory(arena, array_count(temp_go_object_array) * sizeof(SortElement));
	for (unsigned i = 0; i < array_count(temp_go_object_array); ++i) {
		GameObject &go = temp_go_object_array[i];
		sorted_entities[i].value = go.name_id;
		sorted_entities[i].index = i;
	}
	quick_sort(sorted_entities, array_count(temp_go_object_array));

	for (unsigned i = 0; i < array_count(temp_go_object_array); ++i) {
		GameObject &go = temp_go_object_array[sorted_entities[i].index];
		array_push(go_array, go);
	}	

	State **states = (State**)allocate_memory(arena, array_count(state_array) * sizeof(State*));
	unsigned state_count = array_count(state_array);

	for (unsigned i = 0; i < array_count(state_array); ++i) {
		states[i] = &state_array[i];
	}

	for (unsigned i = 0; i < array_count(state_machine_array); ++i) {
		StateMachine &state_machine = state_machine_array[i];

		for (int j = 0; j < state_count; ++j) {
			State *state = states[j];
			if (state->machine_name_id == state_machine.name_id) {
				states[j--] = states[--state_count];
				array_push(state_machine.states, state);
			}
		}
	}

	unsigned hashmap_count = array_count(event_array) * 2;
	HashEntry *hashmap = (HashEntry *) allocate_memory(arena, hashmap_count * sizeof(HashEntry));

	for (unsigned i = 0; i < array_count(event_array); ++i) {
		Event &event = event_array[i];

		HASH_LOOKUP(entry, hashmap, hashmap_count, event.function.name_id);
		if (entry->key == event.function.name_id) { // was already here!
			Event &existing_event = event_array[entry->value];

			for (unsigned j = 0; j < event.component_count; ++j) {
				bool should_add = true;
				for (unsigned k = 0; k < existing_event.component_count; ++k) {
					if (existing_event.component_ids[k] == event.component_ids[j]) {
						should_add = false;
						break;
					}
				}
				if (should_add) {
					existing_event.component_ids[existing_event.component_count++] = event.component_ids[j];;
				}
			}
			event.component_count = 0;
		} else {
			entry->key = event.function.name_id;
			entry->value = i;
		}
	}	

	//// Calculate the number of possible entities for each entity events
	for (unsigned i = 0; i < array_count(event_array); ++i) {
		Event &event = event_array[i];

		for (unsigned j = 0; j < event.component_count; ++j) {
			unsigned component_id = event.component_ids[j];

			bool should_i_be_kept = true;

			ASSERT(does_exists(component_array, component_id), "Cannot find component referenced from entity event %s", *event.function.name);
			HASH_LOOKUP(entry, component_hash_map, hash_count, component_id);
			Component &component = *entry->value;
			int number = atoi(*component.max_instances);

			for (unsigned k = 0; k < array_count(settings_array) && should_i_be_kept; ++k) {
				Settings &settings = settings_array[k];

				// See if we are part of this settings file
				int index = get_component_settings_index_for(settings, component_id);
				if (index != -1) {

					// If so, then check if the settings file contains any of the other component_ids
					// NOTE(kalle): We only need to check ahead (j+1), because we have already handled the ones behind us
					for (unsigned l = j+1; l < event.component_count; ++l) {
						int settings_index = get_component_settings_index_for(settings, event.component_ids[l]);
						if (settings_index != -1) {
							HASH_LOOKUP(entry, component_hash_map, hash_count, event.component_ids[l]);
							Component &other_component = *entry->value;

							int other_number = atoi(*other_component.max_instances);
							if (other_number > number) {
								// printf("Component '%s'(%d) is removed due to number in '%s'(%d) being heigher\n", *component.name, number, *other_component.name, other_number);
								should_i_be_kept = false;
								event.component_ids[j--] = event.component_ids[--event.component_count];
								break;
							} else {
								// printf("Component '%s'(%d) removed due to number in '%s'(%d) being heigher\n", *other_component.name, other_number, *component.name, number);
								event.component_ids[l--] = event.component_ids[--event.component_count];
							}
						}
					}
				}
			}
		}


		for (unsigned j = 0; j < event.component_count; ++j) {
			unsigned component_id = event.component_ids[j];
			HASH_LOOKUP(entry, component_hash_map, hash_count, component_id);
			Component &component = *entry->value;
			int number = atoi(*component.max_instances);
			event.max_number_of_entities += number;
			// printf("Event '%s' gets '%d' max added due to component '%s'\n", *event.function.name, number, *component.stem);
		}

		// printf("Event '%s' got '%d' max\n", *event.function.name, event.max_number_of_entities);
	}

	for (unsigned i = 0; i < array_count(script_node_array); i++) {
		ScriptNode &script_node = script_node_array[i];

		HASH_LOOKUP(entry, component_hash_map, hash_count, script_node.component_stem_id);
		script_node.component = entry->value;
		SubCompStruct &sub_comp = entry->value->sub_comps[script_node.sub_component_type];
		script_node.member = &sub_comp.member_array[script_node.member_index];
	}

	return sorted_components;
}
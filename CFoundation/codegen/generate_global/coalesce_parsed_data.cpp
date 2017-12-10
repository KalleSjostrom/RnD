struct ComponentHashEntry {
	unsigned key;
	Component *value;
};

struct SortedComponents {
	Component **entries;
	unsigned count;
};
inline void append(SortedComponents &list, Component *component) { list.entries[list.count++] = component; }

// For sorting the game objects on id (why? because stingray..)
struct SortElement {
	u32 value;
	u32 index;
};
inline u32 calculate_pivot(SortElement *list, u32 size) {
	u64 a = (u64)list[0].value;
	u64 b = (u64)list[size-1].value;
	u64 c = 2;
	u64 average = (a + b)/c;
	return (u32)average;
}
void quick_sort(SortElement *list, u32 size) {
	if (size <= 1)
		return;

	u32 pivot = calculate_pivot(list, size);

	int right = size;
	int left = -1;

	while (1) {
		do {
			right--;
		} while ((list + right)->value > pivot);
		do {
			left++;
		} while ((list + left)->value < pivot);

		if (left < right) {
			SortElement temp = list[left];
			list[left] = list[right];
			list[right] = temp;
		} else {
			break;
		}
	}
	quick_sort(list, left);
	quick_sort(list+right+1, size-(right+1));
}

Settings *get_settings_for(SettingsArray &settings_array, uint64_t path_id) {
	for (int j = 0; j < settings_array.count; ++j) {
		Settings &settings = settings_array.entries[j];
		if (settings.path_id == path_id) {
			return &settings;
		}
	}
	return 0;
}

int get_component_settings_index_for(Settings &settings, unsigned component_stem_id) {
	for (int i = 0; i < settings.component_settings_count; ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.component_stem_id == component_stem_id) {
			return i;
		}
	}
	return -1;
}

ComponentSettings *find_component_in_prefab(unsigned guid_id, Settings &settings) {
	for (int i = 0; i < settings.component_settings_count; ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.guid_id == guid_id) {
			return &component_settings;
		}
	}
	return 0;
}

void ensure_dependencies(Settings &settings, Component &component) {
	for (int i = 0; i < component.dependency_array.count; ++i) {
		unsigned stem_id = component.dependency_array.entries[i].stem_id;

		bool found_dependent_component = false;
		int index = get_component_settings_index_for(settings, stem_id);
		ASSERT(index >= 0, "Entity is missing a dependency! (component=%s, dependency=%s, entity=%s)", *component.name, *component.dependency_array.entries[i].stem, *settings.path);
	}
}

void resolve_data_store_inheritance(SettingsDataStore *data_store, SettingsDataStore *prefab_data_store) {
	for (int i = 0; i < prefab_data_store->count; ++i) {
		SettingsData &prefab_settings_data = prefab_data_store->entries[i];

		HASH_LOOKUP(entry, data_store->map, ARRAY_COUNT(prefab_data_store->map), prefab_settings_data.name_id);
		if (entry->key == 0) { // We found a setting for the prefab, but no override. Copy the prefab settings.
			add_settings_data(*data_store, prefab_settings_data);
		} else { // We found a setting for the prefab but an override. If the value is simple (e.g. a number), we do nothing but if it is table we need to continue recursively.
			if (prefab_settings_data.next) {
				SettingsData *settings_data = entry->value;
				ASSERT(settings_data->next, "Prefab is a table but override is not! (name=%s)", *prefab_settings_data.name);
				resolve_data_store_inheritance(settings_data->next, prefab_settings_data.next);
			}
		}
	}
}

void resolve_inheritance(SettingsArray &settings_array, Settings &settings) {
	if (settings.inherit_path.length == 0)
		return;

	Settings &inherit_settings = *get_settings_for(settings_array, settings.inherit_path_id);
	resolve_inheritance(settings_array, inherit_settings);

	for (int i = 0; i < settings.component_settings_count; ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.component_stem_id == 0) { // a modified component
			ComponentSettings &inherit_component_settings = *find_component_in_prefab(component_settings.guid_id, inherit_settings);

			component_settings.component_stem_id = inherit_component_settings.component_stem_id;

			SettingsDataStore *settings_data_store = component_settings.settings_data_store;
			SettingsDataStore *prefab_settings_data_store = inherit_component_settings.settings_data_store;
			resolve_data_store_inheritance(settings_data_store, prefab_settings_data_store);
		}
	}

	unsigned component_settings_count = settings.component_settings_count;
	for (int i = 0; i < inherit_settings.component_settings_count; ++i) {
		ComponentSettings &inherit_component_settings = inherit_settings.component_settings[i];

		bool found = false;
		for (int j = 0; j < component_settings_count && !found; ++j) {
			ComponentSettings &component_settings = settings.component_settings[j];
			found = component_settings.component_stem_id == inherit_component_settings.component_stem_id;
		}

		if (!found) {
			ARRAY_CHECK_BOUNDS_COUNT(settings.component_settings, settings.component_settings_count);
			settings.component_settings[settings.component_settings_count++] = inherit_component_settings;
		}
	}
}

SortedComponents coalesce_parsed_data(MemoryArena &arena, ComponentArray &component_array, SettingsArray &settings_array, GameObjectArray &go_array, StateMachineArray &state_machine_array, StateArray &state_array) {
	// Sort components on dependencies
	unsigned hash_count = component_array.count * 2;
	unsigned hash_size = sizeof(ComponentHashEntry) * hash_count;
	ComponentHashEntry *component_hash_map = (ComponentHashEntry *)allocate_memory(arena, hash_size);
	memset(component_hash_map, 0, hash_size);

	unsigned left_count = 0;
	unsigned left_size = sizeof(Component*) * component_array.count;
	Component **left = (Component **)allocate_memory(arena, left_size);
	memset(left, 0, left_size);

	SortedComponents sorted_components = {};

	sorted_components.count = 0;
	unsigned sorted_components_size = sizeof(Component*) * component_array.count;
	sorted_components.entries = (Component **)allocate_memory(arena, sorted_components_size);
	memset(sorted_components.entries, 0, sorted_components_size);

	for (int i = 0; i < component_array.count; ++i) {
		Component *component = component_array.entries + i;

		if (component->prior_component_array.count == 0) { // It's not dependent on anything being prior to this
			HASH_LOOKUP(entry, component_hash_map, hash_count, component->stem_id);
			entry->key = component->stem_id;
			entry->value = component;
			append(sorted_components, component);
		} else {
			left[left_count++] = component;
		}
	}

	unsigned previous_left_count = left_count;
	while (left_count > 0) {
		for (int i = 0; i < left_count; ++i) {
			Component *component = left[i];
			unsigned count = component->prior_component_array.count;
			ASSERT(count > 0, "Component dependency error");

			bool all_prior_done = true;
			for (int j = 0; j < count; ++j) {
				ComponentReference &prior = component->prior_component_array.entries[j];
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
				append(sorted_components, component);
				left[i] = left[--left_count];
			}
		}
		ASSERT(left_count < previous_left_count, "Circular component dependencies detected!");
		previous_left_count = left_count;
	}

	ASSERT(left_count == 0 && sorted_components.count == component_array.count, "Component dependency error");

	GameObjectArray temp_go_object_array = {};
	temp_go_object_array.entries = (GameObject*)allocate_memory(arena, settings_array.count * sizeof(GameObject));

	// Collapse settings_path to settings_index
	for (int i = 0; i < settings_array.count; ++i) {
		Settings &settings = settings_array.entries[i];

		// Create game_object reference
		bool create_game_object = false;
		// ComponentSettings *settings = entity.settings_list;
		for (int j = 0; j < settings.component_settings_count; ++j) {
			ComponentSettings &component_settings = settings.component_settings[j];

			Component *component = get_component(component_settings.component_stem_id, component_array);
			ASSERT(component, "Found no valid game component in settings='%s'", *settings.path);
			component_settings.component = component;
			SubCompStruct *sub_comps = component->sub_comps;
			if (!create_game_object && HAS_SUB_COMP(NETWORK)) {
				create_game_object = true;
			}
		}

		// Sort the component settings
		unsigned target_index = 0;
		for (int j = 0; j < sorted_components.count; ++j) {
			Component *component = sorted_components.entries[j];
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
			for (int i = 0; i < game_object_name.length; i++) {
				char c = game_object_name[i];
				if (c == '\\' || c == '/')
					game_object_name.text[i] = '_';
			}

			GameObject &game_object = temp_go_object_array.entries[temp_go_object_array.count++];
			game_object.name = game_object_name;
			game_object.name_id = make_string_id(game_object_name);

			// Bidirectional lookup
			game_object.settings = &settings;
			settings.game_object = &game_object;
		}
	}

	// We need to sort our game objects because stingray does it.
	SortElement *sorted_entities = (SortElement*)allocate_memory(arena, temp_go_object_array.count * sizeof(SortElement));
	for (int i = 0; i < temp_go_object_array.count; ++i) {
		GameObject &go = temp_go_object_array.entries[i];
		sorted_entities[i].value = go.name_id;
		sorted_entities[i].index = i;
	}
	quick_sort(sorted_entities, temp_go_object_array.count);

	for (int i = 0; i < temp_go_object_array.count; ++i) {
		GameObject &go = temp_go_object_array.entries[sorted_entities[i].index];
		ARRAY_CHECK_BOUNDS(go_array);
		go_array.entries[go_array.count++] = go;
	}

	for (int i = 0; i < component_array.count && !component_array.changed; ++i) {
		component_array.changed = component_array.reparsed[i];
	}

	State **states = (State**)allocate_memory(arena, state_array.count * sizeof(State*));
	unsigned state_count = state_array.count;

	for (int i = 0; i < state_array.count; ++i) {
		states[i] = &state_array.entries[i];
	}

	for (int i = 0; i < state_machine_array.count; ++i) {
		StateMachine &state_machine = state_machine_array.entries[i];

		for (int j = 0; j < state_count; ++j) {
			State *state = states[j];
			if (state->machine_name_id == state_machine.name_id) {
				states[j--] = states[--state_count];
				ARRAY_CHECK_BOUNDS_COUNT(state_machine.states, state_machine.state_count);
				state_machine.states[state_machine.state_count++] = state;
			}
		}
	}

	return sorted_components;
}
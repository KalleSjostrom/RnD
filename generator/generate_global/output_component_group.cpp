unsigned get_sorted_component_index(SortedComponents &sorted_components, unsigned stem_id) {
	for (unsigned i = 0; i < sorted_components.count; ++i) {
		Component &component = *sorted_components[i];
		if (component.stem_id == stem_id)
			return i;
	}
	ASSERT(false, "Unknown component stem id! %u", stem_id);
	return 0xffffffff;
}

// Generate the component group
void output_component_group(SortedComponents &sorted_components, ComponentArray &component_array, SettingsArray &settings_array, MemoryArena &arena) {
	static String generated_component_folder = MAKE_STRING(GAME_CODE_DIR "/generated/components");

	{ // Output h file
		String output_filepath = make_filepath(arena, _folder_root, generated_component_folder, MAKE_STRING("component_group.generated"), MAKE_STRING(".h"));
		MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

		// Revert the order when including so that those that aren't dependent on anyone (the leaves of the dependency tree) are inluded first
		for (int i = sorted_components.count - 1; i >= 0; --i) {
			Component &component = *sorted_components[i];
fprintf(output, "#include \"components/%s_component.h\"\n", *component.stem);
		}
fprintf(output, "\n");
fprintf(output, "#include \"../entity_settings.generated.h\"\n");
fprintf(output, "\n");
fprintf(output, "class ComponentGroup {\n");
fprintf(output, "	friend struct Reloader;\n");
fprintf(output, "public:\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components[i];
fprintf(output, "	%s %s;\n", *component.name, *component.stem);
		}
fprintf(output, "\n");
fprintf(output, "	void init_components(CreationContext &context);\n");
fprintf(output, "	void deinit_components();\n");
fprintf(output, "	void script_reload();\n");
fprintf(output, "	void update(float dt);\n");
fprintf(output, "	void add_to_component(Instance *instance, SpawnContext &spawn_context, GameObjectManager *gom);\n");
fprintf(output, "	void notify_on_added(Instance *instance, SpawnContext &spawn_context);\n");
fprintf(output, "	void remove_from_component(Instance *instance);\n");
fprintf(output, "	void migrated_to_me(Instance *instance);\n");
fprintf(output, "	void migrated_away(Instance *instance);\n");
fprintf(output, "	void update_network_state(Instance *instance, const char *buffer, unsigned *buffer_offset);\n");
fprintf(output, "	EventDelegate *event_delegate;\n");
fprintf(output, "};\n");
fprintf(output, "\n");

		fclose(output);
	}

	{ // Output cpp file
		String output_filepath = make_filepath(arena, _folder_root, generated_component_folder, MAKE_STRING("component_group.generated"), MAKE_STRING(".cpp"));
		MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

		// Revert the order when including so that those that aren't dependent on anyone (the leaves of the dependency tree) are inluded first
		for (int i = sorted_components.count - 1; i >= 0; --i) {
			Component &component = *sorted_components[i];
fprintf(output, "#include \"components/%s_component.cpp\"\n", *component.stem);
		}
fprintf(output, "\n");

		unsigned component_mask_count = (unsigned)ceilf(sorted_components.count / 64.0f);
fprintf(output, "struct ComponentList {\n");
fprintf(output, "	uint64_t masks[%u]; // ceilf(component count / 64);\n", component_mask_count);
fprintf(output, "};\n");
fprintf(output, "ComponentList entity_component_mask_array[] = {\n");
		for (int i = 0; i < array_count(settings_array); ++i) {
			Settings &settings = settings_array[i];
			uint64_t masks[64] = {};

fprintf(output, "	{");
			for (int j = 0; j < array_count(settings.component_settings); ++j) {
				ComponentSettings &component_settings = settings.component_settings[j];
				Component &component = *component_settings.component;
				unsigned component_index = get_sorted_component_index(sorted_components, component.stem_id);

				int mask_index = component_index / 64;
				int bit_index  = component_index % 64;
				masks[mask_index] |= ((uint64_t)1 << bit_index);
			}
			for (int j = 0; j < component_mask_count; ++j) {
fprintf(output, "0x%016llx,", masks[j]);
			}
fprintf(output, "}, // %s\n", *settings.path);
		}
fprintf(output, "};\n");
fprintf(output, "void ComponentGroup::init_components(CreationContext &context) {\n");
fprintf(output, "	event_delegate = context.event_delegate;\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components[i];
			if (IS_SET(component, ComponentMask_Init)) {
fprintf(output, "	%s.init(context);\n", *component.stem);
			}
			if (component.num_received_events)
fprintf(output, "	event_delegate->register_%s_component(&%s);\n", *component.stem, *component.stem);
		}
fprintf(output, "}\n\n");
fprintf(output, "void ComponentGroup::deinit_components() {\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components[i];
			if (IS_SET(component, ComponentMask_Deinit)) {
fprintf(output, "	%s.deinit();\n", *component.stem);
			}
			if (component.num_received_events)
fprintf(output, "	event_delegate->unregister_%s_component(&%s);\n", *component.stem, *component.stem);
		}
fprintf(output, "}\n\n");

fprintf(output, "void ComponentGroup::script_reload() {\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components[i];
			if (IS_SET(component, ComponentMask_ScriptReload)) {
fprintf(output, "	%s.script_reload();\n", *component.stem);
			}
		}
fprintf(output, "}\n\n");

fprintf(output, "void ComponentGroup::update(float dt) {\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components[i];
			if (IS_SET(component, ComponentMask_Update)) {
fprintf(output, "	{ Profile p(\"%s::update\"); %s.update(dt); }\n", *component.name, *component.stem);
			}
		}
fprintf(output, "}\n\n");

		{ /////////////////// ADD TO COMPONENT //////////////////
fprintf(output,  "void ComponentGroup::add_to_component(Instance *instance, SpawnContext &spawn_context, GameObjectManager *gom) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	AhTempAllocator ta;\n");
fprintf(output,  "	GameObjectField default_fields[MAX_GO_FIELDS];\n");
fprintf(output,  "	unsigned field_counter = 0;\n");
fprintf(output,  "	Id32 game_object_type = 0xffffffff;\n");
fprintf(output,  "\n");
fprintf(output,  "	ComponentList component_list = entity_component_mask_array[instance->entity_id];\n");
fprintf(output,  "\n");
fprintf(output,  "	unsigned count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
fprintf(output, "					case %d: { // %s\n", i, *component.stem);
				SubCompStruct *sub_comps = component.sub_comps;
				if (IS_SET(component, ComponentMask_Slave)) {
fprintf(output,  "						%s.add(instance, spawn_context);\n", *component.stem);
				} else {
fprintf(output,  "						if (spawn_context.is_master) { %s.add(instance, spawn_context); }\n", *component.stem);
				}
				if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(SubCompType_Network)) {
fprintf(output,  "						%s.fill_in_default_go_fields(instance, spawn_context, default_fields, &field_counter, ta);\n", *component.stem);
				}
fprintf(output, "					} break;\n");
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "	if (spawn_context.is_master) {\n");
fprintf(output,  "		game_object_type = entity_id_to_game_object_type(instance->entity_id);\n");
fprintf(output,  "	}\n");
fprintf(output,  "	if (game_object_type != 0xffffffff) {\n");
fprintf(output,  "		instance->go_id = gom->create_game_object(game_object_type, default_fields, field_counter);\n");
fprintf(output,  "\n");
fprintf(output,  "#if DEVELOPMENT\n");
fprintf(output,  "		GameSessionPtr game_session = _Network.game_session();\n");
fprintf(output,  "\n");
fprintf(output,  "		unsigned type_index = _GameSession.game_object_type(game_session, instance->go_id);\n");
fprintf(output,  "		IdString64 entity_path = get_entity_path(instance->entity_id);\n");
fprintf(output,  "\n");
fprintf(output,  "		IdString64 lookup_entity_path = game_object_index_to_entity_path(type_index);\n");
fprintf(output,  "		ASSERT(lookup_entity_path == entity_path, \"entity_path mismatch, expected '%%s', got '%%s' (type_index=%%u, game_object_type=0x%%x, entity_id=%%u)!\", ID64_STR(entity_path), ID64_STR(lookup_entity_path), type_index, game_object_type, instance->entity_id);\n");
fprintf(output,  "#endif // DEVELOPMENT\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n\n");
		}

		{ /////////////////// ON ADDED //////////////////
fprintf(output,  "void ComponentGroup::notify_on_added(Instance *instance, SpawnContext &spawn_context) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	ComponentList component_list = entity_component_mask_array[instance->entity_id];\n");
fprintf(output,  "\n");
fprintf(output,  "	unsigned count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				if (IS_SET(component, ComponentMask_OnAdded)) {
					if (IS_SET(component, ComponentMask_Slave)) {
fprintf(output,  "					case %d: { %s.on_added(instance, spawn_context); } break;\n", i, *component.stem);
					} else {
fprintf(output,  "					case %d: { if (spawn_context.is_master) { %s.on_added(instance, spawn_context); } } break;\n", i, *component.stem);
					}
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// REMOVE FROM COMPONENT //////////////////
fprintf(output,  "void ComponentGroup::remove_from_component(Instance *instance) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	ComponentList component_list = entity_component_mask_array[instance->entity_id];\n");
fprintf(output,  "\n");
fprintf(output,  "	unsigned count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				if (IS_SET(component, ComponentMask_OnRemoved)) {
					if (IS_SET(component, ComponentMask_Slave)) {
fprintf(output,  "					case %d: { %s.on_removed(instance); } break;\n", i, *component.stem);
					} else {
fprintf(output,  "					case %d: { if (HAS_COMPONENT(%s, instance->entity)) { %s.on_removed(instance); } } break;\n", i, *component.stem, *component.stem);
					}
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "	count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				if (IS_SET(component, ComponentMask_Slave)) {
fprintf(output,  "					case %d: { %s.remove(instance); } break;\n", i, *component.stem);
				} else {
fprintf(output,  "					case %d: { if (HAS_COMPONENT(%s, instance->entity)) { %s.remove(instance); } } break;\n", i, *component.stem, *component.stem);
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// MIGRATED TO ME //////////////////
fprintf(output,  "void ComponentGroup::migrated_to_me(Instance *instance) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	ComponentList component_list = entity_component_mask_array[instance->entity_id];\n");
fprintf(output,  "\n");
fprintf(output,  "	unsigned count = 0;\n");
fprintf(output,  "	// TODO(kalle): What should the spawn context be in-case of migration??\n");
fprintf(output,  "	Matrix4x4 pose = matrix4x4_identity();\n");
fprintf(output,  "	SpawnContext spawn_context = make_spawn_context(true, pose, 0); \n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				SubCompStruct *sub_comps = component.sub_comps;
				if (HAS_SUB_COMP(SubCompType_Master)) { // We can only do anything with a component that has a master
					if (HAS_SUB_COMP(SubCompType_Slave)) {
fprintf(output,  "					case %d: { %s.migrated_to_me(instance); } break;\n", i, *component.stem);
					} else if (!IS_SET(component, ComponentMask_Slave)) { // Add this as a master
fprintf(output,  "					case %d: { %s.add(instance, spawn_context); } break;\n", i, *component.stem);
					}
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "	count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				SubCompStruct *sub_comps = component.sub_comps;
				if (IS_SET(component, ComponentMask_MigrationEvents)) {
fprintf(output,  "					case %d: { %s.on_migrated_to_me(instance); } break;\n", i, *component.stem);
				} else if (HAS_SUB_COMP(SubCompType_Master) && !IS_SET(component, ComponentMask_Slave)) { // If I only have a master but no slave, the instance will get added when migrating to me
					if (IS_SET(component, ComponentMask_OnAdded)) {
fprintf(output,  "					case %d: { %s.on_added(instance, spawn_context); } break;\n", i, *component.stem);
					}
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// MIGRATED AWAY //////////////////
fprintf(output,  "void ComponentGroup::migrated_away(Instance *instance) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	ComponentList component_list = entity_component_mask_array[instance->entity_id];\n");
fprintf(output,  "\n");
fprintf(output,  "	unsigned count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				SubCompStruct *sub_comps = component.sub_comps;
				if (IS_SET(component, ComponentMask_MigrationEvents)) {
fprintf(output,  "					case %d: { %s.on_migrated_away(instance); } break;\n", i, *component.stem);
				} else if (HAS_SUB_COMP(SubCompType_Master) && !IS_SET(component, ComponentMask_Slave)) { // If I only have a master but no slave, the instance will get removed when migrating away
					if (IS_SET(component, ComponentMask_OnRemoved)) {
fprintf(output,  "					case %d: { %s.on_removed(instance); } break;\n", i, *component.stem);
					}
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "	count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				SubCompStruct *sub_comps = component.sub_comps;
				if (HAS_SUB_COMP(SubCompType_Master) && HAS_SUB_COMP(SubCompType_Slave)) {
fprintf(output,  "					case %d: { %s.migrated_away(instance); } break;\n", i, *component.stem);
				} else if (HAS_SUB_COMP(SubCompType_Master) && !IS_SET(component, ComponentMask_Slave)) { // If I only have a master but no slave, I will get removed when migrating away
fprintf(output,  "					case %d: { %s.remove(instance); } break;\n", i, *component.stem);
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// UPDATE NETWORK STATE //////////////////
fprintf(output,  "void ComponentGroup::update_network_state(Instance *instance, const char *field_buffer, unsigned *offset) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	ComponentList component_list = entity_component_mask_array[instance->entity_id];\n");
fprintf(output,  "\n");
fprintf(output,  "	unsigned count = 0;\n");
fprintf(output,  "	for (unsigned mask_index = 0; mask_index < ARRAY_COUNT(component_list.masks) && count < %u; mask_index++) {\n", sorted_components.count);
fprintf(output,  "		for (unsigned bit_index = 0; bit_index < 64 && count < %u; bit_index++) {\n", sorted_components.count);
fprintf(output,  "			uint64_t mask = component_list.masks[mask_index];\n");
fprintf(output,  "			if (mask & ((uint64_t)1 << bit_index)) {\n");
fprintf(output,  "				switch (mask_index * 64 + bit_index) {\n");
			for (int i = 0; i < sorted_components.count; i++) {
				Component &component = *sorted_components[i];
				SubCompStruct *sub_comps = component.sub_comps;
				if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(SubCompType_Network)) {
fprintf(output,  "					case %d: { %s.update_network_state(instance->entity, field_buffer, offset); } break;\n", i, *component.stem);
				}
			}
fprintf(output,  "				}\n");
fprintf(output,  "			}\n");
fprintf(output,  "\n");
fprintf(output,  "			count++;\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		fclose(output);
	}
}


// Generate the component group
void output_component_group(SortedComponents &sorted_components, ComponentArray &component_array, SettingsArray &settings_array, MemoryArena &arena) {
	{ // Output h file
		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_CODE_FOLDER, MAKE_STRING("component_group.generated"), MAKE_STRING(".h"));
		MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

		// Revert the order when including so that those that aren't dependent on anyone (the leaves of the dependency tree) are inluded first
		for (int i = sorted_components.count - 1; i >= 0; --i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "#include \"components/%s_component.h\"\n", *component.stem);
		}
fprintf(output, "\n");
fprintf(output, "#include \"../entity_settings.generated.h\"\n");
fprintf(output, "\n");
fprintf(output, "class ComponentGroup {\n");
fprintf(output, "	friend struct Reloader;\n");
fprintf(output, "public:\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "	%s %s;\n", *component.name, *component.stem);
		}
fprintf(output, "\n");
fprintf(output, "	void build_settings();\n");
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
fprintf(output, "private:\n");
fprintf(output, "	EventDelegate *event_delegate;\n");
fprintf(output, "};\n");
fprintf(output, "\n");
	}

	{ // Output cpp file
		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_CODE_FOLDER, MAKE_STRING("component_group.generated"), MAKE_STRING(".cpp"));
		MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

		// Revert the order when including so that those that aren't dependent on anyone (the leaves of the dependency tree) are inluded first
		for (int i = sorted_components.count - 1; i >= 0; --i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "#include \"components/%s_component.cpp\"\n", *component.stem);
		}
fprintf(output, "\n");
fprintf(output, "void ComponentGroup::build_settings() {\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components.entries[i];
			SubCompStruct *sub_comps = component.sub_comps;
			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(MASTER)) {
fprintf(output, "	%s::build_master_settings();\n", *component.stem);
			}
			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(MASTER_INPUT)) {
fprintf(output, "	%s::build_master_input_settings();\n", *component.stem);
			}
// 			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(NETWORK)) {
// fprintf(output, "	%s::build_network_settings();\n", *component.stem);
// 			}
			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(SLAVE)) {
fprintf(output, "	%s::build_slave_settings();\n", *component.stem);
			}
			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(STATIC)) {
fprintf(output, "	%s::build_static_settings();\n", *component.stem);
			}
		}
fprintf(output, "}\n\n");
fprintf(output, "void ComponentGroup::init_components(CreationContext &context) {\n");
fprintf(output, "	event_delegate = context.event_delegate;\n");
fprintf(output, "	build_settings();\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "	%s.init(context);\n", *component.stem);
			if (component.num_received_events)
fprintf(output, "	event_delegate->register_%s_component(&%s);\n", *component.stem, *component.stem);
		}
fprintf(output, "}\n\n");
fprintf(output, "void ComponentGroup::deinit_components() {\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "	%s.deinit();\n", *component.stem);
			if (component.num_received_events)
fprintf(output, "	event_delegate->unregister_%s_component(&%s);\n", *component.stem, *component.stem);
		}
fprintf(output, "}\n\n");

fprintf(output, "void ComponentGroup::script_reload() {\n");
fprintf(output, "	build_settings();\n");
fprintf(output, "\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "	%s.script_reload();\n", *component.stem);
		}
fprintf(output, "}\n\n");

fprintf(output, "void ComponentGroup::update(float dt) {\n");
		for (int i = 0; i < sorted_components.count; ++i) {
			Component &component = *sorted_components.entries[i];
fprintf(output, "	{ Profile p(\"%s::update\"); %s.update(dt); }\n", *component.name, *component.stem);
		}
fprintf(output, "}\n\n");

		{ /////////////////// ADD TO COMPONENT //////////////////
fprintf(output,  "void ComponentGroup::add_to_component(Instance *instance, SpawnContext &spawn_context, GameObjectManager *gom) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	AhTempAllocator ta;\n");
fprintf(output,  "\n");
fprintf(output,  "	switch (instance->entity_id.id()) {\n");
			for (int i = 0; i < settings_array.count; ++i) {
				Settings &settings = settings_array.entries[i];

				bool has_any_fields = false;
fprintf(output,  "		case 0x%016llx: { // %s\n", settings.path_id, *settings.path);

				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];

					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
fprintf(output,  "			%s.add(instance, spawn_context);\n", *component.stem);
					if (!has_any_fields && HAS_SUB_COMP(NETWORK)) {
						has_any_fields = true;
					}
				}
fprintf(output,  "\n");
				if (has_any_fields) {
fprintf(output,  "			if (spawn_context.is_master) {\n");
fprintf(output,  "				// Create game object\n");
fprintf(output,  "				GameObjectField default_fields[MAX_GO_FIELDS_PER_COMPONENTS];\n");
fprintf(output,  "				unsigned field_counter = 0;\n");
fprintf(output,  "\n");
					for (int j = 0; j < settings.component_settings_count; ++j) {
						ComponentSettings &component_settings = settings.component_settings[j];
						Component &component = *component_settings.component;
						SubCompStruct *sub_comps = component.sub_comps;
						if (HAS_SUB_COMP(NETWORK)) {
fprintf(output,  "				%s.fill_in_default_go_fields(instance, spawn_context, default_fields, &field_counter, ta);\n", *component.stem);
						}
					}
fprintf(output,  "\n");

					PARSER_ASSERT(settings.game_object, "Found fields for entity but no game object! (entity_path=%s)", *settings.path);
					GameObject &game_object = *settings.game_object;

fprintf(output,  "				Id32 game_object_type = 0x%x; // %s\n", game_object.name_id, *game_object.name);
fprintf(output,  "				instance->go_id = gom->create_game_object(game_object_type, default_fields, field_counter);\n");
fprintf(output,  "			} \n");
fprintf(output,  "\n");
				}
fprintf(output,  "		} break;\n");
			}
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity! Forgot to run the generators? (entity=%%s)\", instance->entity_id.to_string()); \n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n\n");
		}

		{ /////////////////// ON ADDED //////////////////
fprintf(output,  "void ComponentGroup::notify_on_added(Instance *instance, SpawnContext &spawn_context) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	AhTempAllocator ta;\n");
fprintf(output,  "\n");
fprintf(output,  "	switch (instance->entity_id.id()) {\n");
			for (int i = 0; i < settings_array.count; ++i) {
				Settings &settings = settings_array.entries[i];
fprintf(output,  "		case 0x%016llx: { // %s\n", settings.path_id, *settings.path);
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
fprintf(output,  "			%s.on_added(instance, spawn_context);\n", *component.stem);
				}
fprintf(output,  "		} break;\n");
			}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity! Forgot to run the generators? (entity=%%s)\", instance->entity_id.to_string()); \n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// REMOVE FROM COMPONENT //////////////////
fprintf(output,  "void ComponentGroup::remove_from_component(Instance *instance) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	switch (instance->entity_id.id()) {\n");
			for (int i = 0; i < settings_array.count; ++i) {
				Settings &settings = settings_array.entries[i];

fprintf(output,  "		case 0x%016llx: { // %s\n", settings.path_id, *settings.path);
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
fprintf(output,  "			%s.on_removed(instance);\n", *component.stem);
				}
fprintf(output,  "\n");
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
fprintf(output,  "			%s.remove(instance);\n", *component.stem);
				}
fprintf(output,  "		} break;\n");
			}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity! Forgot to run the generators? (entity=%%s)\", instance->entity_id.to_string()); \n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// MIGRATED TO ME //////////////////
fprintf(output,  "void ComponentGroup::migrated_to_me(Instance *instance) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	switch (instance->entity_id.id()) {\n");
			for (int i = 0; i < settings_array.count; ++i) {
				Settings &settings = settings_array.entries[i];

fprintf(output,  "		case 0x%016llx: { // %s\n", settings.path_id, *settings.path);
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "			%s.migrated_to_me(instance);\n", *component.stem);
					}
				}
fprintf(output,  "\n");
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "			%s.on_migrated_to_me(instance);\n", *component.stem);
					}
				}
fprintf(output,  "		} break;\n");
			}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity! Forgot to run the generators? (entity=%%s)\", instance->entity_id.to_string()); \n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// MIGRATED AWAY //////////////////
fprintf(output,  "void ComponentGroup::migrated_away(Instance *instance) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	switch (instance->entity_id.id()) {\n");
			for (int i = 0; i < settings_array.count; ++i) {
				Settings &settings = settings_array.entries[i];

fprintf(output,  "		case 0x%016llx: { // %s\n", settings.path_id, *settings.path);
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "			%s.migrated_away(instance);\n", *component.stem);
					}
				}
fprintf(output,  "\n");
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "			%s.on_migrated_away(instance);\n", *component.stem);
					}
				}
fprintf(output,  "		} break;\n");
			}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity! Forgot to run the generators? (entity=%%s)\", instance->entity_id.to_string()); \n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		{ /////////////////// UPDATE NETWORK STATE //////////////////
fprintf(output,  "void ComponentGroup::update_network_state(Instance *instance, const char *field_buffer, unsigned *offset) {\n");
fprintf(output,  "	ASSERT(instance, \"instance can't be null!\");\n");
fprintf(output,  "	switch (instance->entity_id.id()) {\n");
			for (int i = 0; i < settings_array.count; ++i) {
				Settings &settings = settings_array.entries[i];
fprintf(output,  "		case 0x%016llx: { // %s\n", settings.path_id, *settings.path);
				for (int j = 0; j < settings.component_settings_count; ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP(NETWORK)) {
fprintf(output,  "			%s.update_network_state(instance->entity, field_buffer, offset);\n", *component.stem);
					}
				}
fprintf(output,  "\n");
fprintf(output,  "		} break;\n");
			}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity! Forgot to run the generators? (entity=%%s)\", instance->entity_id.to_string()); \n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
		}

		fclose(output);
	}
}

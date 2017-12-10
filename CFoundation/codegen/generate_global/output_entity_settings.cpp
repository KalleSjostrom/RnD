ComponentSettings *get_component_settings_for(Component *component, Settings &settings) {
	for (int i = 0; i < settings.component_settings_count; ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.component == component) {
			return &component_settings;
		}
	}
	return 0;
}

static unsigned type_id64_id = make_string_id(MAKE_STRING("IdString64"));
static unsigned type_id32_id = make_string_id(MAKE_STRING("IdString32"));
static unsigned type_vector3_id = make_string_id(MAKE_STRING("Vector3"));

struct ValidSettings {
	Settings *settings;
	ComponentSettings *component_settings;
};

// FULKOD(bauer): Quick n dirty function for extracting a float out of a string
unsigned parse_float(const char* str, float& out) {
	char buffer[32] = {};
	unsigned count = 0;
	unsigned step = 0;
	while(str) {
		char c = *str;
		if((c >= '0' && c <= '9') || (c == '-' && count == 0) || (c == '.' && count > 0)) {
			buffer[count++] = c;
		} else if(count > 0) {
			break;
		}

		str++;
		step++;
	}

	ASSERT(count > 0, "Failed to parse float from string: '%s", str);
	out = atof(buffer);
	return step;
}

void set_has_default_settings(SettingsStructArray &settings_struct_array, Member &member) {
	member.has_default_settings = false;

	if (member.default_value.length > 0) {
		member.has_default_settings = true;
	} else if (member.exported_line_counter > 0) {
		// TODO(kalle): Don't reparse for the default string every time this function is called!
		// Do it once and reuse!
		for (int m = 0; m < member.exported_line_counter; m++) {
			String &line = member.exported_lines[m];
			if (starts_with(line, MAKE_STRING("default"))) {
				member.has_default_settings = true;
			}
		}
	}

	if (!member.has_default_settings) {
		SettingsStruct *settings_struct = find_settings_struct(settings_struct_array, member.type_id);
		if (settings_struct)
			member.has_default_settings = true;
	}
}

void output_settings(MemoryArena &arena, FILE *output, FILE *game_strings_output, unsigned member_count, Member *members, SettingsDataStore *settings_data_store, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array) {
	bool printed_one_value = false;
	for (int i = 0; i < member_count; i++) {
		Member &member = members[i];

		// The settings data store contain overrides of the default values. These are basically the content of the _settings.entity files (or the content of a struct)
		// Therefore, try to lookup if we have an override! In case we are outputting the settings for a struct that is _not_ in an _settings.entity file we don't need to check for overrides for each member (in this case settings_data_store is 0)
		settings::DataHashEntry *settings_entry = 0;
		if (settings_data_store) {
			HASH_LOOKUP(entry, settings_data_store->map, ARRAY_COUNT(settings_data_store->map), member.name_id);
			settings_entry = entry;
		}

		bool default_value_found = false;

		SettingsStruct *settings_struct = find_settings_struct(settings_struct_array, member.type_id);
		if (settings_struct) {
			if (printed_one_value)
				fprintf(output, ", ");

			String underscore_case = make_underscore_case(member.type, arena);

			fprintf(output, "make_settings::%s(", *underscore_case);
			SettingsDataStore *child_settings_data_store = 0;
			if (settings_entry && settings_entry->key == member.name_id) {
				child_settings_data_store = settings_entry->value->next;
			}
			output_settings(arena, output, game_strings_output, settings_struct->member_count, settings_struct->members, child_settings_data_store, settings_struct_array, settings_enum_array);
			fprintf(output, ")");
			default_value_found = true;
			printed_one_value = true;
			continue;
		}

		if (!member.has_default_settings)
			continue;

		if (settings_entry && settings_entry->key == member.name_id) {
			String *value = &settings_entry->value->value;
			if (printed_one_value)
				fprintf(output, ", ");

			if (member.type_id == type_id64_id) {
				String id64 = *value;
				if (are_strings_equal(id64, MAKE_STRING("null"))) {
					id64.length = 0;
				}
				fprintf(output, "IdString64(0x%016llx /* %.*s */)", make_string_id64(id64), id64.length, *id64);

				if (id64.length > 0) {
					fprintf(game_strings_output, "ID64(%s, \"%.*s\")\n", *slash_to_underscore(id64, arena), id64.length, *id64);
				}
			} else if (member.type_id == type_id32_id) {
				String id32 = *value;
				id32.length-=2;
				id32.text++;
				fprintf(output, "IdString32(0x%x) /* %.*s */", make_string_id(id32), id32.length, *id32);

				if (id32.length > 0) {
					fprintf(game_strings_output, "ID32(%s, \"%.*s\")\n", *slash_to_underscore(id32, arena), id32.length, *id32);
				}
			} else if(member.type_id == type_vector3_id) {
				float x = 0.0f, y = 0.0f, z = 0.0f;
				const char* text = value->text;
				text += parse_float(text, x);
				text += parse_float(text, y);
				text += parse_float(text, z);

				fprintf(output, "vector3(%f, %f, %f)", x,y,z);
			} else {
				SettingsEnum *settings_enum = find_settings_enum(settings_enum_array, member.type_id);
				if (settings_enum) {
					fprintf(output, "%.*s", value->length-2, value->text + 1); // output 2 less than the full length to avoid outputting the last "
				} else {
					fprintf(output, "%s", value->text);
				}
			}
			default_value_found = true;
		} else {
			if (printed_one_value)
				fprintf(output, ", ");

			if (member.default_value.length > 0) {
				fprintf(output, *member.default_value);
				default_value_found = true;
			} else if (member.exported_line_counter > 0) {
				for (int m = 0; m < member.exported_line_counter; m++) {
					String &line = member.exported_lines[m];
					if (starts_with(line, MAKE_STRING("default"))) {
						char *start = line.text + sizeof("default = ") - 1;

						if (member.type_id == type_id64_id) {
							unsigned length = (line.text + line.length) - start;
							start++; // swallow first "
							length-=2; // removed the enclosing "
							fprintf(output, "IdString64(0x%016llx /* %.*s */)", to_id64(length, start), length, start);
						} else if (member.type_id == type_id32_id) {
							unsigned length = (line.text + line.length) - start;
							start++; // swallow first "
							length-=2; // removed the enclosing "
							fprintf(output, "IdString32(0x%x) /* %.*s */", to_id32(length, start), length, start);
						} else if(member.type_id == type_vector3_id) {
							float x = 0.0f, y = 0.0f, z = 0.0f;
							sscanf(start, "[%f %f %f]", &x, &y, &z);
							fprintf(output, "vector3(%f, %f, %f)", x,y,z);
						} else {
							fprintf(output, "%s", start);
						}

						default_value_found = true;
						break;
					}
				}
			}
		}

		if (default_value_found)
			printed_one_value = true;

		// if (!default_value_found) {
		// 	char *default_value = default_value_for_type(member.type_id);
		// 	if (default_value) {
		// 		fprintf(output, default_value);
		// 	} else {
		// 		fprintf(output, "0");
		// 	}
		// }
	}
}

void output_for_subcomp(FILE *output, FILE *game_strings_output, MemoryArena &arena, Component &component, int sub_comp_index, SettingsArray &settings_array, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, char *name, char *name_lower) {
	SubCompStruct *sub_comps = component.sub_comps;

	if (HAS_SUB_COMP(sub_comp_index)) {
		SubCompStruct &subcomp = sub_comps[sub_comp_index];
		if (subcomp.member_count > 0) {
			MemoryBlockHandle memory_block = begin_block(arena);

			// Set has default settings on all members
			for (int i = 0; i < subcomp.member_count; i++) {
				Member &member = subcomp.members[i];
				set_has_default_settings(settings_struct_array, member);
			}

			ValidSettings *valid_settings = (ValidSettings *)allocate_memory(arena, sizeof(ValidSettings) * settings_array.count);
			unsigned valid_settings_count = 0;

			for (int j = 0; j < settings_array.count; ++j) {
				Settings &settings = settings_array.entries[j];
				ComponentSettings *component_settings = get_component_settings_for(&component, settings);
				if (component_settings) {
					ValidSettings &ve = valid_settings[valid_settings_count++];
					ve.settings = &settings;
					ve.component_settings = component_settings;
				}
			}

			if (valid_settings_count > 0) {
fprintf(output,  "	// %s Settings\n", name);
fprintf(output,  "	%s %ss[%d];\n", name, name_lower, valid_settings_count);
fprintf(output,  "	\n");
fprintf(output,  "	__forceinline %s make_%s(", name, name_lower);
			bool printed_one_value = false;
			for (int j = 0; j < subcomp.member_count; j++) {
				Member &member = subcomp.members[j];
				if (member.has_default_settings) {
					if (printed_one_value)
						fprintf(output,  ", ");
					fprintf(output,  "%s %s%s", *member.type, member.is_pointer?"*":"", *member.name);
					printed_one_value = true;
				}
			}
			fprintf(output,  ") {\n");
fprintf(output,  "		%s obj = {};\n", name);
			for (int j = 0; j < subcomp.member_count; j++) {
				Member &member = subcomp.members[j];
				if (member.has_default_settings) {
fprintf(output,  "		obj.%s = %s;\n", *member.name, *member.name);
				}
			}
fprintf(output,  "		return obj;\n");
fprintf(output,  "	}\n");
fprintf(output,  "\n");
fprintf(output,  "	void build_%s_settings() {\n", name_lower);

			for (int j = 0; j < valid_settings_count; ++j) {
				ValidSettings &ve = valid_settings[j];
				Settings &settings = *ve.settings;
				ComponentSettings *component_settings = ve.component_settings;
fprintf(output,  "		%ss[%d] = make_%s(", name_lower, j, name_lower);

				SubCompStruct &subcomp = sub_comps[sub_comp_index];
				output_settings(arena, output, game_strings_output, subcomp.member_count, subcomp.members, component_settings->settings_data_store, settings_struct_array, settings_enum_array);

				fprintf(output,  "); // %s\n", *settings.path);
			}
fprintf(output,  "	}\n");
fprintf(output,  "\n");
fprintf(output,  "	const %s *get_%s_settings(IdString64 entity_path) {\n", name, name_lower);
fprintf(output,  "		switch (entity_path.id()) {\n");
			for (int j = 0; j < valid_settings_count; ++j) {
				ValidSettings &ve = valid_settings[j];
				Settings &settings = *ve.settings;
fprintf(output,  "			case 0x%016llx: { return %ss + %d; }; break; // %s \n", settings.path_id, name_lower, j, *settings.path);
			}
fprintf(output,  "			default: { return 0; }\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "\n");
			} else {
fprintf(output,  "	void build_%s_settings() { }\n", name_lower);
fprintf(output,  "	const %s *get_%s_settings(IdString64 entity_path) { return 0; }\n", name, name_lower);
			}

			end_block(arena, memory_block);
		}
	}
}

void output_entity_settings(ComponentArray &component_array, SettingsArray &settings_array, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, MemoryArena &arena) {
	{
		char *filepath = "../../"GAME_CODE_DIR"/generated/entity_settings.generated.h";
		MAKE_OUTPUT_FILE_WITH_HEADER(output, filepath);

		char *game_strings_filepath = "../../"GAME_CODE_DIR"/generated/entity_settings.generated.game_strings";
		MAKE_OUTPUT_FILE_WITH_HEADER(game_strings_output, game_strings_filepath);

fprintf(output,  "namespace make_settings {\n");
		for (int i = 0; i < settings_struct_array.count; ++i) {
			SettingsStruct &settings_struct = settings_struct_array.entries[i];

			// Set has defalt settings on all members
			for (int j = 0; j < settings_struct.member_count; j++) {
				Member &member = settings_struct.members[j];
				set_has_default_settings(settings_struct_array, member);
			}

			String underscore_case = make_underscore_case(settings_struct.name, arena);
fprintf(output,  "	__forceinline %s %s(", *settings_struct.name, *underscore_case);
			bool printed_one_value = false;
			for (int j = 0; j < settings_struct.member_count; j++) {
				Member &member = settings_struct.members[j];
				if (member.has_default_settings) {
					if (printed_one_value)
						fprintf(output,  ", ");
					fprintf(output,  "%s %s%s", *member.type, member.is_pointer?"*":"", *member.name);
					printed_one_value = true;
				}
			}
			fprintf(output,  ") {\n");
fprintf(output,  "		%s obj = {};\n", *settings_struct.name);
			for (int j = 0; j < settings_struct.member_count; j++) {
				Member &member = settings_struct.members[j];
				if (member.has_default_settings) {
fprintf(output,  "		obj.%s = %s;\n", *member.name, *member.name);
				}
			}
fprintf(output,  "		return obj;\n");
fprintf(output,  "	}\n");
		}
		fprintf(output,  "}\n");

		for (int i = 0; i < component_array.count; ++i) {
			Component &component = component_array.entries[i];
			SubCompStruct *sub_comps = component.sub_comps;

fprintf(output,  "namespace %s {\n", *component.stem);
			output_for_subcomp(output, game_strings_output, arena, component, MASTER, settings_array, settings_struct_array, settings_enum_array, "Master", "master");
			output_for_subcomp(output, game_strings_output, arena, component, SLAVE, settings_array, settings_struct_array, settings_enum_array, "Slave", "slave");
			output_for_subcomp(output, game_strings_output, arena, component, STATIC, settings_array, settings_struct_array, settings_enum_array, "Static", "static");
			output_for_subcomp(output, game_strings_output, arena, component, MASTER_INPUT, settings_array, settings_struct_array, settings_enum_array, "MasterInput", "master_input");
fprintf(output,  "}\n");
		}

		fclose(output);
		fclose(game_strings_output);
	}
}

ComponentSettings *get_component_settings_for(Component *component, Settings &settings) {
	for (int i = 0; i < array_count(settings.component_settings); ++i) {
		ComponentSettings &component_settings = settings.component_settings[i];
		if (component_settings.component == component) {
			return &component_settings;
		}
	}
	return 0;
}

struct ValidSettings {
	Settings *settings;
	ComponentSettings *component_settings;
	int original_index;
};

void set_has_default_settings(SettingsStructArray &settings_struct_array, Member &member) {
	member.default_settings_type = DefaultSettingsType_None;

	if (member.default_value.length > 0) {
		member.default_settings_type = DefaultSettingsType_Value;
	} else if (member.exported_line_counter > 0) {
		// TODO(kalle): Don't reparse for the default string every time this function is called!
		// Do it once and reuse!
		for (int m = 0; m < member.exported_line_counter; m++) {
			String &line = member.exported_lines[m];
			if (starts_with(line, MAKE_STRING("default"))) {
				member.default_settings_type = DefaultSettingsType_Exported;
			}
		}
	}

	if (member.default_settings_type == DefaultSettingsType_None) {
		SettingsStruct *settings_struct = find_settings_struct(settings_struct_array, member.type_id);
		if (settings_struct)
			member.default_settings_type = DefaultSettingsType_SettingsStruct;
	}
}

// Outputs a member's default value as written in the declaration (component h-file).
void output_settings_string_from_declaration(MemoryArena &arena, FILE *output, Member &member, String string, bool trim_quotations, GameStringArray &game_string_array) {
	if (member.type_id == type_id64_id) {
		unsigned length = string.length;
		char *at = string.text;
		if (trim_quotations) {
			length -= 2;
			at++;
		}
		uint64_t value_id = to_id64(length, at);
		fprintf(output, "IdString64(0x%016llx /* %.*s */)", value_id, length, at);

		if (length > 0) {
			String value = make_string(at, length);
			String key = slash_to_underscore(value, arena);
			uint64_t key_id = make_string_id64(key);
			add_game_string(key, key_id, value, value_id, StringFormat_ID64, game_string_array);
		}
	} else if (member.type_id == type_id32_id) {
		unsigned length = string.length;
		char *at = string.text;
		if (trim_quotations) {
			length -= 2;
			at++;
		}

		unsigned value_id = to_id32(length, at);
		fprintf(output, "IdString32(0x%x) /* %.*s */", value_id, length, at);

		if (length > 0) {
			String value = make_string(at, length);
			uint64_t key_id = make_string_id64(value);
			add_game_string(value, key_id, value, key_id, StringFormat_ID32, game_string_array);
		}
	} else if(member.type_id == type_vector4_id) {
		float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
		sscanf(string.text, "[%f %f %f %f]", &x, &y, &z, &w);
		fprintf(output, "vector4(%f, %f, %f, %f)", x,y,z,w);
	} else if(member.type_id == type_vector3_id) {
		float x = 0.0f, y = 0.0f, z = 0.0f;
		sscanf(string.text, "[%f %f %f]", &x, &y, &z);
		fprintf(output, "vector3(%f, %f, %f)", x,y,z);
	} else if(member.type_id == type_vector2_id) {
		float x = 0.0f, y = 0.0f;
		sscanf(string.text, "[%f %f]", &x, &y);
		fprintf(output, "vector2(%f, %f)", x, y);
	} else {
		fprintf(output, "%.*s", string.length, string.text);
	}
}

// Outputs a member's default value as written in the settings (entity file).
void output_settings_string_from_entity(MemoryArena &arena, FILE *output, Member &member, String string, SettingsEnumArray &settings_enum_array, bool trim_quotations, GameStringArray &game_string_array) {
	if (member.type_id == type_id64_id) {
		unsigned length = string.length;
		char *at = string.text;
		if (are_strings_equal(string, MAKE_STRING("null"))) {
			length = 0;
		}
		uint64_t value_id = to_id64(length, at);
		fprintf(output, "IdString64(0x%016llx /* %.*s */)", value_id, length, at);

		if (length > 0) {
			String value = make_string(at, length);
			String key = slash_to_underscore(value, arena);
			uint64_t key_id = make_string_id64(key);
			add_game_string(key, key_id, value, value_id, StringFormat_ID64, game_string_array);
		}
	} else if (member.type_id == type_id32_id) {
		unsigned length = string.length;
		char *at = string.text;
		if (trim_quotations) {
			length -= 2;
			at++;
		}

		unsigned value_id = to_id32(length, at);
		fprintf(output, "IdString32(0x%x) /* %.*s */", value_id, length, at);

		if (length > 0) {
			String value = make_string(at, length);
			uint64_t key_id = make_string_id64(value);
			add_game_string(value, key_id, value, key_id, StringFormat_ID32, game_string_array);
		}
	} else if(member.type_id == type_vector4_id) {
		trim_stingray_array_inplace(string);
		fprintf(output, "vector4(%s)", string.text);
	} else if(member.type_id == type_vector3_id) {
		trim_stingray_array_inplace(string);
		fprintf(output, "vector3(%s)", string.text);
	} else if(member.type_id == type_vector2_id) {
		trim_stingray_array_inplace(string);
		fprintf(output, "vector2(%s)", string.text);
	} else {
		if (string.text[0] == '"') {
			fprintf(output, "%.*s", string.length-2, string.text + 1); // output 2 less than the full length to avoid outputting the last "
		} else {
			fprintf(output, "%s", string.text);
		}
	}
}

void output_settings(MemoryArena &arena, FILE *output, MemberArray &member_array, SettingsDataStore *settings_data_store, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, GameStringArray &game_string_array);

void output_settings_data(MemoryArena &arena, FILE *output, bool &printed_one_value, Member &member, SettingsData *data, SettingsDataStore *settings_data_store, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, GameStringArray &game_string_array) {
	if (printed_one_value)
		fprintf(output, ", ");

	if (FLAG_SET(member, MemberFlag_IsArray) && ((data && data->next) || !data)) {
		Member child = member;
		CLEAR_FLAG(child, MemberFlag_IsArray);
		bool child_printed_one_value = false;

		fprintf(output, "{ ");
		if (data) {
			SettingsDataStore *array_content = data->next;
			ASSERT(array_content->count <= member.max_count, "Found more entries in entity array than allowed. (count=%u, max=%u)", array_content->count, member.max_count);
			for (int i = 0; i < array_content->count; ++i) {
				output_settings_data(arena, output, child_printed_one_value, child, array_content->entries + i, settings_data_store, settings_struct_array, settings_enum_array, game_string_array);
			}
		} else {
			// TODO(kalle): How do we script in a default number of array entries?
			for (int i = 0; i < 1/* member.default_count */; ++i) {
				output_settings_data(arena, output, child_printed_one_value, child, data, settings_data_store, settings_struct_array, settings_enum_array, game_string_array);
			}
		}
		fprintf(output, " }");
		printed_one_value = true;
		return;
	}

	if (FLAG_SET(member, MemberFlag_IsPointer) && member.max_count == 0) {
		printed_one_value = true;
		fprintf(output, "0");
		return;
	}

	SettingsStruct *settings_struct = find_settings_struct(settings_struct_array, member.type_id);
	if (settings_struct) {
		String underscore_case = make_underscore_case(member.type, arena);

		fprintf(output, "{ ");
			SettingsDataStore *child_settings_data_store = data ? data->next : 0;
			output_settings(arena, output, settings_struct->member_array, child_settings_data_store, settings_struct_array, settings_enum_array, game_string_array);
		fprintf(output, " }");
		printed_one_value = true;
		return;
	}

	if (member.default_settings_type == DefaultSettingsType_None && data == 0) {
		printed_one_value = true;
		fprintf(output, "{}");
		return;
	}

	bool default_value_found = false;
	if (data) {
		String &value = data->value;

		if (FLAG_SET(member, MemberFlag_IsArray)) {
			fputc('{', output);
			parser::Tokenizer tok = parser::make_tokenizer(true, value.text, value.length);

			unsigned array_member_count = 0;
			parser::Token token = parser::next_token(&tok);
			if (token.type != ']') {
				while ((tok.at - value.text) < value.length) {
					ASSERT(array_member_count < member.max_count, "Number of default values is larger than the array count! (array count=%d)", member.max_count);
					array_member_count++;

					output_settings_string_from_entity(arena, output, member, token.string, settings_enum_array, false, game_string_array);
					token = parser::next_token(&tok);
					if (token.type == '\0')
						break;
					fputc(',', output);
				}
			}
			fputc('}', output);
		} else {
			output_settings_string_from_entity(arena, output, member, value, settings_enum_array, true, game_string_array);
		}

		default_value_found = true;
	} else {
		if (member.default_value.length > 0) {
			fprintf(output, *member.default_value);
			default_value_found = true;
		} else if (member.exported_line_counter > 0) {
			for (int m = 0; m < member.exported_line_counter; m++) {
				String &line = member.exported_lines[m];
				if (starts_with(line, MAKE_STRING("default"))) {
					char *start = line.text + sizeof("default = ") - 1;

					if (FLAG_SET(member, MemberFlag_IsArray)) {
						fputc('{', output);
						parser::Tokenizer tok = parser::make_tokenizer(true, start, line.length);
						ASSERT_NEXT_TOKEN_TYPE(tok, '[');

						unsigned array_member_count = 0;

						parser::Token token = parser::next_token(&tok);
						if (token.type != ']') {
							while (true) {
								ASSERT(array_member_count < member.max_count, "Number of default values is larger than the array count! (array count=%d)", member.max_count);
								array_member_count++;

								output_settings_string_from_declaration(arena, output, member, token.string, false, game_string_array);
								token = parser::next_token(&tok);
								if (token.type == ']')
									break;

								ASSERT_TOKEN_TYPE(token, ',');
								token = parser::next_token(&tok);

								fputc(',', output);
							}
						}
						fputc('}', output);
					} else {
						output_settings_string_from_declaration(arena, output, member, make_string(start, (line.text + line.length) - start), true, game_string_array);
					}

					default_value_found = true;
					break;
				}
			}
		}
	}

	if (default_value_found)
		printed_one_value = true;
}

void output_settings(MemoryArena &arena, FILE *output, MemberArray &member_array, SettingsDataStore *settings_data_store, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, GameStringArray &game_string_array) {
	bool printed_one_value = false;
	for (int i = 0; i < array_count(member_array); i++) {
		Member &member = member_array[i];
		set_has_default_settings(settings_struct_array, member);

		// The settings data store contain overrides of the default values. These are basically the content of the _settings.entity files (or the content of a struct)
		// Therefore, try to lookup if we have an override! In case we are outputting the settings for a struct that is _not_ in an _settings.entity file we don't need to check for overrides for each member (in this case settings_data_store is 0)
		SettingsHashEntry *settings_entry = 0;
		if (settings_data_store) {
			HASH_LOOKUP(entry, settings_data_store->map, ARRAY_COUNT(settings_data_store->map), member.name_id);
			settings_entry = entry;
		}
		SettingsData *data = (settings_entry && settings_entry->key == member.name_id) ? (settings_entry->value) : 0;
		output_settings_data(arena, output, printed_one_value, member, data, settings_data_store, settings_struct_array, settings_enum_array, game_string_array);
	}
}

void output_for_subcomp(FILE *output, MemoryArena &arena, Component &component, int sub_comp_index, SettingsArray &settings_array, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, GameStringArray &game_string_array, char *name, char *name_lower) {
	SubCompStruct *sub_comps = component.sub_comps;

	if (HAS_SUB_COMP(sub_comp_index)) {
		SubCompStruct &subcomp = sub_comps[sub_comp_index];
		if (array_count(subcomp.member_array) > 0) {
			MemoryBlockHandle memory_block = begin_block(arena);

			// Set has default settings on all members
			for (int i = 0; i < array_count(subcomp.member_array); i++) {
				Member &member = subcomp.member_array[i];
				set_has_default_settings(settings_struct_array, member);
			}

			ValidSettings *valid_settings = (ValidSettings *)allocate_memory(arena, sizeof(ValidSettings) * array_count(settings_array));
			unsigned valid_settings_count = 0;

			for (int j = 0; j < array_count(settings_array); ++j) {
				Settings &settings = settings_array[j];
				ComponentSettings *component_settings = get_component_settings_for(&component, settings);
				if (component_settings) {
					ValidSettings &ve = valid_settings[valid_settings_count++];
					ve.settings = &settings;
					ve.component_settings = component_settings;
					ve.original_index = j;
				}
			}

			if (valid_settings_count > 0) {
				if (sub_comp_index == SubCompType_Static) {
fprintf(output,  "	Static statics[] = {\n");
					for (int j = 0; j < valid_settings_count; ++j) {
						ValidSettings &ve = valid_settings[j];
						Settings &settings = *ve.settings;
						ComponentSettings *component_settings = ve.component_settings;

						SubCompStruct &subcomp = sub_comps[sub_comp_index];
						fprintf(output, "\t\t{");
						// We only allow stuff that is declared in the Static struct to lookup defaults in the settings entity.
						// This is to avoid name-collisions since we only lookup by name.
						// For instance, imagine we have IdString32 attach_node in static but unsigned attach_node in Master (the latter is looked up using the name)
						// We can't allow attach_node in Master to try and get it's default from the entity.
						SettingsDataStore *settings_data_store = component_settings->settings_data_store;
						output_settings(arena, output, subcomp.member_array, settings_data_store, settings_struct_array, settings_enum_array, game_string_array);
						fprintf(output, "}, ");

						fprintf(output, "\n");
					}
fprintf(output,  "	};\n");
fprintf(output,  "\n");
fprintf(output,  "	const Static *get_static_settings(EntityId entity_id) {\n");
fprintf(output,  "		switch (entity_id) {\n");
					for (int j = 0; j < valid_settings_count; ++j) {
						ValidSettings &ve = valid_settings[j];
						Settings &settings = *ve.settings;
fprintf(output,  "			case %d: { return %ss + %d; }; break; // %s \n", ve.original_index, name_lower, j, *settings.path);
					}
fprintf(output,  "			default: { return 0; }\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "\n");
				} else {
fprintf(output,  "	%s %s = {", name, name_lower);
					ValidSettings &ve = valid_settings[0];
					Settings &settings = *ve.settings;
					ComponentSettings *component_settings = ve.component_settings;

					SubCompStruct &subcomp = sub_comps[sub_comp_index];
					output_settings(arena, output, subcomp.member_array, 0, settings_struct_array, settings_enum_array, game_string_array);
					fprintf(output, "};\n");
fprintf(output,  "\n");
				}
			} else {
				if (sub_comp_index == SubCompType_Static) {
fprintf(output,  "	Static statics = {};\n");
fprintf(output,  "	const Static *get_static_settings(EntityId entity_id) {\n");
fprintf(output,  "		return &statics;\n");
fprintf(output,  "	}\n");
				} else {
fprintf(output,  "	%s %s = {};\n", name, name_lower);
				}
			}

			end_block(arena, memory_block);
		}
	}
}

void output_entity_settings(ComponentArray &component_array, SettingsArray &settings_array, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, MemoryArena &arena) {
	{
		char *filepath = "../../" GAME_CODE_DIR "/generated/entity_settings.generated.h";
		MAKE_OUTPUT_FILE_WITH_HEADER(output, filepath);

		unsigned max_strings_in_settings = 1024 * 4;
		GameStringArray game_string_array;
		array_init(game_string_array, max_strings_in_settings);

		for (int i = 0; i < array_count(component_array); ++i) {
			Component &component = component_array[i];
			SubCompStruct *sub_comps = component.sub_comps;

fprintf(output,  "namespace %s {\n", *component.stem);
			output_for_subcomp(output, arena, component, SubCompType_Master, settings_array, settings_struct_array, settings_enum_array, game_string_array, "Master", "master");
			output_for_subcomp(output, arena, component, SubCompType_Slave, settings_array, settings_struct_array, settings_enum_array, game_string_array, "Slave", "slave");
			output_for_subcomp(output, arena, component, SubCompType_Static, settings_array, settings_struct_array, settings_enum_array, game_string_array, "Static", "static");
			output_for_subcomp(output, arena, component, SubCompType_MasterInput, settings_array, settings_struct_array, settings_enum_array, game_string_array, "MasterInput", "master_input");
fprintf(output,  "}\n");
		}

		for (unsigned i = 0; i < array_count(settings_array); ++i) {
			Settings &settings = settings_array[i];
			String key = slash_to_underscore(settings.path, arena);
			uint64_t key_id = make_string_id64(key);
			add_game_string(key, key_id, settings.path, settings.path_id, StringFormat_ID64, game_string_array);
		}

		GameStringArray unique_game_strings = make_unique(arena, game_string_array);

fprintf(output, "\n");
fprintf(output, "const char* settings_idstring_to_str(IdString32 id) {\n");
fprintf(output, "	switch(id.id()) {\n");
		for (unsigned i = 0; i < array_count(unique_game_strings); ++i) {
			GameString &game_string = unique_game_strings[i];
			if (game_string.format == StringFormat_ID32) {
				ASSERT(game_string.value_id, "Invalid id32 string! (key=%.*s)", game_string.key.length, *game_string.key);
fprintf(output, "		case 0x%llx: { return \"%.*s\"; }\n", game_string.value_id >> 32, game_string.value.length, *game_string.value);
			}
		}
fprintf(output, "		default: { return id.to_string(); }\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

fprintf(output, "\n");
fprintf(output, "const char* settings_idstring_to_str(IdString64 id) {\n");
fprintf(output, "	switch(id.id()) {\n");
		for (unsigned i = 0; i < array_count(unique_game_strings); ++i) {
			GameString &game_string = unique_game_strings[i];
			if (game_string.format == StringFormat_ID64) {
				ASSERT(game_string.value_id, "Invalid id64 string! (key=%.*s)", game_string.key.length, *game_string.key);
fprintf(output, "		case 0x%016llx: { return \"%.*s\"; }\n", game_string.value_id, game_string.value.length, *game_string.value);
			}
		}
fprintf(output, "		default: { return id.to_string(); }\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);

		fclose(output);
	}
}

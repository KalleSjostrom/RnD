/*
* `:array` - A simple immutable collection of values used to store raw buffers such as animation or geometry data.
* `:bool` - A simple boolean value, either `true` or `false`.
* `:dict` - A simple mutable dictionary mapping keys to values.
* `:guid` - A universally unique identifier. Represented as a string in JSON files.
* `:interface` - A virtual data type that specifies a protocol for concrete types to implement.
* `:number` - A simple numeric value. Represented as a double-precision floating-point value in JSON files.
* `:resource` - A reference to another file in the project.
* `:set` - A mutable unordered collection of simple equatable values.
* `:string` - A simple immutable UTF-8 encoded string.
* `:struct` - A complex mutable type that can contain typed fields and implement interfaces.
* `:switch` - A virtual data type that will resolve to one of the specified types after examining the data itself.
* `:value` - A virtual data type that can represent any uncustomized simple type.
* `:vector3 - A Vector3.
*/
inline char *convert_type(String var_type) {
	if (are_strings_equal(MAKE_STRING("float"), var_type)
	  || are_strings_equal(MAKE_STRING("int"), var_type)
	  || are_strings_equal(MAKE_STRING("unsigned"), var_type)
	  || are_strings_equal(MAKE_STRING("double"), var_type)
	  || are_strings_equal(MAKE_STRING("long"), var_type)) {
	 	return ":number";
	} else if (are_strings_equal(MAKE_STRING("bool"), var_type)) {
	 	return ":bool";
	} else if (are_strings_equal(MAKE_STRING("IdString64"), var_type)) {
	 	return ":resource";
	} else if (are_strings_equal(MAKE_STRING("IdString32"), var_type)) {
	 	return ":string";
	} else if (are_strings_equal(MAKE_STRING("Vector3"), var_type)) {
		return "core/types/vector3";
	}

	return 0;
}

// TODO(kalle): Speed this up!
SettingsStruct *find_settings_struct(SettingsStructArray &settings_struct_array, unsigned type_id) {
	for (int i = 0; i < settings_struct_array.count; ++i) {
		SettingsStruct &settings_struct = settings_struct_array.entries[i];
		if (settings_struct.name_id == type_id) {
			return &settings_struct;
		}
	}

	return 0;
}
SettingsEnum *find_settings_enum(SettingsEnumArray &settings_enum_array, unsigned type_id) {
	for (int i = 0; i < settings_enum_array.count; ++i) {
		SettingsEnum &settings_enum = settings_enum_array.entries[i];
		if (settings_enum.name_id == type_id) {
			return &settings_enum;
		}
	}

	return 0;
}

struct ResourceDeclaration {
	String default_value;
	unsigned default_value_index;

	String extension;
	unsigned extension_index;
};

void extract_exported_tag(String &line, String *output_string, char *tag) {
	int start, stop;
	bool found = string_split(line, MAKE_STRING("="), &start, &stop);
	ASSERT(found, "Malformed '%s' export line! (expected 'extension = \"something\"', found='%s')", tag, *line);

	*output_string = make_string(line.text + stop, line.length - stop);
	parser::trim_whitespaces(*output_string);
	null_terminate(*output_string);
}

inline ResourceDeclaration parse_resource_declaration(String *lines, unsigned count) {
	ResourceDeclaration rd = {};
	for (int i = 0; i < count; ++i) {
		String &line = lines[i];
		if (starts_with(line, MAKE_STRING("default"))) {
			extract_exported_tag(line, &rd.default_value, "default");
			rd.default_value_index = i;
		} else if (starts_with(line, MAKE_STRING("extension"))) {
			extract_exported_tag(line, &rd.extension, "extension");
			rd.extension_index = i;
		}
	}
	return rd;
}

void write_indentation(unsigned indentation, FILE *output) {
	for (int i = 0; i < indentation; ++i) {
		fputc('\t', output);
	}
}

void write_fields(MemoryArena &arena, SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, unsigned member_count, Member *members, unsigned indentation, FILE *output) {
	for (int i = 0; i < member_count; i++) {
		Member &member = members[i];
		if (member.exported_line_counter > 0) {
write_indentation(indentation, output);
fprintf(output,  "%s = {\n", *member.name);

			char *converted_type = convert_type(member.type);
			if (converted_type == 0) { // struct type
write_indentation(indentation, output);
				SettingsStruct *settings_struct = find_settings_struct(settings_struct_array, member.type_id);
				if (settings_struct) {
fprintf(output,  "	type = \":struct\"\n");
write_indentation(indentation, output);
fprintf(output,  "	fields = {\n");
					write_fields(arena, settings_struct_array, settings_enum_array, settings_struct->member_count, settings_struct->members, indentation + 2, output);
write_indentation(indentation, output);
fprintf(output,  "	}\n");
				} else {
					SettingsEnum *settings_enum = find_settings_enum(settings_enum_array, member.type_id);
					String lower_case = make_underscore_case(settings_enum->name, arena);
					fprintf(output,  "	type = \"content/generated/types/%s#enum_%s\"\n", *lower_case, *lower_case);
				}
			} else {
write_indentation(indentation, output);
fprintf(output,  "	type = \"%s\"\n", converted_type);
				if (converted_type == ":resource") { // NOTE(kalle): If string pooling is not on, this won't work!
					ResourceDeclaration resource = parse_resource_declaration(member.exported_lines, member.exported_line_counter);
write_indentation(indentation, output);
fprintf(output,  "	extension = %s\n", *resource.extension);
write_indentation(indentation, output);
					if (are_strings_equal(resource.default_value, MAKE_STRING("\"\""))) {
fprintf(output,  "	default = null\n");
					} else {
fprintf(output,  "	default = {\n");
write_indentation(indentation, output);
fprintf(output,  "		\"$resource_name\" = %s\n", *resource.default_value);
write_indentation(indentation, output);
fprintf(output,  "		\"$resource_type\" = %s\n", *resource.extension);
write_indentation(indentation, output);
fprintf(output,  "	}\n");
					}

					for (int j = 0; j < member.exported_line_counter; j++) {
						if (j != resource.default_value_index && j != resource.extension_index) {
write_indentation(indentation, output);
fprintf(output,  "	%s\n", *member.exported_lines[j]);
						}
					}

				} else {
					for (int j = 0; j < member.exported_line_counter; j++) {
write_indentation(indentation, output);
fprintf(output,  "	%s\n", *member.exported_lines[j]);
					}
				}
			}
write_indentation(indentation, output);
fprintf(output,  "}\n");
		}
	}
}

// Generate the stingray data component
void output_data_components(SettingsStructArray &settings_struct_array, SettingsEnumArray &settings_enum_array, ComponentArray &component_array, MemoryArena &arena) {
	{
		for (int i = 0; i < component_array.count; ++i) {
			Component *component = component_array.entries + i;
			String stem = component->stem;

			SubCompStruct *sub_comps = component->sub_comps;

			String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_COMPONENT_FOLDER, stem, GENERATED_COMPONENT_ENDING);
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

fprintf(output,  "export = \"#component\"\n");
fprintf(output,  "types = {\n");
fprintf(output,  "	component = {\n");
fprintf(output,  "		type = \":struct\"\n");
fprintf(output,  "		implements = {\n");
fprintf(output,  "			\"core/types/component\" = true\n");
fprintf(output,  "		}\n");
fprintf(output,  "		fields = {\n");
fprintf(output,  "			name = {\n");
fprintf(output,  "				type = \":string\"\n");
fprintf(output,  "				default = \"%s\"\n", *component->name);
fprintf(output,  "			}\n");
			if (HAS_SUB_COMP(MASTER))
				write_fields(arena, settings_struct_array, settings_enum_array, sub_comps[MASTER].member_count, sub_comps[MASTER].members, 3, output);
			if (HAS_SUB_COMP(SLAVE))
				write_fields(arena, settings_struct_array, settings_enum_array, sub_comps[SLAVE].member_count, sub_comps[SLAVE].members, 3, output);
			if (HAS_SUB_COMP(STATIC))
				write_fields(arena, settings_struct_array, settings_enum_array, sub_comps[STATIC].member_count, sub_comps[STATIC].members, 3, output);
fprintf(output,  "		}\n");
fprintf(output,  "		editor = {\n");
fprintf(output,  "			category = \"%s\"\n", *component->name);
fprintf(output,  "			priority = 1000\n");
fprintf(output,  "			icon = \"%s\"\n", *component->icon);
fprintf(output,  "		}\n");
fprintf(output,  "		metadata = {\n");
fprintf(output,  "			component = \"data\"\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
			fclose(output);
		}
	}

	#pragma region game_entity.component
	{
		// Open output file and insert the standard timestamp
		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_COMPONENT_FOLDER, MAKE_STRING("game_entity"), MAKE_STRING(".component"));
		if (!file_exists(*output_filepath)) {
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

fprintf(output,  "export = \"#component\"\n");
fprintf(output,  "types = {\n");
fprintf(output,  "	component = {\n");
fprintf(output,  "		type = \":struct\"\n");
fprintf(output,  "		implements = {\n");
fprintf(output,  "			\"core/types/component\" = true\n");
fprintf(output,  "		}\n");
fprintf(output,  "		fields = {\n");
fprintf(output,  "			name = {\n");
fprintf(output,  "				type = \":string\"\n");
fprintf(output,  "				default = \"game_entity\"\n");
fprintf(output,  "			}\n");
fprintf(output,  "			settings_entity = {\n");
fprintf(output,  "				type = \":string\"\n");
fprintf(output,  "				default = \"\"\n");
fprintf(output,  "			}\n");
fprintf(output,  "		}\n");
fprintf(output,  "		metadata = {\n");
fprintf(output,  "			component = \"data\"\n");
fprintf(output,  "		}\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
			fclose(output);
		}
	}
	#pragma endregion
}

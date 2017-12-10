// Generate the stingray type
void output_settings_enums(SettingsEnumArray &settings_enum_array, MemoryArena &arena) {
	{
		for (int i = 0; i < settings_enum_array.count; ++i) {
			SettingsEnum &settings_enum = settings_enum_array.entries[i];
			String underscore_name = make_underscore_case(settings_enum.name, arena);

			String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_TYPES_FOLDER, underscore_name, GENERATED_TYPE_ENDING);
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

fprintf(output,  "export = \"#enum_%s\"\n", *underscore_name);
fprintf(output,  "types = {\n");
fprintf(output,  "	enum_%s = {\n", *underscore_name);
fprintf(output,  "		type = \":enum\"\n");
fprintf(output,  "		editor = {\n");
fprintf(output,  "			control = \"Choice\"\n");
fprintf(output,  "			case_labels = {\n");
			for (int j = 0; j < settings_enum.count; ++j) {
				String &member = settings_enum.entries[j];
				char *at = member.text + settings_enum.name.length + 1; // also swallow the '_'
				if (are_cstrings_equal(at, "count")) {
					break;
				}
fprintf(output,  "				\"%s\" = \"%s\"\n", *member, at);
			}
fprintf(output,  "			}\n");
fprintf(output,  "		}\n");
fprintf(output,  "		cases = [\n");
			for (int j = 0; j < settings_enum.count; ++j) {
fprintf(output,  "			\"%s\"\n", *settings_enum.entries[j]);
			}
fprintf(output,  "		]\n");
			if (settings_enum.count > 0) {
fprintf(output,  "		default = \"%s\"\n", *settings_enum.entries[0]);
			}
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
			fclose(output);
		}
	}
}

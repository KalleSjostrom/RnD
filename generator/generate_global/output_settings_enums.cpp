// Generate the stingray type
void output_settings_enums(SettingsEnumArray &settings_enum_array, MemoryArena &arena) {
	{
		for (int i = 0; i < array_count(settings_enum_array); ++i) {
			SettingsEnum &settings_enum = settings_enum_array[i];
			String underscore_name = make_underscore_case(settings_enum.name, arena);

			String output_filepath = make_filepath(arena, _folder_root, _folder_generated_types, underscore_name, MAKE_STRING(".type"));
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

fprintf(output,  "export = \"#enum_%s\"\n", *underscore_name);
fprintf(output,  "types = {\n");
fprintf(output,  "	enum_%s = {\n", *underscore_name);
fprintf(output,  "		type = \":enum\"\n");
fprintf(output,  "		editor = {\n");
fprintf(output,  "			control = \"Choice\"\n");
fprintf(output,  "			case_labels = {\n");
			for (int j = 0; j < array_count(settings_enum.entry_array); ++j) {
				String &member = settings_enum.entry_array[j];
				char *at = member.text + settings_enum.name.length + 1; // also swallow the '_'
				if (are_cstrings_equal(at, "count")) {
					break;
				}
fprintf(output,  "				\"%s\" = \"%s\"\n", *member, at);
			}
fprintf(output,  "			}\n");
fprintf(output,  "		}\n");
fprintf(output,  "		cases = [\n");
			for (int j = 0; j < array_count(settings_enum.entry_array); ++j) {
fprintf(output,  "			\"%s\"\n", *settings_enum.entry_array[j]);
			}
fprintf(output,  "		]\n");
			if (array_count(settings_enum.entry_array) > 0) {
fprintf(output,  "		default = \"%s\"\n", *settings_enum.entry_array[0]);
			}
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
			fclose(output);
		}
	}
}

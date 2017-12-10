static String id32_prefix = MAKE_STRING("");


void output_game_strings(GameStringArray &game_strings) {
	{
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/game_strings.generated.cpp");
#if 0
fprintf(output, "unsigned string_lookup_32(char *string) {\n");
fprintf(output, "	intptr_t string_type = (intptr_t)string;\n");
fprintf(output, "	switch(string_type) {\n");

		for (int i = 0; i < game_strings.count; ++i) {
			GameString &game_string = game_strings.entries[i];
fprintf(output, "		case (intptr_t)\"%s\": { return 0x%x; } break;\n", *game_string.string, game_string.id);
		}

fprintf(output, "	}\n");
fprintf(output, "	return 0;\n");
fprintf(output, "}\n");
fprintf(output, "\n");
#endif
		for (int i = 0; i < game_strings.count; ++i) {
			GameString &game_string = game_strings.entries[i];
			switch (game_string.format) {
				case StringFormat_ID32: {
fprintf(output, "static const IdString32 _id32_%s = IdString32(0x%x); // %s\n", *game_string.key, (unsigned)(game_string.value_id >> 32), *game_string.value);
				} break;
				case StringFormat_ID64: {
fprintf(output, "static const IdString64 _id64_%s = IdString64(0x%016llx); // %s\n", *game_string.key, game_string.value_id, *game_string.value);
				} break;
			}
		}

fprintf(output, "\n");
fprintf(output, "const char* idstring_to_str(IdString32 id) {\n");
fprintf(output, "	switch(id.id()) {\n");
		for (int i = 0; i < game_strings.count; ++i) {
			GameString &game_string = game_strings.entries[i];
			switch (game_string.format) {
				case StringFormat_ID32: {
fprintf(output, "		case 0x%x: { return \"%s\"; }\n", (unsigned)(game_string.value_id >> 32), *game_string.value);
				} break;
			}
		}
fprintf(output, "		default: { return \"\"; }\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

fprintf(output, "\n");
fprintf(output, "const char* idstring_to_str(IdString64 id) {\n");
fprintf(output, "	switch(id.id()) {\n");
		for (int i = 0; i < game_strings.count; ++i) {
			GameString &game_string = game_strings.entries[i];
			switch (game_string.format) {
				case StringFormat_ID64: {
fprintf(output, "		case 0x%016llx: { return \"%s\"; }\n", game_string.value_id, *game_string.value);
				} break;
			}
		}
fprintf(output, "		default: { return \"\"; }\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}
}
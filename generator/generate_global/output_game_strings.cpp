void output_game_strings(GameStringArray &game_strings, MemoryArena &arena) {
	{
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../" GAME_CODE_DIR "/generated/game_strings.generated.cpp");

		GameStringArray unique_game_strings = make_unique(arena, game_strings);

		for (unsigned i = 0; i < array_count(unique_game_strings); ++i) {
			GameString &game_string = unique_game_strings[i];
			switch (game_string.format) {
				case StringFormat_ID32: {
fprintf(output, "static const IdString32 _id32_%s = IdString32(0x%llx); // %s\n", *game_string.key, game_string.value_id >> 32, *game_string.value);
				} break;
				case StringFormat_ID64: {
fprintf(output, "static const IdString64 _id64_%s = IdString64(0x%016llx); // %s\n", *game_string.key, game_string.value_id, *game_string.value);
				} break;
			}
		}

fprintf(output, "\n");
fprintf(output, "const char* idstring_to_str(IdString32 id) {\n");
fprintf(output, "	switch(id.id()) {\n");
		for (unsigned i = 0; i < array_count(unique_game_strings); ++i) {
			GameString &game_string = unique_game_strings[i];
			if (game_string.format == StringFormat_ID32) {
				ASSERT(game_string.value_id, "Invalid id32 string! (key=%.*s)", game_string.key.length, *game_string.key);
fprintf(output, "		case 0x%llx: { return \"%.*s\"; }\n", game_string.value_id >> 32, game_string.value.length, *game_string.value);
			}
		}
fprintf(output, "		default: { return settings_idstring_to_str(id); }\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

fprintf(output, "\n");
fprintf(output, "const char* idstring_to_str(IdString64 id) {\n");
fprintf(output, "	switch(id.id()) {\n");
		for (unsigned i = 0; i < array_count(unique_game_strings); ++i) {
			GameString &game_string = unique_game_strings[i];
			if (game_string.format == StringFormat_ID64) {
				ASSERT(game_string.value_id, "Invalid id64 string! (key=%.*s)", game_string.key.length, *game_string.key);
fprintf(output, "		case 0x%016llx: { return \"%.*s\"; }\n", game_string.value_id, game_string.value.length, *game_string.value);
			}
		}
fprintf(output, "		default: { return settings_idstring_to_str(id); }\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}
}

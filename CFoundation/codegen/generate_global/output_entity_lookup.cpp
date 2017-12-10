
void output_entity_lookup(ComponentArray &component_array, SettingsArray &settings_array, GameObjectArray &go_array, MemoryArena &arena) {
	{
		char* filepath = "../../"GAME_CODE_DIR"/generated/entity_lookup.generated.h";
		MAKE_OUTPUT_FILE_WITH_HEADER(output, filepath);

fprintf(output,  "static const unsigned ENTITY_COUNT = %d;\n", settings_array.count);
fprintf(output,  "\n");
fprintf(output,  "// Maps entity path to game object type. Returns 0 if there is no game object type for this entity path.\n");
fprintf(output,  "// One entity path can _only_ have one game object type.\n");
fprintf(output,  "IdString32 entity_to_game_object_type(IdString64 entity_path) {\n");
fprintf(output,  "	switch (entity_path.id()) {\n");
		for (int i = 0; i < go_array.count; ++i) {
			GameObject &game_object = go_array.entries[i];
			Settings &settings = *game_object.settings;
fprintf(output,  "		case 0x%016llx: { return 0x%x; }; break; // %s -> %s\n", settings.path_id, game_object.name_id, *settings.path, *game_object.name);
		}
fprintf(output,  "		default: {\n");
fprintf(output,  "			return IdString32((uint32_t)0);\n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "// Maps game object type to entity path. Returns 0 if there is no entity path for this game object type.\n");
fprintf(output,  "// A game object can only point to one specific entity file.\n");
fprintf(output,  "IdString64 game_object_index_to_entity(unsigned game_object_index) {\n");
fprintf(output,  "	switch (game_object_index) {\n");
		for (int i = 0; i < go_array.count; ++i) {
			GameObject &game_object = go_array.entries[i];
			Settings &settings = *game_object.settings;
fprintf(output,  "		case %d: { return 0x%016llx; }; break; // %s -> %s\n", i, settings.path_id, *game_object.name, *settings.path);
		}
fprintf(output,  "		default: {\n");
fprintf(output,  "			return IdString64((uint64_t)0);\n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "/////////////////////////////////////\n");
fprintf(output,  "///// COMPACT PATHS FOR NETWORK /////\n");
fprintf(output,  "/////////////////////////////////////\n");
fprintf(output,  "unsigned get_entity_index(IdString64 entity_path) {\n");
fprintf(output,  "	switch (entity_path.id()) {\n");
		for (int i = 0; i < settings_array.count; ++i) {
			Settings &settings = settings_array.entries[i];
fprintf(output,  "		case 0x%016llx: { return %d; }; break; // %s \n", settings.path_id, i, *settings.path);
		}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized entity_path! Forgot to run the generators? (entity_path=%%s)\", entity_path.to_string());\n");
fprintf(output,  "			return UINT_MAX;\n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "IdString64 get_entity_path(unsigned index) {\n");
fprintf(output,  "	switch (index) {\n");
		for (int i = 0; i < settings_array.count; ++i) {
			Settings &settings = settings_array.entries[i];
fprintf(output,  "		case %d: { return 0x%016llx; }; break; // %s \n", i, settings.path_id, *settings.path);
		}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			ASSERT(false, \"Unrecognized index! Forgot to run the generators? (index=%%u)\", index);\n");
fprintf(output,  "			return IdString64(0);\n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
		fclose(output);
	}
}

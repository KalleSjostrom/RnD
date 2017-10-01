
void output_entity_lookup(ComponentArray &component_array, SettingsArray &settings_array, GameObjectArray &go_array, MemoryArena &arena) {
	{
		char* filepath = "../../" GAME_CODE_DIR "/generated/entity_lookup.generated.h";
		MAKE_OUTPUT_FILE_WITH_HEADER(output, filepath);

fprintf(output,  "static const unsigned ENTITY_COUNT = %d;\n", array_count(settings_array));
fprintf(output,  "\n");
fprintf(output,  "// Maps game object type to entity path. Returns 0 if there is no entity path for this game object type.\n");
fprintf(output,  "// A game object can only point to one specific entity file.\n");
fprintf(output,  "IdString64 game_object_index_to_entity_path_array[] = {\n");
		for (int i = 0; i < array_count(go_array); ++i) {
			GameObject &game_object = go_array[i];
			Settings &settings = *game_object.settings;
fprintf(output,  "	0x%016llx, // %s -> %s\n", settings.path_id, *game_object.name, *settings.path);
		}
fprintf(output,  "};\n");
fprintf(output,  "IdString64 game_object_index_to_entity_path(unsigned game_object_index) {\n");
fprintf(output,  "	ASSERT(game_object_index < ARRAY_COUNT(game_object_index_to_entity_path_array), \"Index out of bounds!\");\n");
fprintf(output,  "	return game_object_index_to_entity_path_array[game_object_index];\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "// Maps entity id to game object type. Returns 0xffffffff if there is no game object type for this entity id.\n");
fprintf(output,  "// One entity id can _only_ have one game object type.\n");
fprintf(output,  "IdString32 entity_id_to_game_object_type_array[] = {\n");
		for (int i = 0; i < array_count(settings_array); ++i) {
			Settings &settings = settings_array[i];
			if (settings.game_object) {
fprintf(output,  "	0x%x, // %s -> %s\n", settings.game_object->name_id, *settings.path, *settings.game_object->name);
			} else {
fprintf(output,  "	0xffffffff, // %s -> No game object\n", *settings.path);
			}
		}
fprintf(output,  "};\n");
fprintf(output,  "IdString32 entity_id_to_game_object_type(EntityId entity_id) {\n");
fprintf(output,  "	ASSERT(entity_id < ARRAY_COUNT(entity_id_to_game_object_type_array), \"Index out of bounds!\");\n");
fprintf(output,  "	return entity_id_to_game_object_type_array[entity_id];\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "/////////////////////////////////////\n");
fprintf(output,  "///// COMPACT PATHS FOR NETWORK /////\n");
fprintf(output,  "/////////////////////////////////////\n");
fprintf(output,  "EntityId try_get_entity_id(IdString64 entity_path) {\n");
fprintf(output,  "	switch (entity_path.id()) {\n");
		for (int i = 0; i < array_count(settings_array); ++i) {
			Settings &settings = settings_array[i];
fprintf(output,  "		case 0x%016llx: { return %d; }; break; // %s \n", settings.path_id, i, *settings.path);
		}
fprintf(output,  "\n");
fprintf(output,  "		default: {\n");
fprintf(output,  "			return INVALID_ENTITY_ID;\n");
fprintf(output,  "		};\n");
fprintf(output,  "	}\n");
fprintf(output,  "}\n");
fprintf(output,  "EntityId get_entity_id(IdString64 entity_path) {\n");
fprintf(output,  "	EntityId entity_id = try_get_entity_id(entity_path);\n");
fprintf(output,  "	ASSERT(entity_id != INVALID_ENTITY_ID, \"Unrecognized entity_path! Forgot to run the generators? (entity_path=%%s)\", entity_path.to_string());\n");
fprintf(output,  "	return entity_id;\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "IdString64 entity_paths[] = {\n");
		for (int i = 0; i < array_count(settings_array); ++i) {
			Settings &settings = settings_array[i];
fprintf(output,  "	0x%016llx, // %s \n", settings.path_id, *settings.path);
		}
fprintf(output,  "};\n");
fprintf(output,  "IdString64 get_entity_path(EntityId entity_id) {\n");
fprintf(output,  "	ASSERT(entity_id < ARRAY_COUNT(entity_paths), \"Invalid entity id!\");\n");
fprintf(output,  "	return entity_paths[entity_id];\n");
fprintf(output,  "}\n");
fprintf(output,  "IdString64 try_get_entity_path(EntityId entity_id) {\n");
fprintf(output,  "	if (entity_id < ARRAY_COUNT(entity_paths))\n");
fprintf(output,  "		return entity_paths[entity_id];\n");
fprintf(output,  "	return IdString64(0);\n");
fprintf(output,  "}\n");
fprintf(output,  "\n");
fprintf(output,  "#define ENTITY_ID_STR(entity_id) ID64_STR(get_entity_path(entity_id))\n");
		fclose(output);
	}
}

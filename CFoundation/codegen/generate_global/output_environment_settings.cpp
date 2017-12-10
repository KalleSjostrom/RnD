struct PascalString {
	String string;
	String string_pc;
};

struct StringSetEntry {
	unsigned key;
	unsigned value;
};

struct Tiles {
	unsigned count;
	unsigned indices[64];
};

inline bool is_array_whitespace(char p) {
	return p == '\n' || p == '\r' || p == '\t' || p == ' ' || p == ',';
}

void trim_stingray_array_inplace(String &string) {
	int source_cursor = 0;
	int dest_cursor = 0;
	int element_counter = 0;
	bool found_element = false;

	for (; source_cursor < string.length; source_cursor++) {
		if (!found_element && !is_array_whitespace(string[source_cursor])) {
			if (element_counter > 0) {
				string.text[dest_cursor++] = ',';
				string.text[dest_cursor++] = ' ';
			}
			found_element = true;
		}

		if (found_element) {
			if (is_array_whitespace(string[source_cursor])) {
				found_element = false;
			} else {
				string.text[dest_cursor++] = string.text[source_cursor];
				++element_counter;
			}
		}
	}
	string.length = dest_cursor;
	null_terminate(string);
}
inline String make_levelpath_from_tilepath(MemoryArena &arena, String &tilepath) {
	String levelpath = clone_string(tilepath, arena);
	levelpath.length -= sizeof("_tile")-1;
	null_terminate(levelpath);
	return levelpath;
}

SettingsDataStore *get_root_settings_store(Settings &settings) {
	ASSERT(settings.component_settings_count == 1, "Environment settings can only have one component. (path=%s)", *settings.path);
	ComponentSettings &component_settings = settings.component_settings[0];

	ASSERT(component_settings.settings_data_store, "Corrupt environment_settings! (path=%s)", *settings.path);
	return component_settings.settings_data_store;
}

SettingsDataStore *get_settings_store(SettingsDataStore *parent, unsigned key, String &debug_key, String &debug_path) {
	HASH_LOOKUP_STATIC(entry, parent->map, key);
	ASSERT(entry->key == key, "No such entry found in settings! (key=%s, path=%s)", *debug_key, *debug_path);
	SettingsData &settings_data = *entry->value;

	ASSERT(settings_data.next, "Invalid entry structure found in environment settings! (key=%s, path=%s)", *debug_key, *debug_path);
	return settings_data.next;
}

void fill_pascal_string(MemoryArena &arena, PascalString &pascal_string, String string) {
	pascal_string.string = string;
	pascal_string.string_pc = allocate_string(arena, pascal_string.string.length + 1);
	to_pascal_case(pascal_string.string_pc, pascal_string.string);
	null_terminate(pascal_string.string_pc);
}

// x, y values will be 0 if failed to parse
void parse_zone_size(String zone_size, int *x, int *y) {
	long int _x = strtol(zone_size.text, 0, 10);
	char *p = strchr(zone_size.text, ' ');
	long int _y = strtol(p, 0, 10);
	*x = (int)_x;
	*y = (int)_y;
}

// zone_size format ex: 10, 10
void write_zone_matrix(FILE *output, String zone_matrix, String zone_size, PascalString &zone_name, unsigned indent){
	int w, h;
	parse_zone_size(zone_size, &w, &h);
	ASSERT(w > 0 && h > 0, "Unexpected zone size, (d)%d<0 || (h)%d<0. Parsed string: %s", w, h, *zone_size);

fprintf(output, "				ASSERT(%d * %d <= max_tile_total_size, \"%s tile is too large for the currently allowed total tile size of %%d\", max_tile_total_size);\n", w, h, *zone_name.string_pc);

	write_indentation(indent, output);


	trim_stingray_array_inplace(zone_matrix);

	// for (int row = 0; row < y; ++row) {
	// 	for (int col = 0; col < x; ++col) {

	int index = 0;
	for (unsigned cursor = 0; cursor < zone_matrix.length; cursor++) {
		if (index >= (w * h))
			break;
		switch (zone_matrix[cursor]) {
			case '0' : { // Empty
				fprintf(output, "zones[%d] = Zone_None; ", index++);
				if (index % 4 == 0 && cursor != zone_matrix.length-1) { fprintf(output, "\n"); write_indentation(indent, output); }
			} break;
			case '1' : { // Myself
				fprintf(output, "zones[%d] = Zone_%s; ", index++, *zone_name.string_pc);
				if (index % 4 == 0 && cursor != zone_matrix.length-1) { fprintf(output, "\n"); write_indentation(indent, output); }
			} break;
			case '2' : { // Other
				fprintf(output, "zones[%d] = Zone_Other; ", index++);
				if (index % 4 == 0 && cursor != zone_matrix.length-1) { fprintf(output, "\n"); write_indentation(indent, output); }
			} break;
			case ' ': {} break; // Ignore
			case ',': {} break; // Ignore
			default : {
				ASSERT(false, "Invalid number in zone matrix, needs to be 0 (empty), 1 (my zone), or 2 (zone to transition to) (number=%c, )", zone_matrix[cursor]);
			}
		}
	}
	fprintf(output, "\n");
}

String get_size_from_store(SettingsDataStore *data_store) {
	HASH_LOOKUP_STATIC(entry, data_store->map, size_id32);
	String size = entry->value->value;
	trim_stingray_array_inplace(size);
	return size;
}

struct SpawnOrderEntry {
	unsigned zone_index;
	unsigned zone_name_id;
};
SpawnOrderEntry make_spawn_order_entry(unsigned zone_index, unsigned zone_name_id) {
	SpawnOrderEntry result = { zone_index, zone_name_id };
	return result;
}

LINK_ENTRY(SpawnOrderLink, SpawnOrderEntry);
LINKED_LIST(SpawnOrderList, SpawnOrderLink);

void output_environment_settings(SettingsArray &environment_settings_array, SettingsArray &tile_settings_array, MemoryArena &arena) {
	{ // Coalese the parsed data
		StringSetEntry string_set_zones[64] = {};
		unsigned zone_count = 0;
		PascalString zones[64];

		StringSetEntry string_set_fill_weights[64] = {};
		unsigned fill_weights_count = 0;
		PascalString fill_weights[64];

		// Zone tiles maps a zone to a list of tile indices.
		unsigned zone_tile_count = 0;
		Tiles zone_tiles[128];

		for (int i = 0; i < environment_settings_array.count; ++i) {
			Settings &environment_settings = environment_settings_array.entries[i];
			SettingsDataStore *environment_data = get_root_settings_store(environment_settings);

			{
				SettingsDataStore *content = get_settings_store(environment_data, connects_to_id32, connects_to_string, environment_settings.path);

				for (int j = 0; j < content->count; ++j) {
					SettingsData &zone_setting = content->entries[j];
					ARRAY_CHECK_BOUNDS_COUNT(zone_tiles, zone_tile_count);
					Tiles &tiles = zone_tiles[zone_tile_count++];
					tiles.count = 0;

					HASH_LOOKUP_STATIC(entry, string_set_zones, zone_setting.name_id);
					if (entry->key == 0) { // new one
						entry->key = zone_setting.name_id;
						entry->value = zone_count;

						fill_pascal_string(arena, zones[zone_count++], zone_setting.name);
					}

					// FULKOD(kalle): Linear search over all the tiles for all zones.
					// Extract the tiles that corresponds to this zone.
					for (int k = 0; k < tile_settings_array.count; ++k) {
						Settings &settings = tile_settings_array.entries[k];
						ComponentSettings &component_settings = settings.component_settings[0];
						SettingsDataStore &settings_data_store = *component_settings.settings_data_store;

						HASH_LOOKUP_STATIC(entry, settings_data_store.map, zone_id32);
						if (entry->key == zone_id32) {
							if (are_strings_equal(trim_quotes(entry->value->value), zone_setting.name)) {
								tiles.indices[tiles.count++] = k;
							}
						}
					}
				}
			}

			{
				SettingsDataStore *content = get_settings_store(environment_data, fill_weights_id32, fill_weights_string, environment_settings.path);
				for (int j = 0; j < content->count; ++j) {
					SettingsData &setting = content->entries[j];
					HASH_LOOKUP_STATIC(entry, string_set_fill_weights, setting.name_id);
					if (entry->key == 0) { // new one
						entry->key = setting.name_id;
						entry->value = fill_weights_count;

						fill_pascal_string(arena, fill_weights[fill_weights_count++], setting.name);
					}
				}
			}
		}

		{
			String output_hfilepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, MAKE_STRING("environment_settings"), MAKE_STRING(".generated.h"));
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_hfilepath);

fprintf(output, "enum Zone {\n");
			for (unsigned i = 0; i < zone_count; ++i) {
				PascalString &zone = zones[i];
fprintf(output, "	Zone_%s,\n", *zone.string_pc);
			}
fprintf(output, "\n");
fprintf(output, "	Zone_Count, // This is how many actual valid zones there are, excluding None and Other which are special markers.\n");
fprintf(output, "\n");
fprintf(output, "	Zone_None,\n");
fprintf(output, "	Zone_Other,\n");
fprintf(output, "};\n");
fprintf(output, "\n");
fprintf(output, "enum FillTag {\n");
fprintf(output, "	FillTag_None,\n");
fprintf(output, "\n");
			for (unsigned i = 0; i < fill_weights_count; ++i) {
				PascalString &fill_weight = fill_weights[i];

fprintf(output, "	FillTag_%s,\n", *fill_weight.string_pc);
			}
fprintf(output, "};\n");
fprintf(output, "\n");

			fclose(output);
		}

		{
			String output_cppfilepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, MAKE_STRING("environment_settings"), MAKE_STRING(".generated.cpp"));
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_cppfilepath);

			String output_game_strings_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, MAKE_STRING("environment_settings"), MAKE_STRING(".generated.game_strings"));
			MAKE_OUTPUT_FILE_WITH_HEADER(game_strings_output, *output_game_strings_filepath);

			unsigned zone_tile_index = 0;
			for (unsigned i = 0; i < environment_settings_array.count; ++i) {
				Settings &environment_settings = environment_settings_array.entries[i];
				SettingsDataStore *environment_data = get_root_settings_store(environment_settings);

				String filename = make_filename(arena, environment_settings.path);
fprintf(output, "void load_%s(ZoneInfo *zones, Zone *spawn_order) {\n", *filename);
fprintf(output, "	ZERO_STRUCTS(*zones, Zone_Count);\n");
fprintf(output, "\n");

				SettingsDataStore *content = get_settings_store(environment_data, connects_to_id32, connects_to_string, environment_settings.path);

				unsigned spawn_order_count = 0;
				SpawnOrderLink spawn_order_store[128];

				SpawnOrderList spawn_order_list = {};
				spawn_order_list.entries = spawn_order_store;
				spawn_order_list.head = spawn_order_list.entries; spawn_order_list.tail = spawn_order_list.entries;

				for (unsigned j = 0; j < content->count; ++j) {
					SettingsData &zone_setting = content->entries[j];
					Tiles &tiles = zone_tiles[zone_tile_index++];

					HASH_LOOKUP_STATIC(entry, string_set_zones, zone_setting.name_id);
					PascalString &zone_name = zones[entry->value];

					// Get new link to represent the current zone.
					SpawnOrderEntry spawn_order_entry = make_spawn_order_entry(entry->value, zone_setting.name_id);
					LINK_NEW(new_link, spawn_order_entry, spawn_order_list);

					// Setup the zone info and connects_to list
fprintf(output, "	{ //// %s\n", *zone_name.string_pc);
fprintf(output, "		ZoneInfo &zone_info = zones[Zone_%s];\n", *zone_name.string_pc);
					SettingsDataStore &connects_to_list = *zone_setting.next;
					for (unsigned k = 0; k < connects_to_list.count; ++k) {
						SettingsData &connects_to = connects_to_list.entries[k];

						HASH_LOOKUP_STATIC(entry, string_set_zones, connects_to.name_id);
						ASSERT(entry->key == connects_to.name_id, "'%s' connects to unknown zone '%s' (path=%s)", *zone_setting.name, *connects_to.name, *environment_settings.path);
						PascalString &zone_name = zones[entry->value];

						if (are_strings_equal(connects_to.value, true_string)) {
fprintf(output, "		zone_info.connects_to[Zone_%s] = true;\n", *zone_name.string_pc);
							// I have to come before whatever is in connects_to
							// loop over and see if we should be inserted before the current connects_to entry
							for (SpawnOrderLink *cursor = spawn_order_list.head; cursor; cursor = cursor->next) {
								if (cursor->key.zone_name_id == zone_setting.name_id) { // Found myself
									break;
								}
								if (cursor->key.zone_name_id == connects_to.name_id) { // Found the connect_to
									LINK_ADD_BEFORE(new_link, cursor, spawn_order_list);
									break;
								}
							}
						}
					}

					// Handle the fills
					SettingsDataStore *fill_content = get_settings_store(environment_data, fill_weights_id32, fill_weights_string, environment_settings.path);
					for (unsigned k = 0; k < fill_content->count; ++k) {
						SettingsData &fill_setting = fill_content->entries[k];

						HASH_LOOKUP_STATIC(entry, string_set_fill_weights, fill_setting.name_id);
						PascalString &filltag_name = fill_weights[entry->value];

						bool found_any_fill_tiles = false;

						for (unsigned m = 0; m < tiles.count; ++m) {
							Settings &tile_settings = tile_settings_array.entries[tiles.indices[m]];
							SettingsDataStore *tile_settings_data_store = get_root_settings_store(tile_settings);

							HASH_LOOKUP_STATIC(entry, tile_settings_data_store->map, fill_tag_id32);
							if (entry->key == fill_tag_id32) {
								if (are_strings_equal(fill_setting.name, trim_quotes(entry->value->value))) {
									if (!found_any_fill_tiles) {
fprintf(output, "\n");
fprintf(output, "		{\n");
fprintf(output, "			TileCategory &fill = zone_info.categories[TileCategoryType_Fill];\n");
fprintf(output, "			TileGroup &fill_group = fill.groups[fill.count++];\n");
fprintf(output, "			fill_group.tag = FillTag_%s;\n", *filltag_name.string_pc);
fprintf(output, "			fill_group.weight = %s;\n\n", *fill_setting.value);
										found_any_fill_tiles = true;
									}


fprintf(output, "			{\n");
									String size = get_size_from_store(tile_settings_data_store);
									char *allow_extraction = "false"; { // Lookup allow_extraction
										HASH_LOOKUP_STATIC(entry, tile_settings_data_store->map, allow_extraction_id32);
										if (entry->key == allow_extraction_id32) {
											allow_extraction = entry->value->value.text;
										}
									}

									String level_path = make_levelpath_from_tilepath(arena, tile_settings.path);
									fprintf(game_strings_output, "ID64(%s, \"%s\")\n", *slash_to_underscore(level_path, arena), *level_path);

fprintf(output, "				// %s\n", *level_path);
fprintf(output, "				fill_group.tiles[fill_group.count] = make_tile_info(IdString64(0x%016llx), %s, %s);\n", make_string_id64(level_path), *size, allow_extraction);

									{ // Lookup and write zone_matrix
										HASH_LOOKUP_STATIC(entry, tile_settings_data_store->map, zone_matrix_id32);
										if (entry->key == zone_matrix_id32) {
fprintf(output, "\n");
fprintf(output, "				fill_group.tiles[fill_group.count].has_zones = true;\n");
fprintf(output, "				Zone *zones = fill_group.tiles[fill_group.count].zones;\n");

											String zone_matrix = entry->value->value;
											write_zone_matrix(output, zone_matrix, size, zone_name, 4);
										}
									}
fprintf(output, "				fill_group.count++;\n");
fprintf(output, "			}\n");
								}
							}
						}
						if (found_any_fill_tiles) {
fprintf(output, "		}\n");
						}
					}

					// Handle the transitions (convex, concave, straight and custom)
					for (unsigned m = 0; m < tiles.count; ++m) {
						Settings &tile_settings = tile_settings_array.entries[tiles.indices[m]];
						SettingsDataStore *tile_settings_data_store = get_root_settings_store(tile_settings);

						HASH_LOOKUP_STATIC(entry, tile_settings_data_store->map, transition_type_id32);
						if (entry->key == transition_type_id32) {
							SettingsData &data = *entry->value;
							String size = get_size_from_store(tile_settings_data_store);
							PascalString transition_type; {
								fill_pascal_string(arena, transition_type, trim_quotes(data.value));
							}
fprintf(output, "		{\n");
fprintf(output, "			TileTransition &transition = zone_info.transitions[TileTransitionType_%s];\n", *transition_type.string_pc);
fprintf(output, "			TileGroup &transition_group = transition.groups[Zone_%s];\n", *zone_name.string_pc);

							String level_path = make_levelpath_from_tilepath(arena, tile_settings.path);
							fprintf(game_strings_output, "ID64(%s, \"%s\")\n", *slash_to_underscore(level_path, arena), *level_path);

fprintf(output, "			// %s\n", *level_path);
fprintf(output, "			transition_group.tiles[transition_group.count] = make_tile_info(IdString64(0x%016llx), %s);\n", make_string_id64(level_path), *size);

							{ // Lookup and write zone_matrix
								HASH_LOOKUP_STATIC(entry, tile_settings_data_store->map, zone_matrix_id32);
								if (entry->key == zone_matrix_id32) {
fprintf(output, "\n");
fprintf(output, "			transition_group.tiles[transition_group.count].has_zones = true;\n");
fprintf(output, "			Zone *zones = transition_group.tiles[transition_group.count].zones;\n");

									String zone_matrix = entry->value->value;
									write_zone_matrix(output, zone_matrix, size, zone_name, 3);
								}
							}
fprintf(output, "			transition_group.count++;\n");
fprintf(output, "		}\n");
						}
					}
fprintf(output, "	}\n");
				}
				unsigned index_counter = 0;
				for (SpawnOrderLink *cursor = spawn_order_list.head; cursor; cursor = cursor->next) {
fprintf(output, "	spawn_order[%d] = Zone_%s;\n", index_counter++, *zones[cursor->key.zone_index].string_pc);
				}
fprintf(output, "	spawn_order[%d] = Zone_None;\n", index_counter++);
fprintf(output, "}\n");
			}

			fclose(output);
			fclose(game_strings_output);
		}
	}
}

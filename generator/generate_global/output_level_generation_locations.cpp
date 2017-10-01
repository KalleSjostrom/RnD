enum LocationFlags {
	LOCATION_FLAG_NONE = 0,
	LOCATION_FLAG_INSIDE_SAME_REGION = 1,
	LOCATION_FLAG_BLOCKS_ROAD = 2,
	LOCATION_FLAG_DELETE_UPON_ROAD = 4,
};

enum FactionSettingTypes {
	FACTION_SETTING_UNSIGNED = 0,
};

// must match with enum
const char* FactionTypeToString[] = {
	"unsigned",
};

struct LevelGenerationFactionDeclaration {
	const char* name;
	unsigned id;
};

LevelGenerationFactionDeclaration faction_types[] = {
	{ "bugs", STATIC_ID32("bugs") },
	{ "cyborgs", STATIC_ID32("cyborgs") },
	{ "enlightened", STATIC_ID32("enlightened") },
};

struct FactionSetting {
	const char *name;
	unsigned id;
	FactionSettingTypes type;
	String default_value;
};

	// name id type default_value.
FactionSetting faction_settings[] = {
	{ "patrolling", STATIC_ID32("patrolling"), FACTION_SETTING_UNSIGNED, MAKE_STRING("0") },
	{ "guarding", STATIC_ID32("guarding"), FACTION_SETTING_UNSIGNED, MAKE_STRING("0") },
};

String parse_faction_setting(SettingsData *faction_data, unsigned variable_index) {
	HASH_LOOKUP_STATIC(variable_entry, faction_data->next->map, faction_settings[variable_index].id);
	if(variable_entry->value) {
		String value = variable_entry->value->value;
		while(is_array_whitespace(value.text[value.length-1])) value.length--;
		return value;
	} else {
		return faction_settings[variable_index].default_value;
	}
}

void output_locations_data(MemoryArena &arena, SettingsArray &level_locations_settings_array, LevelGenerationParseContext &context) {
 	String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_locations"), MAKE_STRING(".generated.cpp"));
 	String satellites_output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_satellites"), MAKE_STRING(".generated.cpp"));
	MAKE_OUTPUT_FILE(output, *output_filepath);
	MAKE_OUTPUT_FILE(satellite_output, *satellites_output_filepath);
	fprintf(output, "//generated file \n\n");
	fprintf(output, "enum LocationFlags {\n");
	fprintf(output, "\tLOCATION_FLAG_NONE = %d,\n", LOCATION_FLAG_NONE);
	fprintf(output, "\tLOCATION_FLAG_INSIDE_SAME_REGION = %d,\n", LOCATION_FLAG_INSIDE_SAME_REGION);
	fprintf(output, "\tLOCATION_FLAG_BLOCKS_ROAD = %d,\n", LOCATION_FLAG_BLOCKS_ROAD);
	fprintf(output, "\tLOCATION_FLAG_DELETE_UPON_ROAD = %d,\n", LOCATION_FLAG_DELETE_UPON_ROAD);
	fprintf(output, "};\n\n");
	fprintf(output, "struct LevelGenerationLocationData {\n");
	fprintf(output, "\tStampInfo stamps[%d];\n", MAX_NUM_STAMPS_PER_LOCATION);
	fprintf(output, "\tfloat radius;\n");
	fprintf(output, "\tfloat padding;\n");
	fprintf(output, "\tfloat total_stamp_weights;\n");
	fprintf(output, "\tunsigned num_stamps;\n");
	fprintf(output, "\tunsigned flags;\n");
	fprintf(output, "\tint satellites_index;\n");
	fprintf(output, "};\n\n");
	fprintf(output, "struct LevelGenerationAffiliationData {\n");
	fprintf(output, "\tunsigned region_affiliation_index;\n");
	fprintf(output, "\tunsigned region_affiliation_count;\n");
	fprintf(output, "\tunsigned faction_affiliation;\n");
	fprintf(output, "};\n\n");
	fprintf(satellite_output, "struct SatelliteSettings {\n");
	fprintf(satellite_output, "\tLevelGenerationLocationType type;\n");
	fprintf(satellite_output, "\tfloat minimum_distance;\n");
	fprintf(satellite_output, "\tfloat maximum_distance;\n");
	fprintf(satellite_output, "\tunsigned maximum_count;\n");
	fprintf(satellite_output, "};\n\n");
	fprintf(satellite_output, "struct LocationInstanceSatellitesRoadTypeData {\n");
	fprintf(satellite_output, "\tunsigned entries_count[8];\n");
	fprintf(satellite_output, "};\n\n");
	fprintf(satellite_output, "struct LocationInstanceSatellitesData {\n");
	fprintf(satellite_output, "\tLocationInstanceSatellitesRoadTypeData road_exits[RoadType_NumRoadTypes];\n");
	fprintf(satellite_output, "};\n\n");
	fprintf(satellite_output, "struct SatelliteList {\n");
	fprintf(satellite_output, "\tSatelliteSettings entries[8];\n");
	fprintf(satellite_output, "\tunsigned count;\n");
	fprintf(satellite_output, "};\n\n");
	fprintf(satellite_output, "struct SatellitesList {\n");
	fprintf(satellite_output, "\tSatelliteList road_exits[RoadType_NumRoadTypes];\n");
	fprintf(satellite_output, "};\n\n");

	fprintf(satellite_output, "void level_generation_fill_satellite_lists(SatellitesList *satellite_data) {\n");
	int total_satellites_count = 0;

	fprintf(output, "void level_generation_fill_locations_data(LevelGenerationLocationData *locations_data) {\n");
	fprintf(output, "\tlocations_data[0] = {};\n");
	unsigned location_index = 1;
	// each file
	for (int location_type_index = 0; location_type_index < array_count(level_locations_settings_array); ++location_type_index) {
		Settings &location_type_settings = level_locations_settings_array[location_type_index];
		String filename = get_filename(location_type_settings.path);
		fprintf(output, "//%s\n", location_type_settings.path.text);
		ComponentSettings& component_settings = location_type_settings.component_settings[0];
		SettingsDataStore *location_data_store = component_settings.settings_data_store;
		// each group in file
		for(unsigned group_index = 0; group_index < location_data_store->entries[0].next->count; ++group_index) {
			SettingsData *group = &location_data_store->entries[0].next->entries[group_index];
			fprintf(output, "\tlocations_data[%d] = { // %s\n", location_index++, group->name.text);
			fprintf(output, "\t\t{ // stamps\n");
			// each group location in group
			for(unsigned group_location_index = 0; group_location_index < group->next->count; ++group_location_index) {
				float total_stamp_weigths = 0.f;
				SettingsData *group_location = &group->next->entries[group_location_index];
				static const unsigned stamps_id = STATIC_ID32("stamps");
				HASH_LOOKUP(stamps_entry, group_location->next->map, ARRAY_COUNT(group_location->next->map), stamps_id);
				ASSERT(stamps_entry->value, "No stamps in stampgroup %.*s in file %.*s",
	 					group->name.length, group->name.text, location_type_settings.path.length, location_type_settings.path.text);
				ASSERT(stamps_entry->value->next->count < MAX_NUM_STAMPS_PER_LOCATION, "Too many stamps in location. Max is currently %d (file %.*s, group %.*s)",
					MAX_NUM_STAMPS_PER_LOCATION, location_type_settings.path.length, location_type_settings.path.text,
					group->name.length, group->name.text);
				// each stamp in location
				for(unsigned stamp_index = 0; stamp_index < stamps_entry->value->next->count; ++stamp_index) {
					fprintf(output, "\t\t\t{\n");
					SettingsData *stamp_data = &stamps_entry->value->next->entries[stamp_index];
					static const unsigned path_id = STATIC_ID32("path");
					HASH_LOOKUP(path_entry, stamp_data->next->map, ARRAY_COUNT(stamp_data->next->map), path_id);
	 				ASSERT(path_entry->value, "No path in stamp %.*s in location %.*s in group %.*s in file %.*s",
	 					stamp_data->name.length, stamp_data->name.text, group_location->name.length, group_location->name.text,
	 					group->name.length, group->name.text, location_type_settings.path.length, location_type_settings.path.text);

	 				String stamp_path = path_entry->value->value;
	 				clean_string_from_quotations(stamp_path);
	 				fprintf(output, "\t\t\t\tnullptr, // stamp\n");
	 				fprintf(output, "\t\t\t\t0x%016llx, // %.*s\n", to_id64(stamp_path.length, stamp_path.text), stamp_path.length, stamp_path.text);

					static const unsigned weight_id = STATIC_ID32("weight");
					HASH_LOOKUP(weight_entry, stamp_data->next->map, ARRAY_COUNT(stamp_data->next->map), weight_id);
	 				ASSERT(weight_entry->value, "No weight in stamp %.*s in location %.*s in group %.*s in file %.*s",
	 					stamp_data->name.length, stamp_data->name.text, group_location->name.length, group_location->name.text,
	 					group->name.length, group->name.text, location_type_settings.path.length, location_type_settings.path.text);
	 				float stamp_weigth = 0.f;
	 				String weight_string = weight_entry->value->value;
	 				clean_string_from_quotations(weight_string);
	 				if(sscanf(weight_string.text, "%f", &stamp_weigth) != 1)
	 					ASSERT(false, "Failed to parse weight string as a float: \"%.*s\"", weight_string.length, weight_string.text);
					fprintf(output, "\t\t\t\t%ff, // weight\n", stamp_weigth);
					total_stamp_weigths += stamp_weigth;
					fprintf(output, "\t\t\t},\n");
				}
				fprintf(output, "\t\t},\n");

				static const unsigned radius_id = STATIC_ID32("radius");
				float radius = 0.f;
				find_and_parse_float(group_location, radius_id, radius);
				static const unsigned padding_id = STATIC_ID32("padding");
				float padding = 0.f;
				find_and_parse_float(group_location, padding_id, padding);

				unsigned flags = 0;
				bool blocks_road = false;
				static const unsigned blocks_road_id = STATIC_ID32("blocks_road");
				find_and_parse_bool(group_location, blocks_road_id, blocks_road);
 				bool force_inside_same_region = false;
				static const unsigned inside_same_region_id = STATIC_ID32("inside_same_region");
				find_and_parse_bool(group_location, inside_same_region_id, force_inside_same_region);

				bool delete_upon_road = false;
				static const unsigned delete_upon_road_id = STATIC_ID32("delete_upon_road");
				find_and_parse_bool(group_location, delete_upon_road_id, delete_upon_road);

				if(force_inside_same_region)
					flags += LOCATION_FLAG_INSIDE_SAME_REGION;
				if(blocks_road)
					flags += LOCATION_FLAG_BLOCKS_ROAD;
				if(delete_upon_road)
					flags += LOCATION_FLAG_DELETE_UPON_ROAD;

				fprintf(output, "\t\t%f, // radius\n", radius);
				fprintf(output, "\t\t%f, // padding\n", padding);
				fprintf(output, "\t\t%f, // total stamp weights\n", total_stamp_weigths);
				fprintf(output, "\t\t%d, // num_stamps\n", stamps_entry->value->next->count);
				fprintf(output, "\t\t%d, // flags\n", flags);
				int satellites_index = -1;
				static const unsigned satellites_id = STATIC_ID32("satellites");
				HASH_LOOKUP_STATIC(satellites_entry, group_location->next->map, satellites_id);
				if(satellites_entry->value) {
					satellites_index = total_satellites_count++;
					SettingsDataStore *satellites_road_store = satellites_entry->value->next;
					fprintf(satellite_output, "\t{ // %.*s\n", group_location->name.length, group_location->name.text);
					for(unsigned satellites_road_type = 0; satellites_road_type < satellites_road_store->count; ++satellites_road_type) {
						SettingsData *satellite_data_list = &satellites_road_store->entries[satellites_index];
						String satellite_name = satellite_data_list->name;
						fprintf(satellite_output, "\t\tSatelliteList *list = &satellite_data[%d].road_exits[RoadType_%.*s];\n",
							satellites_index, satellite_name.length, satellite_name.text);
						ASSERT(satellite_data_list->next->count <= 8, "Too many satellites for one location. We need to bump the max amount of satellites per road type. Please contact Simon.");
						for(unsigned satellite_link_index = 0; satellite_link_index < satellite_data_list->next->count; ++satellite_link_index) {
							String satellite_link_name = satellite_data_list->next->entries[satellite_link_index].name;
							fprintf(satellite_output, "\t\t\tlist->entries[%d].type = LOCATION_TYPE_%.*s;\n", satellite_link_index,
								satellite_link_name.length, satellite_link_name.text);
							SettingsData *satellite_settings_store = &satellite_data_list->next->entries[satellite_link_index];
							static const unsigned minimum_distance_id = STATIC_ID32("minimum_distance");
							static const unsigned maximum_distance_id = STATIC_ID32("maximum_distance");
							static const unsigned maximum_count_id = STATIC_ID32("maximum_count");
							float minimum_distance = 0.f;
							float maximum_distance = 100.f;
							unsigned maximum_count = 255;
							find_and_parse_float(satellite_settings_store, minimum_distance_id, minimum_distance);
							fprintf(satellite_output, "\t\t\tlist->entries[%d].minimum_distance = %f;\n", satellite_link_index, minimum_distance);
							fprintf(satellite_output, "\t\t\tlist->entries[%d].maximum_distance = %f;\n", satellite_link_index, maximum_distance);
							fprintf(satellite_output, "\t\t\tlist->entries[%d].maximum_count = %d;\n", satellite_link_index, maximum_count);
						}
						fprintf(satellite_output, "\t\tlist->count = %d;\n", satellite_data_list->next->count);
					}
					fprintf(satellite_output, "\t}\n");
				}
				fprintf(output, "\t\t%d, // satellites index\n", satellites_index);
				fprintf(output, "\t};\n");
			}
		}
	}
	fprintf(satellite_output, "}\n\n");
	fprintf(satellite_output, "static const unsigned total_number_of_satellite_groups = %d;\n\n", total_satellites_count);

	fprintf(output, "};\n\n");
	fprintf(output, "static const unsigned total_number_of_level_generation_locations = %d;\n\n", location_index);
	unsigned num_factions = ARRAY_COUNT(faction_types);
	fprintf(output, "enum LevelGenerationFactionToIndex {\n");
	for(unsigned i = 0;i < num_factions; ++i) {
		fprintf(output, "\tLEVEL_GENERATION_FACTION_%s = %d,\n", faction_types[i].name, i);
	}
	fprintf(output, "};\n\n");

	fprintf(output, "void level_generation_fill_location_affiliations(LevelGenerationAffiliationData *data, unsigned *region_affiliations, uint8_t *faction_affiliations) {\n");
	location_index = 0;
	unsigned region_affiliation_count = 0;
	for (int location_type_index = 0; location_type_index < array_count(level_locations_settings_array); ++location_type_index) {
		Settings &location_type_settings = level_locations_settings_array[location_type_index];
		String filename = get_filename(location_type_settings.path);
		ComponentSettings& component_settings = location_type_settings.component_settings[0];
		SettingsDataStore *location_data_store = component_settings.settings_data_store;
		// each group in file
		for(unsigned group_index = 0; group_index < location_data_store->entries[0].next->count; ++group_index) {
			SettingsData *group = &location_data_store->entries[0].next->entries[group_index];
			// each group location in group
			for(unsigned group_location_index = 0; group_location_index < group->next->count; ++group_location_index) {
				// Parse region affiliations
				SettingsData *group_location = &group->next->entries[group_location_index];
				static const unsigned stamps_id = STATIC_ID32("stamps");
				HASH_LOOKUP_STATIC(stamps_entry, group_location->next->map, stamps_id);
				for(unsigned stamp_index = 0; stamp_index < stamps_entry->value->next->count; ++stamp_index) {
					SettingsData *stamp_data = &stamps_entry->value->next->entries[stamp_index];

					static const unsigned region_affiliation_id = STATIC_ID32("region_affiliation");
					HASH_LOOKUP_STATIC(region_affiliation_entry, stamp_data->next->map, region_affiliation_id);
					if(region_affiliation_entry->value) {
						fprintf(output, "\tdata[%d].region_affiliation_index = %d;\n", location_index, region_affiliation_count);
						fprintf(output, "\tdata[%d].region_affiliation_count = %d;\n", location_index, region_affiliation_entry->value->next->count);
						for(unsigned i = 0;i < region_affiliation_entry->value->next->count; ++i) {
							String region_name = region_affiliation_entry->value->next->entries[i].name;
//							fprintf(output, "\tregion_affiliations[%d] = 0x%x;\n", region_affiliation_count++, to_id32(region_name.length, region_name.text));
							fprintf(output, "\tregion_affiliations[%d] = LevelGenerationRegion_%.*s;\n", region_affiliation_count++, region_name.length, region_name.text);
						}

					} else {
					}

					static const unsigned faction_affiliation_id = STATIC_ID32("faction_affiliation");
					HASH_LOOKUP_STATIC(faction_affiliation_entry, stamp_data->next->map, faction_affiliation_id);
					if(faction_affiliation_entry->value) {
						fprintf(output, "\tdata[%d].faction_affiliation = FactionType_None", location_index);
						for(unsigned i = 0;i < faction_affiliation_entry->value->next->count; ++i) {
							String faction_name = faction_affiliation_entry->value->next->entries[i].name;
							faction_name.text[0] = toupper(faction_name.text[0]);
							fprintf(output, " | FactionType_%.*s", faction_name.length, faction_name.text);
						}
						fprintf(output, ";\n");
					} else {
						fprintf(output, "\tdata[%d].faction_affiliation = FactionType_None;\n", location_index);
					}
					location_index++;
				}
			}
		}
	}
	fprintf(output, "}\n\n");
	fprintf(output, "static const unsigned total_number_of_level_generation_region_affiliations = %d;\n", region_affiliation_count);

	// Faction settings
	unsigned num_faction_settings = ARRAY_COUNT(faction_settings);

	fprintf(output, "struct LevelGenerationLocationFactionSettings {\n");
	for(unsigned j = 0; j < num_faction_settings; ++j) {
		fprintf(output, "\t%s %s;\n", FactionTypeToString[faction_settings[j].type], faction_settings[j].name);
	}
	fprintf(output, "};\n\n");
	fprintf(output, "struct LevelGenerationLocationFactionsSettings {\n");
	fprintf(output, "\tLevelGenerationLocationFactionSettings faction_settings[%d];\n", num_factions);
	fprintf(output, "};\n\n");
	// Parse faction settings.
	unsigned faction_setting_index = 1;
	fprintf(output, "void level_generation_fill_faction_settings(LevelGenerationLocationFactionsSettings *level_generation_faction_settings) {\n");
	for (int location_type_index = 0; location_type_index < array_count(level_locations_settings_array); ++location_type_index) {
		Settings &location_type_settings = level_locations_settings_array[location_type_index];
		String filename = get_filename(location_type_settings.path);
		fprintf(output, "//%s\n", location_type_settings.path.text);
		ComponentSettings& component_settings = location_type_settings.component_settings[0];
		SettingsDataStore *location_data_store = component_settings.settings_data_store;
		// each group in file
		for(unsigned group_index = 0; group_index < location_data_store->entries[0].next->count; ++group_index) {
			SettingsData *group = &location_data_store->entries[0].next->entries[group_index];
			for(unsigned group_location_index = 0; group_location_index < group->next->count; ++group_location_index) {
				fprintf(output, "\tlevel_generation_faction_settings[%d] = { // %.*s\n", faction_setting_index++, group->name.length, group->name.text);
				fprintf(output, "\t\t{\n");
				for(unsigned faction_index = 0;faction_index < num_factions; ++faction_index) {
					SettingsData *group_location = &group->next->entries[group_location_index];
					HASH_LOOKUP_STATIC(faction_entry, group_location->next->map, faction_types[faction_index].id);
					if(faction_entry->value) {
						// We have settings for this faction.
						fprintf(output, "\t\t\t{ // HAS %s\n", faction_types[faction_index].name);
						for(unsigned variable_index = 0; variable_index < num_faction_settings; ++variable_index) {
							String variable_text = parse_faction_setting(faction_entry->value, variable_index);
							fprintf(output, "\t\t\t\t%.*s,\n", variable_text.length, variable_text.text);
						}
						fprintf(output, "\t\t\t},\n");
					} else {
						fprintf(output, "\t\t\t{}, // %s\n", faction_types[faction_index].name);
					}
				}
				fprintf(output, "\t\t}\n");
				fprintf(output, "\t};\n");
			}
		}
	}
	fprintf(output, "};\n\n");

	for (int location_type_index = 0; location_type_index < array_count(level_locations_settings_array); ++location_type_index) {
		Settings &location_type_settings = level_locations_settings_array[location_type_index];
		ComponentSettings& component_settings = location_type_settings.component_settings[0];
		SettingsDataStore *location_data_store = component_settings.settings_data_store;
		// each group in file
		for(unsigned group_index = 0; group_index < location_data_store->entries[0].next->count; ++group_index) {
			SettingsData *group = &location_data_store->entries[0].next->entries[group_index];
			bool found = false;

			for (unsigned location_type_index = 0; location_type_index < context.num_location_types; ++location_type_index) {
				if(are_strings_equal(group->name, context.location_types[location_type_index])) {
					found = true;
					break;
				}
			}
			ASSERT(!found, "Wtf found group when not needed?");
			context.location_types[context.num_location_types++] = group->name;
			ASSERT(context.num_location_types < GENERATION_MAX_NUM_LOCATION_TYPES, "Ran out of location type space. Contact Simon or increase the GENERATION_MAX_NUM_LOCATION_TYPES in output_level_generation_settings.cpp.");
		}
	}

	fclose(output);
	fclose(satellite_output);
}

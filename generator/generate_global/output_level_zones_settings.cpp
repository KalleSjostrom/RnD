String get_filename(const String& str) {
	unsigned index = str.length-1;
	while(str.text[index] != '/') {
		index--;
	}
	String new_str;
	new_str.text = &str.text[index+1];
	new_str.length = str.length - index - 1;
	return new_str;
}

// Todo: maybe macrofy this?
static const String zone_stamp_weight_names[] = {
	MAKE_STRING("slope_weight"),
	MAKE_STRING("height_weight"),
	MAKE_STRING("zone_depth_weight"),
	MAKE_STRING("zone_index_weight"),
	MAKE_STRING("region_weight"),
};

static const String zone_stamp_weight_variables[] = {
	MAKE_STRING("slope"),
	MAKE_STRING("height"),
	MAKE_STRING("zone_depth"),
	MAKE_STRING("zone_index"),
	MAKE_STRING("region_weight"),
};

enum ZoneStampWeightType {
	WEIGHT_TYPE_NONE = 0,
	WEIGHT_TYPE_INTERVAL,
	WEIGHT_TYPE_ZONE_INDEX,
};

static const ZoneStampWeightType zone_stamp_weight_type[] = {
	WEIGHT_TYPE_INTERVAL,
	WEIGHT_TYPE_INTERVAL,
	WEIGHT_TYPE_INTERVAL,
	WEIGHT_TYPE_ZONE_INDEX,
	WEIGHT_TYPE_INTERVAL,
};

static const String zone_stamp_variables[] = {
	MAKE_STRING("path"),
	MAKE_STRING("weight"),
	MAKE_STRING("max_ledge_angle"),
	//MAKE_STRING("region_weight"),
};

enum ZoneStampVariableType {
	STAMP_VARIABLE_PATH,
	STAMP_VARIABLE_FLOAT,
};

static const ZoneStampVariableType zone_stamp_variable_types[] = {
	STAMP_VARIABLE_PATH,
	STAMP_VARIABLE_FLOAT,
	STAMP_VARIABLE_FLOAT,
	STAMP_VARIABLE_FLOAT,
};

static const String zone_stamp_variable_defaults[] = {
	make_string("", 0),
	MAKE_STRING("1.f"),
	MAKE_STRING("90.f"),
	MAKE_STRING("0.f"),
};

static const unsigned NUM_STAMP_WEIGHT_TYPES = sizeof(zone_stamp_weight_type) / sizeof(zone_stamp_weight_type[0]);
static const unsigned NUM_STAMP_VARIABLES = sizeof(zone_stamp_variables) / sizeof(zone_stamp_variables[0]);

void output_level_zones_settings(SettingsArray &level_zones_settings_array, MemoryArena &arena) {
	{
		String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_zone_settings"), MAKE_STRING(".generated.cpp"));
		MAKE_OUTPUT_FILE(output, *output_filepath);
		String header_output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_zone_settings"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(header_output, *header_output_filepath);

		fprintf(output, "// Automatically generated file\n\n");

		// ZoneId enum
		fprintf(output, "enum ZoneId {\n");
		fprintf(output, "\tZONE_ID_UNKNOWN = 0,\n");
		int level_zone_index;
		for (level_zone_index = 0; level_zone_index < array_count(level_zones_settings_array); ++level_zone_index) {
			String filename = get_filename(level_zones_settings_array[level_zone_index].path);
			fprintf(output, "\tZONE_ID_%.*s,\n", filename.length, filename.text);
		}
		fprintf(output, "\tZONE_ID_NUM_ZONES,\n");
		fprintf(output, "};\n\n");

		fprintf(output, "//String ZoneIdToName[] = {\n");
		fprintf(output, "\t//MAKE_STRING(\"ZONE_ID_UNKNOWN\"),\n");
		for (level_zone_index = 0; level_zone_index < array_count(level_zones_settings_array); ++level_zone_index) {
			String filename = get_filename(level_zones_settings_array[level_zone_index].path);
			fprintf(output, "\t//MAKE_STRING(\"%.*s\"),\n", filename.length, filename.text);
		}
		fprintf(output, "//};\n\n");

		// Zone settings loading
		for (int i = 0; i < array_count(level_zones_settings_array); ++i) {
			String filename = get_filename(level_zones_settings_array[i].path);
			fprintf(output, "void load_%.*s(ZoneSettings *zone_settings) {\n", filename.length, filename.text);
			Settings &zone_settings = level_zones_settings_array[i];
			char zone_id[128];
			ASSERT(filename.length + strlen("ZONE_ID_") < 128, "Too long filename in file: %s", filename.text);
			sprintf(zone_id, "ZONE_ID_%.*s", filename.length, filename.text);
			zone_id[strlen("ZONE_ID_") + filename.length] = '\0';
			ComponentSettings& component_settings = zone_settings.component_settings[0];
			unsigned stamp_group_index = 0;

			for(int setting_index = 0; setting_index < component_settings.settings_data_store->count; ++setting_index) {
				SettingsData& settings_data = component_settings.settings_data_store->entries[setting_index];
				if(settings_data.name.length >= strlen("settings") && memcmp(settings_data.name.text, "settings", strlen("settings")) == 0) {
					SettingsDataStore *stamps_data = settings_data.next;
					for(unsigned index = 0; index < stamps_data->count; ++index) {
						String name = stamps_data->entries[index].name;
						String value = stamps_data->entries[index].value;
						clean_string_from_quotations(value);
						fprintf(output, "\tzone_settings[%s].%s = %.*s;\n", zone_id, name.text, value.length, value.text);
					}
				} else {
					String name = settings_data.name;
					fprintf(header_output, "static const unsigned %s_%s = %d;\n", zone_id, name.text, stamp_group_index++);
					fprintf(output, "\t{\n");
					fprintf(output, "\t\tStampGroup *stamp_group = &zone_settings[%s].stamp_groups[%s_%s];\n", zone_id, zone_id, name.text);

					static unsigned stamps_id = to_id32(strlen("stamps"), "stamps");
					HASH_LOOKUP(stamps_entry, settings_data.next->map, ARRAY_COUNT(settings_data.next->map), stamps_id);
					ASSERT(stamps_entry->value != nullptr, "Fail! No stamps-entry in stamp group %.*s in file %.*s!", name.length, name.text, filename.length, filename.text);

					SettingsDataStore *stamps_data = stamps_entry->value->next;
					for(unsigned stamp_info_index = 0; stamp_info_index < stamps_data->count; ++stamp_info_index) {
						SettingsDataStore *stamp_settings = stamps_data->entries[stamp_info_index].next;
						for(unsigned stamp_variable_index = 0; stamp_variable_index < NUM_STAMP_VARIABLES; ++stamp_variable_index) {
							String variable_name = zone_stamp_variables[stamp_variable_index];
							ZoneStampVariableType variable_type = zone_stamp_variable_types[stamp_variable_index];
							HASH_LOOKUP(variable_entry, stamp_settings->map, ARRAY_COUNT(stamp_settings->map), to_id32(variable_name.length, variable_name.text));
							if(variable_entry->value) {
								switch(variable_type) {
									case STAMP_VARIABLE_PATH: {
										String setting_value = variable_entry->value->value;
										clean_string_from_quotations(setting_value);
										String path_formatted = slash_to_asset_string(setting_value, arena);
										fprintf(output, "\t\tstamp_group->stamps[%d].%s = %.*s;\n", stamp_info_index, variable_name.text, path_formatted.length, path_formatted.text);

									} break;
									case STAMP_VARIABLE_FLOAT: {
										float value;
										String string_value = variable_entry->value->value;
										clean_string_from_quotations(string_value);
										sscanf(string_value.text, "%f", &value);
										fprintf(output, "\t\tstamp_group->stamps[%d].%s = %ff;\n", stamp_info_index, variable_name.text, value);
									} break;
									default:
										ASSERT(false, "Unsupported variable type.");
								}
							} else {
								if(zone_stamp_variable_defaults[stamp_variable_index].length > 0)
									fprintf(output, "\t\tstamp_group->stamps[%d].%s = %s;\n", stamp_info_index, variable_name.text, zone_stamp_variable_defaults[stamp_variable_index].text);
							}
						}
					}

					static unsigned settings_id = to_id32(strlen("settings"), "settings");
					HASH_LOOKUP(settings_entry, settings_data.next->map, ARRAY_COUNT(settings_data.next->map), settings_id);
					ASSERT(settings_entry->value != nullptr, "Fail! No settings-entry in stamp group %.*s in file %.*s", name.length, name.text, filename.length, filename.text);

					SettingsData *group_settings = settings_entry->value;
					//SettingsDataStore *group_settings = group_settings_data->entries[settings_index].next;
					static const unsigned maximum_nudge_distance_id = STATIC_ID32("maximum_nudge_distance");
					static const unsigned grid_cell_size_id = STATIC_ID32("grid_cell_size");
					static const unsigned max_random_in_cell_id = STATIC_ID32("max_random_in_cell");
					static const unsigned fill_rate_id = STATIC_ID32("fill_rate");
					float maximum_nudge_distance = 0.f;
					float grid_cell_size = 1.f;
					float max_random_in_cell = 0.5f;
					float fill_rate = 1.f;
					find_and_parse_float(group_settings, maximum_nudge_distance_id, maximum_nudge_distance);
					find_and_parse_float(group_settings, grid_cell_size_id, grid_cell_size);
					find_and_parse_float(group_settings, max_random_in_cell_id, max_random_in_cell);
					find_and_parse_float(group_settings, fill_rate_id, fill_rate);
					fprintf(output, "\t\tstamp_group->maximum_nudge_distance = %f;\n", maximum_nudge_distance);
					fprintf(output, "\t\tstamp_group->grid_cell_size = %f;\n", grid_cell_size);
					fprintf(output, "\t\tstamp_group->max_random_in_cell = %f;\n", max_random_in_cell);
					fprintf(output, "\t\tstamp_group->fill_rate = %f;\n", fill_rate);

					fprintf(output, "\t\tstamp_group->count = %d;\n", stamps_data->count);
					fprintf(output, "\t\tASSERT(MAX_STAMPGROUPS_IN_ZONESETTINGS > stamp_group->count, \"Too many stamps in stampgroup! Max is %%d\", MAX_STAMPGROUPS_IN_ZONESETTINGS);\n");
					fprintf(output, "\t}\n");
				}
			}
			fprintf(output, "\tzone_settings[%s].num_stampgroups = %d;\n", zone_id, stamp_group_index);
			fprintf(output, "}\n\n");
		}

		fprintf(output, "\n");


		fprintf(output, "struct WeightRuleContext {\n");
		fprintf(output, "\tconst RegionSettings *region_settings;\n");
		fprintf(output, "\tRandom *random;\n");
		fprintf(output, "\tfloat height;\n");
		fprintf(output, "\tfloat slope;\n");
		fprintf(output, "\tint zone_index;\n");
		fprintf(output, "\tfloat region_weight;\n");
		fprintf(output, "};\n\n");

		// Stamp group rules
		for (int i = 0; i < array_count(level_zones_settings_array); ++i) {
			String filename = get_filename(level_zones_settings_array[i].path);
			// fprintf(output, "void load_%.*s(ZoneSettings *zone_settings) {\n", filename.length, filename.text);
			char zone_id[128];
			ASSERT(filename.length + strlen("ZONE_ID_") < 128, "Too long filename in file: %s", filename.text);
			sprintf(zone_id, "ZONE_ID_%.*s", filename.length, filename.text);
			zone_id[strlen("ZONE_ID_") + filename.length] = '\0';

			Settings &zone_settings = level_zones_settings_array[i];
			ComponentSettings& component_settings = zone_settings.component_settings[0];
			unsigned stamp_group_index = 0;

			for(int setting_index = 0; setting_index < component_settings.settings_data_store->count; ++setting_index) {
				SettingsData& settings_data = component_settings.settings_data_store->entries[setting_index];
				if(!are_strings_equal(settings_data.name, make_string("settings"))) {
					String name = settings_data.name;
					fprintf(output, "int evaluate_stamp_group__%s_%s(WeightRuleContext& context) {\n", zone_id, name.text);
					String stamp_group_name = settings_data.name;

					static unsigned stamps_id = to_id32(strlen("stamps"), "stamps");
					HASH_LOOKUP(stamps_entry, settings_data.next->map, ARRAY_COUNT(settings_data.next->map), stamps_id);
					ASSERT(stamps_entry->value != nullptr, "Fail! No stamps-entry in stamp group %.*s in file %.*s!", name.length, name.text, filename.length, filename.text);

					SettingsDataStore *stamps_data = stamps_entry->value->next;

					fprintf(output, "\tfloat height = context.height;\n");
					fprintf(output, "\tfloat slope = context.slope;\n");
					fprintf(output, "\tfloat region_weight = context.region_weight;\n");
					fprintf(output, "\tint zone_index = context.zone_index;\n");
					fprintf(output, "\n");
					fprintf(output, "\tfloat rule_scores[%d];\n", stamps_data->count);
					fprintf(output, "\tfloat total_score = 0;\n");
					fprintf(output, "\n");

					if (stamps_data->count == 0)
					{
						fprintf(output, "\t// No stamps defined;\n");
						fprintf(output, "\treturn -1;\n");
						fprintf(output, "}\n\n");
						continue;
					}

					for(unsigned stamp_info_index = 0; stamp_info_index < stamps_data->count; ++stamp_info_index) {

						SettingsDataStore *stamp_settings = stamps_data->entries[stamp_info_index].next;
						fprintf(output, "\t{\t// %d: %.*s\n", stamp_info_index, stamps_data->entries[stamp_info_index].name.length, stamps_data->entries[stamp_info_index].name.text);
						String weight;
						HASH_LOOKUP(weight_entry, stamp_settings->map, ARRAY_COUNT(stamp_settings->map), to_id32(strlen("weight"), "weight"));
						if(weight_entry->value) {
							weight = weight_entry->value->value;
							clean_string_from_quotations(weight);
						}
						fprintf(output, "\t\tfloat score = %.*s;\n", weight.length, weight.text);

						for(unsigned stamp_weight_index = 0; stamp_weight_index < NUM_STAMP_WEIGHT_TYPES; ++stamp_weight_index) {
							String weight_name = zone_stamp_weight_names[stamp_weight_index];
							String weight_variable = zone_stamp_weight_variables[stamp_weight_index];
							ZoneStampWeightType weight_type = zone_stamp_weight_type[stamp_weight_index];
							HASH_LOOKUP(entry, stamp_settings->map, ARRAY_COUNT(stamp_settings->map), to_id32(weight_name.length, weight_name.text));
							if(!entry->value)
								continue;

							if(weight_type == WEIGHT_TYPE_ZONE_INDEX) {
								String value = entry->value->value;
								clean_string_from_quotations(value);
								fprintf(output, "\t\tif(%s != ZONE_ID_%.*s)\n", weight_variable.text, value.length, value.text);
								fprintf(output, "\t\t\tscore = 0.f;\n");
							} else if( weight_type != WEIGHT_TYPE_NONE ) {
								ASSERT(weight_type == WEIGHT_TYPE_INTERVAL, "Unknown weight type for variable %.*s (%.*s)", weight_name.length, weight_name.text, weight_variable.length, weight_variable.text);
								float v0, v1, v2, v3;
								v0 = v1 = v2 = v3 = 0.f;
								const char* strtoparse = entry->value->value.text;
								int val = sscanf(strtoparse, "\"%f , %f , %f , %f\"", &v0, &v1, &v2, &v3);
								ASSERT(val == 4, "Couldn't parse string %s correctly for weight values %s in %s", strtoparse, weight_name.text, stamps_data->entries[stamp_info_index].name.text);
								fprintf(output, "\t\tif(%s < %f)\n", weight_variable.text, v0)
								;
								fprintf(output, "\t\t\tscore = 0.f;\n");
								if((v1-v0) > FLT_EPSILON) {
									fprintf(output, "\t\tif(%s < %f)\n", weight_variable.text, v1);
									fprintf(output, "\t\t\tscore *= (%s - %f) / (%f);\n", weight_variable.text, v0, v1 - v0);
								}
								fprintf(output, "\t\telse if(%s > %f)\n", weight_variable.text, v3);
								fprintf(output, "\t\t\tscore = 0.f;\n");
								if((v3-v2) > FLT_EPSILON) {
									fprintf(output, "\t\telse if(%s > %f)\n", weight_variable.text, v2);
									fprintf(output, "\t\t\tscore *= (1.f - ((%s - %f) / %f));\n", weight_variable.text, v2, v3 - v2);
								}
								fprintf(output, "\n");
							}
						}

						fprintf(output, "\t\trule_scores[%d] = score;\n", stamp_info_index);
						fprintf(output, "\t\ttotal_score += score;\n");
						fprintf(output, "\t\tASSERT(score >= 0, \"Negative score for weight rule '%.*s'\");\n", stamps_data->entries[stamp_info_index].name.length, stamps_data->entries[stamp_info_index].name.text);
						fprintf(output, "\t}\n\n");
					}

					fprintf(output, "\tif(total_score <= FLT_EPSILON*%d)\n\t\treturn -1;\n\n", stamps_data->count);
					fprintf(output, "\t// Utility based rule for weights\n");
					fprintf(output, "\tfloat score_lookup = context.random->rand_float() * total_score;\n");
					fprintf(output, "\tfor (int i = 0; i < %d; ++i) {\n", stamps_data->count);
					fprintf(output, "\t\tscore_lookup -= rule_scores[i];\n");
					fprintf(output, "\t\tif (score_lookup <= 0) {\n");
					fprintf(output, "\t\t\treturn i;\n");
					fprintf(output, "\t\t}\n");
					fprintf(output, "\t}\n");
					fprintf(output, "\n");
					fprintf(output, "\treturn 0;\n");
					fprintf(output, "}\n\n");
				}
			}
		}

		// Stamp group rule function
		fprintf(output, "\n");
		fprintf(output, "int evaluate_stamp_group(WeightRuleContext& context, ZoneId zone_id, unsigned stamp_group_id) {\n");


		fprintf(output, "\n\t// TODO: Smarter lookup than if-if-if\n\n");

		for (int i = 0; i < array_count(level_zones_settings_array); ++i) {
			String filename = get_filename(level_zones_settings_array[i].path);
			// fprintf(output, "void load_%.*s(ZoneSettings *zone_settings) {\n", filename.length, filename.text);
			char zone_id[128];
			ASSERT(filename.length + strlen("ZONE_ID_") < 128, "Too long filename in file: %s", filename.text);
			sprintf(zone_id, "ZONE_ID_%.*s", filename.length, filename.text);
			zone_id[strlen("ZONE_ID_") + filename.length] = '\0';

			Settings &zone_settings = level_zones_settings_array[i];
			ComponentSettings& component_settings = zone_settings.component_settings[0];
			unsigned stamp_group_index = 0;

			for(int setting_index = 0; setting_index < component_settings.settings_data_store->count; ++setting_index) {
				SettingsData& settings_data = component_settings.settings_data_store->entries[setting_index];
				if(settings_data.name.length >= strlen("settings") && memcmp(settings_data.name.text, "settings", strlen("settings")) == 0) {
				} else {
					String name = settings_data.name;
					fprintf(output, "\tif (zone_id == %s && stamp_group_id == %s_%s)\n", zone_id, zone_id, name.text);
					fprintf(output, "\t\treturn evaluate_stamp_group__%s_%s(context);\n\n", zone_id, name.text);
				}
			}
		}

		fprintf(output, "\tASSERT(false, \"No rule defined for stamp group!\");\n");
		fprintf(output, "\treturn -1;\n");
		fprintf(output, "}\n");


		fclose(output);
		fclose(header_output);
	}
}
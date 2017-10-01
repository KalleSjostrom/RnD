void output_planets_data(MemoryArena &arena, SettingsArray &planets_settings_array) {
	unsigned max_num_resource_packages = 0;
	{
	 	String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_planets"), MAKE_STRING(".generated.cpp"));
		MAKE_OUTPUT_FILE(output, *output_filepath);

		for (unsigned planet_index = 0; planet_index < array_count(planets_settings_array); ++planet_index) {
			Settings &planet_settings = planets_settings_array[planet_index];
			String filename = get_filename(planet_settings.path);
			ComponentSettings& component_settings = planet_settings.component_settings[0];

			static const unsigned blocks_id = STATIC_ID32("regions");
			HASH_LOOKUP(regions_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), blocks_id);
			ASSERT(regions_entry->value, "No steps data in level generation settings %s.", filename.text);
			SettingsData& regions = *regions_entry->value;
			SettingsDataStore *regions_data = regions.next;
			float total_weight = 0.f;

			fprintf(output, "void planet_%.*s_fill_data(LevelGenerationPlanetData &planet_data)\n", filename.length, filename.text);
			fprintf(output, "{\n");

			for(unsigned region_index = 0; region_index < regions_data->count; ++region_index) {
				SettingsData *region = &regions_data->entries[region_index];
				fprintf(output, "\tplanet_data.regions[%d].region_name = \"%.*s\";\n", region_index, region->name.length, region->name.text);
				fprintf(output, "\tplanet_data.regions[%d].region_id = 0x%x;\n", region_index, region->name_id);
				fprintf(output, "\tplanet_data.regions[%d].region_type = LevelGenerationRegion_%.*s,\n", region_index, region->name.length, region->name.text);
				static const unsigned file_id = STATIC_ID32("file");
				HASH_LOOKUP(file_entry, region->next->map, ARRAY_COUNT(region->next->map), file_id);
				fprintf(output, "\tplanet_data.regions[%d].region_path = 0x%016llx; // %.*s\n", region_index, to_id64(file_entry->value->value.length, file_entry->value->value.text), file_entry->value->value.length, file_entry->value->value.text);

				static const unsigned weight_id = STATIC_ID32("weight");
				float weight = 1.f;
				find_and_parse_float(region, weight_id, weight);
				fprintf(output, "\tplanet_data.regions[%d].region_weight = %f;\n", region_index, weight);
				total_weight += weight;

				static const unsigned spread_factor_id = STATIC_ID32("spread_factor");
				float spread_factor = 1.f;
				find_and_parse_float(region, spread_factor_id, spread_factor);
				fprintf(output, "\tplanet_data.regions[%d].region_spread_factor = %f;\n", region_index, spread_factor);

				String region_filename = file_entry->value->value;
				clean_string_from_quotations(region_filename);
				region_filename = get_filename(region_filename);
				fprintf(output, "\tplanet_data.regions[%d].fill_functions = level_generation_region_%.*s_fill_functions;\n", region_index, region_filename.length, region_filename.text);
			}
			fprintf(output, "\tplanet_data.regions_count = %d;\n", regions_data->count);
			fprintf(output, "\tplanet_data.total_weight = %f;\n", total_weight);

			{
				static const unsigned utility_levels_id = STATIC_ID32("utility_levels");
				HASH_LOOKUP(utility_levels_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), utility_levels_id);
				ASSERT(utility_levels_entry->value != nullptr, "Failed to find utility levels for planet file %s.", planet_settings.path.text);
				SettingsDataStore *utility_levels_data = utility_levels_entry->value->next;
				ASSERT(utility_levels_data->count > 0, "Planet must have more than one utility level (file: %s)", planet_settings.path.text);
				for(unsigned utility_level_index = 0;utility_level_index < utility_levels_data->count; ++utility_level_index) {
					static const unsigned path_id = STATIC_ID32("path");
					SettingsData *utility_level = &utility_levels_data->entries[utility_level_index];
					HASH_LOOKUP(path_entry, utility_level->next->map, ARRAY_COUNT(utility_level->next->map), path_id);
					ASSERT(path_entry->value != nullptr, "Missing path for utility level %s in file %s.", utility_level->name.text, planet_settings.path.text);
					String path_value = path_entry->value->value;
					clean_string_from_quotations(path_value);
					fprintf(output, "\tplanet_data.utility_levels[%d] = IdString64(%d, \"%.*s\");\n", utility_level_index, path_value.length, path_value.length, path_value.text);
				}
				fprintf(output, "\tplanet_data.utility_levels_count = %d;\n", utility_levels_data->count);
			}

			{
				static const unsigned resource_packages_id = STATIC_ID32("resource_packages");
				HASH_LOOKUP(resource_packages_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), resource_packages_id);
				ASSERT(resource_packages_entry->value != nullptr, "Failed to find resource packages for planet file %s.", planet_settings.path.text);
				SettingsDataStore *resource_packages_data = resource_packages_entry->value->next;
				max_num_resource_packages = max(max_num_resource_packages, resource_packages_data->count);
				for(unsigned resource_package_index = 0; resource_package_index < resource_packages_data->count; ++resource_package_index) {
					SettingsData *resource_package_entry = &resource_packages_data->entries[resource_package_index];
					String resource_package = resource_package_entry->value;
					clean_string_from_quotations(resource_package);
					fprintf(output, "\tplanet_data.resource_packages[%d] = %.*s;\n", resource_package_index, resource_package.length, resource_package.text);
				}
				fprintf(output, "\tplanet_data.num_resource_packages = %d;\n", resource_packages_data->count);
			}

			fprintf(output, "}\n\n");
		}

		fprintf(output, "PlanetFillFunction level_generation_planet_fill_functions[] = {\n");
		for(unsigned planet_index = 0; planet_index < array_count(planets_settings_array); ++planet_index) {
			Settings &planet_settings = planets_settings_array[planet_index];
			String filename = get_filename(planet_settings.path);
			fprintf(output, "\tplanet_%.*s_fill_data,\n", filename.length, filename.text);
		}
		fprintf(output, "};\n\n");
		fprintf(output, "uint64_t level_generation_planet_fill_ids[] = {\n");
		for(unsigned planet_index = 0; planet_index < array_count(planets_settings_array); ++planet_index) {
			Settings &planet_settings = planets_settings_array[planet_index];
			String filename = planet_settings.path; //get_filename(planet_settings.path);
			fprintf(output, "\t0x%016llx, // %.*s\n", to_id64(filename.length, filename.text), filename.length, filename.text);
		}
		fprintf(output, "};\n\n");
		fprintf(output, "const char *level_generation_planet_names[] = {\n");
		for(unsigned planet_index = 0; planet_index < array_count(planets_settings_array); ++planet_index) {
			Settings &planet_settings = planets_settings_array[planet_index];
			String filename = get_filename(planet_settings.path);
			fprintf(output, "\t\"%.*s\", \n", filename.length, filename.text);
		}
		fprintf(output, "};\n\n");
		fclose(output);
	}

	{
		String output_header_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_planets"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(output, *output_header_filepath);
		fprintf(output, "struct GeneratorQueue;\n");
		fprintf(output, "struct LevelGenerationResources;\n");
		fprintf(output, "struct GeneratorStep;\n");
		fprintf(output, "struct GeneratorRegionBlock;\n");
		fprintf(output, "typedef void(*RegionQueueFillFunction)(GeneratorQueue* queue, GeneratorRegionBlock *block, LevelGenerationResources* resources, unsigned region_index);\n\n");
		fprintf(output, "enum LevelGenerationRegionType {\n");
		for (unsigned planet_index = 0; planet_index < array_count(planets_settings_array); ++planet_index) {
			Settings &planet_settings = planets_settings_array[planet_index];
			ComponentSettings& component_settings = planet_settings.component_settings[0];
			static const unsigned blocks_id = STATIC_ID32("regions");
			HASH_LOOKUP(regions_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), blocks_id);
			SettingsData& regions = *regions_entry->value;
			SettingsDataStore *regions_data = regions.next;
			for (unsigned region_index = 0; region_index < regions_data->count; ++region_index) {
				SettingsData *region = &regions_data->entries[region_index];
				fprintf(output, "\tLevelGenerationRegion_%.*s,\n", region->name.length, region->name.text);
			}
		}
		fprintf(output, "};\n\n");
		fprintf(output, "struct LevelGenerationRegion {\n");
		fprintf(output, "\tconst char* region_name;\n");
		fprintf(output, "\tunsigned region_id;\n");
		fprintf(output, "\tLevelGenerationRegionType region_type;\n");
		fprintf(output, "\tuint64_t region_path;\n");
		fprintf(output, "\tfloat region_weight;\n");
		fprintf(output, "\tfloat region_spread_factor;\n");
		fprintf(output, "\tRegionQueueFillFunction *fill_functions;\n");
		fprintf(output, "\tunsigned origin_corner;\n");
		fprintf(output, "};\n\n");
		fprintf(output, "struct LevelGenerationPlanetData {\n");
		fprintf(output, "\tLevelGenerationRegion regions[%d];\n", GENERATION_MAX_NUM_REGIONS_PER_PLANET);
		fprintf(output, "\tunsigned regions_count;\n");
		fprintf(output, "\tfloat total_weight;\n");
		fprintf(output, "\tIdString64 utility_levels[8];\n");
		fprintf(output, "\tunsigned utility_levels_count;\n");
		fprintf(output, "\tuint64_t resource_packages[%d];\n", max_num_resource_packages);
		fprintf(output, "\tunsigned num_resource_packages;\n");
		fprintf(output, "};\n\n");
		fprintf(output, "typedef LevelGenerationRegion* LevelGenerationRegionPointer;\n\n");

		fprintf(output, "typedef void(*PlanetFillFunction)(LevelGenerationPlanetData &planet_data);\n");

		fclose(output);
	}
}
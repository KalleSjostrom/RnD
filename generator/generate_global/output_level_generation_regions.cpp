
void output_regions_data(MemoryArena &arena, SettingsArray &region_settings_array, LevelGenerationParseContext &context) {
	FILE *output = context.output;

 	String header_output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_region_settings"), MAKE_STRING(".generated.h"));
	MAKE_OUTPUT_FILE(header_output, *header_output_filepath);
	unsigned max_num_blocks_in_region = 0;
	fprintf(header_output, "enum RegionBlockIndex {\n");
	String region_block_names[32];
	unsigned region_block_indexes[32];
	unsigned region_block_names_count = 0;
	for (int region_index = 0; region_index < array_count(region_settings_array); ++region_index) {
		Settings &region_settings = region_settings_array[region_index];
		String filename = get_filename(region_settings.path);
		ComponentSettings& component_settings = region_settings.component_settings[0];
		static const unsigned blocks_id = to_id32(strlen("blocks"), "blocks");
		HASH_LOOKUP(blocks_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), blocks_id);
		ASSERT(blocks_entry->value, "No steps data in level generation settings %s.", filename.text);
		SettingsData& blocks = *blocks_entry->value;
		SettingsDataStore *blocks_data = blocks.next;
		// Create the fill queue-function.
		for(unsigned block_index = 0; block_index < blocks_data->count; ++block_index) {
			SettingsData *block = &blocks_data->entries[block_index];
			bool already_done = false;
			for(unsigned i = 0; i < region_block_names_count; ++i) {
				if(are_strings_equal(block->name, region_block_names[i])) {
					ASSERT(block_index == region_block_indexes[i], "Mismatched region indexes for %.*s", block->name.length, block->name.text);
					already_done = true;
					break;
				}
			}
			if(!already_done) {
				fprintf(header_output, "\tLEVEL_GENERATION_REGION_BLOCK_%.*s = %d,\n", block->name.length, block->name.text, block_index);
				region_block_indexes[region_block_names_count] = block_index;
				region_block_names[region_block_names_count++] = block->name;
				ASSERT(region_block_names_count <= 32, "Too many region block names. Contact simon to increase count.");
			}
		}
	}
	fprintf(header_output, "};\n\n");
	fprintf(header_output, "struct RegionBlocksArray {\n");
	fprintf(header_output, "\tRegionBlockIndex indexes[16];\n");
	fprintf(header_output, "\tunsigned count;\n");
	fprintf(header_output, "};\n\n");
	fclose(header_output);

	// Parse regions
	for (int region_index = 0; region_index < array_count(region_settings_array); ++region_index) {
		Settings &region_settings = region_settings_array[region_index];
		String filename = get_filename(region_settings.path);
		ComponentSettings& component_settings = region_settings.component_settings[0];
		static const unsigned blocks_id = to_id32(strlen("blocks"), "blocks");
		HASH_LOOKUP(blocks_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), blocks_id);
		ASSERT(blocks_entry->value, "No steps data in level generation settings %s.", filename.text);
		SettingsData& blocks = *blocks_entry->value;
		SettingsDataStore *blocks_data = blocks.next;
		// Create the fill queue-function.
		for(unsigned block_index = 0; block_index < blocks_data->count; ++block_index) {
			SettingsData *block = &blocks_data->entries[block_index];

			SettingsDataStore *steps_data = block->next;
			for(unsigned substep_index = 0; substep_index < steps_data->count; ++substep_index) {
				SettingsDataStore* substep = steps_data->entries[substep_index].next;
				String substep_name = steps_data->entries[substep_index].name;
				String substep_type = substep->entries[0].value;
				String substep_type_name = substep->entries[0].name;
				clean_string_from_quotations(substep_type);
				unsigned substep_type_id = to_id32(substep_type.length, substep_type.text);
				PassDefinition *pass_definition = get_pass_definition_from_string_id(substep_type, filename, substep_name, substep_type_id);
				if(pass_definition->debug_draw_count > 0) {
		 			fprintf(output, "void debug_draw_region_%.*s_%.*s_step_%.*s(LevelGenerationDebugDrawContext *context, GeneratorStep *step) {\n",
		 				filename.length, filename.text,
		 				block->name.length, block->name.text,
		 				substep_name.length, substep_name.text);
					output_debugrender_info(output, pass_definition, substep_type);
					fprintf(output, "}\n\n");
				}
			}

			fprintf(output, "void region_%.*s_%.*s_fill_queue(GeneratorQueue* queue, GeneratorRegionBlock *block, LevelGenerationResources* resources, unsigned region_index)\n", filename.length, filename.text, block->name.length, block->name.text);
			fprintf(output, "{\n");
			fprintf(output, "\tGeneratorStep *steps = block->queue;\n");
			fprintf(output, "\tblock->num_queue = %d;\n", steps_data->count);
			if(steps_data->count > 0) {
			 	fprintf(output, "\tLevelGenerationDebugDrawFunction region_debug_draw_functions[] = {\n");
				for(unsigned substep_index = 0; substep_index < steps_data->count; ++substep_index) {
					SettingsDataStore* substep = steps_data->entries[substep_index].next;
					String substep_name = steps_data->entries[substep_index].name;
					String substep_type = substep->entries[0].value;
					String substep_type_name = substep->entries[0].name;
					clean_string_from_quotations(substep_type);
					unsigned substep_type_id = to_id32(substep_type.length, substep_type.text);
					PassDefinition *pass_definition = get_pass_definition_from_string_id(substep_type, filename, substep_name, substep_type_id);
					if(pass_definition->debug_draw_count > 0) {
			 			fprintf(output, "\t\tdebug_draw_region_%.*s_%.*s_step_%.*s,\n",
			 				filename.length, filename.text,
			 				block->name.length, block->name.text,
			 				substep_name.length, substep_name.text);
			 		} else {
			 			fprintf(output, "nullptr,");
			 		}
			 	}
		 		fprintf(output, "\t};\n");

				unsigned num_objective_group_index = context.num_objective_group_index;
				String objective_groups[1];

				bool replace_region_index = true;
				context.use_step_offset = true;
				parse_and_output_steps(block->next, region_settings.path, filename, context, replace_region_index);
				context.use_step_offset = false;
			}
			context.max_num_steps_per_region_block = max(context.max_num_steps_per_region_block, steps_data->count);
			fprintf(output, "}\n\n");
		}
		max_num_blocks_in_region = max(blocks_data->count, max_num_blocks_in_region);
	}

	fprintf(output, "typedef void(*RegionQueueFillFunction)(GeneratorQueue* queue, GeneratorRegionBlock *block, LevelGenerationResources* resources, unsigned region_index);\n\n");
	for (int region_index = 0; region_index < array_count(region_settings_array); ++region_index) {
		Settings &region_settings = region_settings_array[region_index];
		String filename = get_filename(region_settings.path);
		ComponentSettings& component_settings = region_settings.component_settings[0];
		fprintf(output, "RegionQueueFillFunction level_generation_region_%.*s_fill_functions[] = {\n", filename.length, filename.text);
		static const unsigned blocks_id = to_id32(strlen("blocks"), "blocks");
		HASH_LOOKUP(blocks_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), blocks_id);
		ASSERT(blocks_entry->value, "No steps data in level generation settings %s.", filename.text);
		SettingsData& blocks = *blocks_entry->value;
		SettingsDataStore *blocks_data = blocks.next;
		for(unsigned block_index = 0; block_index < blocks_data->count; ++block_index) {
			SettingsData *block = &blocks_data->entries[block_index];
			fprintf(output, "\tregion_%.*s_%.*s_fill_queue,\n", filename.length, filename.text, block->name.length, block->name.text);
		}
		fprintf(output, "};\n\n");
	}
	fprintf(output, "RegionQueueFillFunction *level_generation_fill_queue_functions[] = {\n");
	for (int region_index = 0; region_index < array_count(region_settings_array); ++region_index) {
		Settings &region_settings = region_settings_array[region_index];
		String filename = get_filename(region_settings.path);
		fprintf(output, "\tlevel_generation_region_%.*s_fill_functions,\n", filename.length, filename.text);
	}
	fprintf(context.header_output, "static const unsigned LEVEL_GENERATION_MAX_NUM_BLOCKS_PER_REGION = %d;\n", max_num_blocks_in_region);
	fprintf(output, "};\n\n");
}


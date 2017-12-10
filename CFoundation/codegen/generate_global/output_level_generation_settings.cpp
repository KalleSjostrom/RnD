enum ValidGeneratorSteps {
		GenerateVoronoi = 0,
		RandomizeBlobs,
		ErodeCorners,
		CreateLedges,
		GenerateZones,
		NormalizeHeights,
		GenerateFbr,
		ConvertNegativeToInverse,
		GenerateRandomizedPoints,
		ResetGenerator,
		RelaxPoints,
		SortEdges,
		GenerateEdgeHeights,
		PaintZoneBottomUp,
		PaintZoneHeightInterval,
		FinalizeZones,
		MergeEdges,
		PlaceStamp,
		CalculateCornerSlopes,
		NUM_STEP_IDS
};

static unsigned long long STEP_NAME_TO_INDEX[ValidGeneratorSteps::NUM_STEP_IDS] = {};
static const char * STEP_INDEX_TO_VARIABLE_NAME[ValidGeneratorSteps::NUM_STEP_IDS] = {};
static const char * STEP_INDEX_TO_PRINTABLE_NAME[ValidGeneratorSteps::NUM_STEP_IDS] = {};

#define ADD_LEVEL_GENERATOR_STEP(TYPE, VARIABLE_NAME) \
	STEP_NAME_TO_INDEX[ValidGeneratorSteps::TYPE] = make_string_id64(MAKE_STRING(#TYPE)); \
	STEP_INDEX_TO_VARIABLE_NAME[ValidGeneratorSteps::TYPE] = #VARIABLE_NAME; \
	STEP_INDEX_TO_PRINTABLE_NAME[ValidGeneratorSteps::TYPE] = #TYPE; 

struct static_struct_to_run_for_level_generation {
	static_struct_to_run_for_level_generation() {
		ADD_LEVEL_GENERATOR_STEP(GenerateVoronoi, generate_voronoi);
		ADD_LEVEL_GENERATOR_STEP(RandomizeBlobs, randomize_blobs);
		ADD_LEVEL_GENERATOR_STEP(ErodeCorners, erode_corners);
		ADD_LEVEL_GENERATOR_STEP(CreateLedges, create_ledges);
		ADD_LEVEL_GENERATOR_STEP(GenerateZones, generate_zones);
		ADD_LEVEL_GENERATOR_STEP(NormalizeHeights, normalize_heights);
		ADD_LEVEL_GENERATOR_STEP(GenerateFbr, generate_fbr);
		ADD_LEVEL_GENERATOR_STEP(ConvertNegativeToInverse, convert_negative_to_inverse);
		ADD_LEVEL_GENERATOR_STEP(GenerateRandomizedPoints, generate_randomized_points);
		ADD_LEVEL_GENERATOR_STEP(ResetGenerator, reset_generator);
		ADD_LEVEL_GENERATOR_STEP(RelaxPoints, relax_points);
		ADD_LEVEL_GENERATOR_STEP(SortEdges, sort_edges);
		ADD_LEVEL_GENERATOR_STEP(GenerateEdgeHeights, generate_edge_heights);
		ADD_LEVEL_GENERATOR_STEP(PaintZoneBottomUp, paint_zone_bottom_up);
		ADD_LEVEL_GENERATOR_STEP(PaintZoneHeightInterval, paint_zone_height_interval);
		ADD_LEVEL_GENERATOR_STEP(FinalizeZones, finalize_zones);
		ADD_LEVEL_GENERATOR_STEP(MergeEdges, merge_edges);
		ADD_LEVEL_GENERATOR_STEP(PlaceStamp, place_stamp);
		ADD_LEVEL_GENERATOR_STEP(CalculateCornerSlopes, calculate_corner_slopes);
	}
};

static_struct_to_run_for_level_generation mysepe;

static ValidGeneratorSteps get_generator_step_id_from_string(uint64_t str) {
	for(unsigned i = 0; i < (unsigned)ValidGeneratorSteps::NUM_STEP_IDS; ++i) {
		if(STEP_NAME_TO_INDEX[i] == str)
			return (ValidGeneratorSteps)i;
	}
	return (ValidGeneratorSteps)-1;
}

void output_level_generation_settings(SettingsArray &level_generation_settings_array, MemoryArena &arena) {
	{
		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, MAKE_STRING("level_generation_settings"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(output, *output_filepath);
fprintf(output, "union StepData {\n");
		for(unsigned i = 0; i < NUM_STEP_IDS; ++i) {
fprintf(output, "\t%s %s;\n", STEP_INDEX_TO_PRINTABLE_NAME[i], STEP_INDEX_TO_VARIABLE_NAME[i]);
		}
fprintf(output, "};\n\n");
fprintf(output, "static const char* get_generator_step_name_from_id[] = {\n");
		for(unsigned i = 0; i < NUM_STEP_IDS; ++i) {
fprintf(output, "\t\"%s\",\n", STEP_INDEX_TO_PRINTABLE_NAME[i]);
		}
fprintf(output, "};\n\n");
fprintf(output, "struct GeneratorStep {\n");
fprintf(output, "\tenum StepId {\n");
		for(unsigned i = 0; i < NUM_STEP_IDS; ++i) {
fprintf(output, "\t\t%s,\n", STEP_INDEX_TO_PRINTABLE_NAME[i]);
		}
fprintf(output, "\t\tNUM_STEP_IDS,\n");
fprintf(output, "\t};\n");
fprintf(output, "\tStepId id;\n");
fprintf(output, "\tStepData data;\n");
fprintf(output, "\tdouble time_taken_ms;\n");
fprintf(output, "};\n\n");
		fclose(output);
	}
	{
		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, MAKE_STRING("generator_queue"), MAKE_STRING(".generated.cpp"));
		MAKE_OUTPUT_FILE(output, *output_filepath);

fprintf(output, "void GeneratorQueue::process_step()\n");
fprintf(output, "{\n");
fprintf(output, "\tif(num_queue > processed_items) {\n");
fprintf(output, "\t\tGeneratorStep& step = queue[processed_items++];\n");
fprintf(output, "\t\tuint64_t start = _Timer.ticks();\n");
fprintf(output, "\t\tswitch(step.id) {\n");
		for(unsigned i = 0; i < NUM_STEP_IDS; ++i) {
fprintf(output, "\t\tcase GeneratorStep::%s: {\n", STEP_INDEX_TO_PRINTABLE_NAME[i]);
fprintf(output, "\t\t\tstep.data.%s.execute(&context);\n", STEP_INDEX_TO_VARIABLE_NAME[i]);
fprintf(output, "\t\t}; break;\n");
		}		
fprintf(output, "\t\tdefault:\n");
fprintf(output, "\t\t\tASSERT(false, \"Unknown generatorstep when trying to process generator queue.\");\n");
fprintf(output, "\t\t}\n");
fprintf(output, "\t\tuint64_t end = _Timer.ticks();\n");
fprintf(output, "\t\tdouble time_in_s = _Timer.ticks_to_seconds(end-start);\n");
fprintf(output, "\t\tdouble time_in_ms = time_in_s * 1000;\n");
fprintf(output, "\t\tstep.time_taken_ms = time_in_ms;\n");
fprintf(output, "\t}\n");
fprintf(output, "}\n");

		fclose(output);
	}
	for (int i = 0; i < level_generation_settings_array.count; ++i) {
		Settings &level_generation_settings = level_generation_settings_array.entries[i];
		ComponentSettings& component_settings = level_generation_settings.component_settings[0];
		SettingsData& steps = component_settings.settings_data_store->entries[0];

		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, MAKE_STRING("level_generation_settings"), MAKE_STRING(".generated.cpp"));
		MAKE_OUTPUT_FILE(output, *output_filepath);
fprintf(output, "void %s_fill_queue(GeneratorQueue* queue)\n", "super_simpes");
fprintf(output, "{\n");
		SettingsDataStore *steps_data = steps.next;
		for(unsigned i = 0; i < steps_data->count; ++i) {
			SettingsDataStore* step = steps_data->entries[i].next;
fprintf(output, "\t{\n\t\tGeneratorStep* step = queue->allocate_step();\n");
			ASSERT(step->count > 0, "No valid ID in step for level generation: %s", steps_data->entries[i].name.text);
			String step_type = step->entries[0].value;
			ASSERT(step_type.text != nullptr, "No valid ID in step for level generation: %s", steps_data->entries[i].name.text);
			// strip out the "'s of the variable id.
			if(step_type.text[step_type.length-1] == '"')
				step_type.length -= 2;
			else if(step_type.text[step_type.length-2] == '"')
				step_type.length -= 3;
			step_type.text += 1;

fprintf(output, "\t\tstep->id = GeneratorStep::%.*s;\n", step_type.length, step_type.text);
			if(step->count > 1) {
				ValidGeneratorSteps step_id = get_generator_step_id_from_string(make_string_id64(step_type));
				for(unsigned j = 1; j < step->count; ++j) {
					String step_variable_name = step->entries[j].name;
					String step_variable_value = step->entries[j].value;
					if(step_variable_value.text[step_variable_value.length-1] == ',')
						step_variable_value.length -= 1;

					if(step_variable_value.text[0] == '"') {
						if(step_variable_value.text[step_variable_value.length-1] == '"')
							step_variable_value.length -= 2;
						else
							ASSERT(false, "String starts with quotes but doesn't end with it: %s", step_variable_value.text);
						step_variable_value.text += 1;
					}

fprintf(output, "\t\tstep->data.%s.%s = %.*s;\n", STEP_INDEX_TO_VARIABLE_NAME[step_id], step_variable_name.text, step_variable_value.length, step_variable_value.text);
				}
			}
//fprintf(output, "\t\tstep->data.%s.%s = %s;\n", step_name, variable_name, variable_data);
fprintf(output, "\t}\n");
		}
fprintf(output, "}\n");
		fclose(output);
	}
}


__forceinline unsigned max(unsigned a, unsigned b) {
	return a > b ? a : b;
}

enum LevelGeneratorSettings {
	GENERATION_MAX_NUM_ZONES = 32,
	GENERATION_MAX_NUM_RULES = 64,
	GENERATION_MAX_NUM_LOCATION_TYPES = 32,
	GENERATION_MAX_NUM_OBJECTIVE_GROUPS = 8,
	GENERATION_MAX_NUM_OBJECTIVES_PER_GROUP = 16,
	GENERATION_MAX_NUM_STAMPS_PER_GROUP = 64,
	GENERATION_MAX_NUM_GENERATORS = 64,
	GENERATION_MAX_NUM_GENERATOR_TYPES = 16,
	MAX_NUM_STAMPS_PER_LOCATION = 16,
	GENERATION_MAX_NUM_REGIONS_PER_LOCATION = 4,
	GENERATION_MAX_NUM_REGIONS_PER_PLANET = 16,
	GENERATION_MAX_NUM_PLANETS = 16,
};

struct LevelGenerationParseContext {
	FILE *output;
	FILE *header_output;
	String *rule_ids;
	unsigned num_rules;
	String *zone_ids;
	unsigned num_zones;
	String *location_types;
	unsigned num_location_types;
	String *region_names;
	unsigned num_regions;
	String *objective_groups;
	unsigned num_objective_group_index;
	unsigned max_num_steps_per_region_block;
	bool use_step_offset;
};

void parse_and_output_steps(SettingsDataStore *steps_data, String level_generation_settings_path, String filename,
	LevelGenerationParseContext &context, bool replace_region_index = false);

void output_debugrender_info(FILE *output, PassDefinition *pass_definition, String step_type) {
	for(unsigned i = 0;i < pass_definition->debug_draw_count; ++i) {
		fprintf(output, "\t{ // %.*s\n", step_type.length, step_type.text);
		fprintf(output, "\t\tLevelGenerationDebugDrawInput_%.*s input;\n", pass_definition->debug_draw_data[i].name.length, pass_definition->debug_draw_data[i].name.text);
		for(unsigned j = 0; j < pass_definition->debug_draw_data[i].input_data_count; ++j) {
			String debug_draw_variable = pass_definition->debug_draw_data[i].input_data[j];
			String debug_draw_lookup_variable = pass_definition->debug_draw_data[i].input_data_lookup[j];
			bool is_input =	false;
			for(unsigned input_var = 0; input_var < pass_definition->input.count; ++input_var) {
				if(are_strings_equal(debug_draw_lookup_variable, pass_definition->input.resources[input_var].name)) {
					is_input = true;
					break;
				}
			}
			if(!is_input) {
				bool is_output = false;
				for(unsigned output_var = 0; output_var < pass_definition->output.count; ++output_var) {
					if(are_strings_equal(debug_draw_lookup_variable, pass_definition->output.resources[output_var].name)) {
						is_output = true;
						break;
					}
				}
				ASSERT(is_output, "Failed to find variable %.*s in debug draw %.*s for pass %.*s!", debug_draw_lookup_variable.length, debug_draw_lookup_variable.text,
					pass_definition->debug_draw_data[i].name.length, pass_definition->debug_draw_data[i].name.text,
					step_type.length, step_type.text);
			}
			fprintf(output, "\t\tinput.%.*s = step->data.%.*s.%s.%.*s;\n",
				debug_draw_variable.length, debug_draw_variable.text,
				pass_definition->name.length, pass_definition->name_lower,
				is_input ? "input" : "output",
				debug_draw_lookup_variable.length, debug_draw_lookup_variable.text);
			fprintf(output, "\t\tinput.%.*s_count = *step->data.%.*s.%s.%.*s_count;\n",
				debug_draw_variable.length, debug_draw_variable.text,
				pass_definition->name.length, pass_definition->name_lower,
				is_input ? "input" : "output",
				debug_draw_lookup_variable.length, debug_draw_lookup_variable.text);
			fprintf(output, "\t\tinput.%.*s_max = step->data.%.*s.%s.%.*s_max;\n",
				debug_draw_variable.length, debug_draw_variable.text,
				pass_definition->name.length, pass_definition->name_lower,
				is_input ? "input" : "output",
				debug_draw_lookup_variable.length, debug_draw_lookup_variable.text);
		}
		fprintf(output, "\t\tLevelGenerationDebugDraw_%.*s(context, &input);\n", pass_definition->debug_draw_data[i].name.length, pass_definition->debug_draw_data[i].name.text);
		fprintf(output, "\t}\n");
	}
}

#include "output_level_generation_regions.cpp"
#include "output_level_generation_planets.cpp"
#include "output_level_generation_locations.cpp"

void parse_and_output_steps(SettingsDataStore *steps_data, String level_generation_settings_path, String filename,
	LevelGenerationParseContext &context, bool replace_region_index) {
	FILE *output = context.output;
	for(unsigned step_index = 0; step_index < steps_data->count; ++step_index) {
		SettingsDataStore* step = steps_data->entries[step_index].next;
		String step_name = steps_data->entries[step_index].name;
		String step_type = step->entries[0].value;
		String step_type_name = step->entries[0].name;
		ASSERT(are_strings_equal(step_type_name, MAKE_STRING("id")), "First variable in step must be id or region_block (Got: %s in file %.*s).", step_type_name.text, filename.length, filename.text);
		ASSERT(step_type.text != nullptr, "In File %.*s: No valid ID in step for level generation: %.*s", level_generation_settings_path.length, level_generation_settings_path.text, step_name.length, step_name.text);
		if(context.use_step_offset) {
			fprintf(output, "\t{\n\t\tGeneratorStep *step = &steps[%d];\n", step_index);
		} else {
			fprintf(output, "\t{\n\t\tGeneratorStep *step = queue->allocate_step();\n");
		}
		ASSERT(step->count > 0, "In File %.*s: No valid ID in step for level generation: %.*s", level_generation_settings_path.length, level_generation_settings_path.text, step_name.length, step_name.text);
		// strip out the "'s of the variable id.
		clean_string_from_quotations(step_type);
		unsigned step_type_id = to_id32(step_type.length, step_type.text);
		PassDefinition *pass_definition = get_pass_definition_from_string_id(step_type, filename, steps_data->entries[step_index].name, step_type_id);
		fprintf(output, "\t\tstep->id = LevelGenerationStep_%.*s;\n", step_type.length, step_type.text);
		if(replace_region_index) {
			fprintf(output, "\t\tstep->debug_draw = region_debug_draw_functions[%d];\n", step_index);
		} else if(pass_definition->debug_draw_count > 0) {
			fprintf(output, "\t\tstep->debug_draw = debug_draw_step_%.*s_step_%.*s;\n", filename.length, filename.text, step_name.length, step_name.text);
		}
		fprintf(output, "\t\t%.*s *typed_step = &step->data.%.*s;\n", step_type.length, step_type.text,
			step_type.length, pass_definition->name_lower);
		bool processed_input = false;
		bool processed_output = false;
		if(step->count > 1) {

			ASSERT(pass_definition != nullptr, "In File %s: Could not find step type in level generation: %s", level_generation_settings_path.text, step_type.text);
			for(unsigned j = 1; j < step->count; ++j) {
				String step_variable_name = step->entries[j].name;
				String step_variable_value = step->entries[j].value;
				unsigned step_variable_name_id = step->entries[j].name_id;

				static const unsigned input_id = STATIC_ID32("input");
				static const unsigned output_id = STATIC_ID32("output");
				static const unsigned region_id = STATIC_ID32("region");
				static const unsigned idstring64_id = STATIC_ID32("IdString64");
				static const unsigned vector2_id = STATIC_ID32("Vector2");
				static const unsigned zoneid_id = STATIC_ID32("ZoneId");
				static const unsigned objectivegroup_id = STATIC_ID32("ObjectiveGroupId");
				static const unsigned stampgroup_id = STATIC_ID32("StampGroupId");
				static const unsigned ruleid_id = STATIC_ID32("rule_id");
				static const unsigned locationtype_id = STATIC_ID32("LevelGenerationLocationType");
				static const unsigned region_block_id = STATIC_ID32("region_block");
				static const unsigned region_blocks_id = STATIC_ID32("region_blocks");
				static const unsigned roadtype_id = STATIC_ID32("RoadType");

				if(step_variable_value.length > 0)
					clean_string_from_quotations(step_variable_value);
				String step_variable_type = get_pass_settings_type(pass_definition, step_variable_name);
				unsigned step_variable_type_id = to_id32(step_variable_type.length, step_variable_type.text);
				if(step_variable_name_id == ruleid_id) {
					char rule_id[128];
					ASSERT(step_variable_value.length + strlen("RULE_ID_") < 128, "Too long filename in file: %s", step_variable_value.text);
					sprintf(rule_id, "RULE_ID_%.*s", step_variable_value.length, step_variable_value.text);
					fprintf(output, "\t\tstep->data.%.*s.%s = %s;\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, rule_id);

					bool found = false;

					for(unsigned rule_index = 0; rule_index < context.num_rules; ++rule_index) {
						if(context.rule_ids[rule_index].length == step_variable_value.length &&
							memcmp(context.rule_ids[rule_index].text, step_variable_value.text, step_variable_value.length) == 0) {
							found = true;
							break;
						}
					}

					if(!found) {
						context.rule_ids[context.num_rules++] = step_variable_value;
					}
				} else if(step_variable_name_id == input_id) {
					SettingsDataStore *input_data = step->entries[j].next;
					for(unsigned step_resource_index = 0; step_resource_index < pass_definition->input.count; ++step_resource_index) {
						GenerationResourceDeclaration* resource_declaration = &pass_definition->input.resources[step_resource_index];
						HASH_LOOKUP(entry, input_data->map, ARRAY_COUNT(input_data->map), resource_declaration->name_id);
						String resource_name = resource_declaration->name;
						if(entry->value) {
							resource_name = entry->value->value;
							clean_string_from_quotations(resource_name);
						}
						GenerationResource *generator = find_generator_from_string(resource_name, filename.text, step_name);
						ASSERT(generator->type_id == resource_declaration->type_id, "Bad type on resource %.*s in pass %.*s. Wanted %.*s got %.*s.",
							resource_name.length, resource_name.text, step_name.length, step_name.text,
							resource_declaration->type.length, resource_declaration->type.text,
							generator->type.length, generator->type.text);
						fprintf(output, "\t\ttyped_step->input.%.*s = resources->%.*s;\n",
							resource_declaration->name.length,
							resource_declaration->name.text,
							resource_name.length,
							resource_name.text);
						fprintf(output, "\t\ttyped_step->input.%.*s_max = %d;\n",
							resource_declaration->name.length, resource_declaration->name.text,
							generator->count);
						fprintf(output, "\t\ttyped_step->input.%.*s_count = &resources->%.*s_count;\n",
							resource_declaration->name.length, resource_declaration->name.text,
							resource_name.length,
							resource_name.text);
					}
					processed_input = true;
				} else if(step_variable_name_id == output_id) {
					SettingsDataStore *output_data = step->entries[j].next;
					for(unsigned step_resource_index = 0; step_resource_index < pass_definition->output.count; ++step_resource_index) {
						GenerationResourceDeclaration* resource_declaration = &pass_definition->output.resources[step_resource_index];
						HASH_LOOKUP(entry, output_data->map, ARRAY_COUNT(output_data->map), resource_declaration->name_id);
						String resource_name = resource_declaration->name;
						if(entry->value) {
							resource_name = entry->value->value;
							clean_string_from_quotations(resource_name);
						}
						GenerationResource *generator = find_generator_from_string(resource_name, filename.text, step_name);
						ASSERT(generator->type_id == resource_declaration->type_id, "Bad type on resource %.*s in pass %.*s. Wanted %.*s got %.*s.",
							resource_name.length, resource_name.text, step_name.length, step_name.text, resource_declaration->type.length, resource_declaration->type.text,
							generator->type.length, generator->type.text);
						fprintf(output, "\t\ttyped_step->output.%.*s = resources->%.*s;\n",
							resource_declaration->name.length,
							resource_declaration->name.text,
							resource_name.length,
							resource_name.text);
						fprintf(output, "\t\ttyped_step->output.%.*s_max = %d;\n",
							resource_declaration->name.length,
							resource_declaration->name.text,
							generator->count);
						fprintf(output, "\t\ttyped_step->output.%.*s_count = &resources->%.*s_count;\n",
							resource_declaration->name.length,
							resource_declaration->name.text,
							resource_name.length,
							resource_name.text);
					}
					processed_output = true;
				} else if (replace_region_index && step_variable_name_id == region_id) {
					fprintf(output, "\t\tstep->data.%.*s.region = region_index;\n", pass_definition->name.length, pass_definition->name_lower);
				} else if (step_variable_type_id == locationtype_id) {
					char location_id[128];
					clean_string_from_quotations(step_variable_value);
					ASSERT(step_variable_value.length + strlen("LOCATION_TYPE_") < 128, "Too long location type %s in file %s", step_variable_value.text, filename.text);
					String location;
					location.text = location_id;
					location.length = sprintf(location_id, "LOCATION_TYPE_%.*s", step_variable_value.length, step_variable_value.text);
					fprintf(output, "\t\tstep->data.%.*s.%s = %s;\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, location_id);
					bool found = false;

					for (unsigned location_type_index = 0; location_type_index < context.num_location_types; ++location_type_index) {
						if(are_strings_equal(step_variable_value, context.location_types[location_type_index])) {
							found = true;
							break;
						}
					}
					ASSERT(found, "Could not find group name %.*s in pass %.*s filename %.*s", step_variable_value.length, step_variable_value.text,
						pass_definition->name.length, pass_definition->name.text, filename.length, filename.text);
				} else if(step_variable_type_id == roadtype_id) {
					char zone_id[128];
					ASSERT(step_variable_value.length + strlen("RoadType_") < 128, "Too long filename in file: %s", step_variable_value.text);
					sprintf(zone_id, "RoadType_%.*s", step_variable_value.length, step_variable_value.text);
					fprintf(output, "\t\tstep->data.%.*s.%s = %s;\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, zone_id);
				} else if(step_variable_type_id == idstring64_id) {
					String string_variable_value = step_variable_value;
					clean_string_from_quotations(string_variable_value);
					fprintf(output, "\t\tstep->data.%.*s.%s = IdString64(%d, \"%.*s\");\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, string_variable_value.length, string_variable_value.length, string_variable_value.text);
				} else if(step_variable_type_id == vector2_id) {
					String string_variable_value = step_variable_value;
					clean_string_from_quotations(string_variable_value);
					fprintf(output, "\t\tstep->data.%.*s.%.*s = { %.*s };\n", pass_definition->name.length, pass_definition->name_lower,
						step_variable_name.length, step_variable_name.text,
						string_variable_value.length, string_variable_value.text);
				} else if(step_variable_type_id == zoneid_id) {
					char zone_id[128];
					ASSERT(step_variable_value.length + strlen("ZONE_ID_") < 128, "Too long filename in file: %s", step_variable_value.text);
					sprintf(zone_id, "ZONE_ID_%.*s", step_variable_value.length, step_variable_value.text);
					fprintf(output, "\t\tstep->data.%.*s.%s = %s;\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, zone_id);

					bool found = false;

					for(unsigned zone_index = 0; zone_index < context.num_zones; ++zone_index) {
						if(context.zone_ids[zone_index].length == step_variable_value.length &&
							memcmp(context.zone_ids[zone_index].text, step_variable_value.text, step_variable_value.length) == 0) {
							found = true;
							break;
						}
					}

					if(!found)
						context.zone_ids[context.num_zones++] = step_variable_value;
				} else if(step_variable_type_id == stampgroup_id) {
					HASH_LOOKUP(entry, step->map, ARRAY_COUNT(step->map), to_id32(strlen("zone_id"), "zone_id"));
					String zone_id = entry->value->value;
					clean_string_from_quotations(zone_id);
					clean_string_from_quotations(step_variable_value);
					fprintf(output, "\t\tstep->data.%.*s.stamp_group = ZONE_ID_%.*s_%.*s;\n", pass_definition->name.length, pass_definition->name_lower, zone_id.length, zone_id.text, step_variable_value.length, step_variable_value.text);
				} else if(step_variable_type_id == objectivegroup_id) {
					char objective_group_id[128];
					ASSERT(step_variable_value.length + strlen("OBJECTIVE_GROUP_") < 128, "Too long filename in file: %s", step_variable_value.text);
					sprintf(objective_group_id, "OBJECTIVE_GROUP_%.*s_%.*s", filename.length, filename.text, step_variable_value.length, step_variable_value.text);
					fprintf(output, "\t\tstep->data.%.*s.%s = %s;\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, objective_group_id);

					bool found = false;

					for(unsigned objective_group_index = 0; objective_group_index < context.num_objective_group_index; ++objective_group_index) {
						if(are_strings_equal(context.objective_groups[objective_group_index], step_variable_value)) {
							found = true;
							break;
						}
					}

					if(!found) {
						context.objective_groups[context.num_objective_group_index++] = step_variable_value;
						ASSERT(context.num_objective_group_index <= GENERATION_MAX_NUM_OBJECTIVE_GROUPS, "Too many group indexes in file! %s", filename.text);
					}
				} else if(step_variable_name_id == region_blocks_id) {
					fprintf(output, "\t\ttyped_step->region_blocks = {\n\t\t\t{\n");
					unsigned num_region_blocks = step->entries[j].next->count;
					for(unsigned block_index = 0; block_index < num_region_blocks; ++block_index) {
						String region_block = step->entries[j].next->entries[block_index].name;
						clean_string_from_quotations(region_block);
						fprintf(output, "\t\t\tLEVEL_GENERATION_REGION_BLOCK_%.*s,\n", region_block.length, region_block.text);
					}
					fprintf(output, "\t\t\t},\n\t\t\t%d,\n", num_region_blocks);
					fprintf(output, "\t\t};\n");
				} else if(step_variable_name_id == region_block_id) {
					String block_name = step_variable_value;
					clean_string_from_quotations(block_name);
					fprintf(output, "\t\ttyped_step->region_block_index = LEVEL_GENERATION_REGION_BLOCK_%.*s;\n", block_name.length, block_name.text);
				} else {
					fprintf(output, "\t\tstep->data.%.*s.%s = %.*s;\n", pass_definition->name.length, pass_definition->name_lower, step_variable_name.text, step_variable_value.length, step_variable_value.text);
				}
			}
		}
		if(!processed_input) {
			for(unsigned step_resource_index = 0; step_resource_index < pass_definition->input.count; ++step_resource_index) {
				GenerationResourceDeclaration* resource_declaration = &pass_definition->input.resources[step_resource_index];
				String resource_name = resource_declaration->name;
				GenerationResource *generator = find_generator_from_string(resource_name, filename.text, step_name);
				ASSERT(generator->type_id == resource_declaration->type_id, "Bad type on resource %.*s in pass %.*s. Wanted %.*s got %.*s.",
					resource_name.length, resource_name.text, step_name.length, step_name.text, resource_declaration->type.length, resource_declaration->type.text,
					generator->type.length, generator->type.text);
				fprintf(output, "\t\ttyped_step->input.%.*s = resources->%.*s;\n",
					resource_declaration->name.length,
					resource_declaration->name.text,
					resource_name.length,
					resource_name.text);
				fprintf(output, "\t\ttyped_step->input.%.*s_max = %d;\n",
					resource_declaration->name.length, resource_declaration->name.text,
					generator->count);
				fprintf(output, "\t\ttyped_step->input.%.*s_count = &resources->%.*s_count;\n",
					resource_declaration->name.length, resource_declaration->name.text,
					resource_name.length,
					resource_name.text);
			}
			processed_input = true;
		}
		if(!processed_output) {
			for(unsigned step_resource_index = 0; step_resource_index < pass_definition->output.count; ++step_resource_index) {
				GenerationResourceDeclaration* resource_declaration = &pass_definition->output.resources[step_resource_index];
				String resource_name = resource_declaration->name;
				GenerationResource *generator = find_generator_from_string(resource_name, filename.text, step_name);
				ASSERT(generator->type_id == resource_declaration->type_id, "Bad type on resource %.*s in pass %.*s. Wanted %.*s got %.*s.",
					resource_name.length, resource_name.text, step_name.length, step_name.text, resource_declaration->type.length, resource_declaration->type.text,
					generator->type.length, generator->type.text);
				fprintf(output, "\t\ttyped_step->output.%.*s = resources->%.*s;\n",
					resource_declaration->name.length,
					resource_declaration->name.text,
					resource_name.length,
					resource_name.text);
				fprintf(output, "\t\ttyped_step->output.%.*s_max = %d;\n",
					resource_declaration->name.length,
					resource_declaration->name.text,
					generator->count);
				fprintf(output, "\t\ttyped_step->output.%.*s_count = &resources->%.*s_count;\n",
					resource_declaration->name.length,
					resource_declaration->name.text,
					resource_name.length,
					resource_name.text);
			}
			processed_output = true;
		}
		fprintf(output, "\t\tstep->step_name = \"%s\";\n", steps_data->entries[step_index].name.text);
		fprintf(output, "\t\tstep->flags = %d;\n", pass_definition->multiple_frames ? 1 : 0);
		fprintf(output, "\t}\n");
	}
}

void output_level_generation_constants(MemoryArena &arena)
{
	String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_constants"), MAKE_STRING(".generated.h"));
	MAKE_OUTPUT_FILE(output, *output_filepath);
	fprintf(output, "enum LevelGenerationConstants {\n");
	for(unsigned i = 0; i < generation_resource_constants_count; ++i) {
		fprintf(output, "\t%.*s = %d,\n", generation_resource_constants[i].name.length,
			generation_resource_constants[i].name.text,
			generation_resource_constants[i].value);
	}
	fprintf(output, "};\n\n");
	fclose(output);
}

void output_generator_queue(MemoryArena &arena) {
 	String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("generator_queue"), MAKE_STRING(".generated.cpp"));
	MAKE_OUTPUT_FILE(output, *output_filepath);
	fprintf(output, "uint64_t GeneratorQueue_hash_resources(LevelGenerationResources *resources, uint64_t initial_hash)\n");
	fprintf(output, "{\n");
	fprintf(output, "\tProfile p(\"level resource hashing\");\n");
	fprintf(output, "\tuint64_t hash = initial_hash;\n");
	fprintf(output, "\thash = murmur_hash_64(resources->voronoi_points, sizeof(resources->voronoi_points), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_tags, sizeof(resources->corner_tags), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->astar_costs, sizeof(resources->astar_costs), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_path_distance, sizeof(resources->corner_path_distance), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->voronoi_areas, sizeof(resources->voronoi_areas), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_heights, sizeof(resources->corner_heights), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_moisture, sizeof(resources->corner_moisture), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_slopes, sizeof(resources->corner_slopes), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_zones, sizeof(resources->corner_zones), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_downhill, sizeof(resources->corner_downhill), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_mask, sizeof(resources->corner_mask), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_closest_path_indexes, sizeof(resources->corner_closest_path_indexes), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->ledges, sizeof(resources->ledges), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->height_increase_areas, sizeof(resources->height_increase_areas), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->area_increase_points, sizeof(resources->area_increase_points), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->corner_lookup, sizeof(resources->corner_lookup), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->road_segment_points, sizeof(resources->road_segment_points), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->road_heights, sizeof(resources->road_heights), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->cutout_triangles, sizeof(resources->cutout_triangles), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->cutout_triangle_heights, sizeof(resources->cutout_triangle_heights), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->zone_paint_noise, sizeof(resources->zone_paint_noise), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->mission_objectives, sizeof(resources->mission_objectives), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->level_to_load, sizeof(resources->level_to_load), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->region_settings, sizeof(resources->region_settings), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->path_segments, sizeof(resources->path_segments), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->spawned_zones, sizeof(resources->spawned_zones), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->satellite_data, sizeof(resources->satellite_data), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->instance_satellites_datas, sizeof(resources->instance_satellites_datas), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->locations_affiliation_settings, sizeof(resources->locations_affiliation_settings), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->region_affiliations, sizeof(resources->region_affiliations), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(resources->faction_affiliations, sizeof(resources->faction_affiliations), hash);\n");
	fprintf(output, "\thash = murmur_hash_64(&resources->voronoi_points_count, sizeof(*resources) - offsetof(LevelGenerationResources, voronoi_points_count), hash);\n");
	fprintf(output, "\treturn hash;\n");
	fprintf(output, "}\n\n");

	fprintf(output, "void GeneratorQueue_process_step(GeneratorStep *queue, GeneratorStepContext &context, uint64_t &final_hash, unsigned &processed_items, unsigned num_queue, bool &processing_multi_frame_pass, GeneratorStepFlag until_flag /* = GENERATOR_STEP_FLAGS_NONE*/ )\n");
	fprintf(output, "{\n");
	fprintf(output, "\tif(num_queue > processed_items && (until_flag == GENERATOR_STEP_FLAGS_NONE || !(queue[processed_items].flags & until_flag))) {\n");
	fprintf(output, "\t\tunsigned step_index_start = processed_items;\n");
	fprintf(output, "\t\tGeneratorStep& step = queue[processed_items++];\n");
	fprintf(output, "\t\tuint64_t start = _Timer.ticks();\n");
	fprintf(output, "\t\tswitch(step.id) {\n");
	for(unsigned i = 0; i < num_generation_passes; ++i) {
		PassDefinition *definition = &generation_pass_definitions[i];
		fprintf(output, "\t\tcase LevelGenerationStep_%.*s: {\n", definition->name.length, definition->name.text);
		if(definition->multiple_frames) {
			fprintf(output, "\t\t\tif(processing_multi_frame_pass)\n");
			fprintf(output, "\t\t\t\tprocessing_multi_frame_pass = step.data.%.*s.update(&step, &context);\n", definition->name.length, definition->name_lower);
			fprintf(output, "\t\t\telse {\n");
			fprintf(output, "\t\t\t\tstep.data.%.*s.execute(&step, &context);\n", definition->name.length, definition->name_lower);
			fprintf(output, "\t\t\t\tprocessing_multi_frame_pass = true;\n");
			fprintf(output, "\t\t\t}\n");
			fprintf(output, "\t\t\tif(processing_multi_frame_pass) processed_items--;\n");
			fprintf(output, "\t\t}; break;\n");
		} else {
			fprintf(output, "\t\t\tstep.data.%.*s.execute(&step, &context);\n", definition->name.length, definition->name_lower);
			fprintf(output, "\t\t}; break;\n");
		}
	}
	fprintf(output, "\t\tdefault:\n");
	fprintf(output, "\t\t\tASSERT(false, \"Unknown generatorstep when trying to process generator queue.\");\n");
	fprintf(output, "\t\t}\n");
	fprintf(output, "\t\tuint64_t end = _Timer.ticks();\n");
	fprintf(output, "\t\tdouble time_in_s = _Timer.ticks_to_seconds(end-start);\n");
	fprintf(output, "\t\tdouble time_in_ms = time_in_s * 1000;\n");
	fprintf(output, "\t\tstep.time_taken_ms = time_in_ms;\n");
	fprintf(output, "\t\tfinal_hash = murmur_hash_64(context.collision_broadphase, sizeof(*context.collision_broadphase), 0);\n");
	fprintf(output, "\t\tfinal_hash = GeneratorQueue_hash_resources(context.resources, final_hash);\n");
	fprintf(output, "\t\tif(step_index_start != processed_items)\n");
	fprintf(output, "\t\t\tLOG_INFO(\"LevelGeneration\", \"Step %%s time taken in ms: %%f. Hash: 0x%%llx)\", step.step_name, time_in_ms, final_hash);\n");
	fprintf(output, "\t\telse if(time_in_ms > 50.f)\n");
	fprintf(output, "\t\t\tLOG_INFO(\"LevelGeneration\", \"Step %%s part step took too long time! (%%f ms). Hash: 0x%%llx)\", step.step_name, time_in_ms, final_hash);\n");

	fprintf(output, "\t}\n");
	fprintf(output, "}\n");

	fclose(output);
}

void output_level_generation_settings(SettingsArray &level_generation_settings_array,
	SettingsArray &level_regions_settings_array, SettingsArray &level_locations_settings_array,
	SettingsArray &level_planet_settings_array, MemoryArena &arena) {

	output_level_generation_constants(arena);
	output_planets_data(arena, level_planet_settings_array);
	String header_output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_settings"), MAKE_STRING(".generated.h"));
	MAKE_OUTPUT_FILE(header_output, *header_output_filepath);
	fprintf(header_output, "union StepData {\n");
	for(unsigned i = 0; i < num_generation_passes; ++i) {
		String name = generation_pass_definitions[i].name;
		fprintf(header_output, "\t%.*s %.*s;\n", name.length, name.text, name.length, generation_pass_definitions[i].name_lower);
	}
	fprintf(header_output, "};\n\n");
	fprintf(header_output, "static const char* get_generator_step_name_from_id[] = {\n");
	for(unsigned i = 0; i < num_generation_passes; ++i) {
		fprintf(header_output, "\t\"%.*s\",\n", generation_pass_definitions[i].name.length, generation_pass_definitions[i].name.text);
	}
	fprintf(header_output, "};\n\n");

	fprintf(header_output, "enum LevelGenerationStepId {\n");
	for(unsigned i = 0; i < num_generation_passes; ++i) {
		fprintf(header_output, "\tLevelGenerationStep_%.*s,\n", generation_pass_definitions[i].name.length, generation_pass_definitions[i].name.text);
	}
	fprintf(header_output, "\tNUM_STEP_IDS,\n");
	fprintf(header_output, "};\n\n");
	fprintf(header_output, "enum GeneratorStepFlag {\n");
	fprintf(header_output, "\tGENERATOR_STEP_FLAGS_NONE = %d,\n", 0);
	fprintf(header_output, "\tGENERATOR_FLAGS_MULTIPLE_FRAMES = %d,\n", 1);
	fprintf(header_output, "};\n\n");
	fprintf(header_output, "struct LevelGenerationDebugDrawContext;\n");
	fprintf(header_output, "struct GeneratorStep;\n");
	fprintf(header_output, "typedef void(*LevelGenerationDebugDrawFunction)(LevelGenerationDebugDrawContext *context, GeneratorStep *step);\n\n");
	fprintf(header_output, "struct GeneratorStep {\n");
	fprintf(header_output, "\tLevelGenerationStepId id;\n");
	fprintf(header_output, "\tLevelGenerationDebugDrawFunction debug_draw;\n");
	fprintf(header_output, "\tStepData data;\n");
	fprintf(header_output, "\tdouble time_taken_ms;\n");
	fprintf(header_output, "\tconst char *step_name;\n");
	fprintf(header_output, "\tint flags;\n");
	fprintf(header_output, "};\n\n");
	fprintf(header_output, "struct LevelGenerationResources {\n");
	for(unsigned i = 0;i < generation_resource_count;++i) {
		GenerationResource *resource = &generation_resources[i];
		fprintf(header_output, "\t%.*s %.*s[%i];\n",
			resource->type.length, resource->type.text,
			resource->name.length, resource->name.text,
			resource->count);
	}
	for(unsigned i = 0;i < generation_resource_count;++i) {
		GenerationResource *resource = &generation_resources[i];
		fprintf(header_output, "\tunsigned %.*s_count;\n", resource->name.length, resource->name.text);
	}
	fprintf(header_output, "};\n");

	//fprintf(header_output, "static const unsigned LEVEL_GENERATION_MEMORY_USAGE = %i; // %.2f MB\n\n", total_memory_usage, total_memory_usage / 1024.0f / 1024.0f);

	output_generator_queue(arena);
	String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_settings"), MAKE_STRING(".generated.cpp"));
	MAKE_OUTPUT_FILE(output, *output_filepath);
	String zone_ids[GENERATION_MAX_NUM_ZONES];
	String rule_ids[GENERATION_MAX_NUM_RULES];
	String location_types[GENERATION_MAX_NUM_LOCATION_TYPES];

	LevelGenerationParseContext context;
	context.zone_ids = zone_ids;
	context.rule_ids = rule_ids;
	context.location_types = location_types;
	context.num_zones = 0;
	context.num_rules = 0;
	context.num_location_types = 0;
	context.output = output;
	context.header_output = header_output;
	context.num_objective_group_index = 0;
	context.max_num_steps_per_region_block = 0;
	context.use_step_offset = false;

	output_locations_data(arena, level_locations_settings_array, context);
	output_regions_data(arena, level_regions_settings_array, context);

	for (int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		Settings &level_generation_settings = level_generation_settings_array[level_setting_index];
		String filename = get_filename(level_generation_settings.path);
		ComponentSettings& component_settings = level_generation_settings.component_settings[0];

		static const unsigned steps_id = to_id32(strlen("steps"), "steps");
		HASH_LOOKUP(steps_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), steps_id);
		ASSERT(steps_entry->value, "No steps data in level generation settings %s.", filename.text);
		SettingsData& steps = *steps_entry->value;

		// Create the debug draw functions
		unsigned block_index = 0;
		SettingsDataStore *steps_data = steps.next;
		for(unsigned step_index = 0; step_index < steps_data->count; ++step_index) {
			SettingsDataStore* step = steps_data->entries[step_index].next;
			String step_name = steps_data->entries[step_index].name;
			ASSERT(step->count > 0, "No data information in step %.*s", step_name.length, step_name.text);
			String step_type = step->entries[0].value;
			String step_type_name = step->entries[0].name;
			clean_string_from_quotations(step_type);
			unsigned step_type_id = to_id32(step_type.length, step_type.text);
			PassDefinition *pass_definition = get_pass_definition_from_string_id(step_type, filename, steps_data->entries[step_index].name, step_type_id);
			if(pass_definition->debug_draw_count > 0) {
				fprintf(output, "void debug_draw_step_%.*s_step_%.*s(LevelGenerationDebugDrawContext *context, GeneratorStep *step) {\n", filename.length, filename.text, step_name.length, step_name.text);
				output_debugrender_info(output, pass_definition, step_type);
				fprintf(output, "};\n");
			}
		}

		// Create the fill queue-function.
		fprintf(output, "void %.*s_fill_queue(GeneratorQueue* queue, LevelGenerationResources* resources)\n", filename.length, filename.text);
		fprintf(output, "{\n");

		context.num_objective_group_index = 0;
		String objective_groups[GENERATION_MAX_NUM_OBJECTIVE_GROUPS];
		context.objective_groups = objective_groups;
		parse_and_output_steps(steps_data, level_generation_settings.path, filename, context);

		fprintf(output, "}\n\n");

		fprintf(output, "void %.*s_load_zones(ZoneSettings *zones) {\n", filename.length, filename.text);
		for(unsigned i = 0;i < context.num_zones; ++i) {
			fprintf(output, "\tload_%.*s(zones);\n", zone_ids[i].length, zone_ids[i].text);
		}
		fprintf(output, "}\n");

		fprintf(header_output, "enum %.*s_objective_groups {\n", filename.length, filename.text);
		for(unsigned i = 0; i < context.num_objective_group_index; ++i) {
			fprintf(header_output, "\tOBJECTIVE_GROUP_%.*s_%.*s = %d,\n", filename.length, filename.text, objective_groups[i].length, objective_groups[i].text, i);
		}
		fprintf(header_output, "};\n\n");
	}

	fprintf(output, "typedef void (*level_generation_setting_queue_filler)(GeneratorQueue*, LevelGenerationResources*);\n\n");
	fprintf(output, "level_generation_setting_queue_filler level_generation_settings[] = {\n");
	for (int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		Settings &level_generation_settings = level_generation_settings_array[level_setting_index];
		String filename = get_filename(level_generation_settings.path);
		fprintf(output, "\t%.*s_fill_queue,\n", filename.length, filename.text);
	}
	fprintf(output, "};\n\n");

	fprintf(output, "typedef void (*level_generation_setting_zone_loader)(ZoneSettings*);\n\n");
	fprintf(output, "level_generation_setting_zone_loader level_generation_zone_loaders[] = {\n");
	for (int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		Settings &level_generation_settings = level_generation_settings_array[level_setting_index];
		String filename = get_filename(level_generation_settings.path);
		fprintf(output, "\t%.*s_load_zones,\n", filename.length, filename.text);
	}
	fprintf(output, "};\n\n");

	fprintf(output, "float level_generation_map_dimensions[] = {\n");
	for(int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		fprintf(output, "\t%f,\n", 2048.f);
	}
	fprintf(output, "};\n\n");

	fprintf(output, "float level_generation_map_max_heights[] = {\n");
	for(int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		fprintf(output, "\t%f,\n", 255.f);
	}
	fprintf(output, "};\n\n");

	fprintf(output, "uint64_t level_generation_id_mapping[] = {\n");
	for (int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		Settings &level_generation_settings = level_generation_settings_array[level_setting_index];
		String setting_path = level_generation_settings.path;
		clean_string_from_quotations(setting_path);
		String path_formatted = slash_to_asset_string(setting_path, arena);
		fprintf(output, "\t%.*s,\n", path_formatted.length, path_formatted.text);
	}
	fprintf(output, "};\n");
	fprintf(output, "const char *level_generation_names[] = {\n");
	for (int level_setting_index = 0; level_setting_index < array_count(level_generation_settings_array); ++level_setting_index) {
		Settings &level_generation_settings = level_generation_settings_array[level_setting_index];
		String filename = get_filename(level_generation_settings.path);
		fprintf(output, "\t\"%.*s\",\n", filename.length, filename.text);
	}
	fprintf(output, "};\n\n");

	{
		String resource_output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_resources"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(resource_output, *resource_output_filepath);

		fprintf(resource_output, "enum LevelGenerationLocationType {\n");
		fprintf(resource_output, "\tLOCATION_TYPE_NONE = 0,\n");
		for(unsigned i = 0;i < context.num_location_types; ++i) {
			fprintf(resource_output, "\tLOCATION_TYPE_%.*s,\n", location_types[i].length, location_types[i].text);
		}
		fprintf(resource_output, "};\n\n");
		fclose(resource_output);
	}

	fprintf(header_output, "static const unsigned LARGEST_AMOUNT_OF_LEVEL_GENERATION_STEPS = %d;\n", 256);
	fprintf(header_output, "static const unsigned LEVEL_GENERATION_MAX_NUM_REGIONS_PER_RUN = %d;\n", 4);
	fprintf(header_output, "static const unsigned LEVEL_GENERATION_STEPS_PER_REGION_BLOCK = %d;\n", context.max_num_steps_per_region_block);
	fclose(header_output);
	fclose(output);
}

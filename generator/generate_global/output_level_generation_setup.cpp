

unsigned find_next_letter(String str, unsigned start_offset, char letter) {
	if(str.text[start_offset] == letter)
		return start_offset;
	unsigned offset = start_offset;
	while(str.text[++offset] != letter && offset < str.length) ;
	if(str.text[offset] != letter)
		return 0xffffffff;
	return offset;
}

enum {
	LEVEL_GENERATION_MAX_NUM_RESOURCES = 128,
	LEVEL_GENERATION_MAX_NUM_RESOURCE_TYPES = 128,
	LEVEL_GENERATION_MAX_NUM_CONSTANTS = 128,
};

struct GenerationResourceType {
	String name;
	unsigned name_id;
	char type_name_cap[128];
};

GenerationResourceType generation_resource_types[LEVEL_GENERATION_MAX_NUM_RESOURCE_TYPES];
unsigned generation_resource_types_count;

int find_generator_type(String str) {
	unsigned id = to_id32(str.length, str.text);
	GenerationResourceType *all_types = generation_resource_types;
	unsigned count = generation_resource_types_count;
	for(unsigned i = 0;i < count; ++i) {
		if(all_types[i].name_id == id) {
			return i;
		}
	}
	ASSERT(false, "Failed to find generator type for id %s.", str.text);
	return -1;
}

struct GenerationConstant {
	String name;
	unsigned name_id;
	unsigned value;
};

GenerationConstant generation_resource_constants[LEVEL_GENERATION_MAX_NUM_CONSTANTS];
unsigned generation_resource_constants_count;

struct GenerationResource {
	String name;
	String type;
	unsigned name_id;
	unsigned type_id;
	unsigned count;
};

GenerationResource generation_resources[LEVEL_GENERATION_MAX_NUM_RESOURCES];
unsigned generation_resource_count;

enum {
	GENERATION_MAX_NUM_RESOURCES_PER_PASS = 32,
	GENERATION_MAX_NUM_SETTINGS_PER_PASS = 32,
};

struct GenerationResourceDeclaration {
	String name;
	String type;
	unsigned name_id;
	unsigned type_id;
};

struct PassDefinition {
	String name;
	char name_lower[64];
	unsigned name_id;
	struct {
		GenerationResourceDeclaration resources[GENERATION_MAX_NUM_RESOURCES_PER_PASS];
		unsigned count;
	} input;
	struct {
		GenerationResourceDeclaration resources[GENERATION_MAX_NUM_RESOURCES_PER_PASS];
		unsigned count;
	} output;
	struct {
		String name[GENERATION_MAX_NUM_SETTINGS_PER_PASS];
		String type[GENERATION_MAX_NUM_SETTINGS_PER_PASS];
		unsigned count;
	} settings;
	struct {
		String name;
		String input_data[16];
		String input_data_lookup[16];
		unsigned input_data_count;
	} debug_draw_data[8];
	unsigned debug_draw_count;
	bool multiple_frames;
};

PassDefinition generation_pass_definitions[256];
unsigned num_generation_passes;

struct DebugDrawInfo {
	String input_data[16];
	String input_data_type[16];
	unsigned input_data_count;
	String name;
	unsigned name_id;
} generation_debug_draws[128];

unsigned generation_debug_draw_count;

void verify_debug_draw_functions(SettingsDataStore *settings_array, PassDefinition *definition) {
	for(unsigned i = 0; i < settings_array->count; ++i) {
		SettingsData *debug_draw_settings = &settings_array->entries[i];
		String name = debug_draw_settings->name;
		unsigned id = to_id32(name.length, name.text);

		bool found_debug_draw = false;
		for(unsigned j = 0; j < generation_debug_draw_count; ++j) {
			if(generation_debug_draws[j].name_id == id) {
				found_debug_draw = true;
				continue;
			}
		}
		if(found_debug_draw)
			continue;
		generation_debug_draws[generation_debug_draw_count].name = name;
		generation_debug_draws[generation_debug_draw_count].name_id = id;
		SettingsDataStore *input_array = debug_draw_settings->next;
		generation_debug_draws[generation_debug_draw_count].input_data_count = input_array->count;
		for(unsigned j = 0; j < input_array->count; ++j) {
			generation_debug_draws[generation_debug_draw_count].input_data[j] = input_array->entries[j].name;
			String value_lookup = input_array->entries[j].value;
			clean_string_from_quotations(value_lookup);
			String found_type = { 0, "" };
			for(unsigned input_index = 0; input_index < definition->input.count; ++input_index) {
				if(are_strings_equal(value_lookup, definition->input.resources[input_index].name)) {
					found_type = definition->input.resources[input_index].type;
					break;
				}
			}
			if(found_type.length == 0) {
				for(unsigned output_index = 0; output_index < definition->output.count; ++output_index) {
					if(are_strings_equal(value_lookup, definition->output.resources[output_index].name)) {
						found_type = definition->output.resources[output_index].type;
						break;
					}
				}
			}
			ASSERT(found_type.length > 0, "Couldn't find lookup type.");
			generation_debug_draws[generation_debug_draw_count].input_data_type[j] = found_type;
		}
		generation_debug_draw_count++;
	}
}

String get_pass_settings_type(PassDefinition *definition, String setting_name) {
	for(unsigned i = 0;i < definition->settings.count; ++i) {
		if(are_strings_equal(definition->settings.name[i], setting_name))
			return definition->settings.type[i];
	}
	return make_string("NOT_A_SETTING", 0);
}

PassDefinition *get_pass_definition_from_string_id(String str, String filename, String pass, unsigned id) {
	for(unsigned i = 0;i < num_generation_passes; ++i) {
		if(generation_pass_definitions[i].name_id == id) {
			return &generation_pass_definitions[i];
		}
	}
	ASSERT(false, "Failed to find pass %.*s (%d) in file %.*s pass %.*s",
		str.length, str.text, id, filename.length, filename.text, pass.length, pass.text);
	return nullptr;
}

void ensure_initial_letter_cap(char *destination, String source) {
	memmove(destination, source.text, source.length);
	destination[0] = destination[0] & ~0x20;
}

GenerationResource *find_generator_from_string(String string, const char* filename, String pass) {
	uint64_t id = to_id32(string.length, string.text);
	for(unsigned i = 0;i < generation_resource_count;++i) {
		if(generation_resources[i].name_id == id) {
			return &generation_resources[i];
		}
	}
	ASSERT(false, "Failed to find generator index for id %.*s in filename %s, pass %s.", string.length, string.text, filename, pass.text);
	return nullptr;
}

unsigned find_generator_index_from_id(unsigned id, const char* filename, const char* pass) {
	for(unsigned i = 0;i < generation_resource_count;++i) {
		if(generation_resources[i].name_id == id) {
			return i;
		}
	}
	ASSERT(false, "Failed to find generator index for id %d in filename %s, pass %s.", id, filename, pass);
	return -1;
}

void output_level_generation_setup(SettingsArray &generation_setup_array, MemoryArena &arena) {
	ASSERT(array_count(generation_setup_array) <= 1, "Level Generation only supports 1 generation settings file for setup.");
	Settings &settings = generation_setup_array[0];
	String filename = get_filename(arena, settings.path);

	ComponentSettings& component_settings = settings.component_settings[0];
	{
		static unsigned resource_type_id = to_id32(strlen("resource_types"), "resource_types");
		HASH_LOOKUP(resource_types_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), resource_type_id);

		SettingsDataStore *resource_type_list = resource_types_entry->value->next;
		for(unsigned resource_type_index = 0; resource_type_index < resource_type_list->count; ++resource_type_index) {
				GenerationResourceType *new_type = &generation_resource_types[generation_resource_types_count++];
				ASSERT(generation_resource_types_count < LEVEL_GENERATION_MAX_NUM_RESOURCE_TYPES, "Increase LEVEL_GENERATION_MAX_NUM_RESOURCE_TYPES");
			new_type->name = resource_type_list->entries[resource_type_index].name;
			new_type->name_id = to_id32(new_type->name.length, new_type->name.text);
			ensure_initial_letter_cap(new_type->type_name_cap, new_type->name);
		}
	}

	{
		static unsigned constants_type_id = to_id32(strlen("constants"), "constants");
		HASH_LOOKUP(constants_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), constants_type_id);
		SettingsDataStore *constants = constants_entry->value->next;
		for(unsigned constant_index = 0; constant_index < constants->count; ++constant_index) {
			GenerationConstant *constant = &generation_resource_constants[constant_index];
			constant->name = constants->entries[constant_index].name;
			constant->name_id = constants->entries[constant_index].name_id;
			constant->value = atoi(constants->entries[constant_index].value.text);
		}
		generation_resource_constants_count = constants->count;
	}


	{
		static unsigned generator_resource_id = to_id32(strlen("generator_resources"), "generator_resources");
		static unsigned type_id = to_id32(strlen("type"), "type");
		static unsigned count_id = to_id32(strlen("count"), "count");
		HASH_LOOKUP(generator_resources_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), generator_resource_id);
		SettingsDataStore *resources = generator_resources_entry->value->next;
		for(unsigned resource_index = 0; resource_index < resources->count; ++resource_index) {
			GenerationResource *resource = &generation_resources[resource_index];
			resource->name = resources->entries[resource_index].name;
			resource->name_id = to_id32(resource->name.length, resource->name.text);

			HASH_LOOKUP(type_entry, resources->entries[resource_index].next->map, ARRAY_COUNT(resources->entries[resource_index].next->map), type_id);
			resource->type = type_entry->value->value;
			clean_string_from_quotations(resource->type);
			resource->type_id = to_id32(resource->type.length, resource->type.text);
			HASH_LOOKUP(count_entry, resources->entries[resource_index].next->map, ARRAY_COUNT(resources->entries[resource_index].next->map), count_id);
			String count_str = count_entry->value->value;
			char *end_str = nullptr;
			resource->count = strtol(count_str.text, &end_str, 10);
			if(end_str == count_str.text) {
				// OK count is a constant. Look it up!
				clean_string_from_quotations(count_str);
				unsigned count_str_id = to_id32(count_str.length, count_str.text);
				unsigned constant_index = 0;
				GenerationConstant *all_constants = generation_resource_constants;
				while(all_constants[constant_index].name_id != count_str_id && constant_index < generation_resource_constants_count)
					constant_index++;
				ASSERT(constant_index < generation_resource_constants_count, "Could not find constant %.*s.", count_str.length, count_str.text);
				resource->count = all_constants[constant_index].value;
			}
		}
		ASSERT(resources->count < LEVEL_GENERATION_MAX_NUM_RESOURCES, "increase LEVEL_GENERATION_MAX_NUM_RESOURCES.");
		generation_resource_count = resources->count;
	}

	{ // Parse passes
		static unsigned passes_id = to_id32(strlen("generator_passes"), "generator_passes");
		static unsigned input_id = to_id32(strlen("input"), "input");
		static unsigned output_id = to_id32(strlen("output"), "output");
		static unsigned settings_id = to_id32(strlen("settings"), "settings");
		static unsigned flags_id = to_id32(strlen("flags"), "flags");
		static unsigned debug_draw_id = to_id32(strlen("debug_draw"), "debug_draw");
		HASH_LOOKUP(passes_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), passes_id);
		SettingsDataStore *passes = passes_entry->value->next;
		for(unsigned pass_index = 0; pass_index < passes->count; ++pass_index) {
			PassDefinition *definition = &generation_pass_definitions[pass_index];
			HASH_LOOKUP(input_entry, passes->entries[pass_index].next->map, ARRAY_COUNT(passes->entries[pass_index].next->map), input_id);
			definition->name = passes->entries[pass_index].name;
			definition->name_id = passes->entries[pass_index].name_id;
			ASSERT(definition->name.length < 64, "Too long name in pass!!!");
			for(unsigned i = 0;i < definition->name.length; ++i)
				definition->name_lower[i] = tolower(definition->name.text[i]);

			if(input_entry->value) {
				SettingsDataStore *input_array = input_entry->value->next;
				ASSERT(input_array->count < GENERATION_MAX_NUM_RESOURCES_PER_PASS, "Too many generators in input for pass %.*s",
					definition->name.length, definition->name.text);

				for(unsigned i = 0;i < input_array->count; ++i) {
					SettingsData *input_settings = &input_array->entries[i];
					GenerationResourceDeclaration *declaration = &definition->input.resources[i];
					declaration->name = input_settings->name;
					declaration->name_id = input_settings->name_id;
					declaration->type = input_settings->value;
					clean_string_from_quotations(declaration->type);
					declaration->type_id = to_id32(declaration->type.length, declaration->type.text);
				}
				definition->input.count = input_array->count;
			}
			HASH_LOOKUP(output_entry, passes->entries[pass_index].next->map, ARRAY_COUNT(passes->entries[pass_index].next->map), output_id);
			if(output_entry->value) {
				SettingsDataStore *output_array = output_entry->value->next;
				ASSERT(output_array->count < GENERATION_MAX_NUM_RESOURCES_PER_PASS, "Too many generators in output for pass %.*s",
					definition->name.length, definition->name.text);

				for(unsigned i = 0;i < output_array->count; ++i) {
					SettingsData *output_settings = &output_array->entries[i];
					GenerationResourceDeclaration *declaration = &definition->output.resources[i];
					declaration->name = output_settings->name;
					declaration->name_id = output_settings->name_id;
					declaration->type = output_settings->value;
					clean_string_from_quotations(declaration->type);
					declaration->type_id = to_id32(declaration->type.length, declaration->type.text);
				}
				definition->output.count = output_array->count;
			}
			HASH_LOOKUP(settings_entry, passes->entries[pass_index].next->map, ARRAY_COUNT(passes->entries[pass_index].next->map), settings_id);
			if(settings_entry->value) {
				SettingsDataStore *settings_array = settings_entry->value->next;
				ASSERT(settings_array->count < GENERATION_MAX_NUM_SETTINGS_PER_PASS, "Too many settings in pass %.*s",
					definition->name.length, definition->name.text);

				for(unsigned i = 0;i < settings_array->count; ++i) {
					SettingsData *settings_settings = &settings_array->entries[i];
					definition->settings.name[i] = settings_settings->name;
					definition->settings.type[i] = settings_settings->value;
					clean_string_from_quotations(definition->settings.type[i]);
				}
				definition->settings.count = settings_array->count;
			}
			HASH_LOOKUP(flags_entry, passes->entries[pass_index].next->map, ARRAY_COUNT(passes->entries[pass_index].next->map), flags_id);
			if(flags_entry->value) {
				SettingsDataStore *settings_array = flags_entry->value->next;
				static unsigned multiple_frames_id = to_id32(strlen("multiple_frames"), "multiple_frames");
				unsigned num_flags_parsed = 0;
				HASH_LOOKUP(multiple_frames_entry, settings_array->map, ARRAY_COUNT(settings_array->map), multiple_frames_id);
				if(multiple_frames_entry) {
					definition->multiple_frames = true;
					num_flags_parsed++;
				}
				ASSERT(num_flags_parsed == settings_array->count, "Invalid flags in pass definition %.*s. Please fix.", definition->name.length, definition->name.text);
			}

			HASH_LOOKUP(debug_draw_entry, passes->entries[pass_index].next->map, ARRAY_COUNT(passes->entries[pass_index].next->map), debug_draw_id);
			if(debug_draw_entry->value) {
				verify_debug_draw_functions(debug_draw_entry->value->next, definition);
				SettingsDataStore *settings_array = debug_draw_entry->value->next;
				definition->debug_draw_count = settings_array->count;
				for(unsigned i = 0; i < settings_array->count; ++i) {
					SettingsData *debug_draw_settings = &settings_array->entries[i];
					String name = debug_draw_settings->name;
					definition->debug_draw_data[i].name = name;
					SettingsDataStore *input_array = debug_draw_settings->next;
					definition->debug_draw_data[i].input_data_count = input_array->count;
					for(unsigned j = 0; j < input_array->count; ++j) {
						definition->debug_draw_data[i].input_data[j] = input_array->entries[j].name;
						definition->debug_draw_data[i].input_data_lookup[j] = input_array->entries[j].value;
						clean_string_from_quotations(definition->debug_draw_data[i].input_data_lookup[j]);
					}
				}
			}
		}
		num_generation_passes = passes->count;
	}

	{
		String road_type_output_filepath = make_filepath(arena, _folder_root, _folder_generated_types, MAKE_STRING("road_type"), MAKE_STRING(".type"));
		MAKE_OUTPUT_FILE(road_output, *road_type_output_filepath);
		String level_generation_types_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_types"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(types_output, *level_generation_types_filepath);
		fprintf(road_output, "// Automatically generated file\n");
		fprintf(road_output, "export = \"#enum_road_types\"\n");
		fprintf(road_output, "types = {\n");
		fprintf(road_output, "\tenum_road_types ={\n");
		fprintf(road_output, "\t\ttype = \":enum\"\n");
		fprintf(road_output, "\t\teditor = {\n");
		fprintf(road_output, "\t\t\tcontrol = \"Choice\"\n");
		fprintf(road_output, "\t\t\tcase_labels = {\n");

		static unsigned road_types_id = to_id32(strlen("road_types"), "road_types");
		HASH_LOOKUP(road_types_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), road_types_id);
		fprintf(types_output, "enum RoadType {\n");
		SettingsDataStore *road_types_list = road_types_entry->value->next;
		for(unsigned road_type_index = 0; road_type_index < road_types_list->count; ++road_type_index) {
			String road_type_name = road_types_list->entries[road_type_index].name;
			clean_string_from_quotations(road_type_name);
			fprintf(road_output, "\t\t\t\t\"RoadType_%.*s\" = \"%.*s\"\n", road_type_name.length, road_type_name.text,
				road_type_name.length, road_type_name.text);
			fprintf(types_output, "\tRoadType_%.*s = %d,\n", road_type_name.length, road_type_name.text, road_type_index);
		}
		fprintf(types_output, "\tRoadType_NumRoadTypes = %d,\n", road_types_list->count);
		fprintf(types_output, "};\n\n");
		fprintf(road_output, "\t\t\t}\n");
		fprintf(road_output, "\t\t}\n");
		fprintf(road_output, "\t\tcases = [\n");
		for(unsigned road_type_index = 0; road_type_index < road_types_list->count; ++road_type_index) {
			String road_type_name = road_types_list->entries[road_type_index].name;
			clean_string_from_quotations(road_type_name);
			fprintf(road_output, "\t\t\t\"RoadType_%.*s\"\n", road_type_name.length, road_type_name.text);
		}
		fprintf(road_output, "\t\t]\n");
		String road_type_name = road_types_list->entries[0].name;
		clean_string_from_quotations(road_type_name);
		fprintf(road_output, "\t\tdefault = \"RoadType_%.*s\"\n", road_type_name.length, road_type_name.text);
		fprintf(road_output, "\t}\n");
		fprintf(road_output, "}\n");
		fclose(road_output);

		fprintf(types_output, "struct RoadSettings {\n");
		fprintf(types_output, "\tfloat slope_multiplier;\n");
		fprintf(types_output, "\tfloat road_width_radius;\n");
		fprintf(types_output, "\tfloat multiplier_when_painting_road_into_terrain;\n");
		fprintf(types_output, "\tfloat mesh_width_multiplier;\n");
		fprintf(types_output, "\tfloat height_multiplier;\n");
		fprintf(types_output, "\tunsigned road_material_index;\n");
		fprintf(types_output, "\tfloat width_multiplier_for_material_paint;\n");
		fprintf(types_output, "\tIdString64 mesh;\n");
		fprintf(types_output, "};\n\n");
		fclose(types_output);
	}
	// Output structs.
	{
		String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("level_generation_pass_declarations"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(output, *output_filepath);
		fprintf(output, "struct VoronoiCorner;\nstruct VoronoiEdge;\n");
		fprintf(output, "typedef VoronoiCorner* VoronoiCornerPointer;\n");
		fprintf(output, "typedef VoronoiEdge* VoronoiEdgePointer;\n");
		fprintf(output, "typedef unsigned StampGroupId;\n");
		fprintf(output, "typedef unsigned ObjectiveGroupId;\n\n");

		static unsigned road_types_id = to_id32(strlen("road_types"), "road_types");
		HASH_LOOKUP(road_types_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), road_types_id);
		fprintf(output, "RoadSettings level_generation_road_settings[] = {\n");
		SettingsDataStore *road_types_list = road_types_entry->value->next;
		for(unsigned road_type_index = 0; road_type_index < road_types_list->count; ++road_type_index) {
			SettingsData *road_settings = &road_types_list->entries[road_type_index];
			String road_type_name = road_types_list->entries[road_type_index].name;
			float slope_multiplier = 1.f;
			float road_width_radius = 1.f;
			float multiplier_when_painting_road_into_terrain = 1.f;
			float mesh_width_multiplier = 1.f;
			float height_multiplier = 1.f;
			unsigned road_material_index = 1;
			float width_multiplier_for_material_paint = 1.f;
			static const unsigned slope_multiplier_id = STATIC_ID32("slope_multiplier");
			static const unsigned road_width_radius_id = STATIC_ID32("road_width_radius");
			static const unsigned multiplier_when_painting_road_into_terrain_id = STATIC_ID32("multiplier_when_painting_road_into_terrain");
			static const unsigned mesh_width_multiplier_id = STATIC_ID32("mesh_width_multiplier");
			static const unsigned height_multiplier_id = STATIC_ID32("height_multiplier");
			static const unsigned road_material_index_id = STATIC_ID32("road_material_index");
			static const unsigned mesh_id = STATIC_ID32("mesh");
			static const unsigned width_multiplier_for_material_paint_id = STATIC_ID32("width_multiplier_for_material_paint");
			find_and_parse_float(road_settings, slope_multiplier_id, slope_multiplier);
			find_and_parse_float(road_settings, road_width_radius_id, road_width_radius);
			find_and_parse_float(road_settings, mesh_width_multiplier_id, mesh_width_multiplier);
			find_and_parse_float(road_settings, multiplier_when_painting_road_into_terrain_id, multiplier_when_painting_road_into_terrain);
			find_and_parse_float(road_settings, height_multiplier_id, height_multiplier);
			find_and_parse_unsigned(road_settings, road_material_index_id, road_material_index);
			find_and_parse_float(road_settings, width_multiplier_for_material_paint_id, width_multiplier_for_material_paint);
			String mesh;
			find_and_parse_idstring(road_settings, mesh_id, mesh);

			fprintf(output, "\t{ %f, %f, %f, %f, %f, %d, %f, IdString64(%d, \"%.*s\") }, // %.*s\n",
				slope_multiplier, road_width_radius, multiplier_when_painting_road_into_terrain, mesh_width_multiplier, height_multiplier,
				road_material_index, width_multiplier_for_material_paint, mesh.length, mesh.length, mesh.text,
				road_type_name.length, road_type_name.text);
		}
		fprintf(output, "};\n\n");

		for(unsigned debug_draw_index = 0; debug_draw_index < generation_debug_draw_count; ++debug_draw_index) {

			fprintf(output, "struct LevelGenerationDebugDrawInput_%.*s {\n", generation_debug_draws[debug_draw_index].name.length,
				generation_debug_draws[debug_draw_index].name.text);
			for(unsigned i = 0; i < generation_debug_draws[debug_draw_index].input_data_count; ++i) {
				String input_data_name = generation_debug_draws[debug_draw_index].input_data[i];
				String type_name = generation_debug_draws[debug_draw_index].input_data_type[i];
				fprintf(output, "\tconst %.*s *%.*s;\n", type_name.length, type_name.text, input_data_name.length, input_data_name.text);
				fprintf(output, "\tunsigned %.*s_count;\n", input_data_name.length, input_data_name.text);
				fprintf(output, "\tunsigned %.*s_max;\n", input_data_name.length, input_data_name.text);

			}
			fprintf(output, "};\n");
		}
		for(unsigned pass_index = 0; pass_index < num_generation_passes; ++pass_index) {
			PassDefinition *definition = &generation_pass_definitions[pass_index];
			if(definition->input.count > 0) {
				fprintf(output, "struct %.*s_Input {\n", definition->name.length, definition->name.text);
				for(unsigned input_index = 0;input_index < definition->input.count; ++input_index) {
					fprintf(output, "	const %.*s *%.*s;\n",
						definition->input.resources[input_index].type.length, definition->input.resources[input_index].type.text,
						definition->input.resources[input_index].name.length, definition->input.resources[input_index].name.text);
				}
				for(unsigned input_index = 0;input_index < definition->input.count; ++input_index) {
					fprintf(output, "	const unsigned *%.*s_count;\n", definition->input.resources[input_index].name.length, definition->input.resources[input_index].name.text);
					fprintf(output, "	unsigned %.*s_max;\n", definition->input.resources[input_index].name.length, definition->input.resources[input_index].name.text);
				}
				fprintf(output, "};\n\n");
			}
			if(definition->output.count > 0) {
				fprintf(output, "struct %.*s_Output {\n", definition->name.length, definition->name.text);
				for(unsigned output_index = 0;output_index < definition->output.count; ++output_index) {
					fprintf(output, "	%.*s *%.*s;\n",
						definition->output.resources[output_index].type.length, definition->output.resources[output_index].type.text,
						definition->output.resources[output_index].name.length, definition->output.resources[output_index].name.text);
				}
				for(unsigned output_index = 0;output_index < definition->output.count; ++output_index) {
					String name = definition->output.resources[output_index].name;
					fprintf(output, "	unsigned *%.*s_count;\n", name.length, name.text);
					fprintf(output, "	unsigned %.*s_max;\n", name.length, name.text);
					fprintf(output, "	void set_%.*s_count(unsigned new_count) {\n", name.length, definition->output.resources[output_index].name.text);
					fprintf(output, "		ASSERT(%.*s_max >= new_count, \"ERROR: Trying to assign too high value to %.*s (Max is %%d, trying to set to %%d)\", %.*s_max, new_count);\n",
						name.length, name.text, name.length, name.text, name.length, name.text);
					fprintf(output, "		*%.*s_count = new_count;\n", name.length, name.text);
					fprintf(output, "	}\n");
				}
				fprintf(output, "};\n\n");
			}
			fprintf(output, "struct %.*s {\n", definition->name.length, definition->name.text);

			if(definition->input.count > 0)
				fprintf(output, "	%.*s_Input input;\n", definition->name.length, definition->name.text);

			if(definition->output.count > 0)
				fprintf(output, "	%.*s_Output output;\n", definition->name.length, definition->name.text);

			if(definition->settings.count > 0) {
				for(unsigned setting_index = 0;setting_index < definition->settings.count; ++setting_index) {
					fprintf(output, "	%.*s %.*s;\n",
						definition->settings.type[setting_index].length, definition->settings.type[setting_index].text,
						definition->settings.name[setting_index].length, definition->settings.name[setting_index].text);
				}
			}
			fprintf(output, "	void execute(GeneratorStep *step, GeneratorStepContext *context);\n");
			if(definition->multiple_frames) {
				fprintf(output, "	bool update(GeneratorStep *step, GeneratorStepContext *context);\n");
			}
			fprintf(output, "};\n\n");
		}

		fprintf(output, "void print_level_generation_resource_sizes() {\n");
		for(unsigned i = 1; i < generation_resource_types_count; ++i) {
			fprintf(output, "\tLOG_INFO(\"LevelGeneration\", \"%s : %%d\", sizeof(%s));\n", generation_resource_types[i].name.text, generation_resource_types[i].name.text);
		}
		fprintf(output, "}\n\n");
		fclose(output);
	}
}


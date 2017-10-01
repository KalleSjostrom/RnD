
unsigned NUM_DISCRIMINATION_SETTINGS = 0;
static const unsigned MAX_NUM_DISCRIMINATION_SETTINGS = 16;
const char* discrimination_setting_names[MAX_NUM_DISCRIMINATION_SETTINGS] = {};
const char* discrimination_setting_types[MAX_NUM_DISCRIMINATION_SETTINGS] = {};
const char* discrimination_setting_default_value[MAX_NUM_DISCRIMINATION_SETTINGS] = {};

#define ADD_DISCRIMINATION_SETTING(NAME, TYPE, DEFAULT_VALUE) \
	discrimination_setting_names[NUM_DISCRIMINATION_SETTINGS] = NAME; \
	discrimination_setting_types[NUM_DISCRIMINATION_SETTINGS] = TYPE; \
	discrimination_setting_default_value[NUM_DISCRIMINATION_SETTINGS++] = #DEFAULT_VALUE;

struct static_struct_to_run_for_stamp_generation {
	static_struct_to_run_for_stamp_generation() {
		ADD_DISCRIMINATION_SETTING("minimum_slope",				"float", 0.f)
		ADD_DISCRIMINATION_SETTING("maximum_slope",				"float", 90.f)
		ADD_DISCRIMINATION_SETTING("minimum_allowed_elevation",	"float", 0.f)
		ADD_DISCRIMINATION_SETTING("maximum_allowed_elevation",	"float", 255.f)
	}
};
static static_struct_to_run_for_stamp_generation my_lol;

void output_stamp_settings(SettingsArray &stamp_settings_array, MemoryArena &arena) {
	{
		String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("stamp_settings"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(output, *output_filepath);
fprintf(output, "// Automatically generated file\n\n");

fprintf(output, "struct StampCircle {\n");
fprintf(output, "	Vector2 position;\n");
fprintf(output, "	float radius;\n");
fprintf(output, "	unsigned userdata;\n");
fprintf(output, "};\n\n");

fprintf(output, "struct StampDiscriminationSettings {\n");
fprintf(output, "	float minimum_slope;\n");
fprintf(output, "	float maximum_slope;\n");
fprintf(output, "	float minimum_allowed_elevation;\n");
fprintf(output, "	float maximum_allowed_elevation;\n");
fprintf(output, "};\n\n");

fprintf(output, "struct Stamp {\n");
fprintf(output, "	StampDiscriminationSettings settings;\n");
fprintf(output, "	StampRotationType rotation_type;\n");
fprintf(output, "	StampCircle circles[8];\n");
fprintf(output, "	uint8_t num_circles;\n");
fprintf(output, "	bool absolute_height;\n");
fprintf(output, "	Vector3 road_local_position[4];\n");
fprintf(output, "	Vector3 road_forward_direction[4];\n");
fprintf(output, "	RoadType road_type[4];\n");
fprintf(output, "	unsigned num_roads;\n");
fprintf(output, "};\n\n");

fprintf(output, "struct StampDiscriminationContext {\n");
fprintf(output, "	float height;\n");
fprintf(output, "	float slope;\n");
fprintf(output, "};\n\n");

fprintf(output, "struct StampLookup {\n");
fprintf(output, "	Stamp stamps[%d];\n", array_count(stamp_settings_array));
fprintf(output, "	uint64_t path_ids[%d];\n", array_count(stamp_settings_array));
fprintf(output, "	unsigned num_stamps;\n");
fprintf(output, "};\n\n");

fprintf(output, "struct StampSegment {\n");
fprintf(output, "	Vector2 start;\n");
fprintf(output, "	Vector2 end;\n");
fprintf(output, "	float radius;\n");
fprintf(output, "	unsigned userdata;\n");
fprintf(output, "};\n\n");

fprintf(output, "struct StampCollision {\n");
fprintf(output, "	Vector2 position;\n");
fprintf(output, "	float radius;\n");
fprintf(output, "	unsigned userdata;\n");
fprintf(output, "};\n");


fprintf(output, "void load_all_stamps(StampLookup& lookup);\n\n");
fprintf(output, "Stamp *get_stamp(StampLookup& lookup, uint64_t path_id);\n");

		fclose(output);
	}

	{
		String output_filepath = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("stamp_settings"), MAKE_STRING(".generated.cpp"));
		MAKE_OUTPUT_FILE(output, *output_filepath);

fprintf(output, "// Automatically generated file\n\n");

fprintf(output, "void load_all_stamps(StampLookup& lookup) {\n");
fprintf(output, "\tlookup.num_stamps = %u;\n", array_count(stamp_settings_array));
fprintf(output, "\n");

		for (int stamp_i = 0; stamp_i < array_count(stamp_settings_array); ++stamp_i) {
			Settings &stamp_settings = stamp_settings_array[stamp_i];
fprintf(output, "\tlookup.path_ids[%d] = 0x%016llx; // %.*s\n", stamp_i, stamp_settings.path_id, stamp_settings.path.length, stamp_settings.path.text);
		}

		for (int stamp_i = 0; stamp_i < array_count(stamp_settings_array); ++stamp_i) {
			Settings &stamp_settings = stamp_settings_array[stamp_i];
			fprintf(output, "\n");

			fprintf(output, "\tlookup.stamps[%d] = {\n", stamp_i);

			ComponentSettings& component_settings = stamp_settings.component_settings[0];

			HASH_LOOKUP(discrimination_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), to_id32(strlen("discrimination_rules"), "discrimination_rules"));
			if(discrimination_entry->value) {
				SettingsDataStore *discrimination_settings = discrimination_entry->value->next;
				fprintf(output, "\t\t{ // settings\n");
				for(unsigned setting_index = 0; setting_index < NUM_DISCRIMINATION_SETTINGS; ++setting_index) {
					const char* setting_name = discrimination_setting_names[setting_index];
					HASH_LOOKUP(entry, discrimination_settings->map, ARRAY_COUNT(discrimination_settings->map), to_id32(strlen(setting_name), setting_name));
					if(entry->value) {
						fprintf(output, "\t\t\t%.*s, // %s\n", entry->value->value.length, entry->value->value.text, setting_name);
					} else {
						fprintf(output, "\t\t\t%s, // %s\n", discrimination_setting_default_value[setting_index], setting_name);
					}
				}
			} else {
				fprintf(output, "\t\t{ // settings(default)\n");
				for(unsigned setting_index = 0; setting_index < NUM_DISCRIMINATION_SETTINGS; ++setting_index) {
					fprintf(output, "\t\t\t%s, // %s\n", discrimination_setting_default_value[setting_index], discrimination_setting_names[setting_index]);
				}
			}
			fprintf(output, "\t\t},\n");

			HASH_LOOKUP(rotation_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), to_id32(strlen("rotation_type"), "rotation_type"));
			if(rotation_entry->value) {
				String rotation_type = remove_quotation_marks(rotation_entry->value->value);
				fprintf(output, "\t\t%.*s,\n", rotation_type.length, rotation_type.text);
			} else {
				String rotation_type = MAKE_STRING("StampRotationType_random");
				fprintf(output, "\t\t%.*s,\n", rotation_type.length, rotation_type.text);
			}

			HASH_LOOKUP(bounding_circles_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), to_id32(strlen("bounding_circles"), "bounding_circles"));
			if(bounding_circles_entry->value) {
				SettingsDataStore *bounding_circles = bounding_circles_entry->value->next;
				ASSERT(bounding_circles->count <= 8, "Too many stamp positions in entity %s!", stamp_settings.path.text);
				fprintf(output, "\t\t{ // circles\n");
				for(unsigned store_i = 0; store_i < bounding_circles->count; ++store_i) {
					SettingsDataStore* circle_data = bounding_circles->entries[store_i].next;
					ASSERT(are_strings_equal(circle_data->entries[0].name, make_string("position")), "Unexpected circle field!");
					ASSERT(are_strings_equal(circle_data->entries[1].name, make_string("radius")), "Unexpected circle field!");

					// Get position string, which is a vector3, remove last part to make it vector2!!!!1.
					String position_string = circle_data->entries[0].value;
					int element_counter = trim_stingray_array_inplace(position_string);
					for (int find_index = position_string.length; find_index > 0; --find_index) {
						if (position_string.text[find_index] == ',') {
							position_string.text[find_index] = '\0';
							position_string.length = find_index;
							break;
						}
					}

					String radius_string = circle_data->entries[1].value;
					fprintf(output, "\t\t\t{ {%.*s}, %.*s },\n", position_string.length, position_string.text, radius_string.length, radius_string.text);
				}
				fprintf(output, "\t\t},\n");
				fprintf(output, "\t\t%d,\n", bounding_circles->count); // num_circles
			}
			else {
				fprintf(output, "\t\t{}, // circles,\n");
				fprintf(output, "\t\t0, // num_circles\n"); // num_circles
			}

			HASH_LOOKUP(absolute_height_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), to_id32(strlen("absolute_height"), "absolute_height"));
			String absolute_height_value = absolute_height_entry->value ? absolute_height_entry->value->value : MAKE_STRING("false");
			fprintf(output, "\t\t%.*s, //absolute height\n", absolute_height_value.length, absolute_height_value.text);

			HASH_LOOKUP(road_entrypoint_entry, component_settings.settings_data_store->map, ARRAY_COUNT(component_settings.settings_data_store->map), to_id32(strlen("road_entrypoints"), "road_entrypoints"));
			if(road_entrypoint_entry->value) {
				SettingsDataStore *road_data = road_entrypoint_entry->value->next;
				ASSERT(road_data->count <= 4, "Too many road entrty points in stamp %s!", stamp_settings.path.text);
				fprintf(output, "\t\t{ // positions\n");
				for(unsigned road_i = 0;road_i < road_data->count; ++road_i) {
					HASH_LOOKUP(position_entry, road_data->entries[road_i].next->map, ARRAY_COUNT(road_data->entries[road_i].next->map), to_id32(strlen("position"), "position"));
					if(position_entry->value) {
						String position_string = position_entry->value->value;

						int element_counter = trim_stingray_array_inplace(position_string);
						fprintf(output, "\t\t\t{%.*s},\n", position_string.length, position_string.text);

					} else {
						fprintf(output, "\t\t\t{ 0, 0, 0 },\n");
					}
				}
				fprintf(output, "\t\t},\n");
				fprintf(output, "\t\t{ // rotations\n");
				static const unsigned rotation_id = to_id32(strlen("rotation"), "rotation");
				for(unsigned road_i = 0;road_i < road_data->count; ++road_i) {
					HASH_LOOKUP(rotation_entry, road_data->entries[road_i].next->map, ARRAY_COUNT(road_data->entries[road_i].next->map), rotation_id);
					if(rotation_entry->value) {
						String rotation_string = rotation_entry->value->value;
						int element_counter = trim_stingray_array_inplace(rotation_string);
						fprintf(output, "\t\t\t{%.*s},\n", rotation_string.length, rotation_string.text);
					} else {
						fprintf(output, "\t\t{ 0, 1, 0 },\n");
					}
				}
				fprintf(output, "\t\t},\n");
				fprintf(output, "\t\t{ // road types\n");
				static const unsigned road_type_id = to_id32(strlen("road_type"), "road_type");
				for(unsigned road_i = 0;road_i < road_data->count; ++road_i) {
					HASH_LOOKUP_STATIC(road_type_entry, road_data->entries[road_i].next->map, road_type_id);
					if(road_type_entry->value) {
						String road_type_string = road_type_entry->value->value;
						clean_string_from_quotations(road_type_string);
						fprintf(output, "\t\t\t%.*s,\n", road_type_string.length, road_type_string.text);
					} else {
						fprintf(output, "\t\t\tRoadType_highway,\n");
					}
				}
				fprintf(output, "\t\t},\n");
				fprintf(output, "\t\t%d, // count\n", road_data->count);
			} else {
				fprintf(output, "\t\t{ }, // positions\n");
				fprintf(output, "\t\t{ }, // rotations\n");
				fprintf(output, "\t\t{ }, // road types\n");
				fprintf(output, "\t\t0, // count\n");
			}

			fprintf(output, "\t};\n");
		}
fprintf(output, "}\n");

fprintf(output, "\n");

fprintf(output, "Stamp *get_stamp(StampLookup& lookup, uint64_t path_id) {\n");
fprintf(output, "\tuint64_t* path_ids = lookup.path_ids;\n");
fprintf(output, "\tfor(unsigned i=0; i < lookup.num_stamps; ++i) {\n");
fprintf(output, "\t\tif (path_ids[i] == path_id)\n");
fprintf(output, "\t\t\treturn &lookup.stamps[i];\n");
fprintf(output, "\t}\n");
fprintf(output, "\n");
fprintf(output, "\tif(path_id != 0)");
fprintf(output, "\t\tLOG_ERROR(\"LevelGeneration\", \"No stamp data for path id 0x%%016llx\", path_id);\n");
fprintf(output, "\treturn nullptr;\n");
fprintf(output, "}\n\n");
fprintf(output, "bool validate_stamp_discrimination(const StampDiscriminationSettings *settings, const StampDiscriminationContext *context) {\n");
fprintf(output, "\tif(context->height < settings->minimum_allowed_elevation || context->height > settings->maximum_allowed_elevation)\n\t\treturn false;\n");
fprintf(output, "\tif(context->slope < settings->minimum_slope || context->slope > settings->maximum_slope)\n\t\treturn false;\n");
fprintf(output, "\treturn true;\n");
fprintf(output, "}\n\n");
		fclose(output);
	}
}

unsigned valid_network_types[] = { float_id32, bool_id32, int_id32, Vector3_id32, Quaternion_id32 };

void output_member_type(FILE *output, String &name_stem, Member &member, MemoryArena &arena) {
	if (member.exported_line_counter > 0) {
		// Only allow types that are supported by the network config
		// TODO(bauer): Add more thorough checks with default params (i.e using 'unsigned' would force 'min' to 0, using 'char' would force 'bits' to 8, etc)
		bool is_valid = false;
		for(unsigned i = 0; i < ARRAY_COUNT(valid_network_types); ++i) {
			if(member.type_id == valid_network_types[i]) {
				is_valid = true;
				break;
			}
		}
		ASSERT(is_valid, "Invalid network type: '%s %s'", member.type.text, member.name.text);

		// TODO(kalle): We are makeing lower_case_type for members in serveral places here, always store it for members?
		String lower_case_type = make_underscore_case(member.type, arena);
fprintf(output,  "	%s_%s = {\n", *name_stem, *member.name);
fprintf(output,  "		type = \"%s\"\n", *lower_case_type);
		for (int m = 0; m < member.exported_line_counter; m++) {
			String &line = member.exported_lines[m];
			if (starts_with(line, MAKE_STRING("bits")) ||
				starts_with(line, MAKE_STRING("min")) ||
				starts_with(line, MAKE_STRING("max")) ||
				starts_with(line, MAKE_STRING("tolerance"))) {
fprintf(output,  "		%s\n", *line);
			}
		}
fprintf(output,  "	}\n");
	}
}

// Output game objects to the network config
void output_network_config(ComponentArray &component_array, RPCArray &rpc_array, NetworkTypeArray &network_type_array, GameObjectArray &go_array, MemoryArena &temp_arena, MemoryArena &arena) {
	{
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/generated.network_config");

fprintf(output,  "network_type = \"peer-to-peer\"\n");
fprintf(output,  "reliable_send_buffer_size    = 32768\n");
fprintf(output,  "reliable_receive_buffer_size = 32768\n");
fprintf(output,  "\n");

		{ // types
fprintf(output,  "types = {\n");

// Default generic types
fprintf(output,  "	// Default types. TODO(kalle): Remove these and force rpcs to use specific types?\n");
fprintf(output,  "	bool = { type = \"bool\" }\n");
fprintf(output,  "	int = { type = \"int\", bits = 32 }\n");
fprintf(output,  "	unsigned = { type = \"int\", bits = 32 }\n");
fprintf(output,  "	float = { type = \"float\", min = -10000, max = 10000, bits = 31 }\n");
fprintf(output,  "	vector3 = { type = \"vector3\", bits = 31 }\n");
fprintf(output,  "	quaternion = { type = \"quaternion\", bits = 31 }\n");
fprintf(output,  "\n");

// Foundation types
fprintf(output,  "	// Foundation / C API types. TODO(kalle): Not sure how to generate these? Parse the c_api_types.h?\n");
fprintf(output,  "	entity_ref = { type = \"int\", min = 0, max = 65535 }\n");
fprintf(output,  "	game_object_id = { type = \"int\", min = 0, max = 65535 }\n");
fprintf(output,  "	peer_id = { type = \"int64\" }\n");
fprintf(output,  "	id_string32 = { type = \"int\", min = 0, bits = 32 }\n");
fprintf(output,  "	id_string64 = { type = \"int64\", min = 0, bits = 64 }\n");
fprintf(output,  "\n");

// Component types
fprintf(output,  "	// Generated component types\n");
			for (int i = 0; i < component_array.count; ++i) {
				Component *component = component_array.entries + i;
				SubCompStruct *sub_comps = component->sub_comps;
				if (HAS_SUB_COMP(NETWORK)) {
					SubCompStruct &network = sub_comps[NETWORK];
					for (int k = 0; k < network.member_count; k++) {
						output_member_type(output, component->stem, network.members[k], arena);
					}
				}
			}
fprintf(output,  "\n");
fprintf(output,  "	// Generated types\n");
			for (int i = 0; i < network_type_array.count; ++i) {
				NetworkType &network_type = network_type_array.entries[i];
				if (network_type.is_array) {
					output_member_type(output, network_type.name_lower_case, network_type.member, arena);
				}
			}

			for (int i = 0; i < network_type_array.count; ++i) {
				NetworkType &network_type = network_type_array.entries[i];
fprintf(output,  "	%s = {\n", *network_type.name_lower_case);
fprintf(output,  "		type = \"%s\"\n", *network_type.type);
				if (network_type.is_array) {
					Member &member = network_type.member;
					// TODO(kalle): We are makeing lower_case_type for members in serveral places here, always store it for members?
					String lower_case_type = make_underscore_case(member.type, arena);
fprintf(output,  "		element = \"%s\"\n", *lower_case_type);
fprintf(output,  "		max_size = %s\n", *network_type.max_size);
				} else {
fprintf(output,  "		min = 0\n");
fprintf(output,  "		max = %d\n", network_type.count-1);
				}
fprintf(output,  "	}\n");
			}
fprintf(output,  "}\n");
		}

		{ // messages
fprintf(output,  "messages = {\n");
			for (int i = 0; i < rpc_array.count; ++i) {
				RPCFunction &rpc = rpc_array.entries[i];
				Function &function = rpc.function;
fprintf(output,  "	%s = {\n", *function.name);
				if (function.parameter_count > 1) {
fprintf(output,  "		args = [\n");
					for (unsigned j = 1; j < function.parameter_count; ++j) {
						Parameter &parameter = function.parameters[j];
fprintf(output,  "			\"%s\" // %s\n", *make_underscore_case(parameter.type, arena), *parameter.name);
					}
fprintf(output,  "		]\n");
				}
				if (rpc.session_bound)
fprintf(output,  "		session_bound = true\n");
fprintf(output,  "	}\n");
			}
fprintf(output,  "}\n");
		}

		{ // objects
fprintf(output,  "objects = {\n");
			for (int i = 0; i < go_array.count; ++i) {
				GameObject &game_object = go_array.entries[i];

fprintf(output,  "	%s = {\n", *game_object.name);
fprintf(output,  "		fields = [\n");

				Settings *settings = game_object.settings;
				for (int j = 0; j < settings->component_settings_count; ++j) {
					ComponentSettings &component_settings = settings->component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP(NETWORK) && sub_comps[NETWORK].member_count > 0) {
						SubCompStruct &network = sub_comps[NETWORK];
fprintf(output,  "			// %s\n", *component.stem);
						for (int k = 0; k < network.member_count; k++) {
							Member &member = network.members[k];
							if (member.exported_line_counter > 0) {
fprintf(output,  "			{ name=\"%s\", type=\"%s_%s\" }\n", *member.name, *component.stem, *member.name);
							} else {
								// TODO(kalle): We are makeing lower_case_type for members in serveral places here, always store it for members?
								String lower_case_type = make_underscore_case(member.type, arena);
fprintf(output,  "			{ name=\"%s\", type=\"%s\" }\n", *member.name, *lower_case_type);
							}
						}
					}
				}
fprintf(output,  "		]\n");
fprintf(output,  "	}\n");
			}
fprintf(output,  "}\n");
		}

		fclose(output);
	}
}
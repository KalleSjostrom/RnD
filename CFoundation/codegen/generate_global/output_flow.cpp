
void write_case_statement(FILE *output, Function &function, String &name, int optional_index, bool suppress_default_output_events) {
	{
fprintf(output, "			case %s_id32: {\n", *name);
fprintf(output, "				%s::Arguments arguments;\n", *function.name);
fprintf(output, "				%s::read_arguments(message, arguments);\n", *function.name);
fprintf(output, "				%s::__global_last_message = &message;\n", *function.name);
fprintf(output, "				%s::__global_last_arguments = &arguments;\n", *function.name);
fprintf(output, "\n");
		if (suppress_default_output_events)
fprintf(output, "				trigger_default_output_events = false;\n");

		if (optional_index >= 0) {
fprintf(output, "				event_delegate->trigger_%s(%d, ", *function.name, optional_index);
		} else {
fprintf(output, "				event_delegate->trigger_%s(", *function.name);
		}
		for (unsigned j = optional_index >= 0 ? 1 : 0; j < function.parameter_count; ++j) {
			Parameter &parameter = function.parameters[j];
			fprintf(output, "arguments.%s", *parameter.name);
			if (j < function.parameter_count-1)
				fprintf(output, ", ");
		}
		fprintf(output, ");\n");
fprintf(output, "			} break;\n");
	}
}

String make_pretty_flow_name(String function_name, MemoryArena &arena) {
	String result = clone_string(function_name, arena);
	result.length -= sizeof("flow_") - 1; // subtract the null terminator!
	result.text += sizeof("flow_") - 1; // subtract the null terminator!
	prettify(result);
	return result;
}

void output_flow(FlowArray &flow_array, MemoryArena &arena) {
	{ // Prepend the function name to the mapped names
		for (int i = 0; i < flow_array.count; ++i) {
			FlowFunction &flow = flow_array.entries[i];
			Function &function = flow.function;
			for (int j = 0; j < flow.function_map_count; ++j) {
				String &name = flow.function_map[j];
				String full_name = allocate_string(arena, function.name.length + 1 + name.length + 1);
				append_string(full_name, function.name);
				append_string(full_name, MAKE_STRING("_"));
				append_string(full_name, name);
				null_terminate(full_name);
				flow.function_map[j] = full_name;
			}
		}
	}

	{ //// OUTPUT flow_router.generated.cpp
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/flow_router.generated.cpp");

fprintf(output, "namespace flow_router {\n");
fprintf(output, "	// EventDelegate to use when broadcasting. This pointer won't persist so it needs to be set in on_script_reload!\n");
fprintf(output, "	static EventDelegate *event_delegate;\n");
fprintf(output, "\n");
fprintf(output, "	// Flow message names:\n");

		for (int i = 0; i < flow_array.count; ++i) {
			FlowFunction &flow = flow_array.entries[i];
			Function &function = flow.function;
			if (flow.function_map_count > 0) {
				for (int j = 0; j < flow.function_map_count; ++j) {
					String &name = flow.function_map[j];
					unsigned id = to_id32(name.length, *name);
fprintf(output, "	static const Id32 %s_id32 = 0x%x; // mapped from %s\n", *name, id, *function.name);
				}
			} else {
fprintf(output, "	static const Id32 %s_id32 = 0x%x;\n", *function.name, function.name_id);
			}
		}
fprintf(output, "\n");

		//////// receive ////////
fprintf(output, "	bool receive(FlowMessage &message) {\n");
fprintf(output, "		bool trigger_default_output_events = true;\n");
fprintf(output, "		if (event_delegate == 0) {\n");
fprintf(output, "			LOG_WARNING(\"FlowRouter\", \"event_delegate set to 0. Ignoring flow callback!\")\n");
fprintf(output, "			return trigger_default_output_events;\n");
fprintf(output, "		}\n");
fprintf(output, "\n");
fprintf(output, "		switch (message.id) {\n");
		for (unsigned i = 0; i < flow_array.count; ++i) {
			FlowFunction &flow = flow_array.entries[i];
			Function &function = flow.function;

			if (flow.function_map_count > 0) {
				for (int j = 0; j < flow.function_map_count; ++j) {
					String &name = flow.function_map[j];
					write_case_statement(output, function, name, j, flow.suppress_default_output_events);
				}
			} else {
				write_case_statement(output, function, function.name, -1, flow.suppress_default_output_events);
			}
		}
fprintf(output, "		}\n");
fprintf(output, "		return trigger_default_output_events;\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}

	{ //// OUTPUT global.script_flow_nodes
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/generated.script_flow_nodes");

fprintf(output, "nodes = [\n");
		for (unsigned i = 0; i < flow_array.count; ++i) {
fprintf(output, "	{\n");
			FlowFunction &flow = flow_array.entries[i];
			Function &function = flow.function;
			bool ignore_first_parameter = flow.function_map_count > 0; // If we have a function map, ignore the first parameter. This is reserved as the index of the input event.
			unsigned start_index = ignore_first_parameter ? 1 : 0;
fprintf(output, "		name = \"%s\"\n", *make_pretty_flow_name(function.name, arena));
fprintf(output, "		args = {\n");
			for (unsigned j = start_index; j < function.parameter_count; ++j) {
				Parameter &parameter = function.parameters[j];
fprintf(output, "			%s = \"%s\"\n", *parameter.name, convert_flow_type(parameter.type_id));
			}
fprintf(output, "		}\n");

			if (flow.return_list_count > 0) {
fprintf(output, "		returns = {\n");
				for (int j = 0; j < flow.return_list_count; ++j) {
					Parameter &parameter = flow.return_list[j];
fprintf(output, "			%s = \"%s\"\n", *parameter.name, convert_flow_type(parameter.type_id));
				}
fprintf(output, "		}\n");
			}

			if (flow.function_map_count > 0) {
fprintf(output, "		function_map = [\n");
				for (int j = 0; j < flow.function_map_count; ++j) {
					String &name = flow.function_map[j];
					char *short_name = name.text + (function.name.length + 1);
fprintf(output, "			[\"%s\", \"%s\"]\n", short_name, *name);
				}
fprintf(output, "		]\n");
			} else {
fprintf(output, "		function = \"%s\"\n", *function.name);
			}
fprintf(output, "		cpp_node = true\n");
fprintf(output, "	}\n");
		}
fprintf(output, "]\n");

		fclose(output);
	}


	{ //// OUTPUT flow_messages.generated.cpp
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/flow_messages.generated.cpp");

fprintf(output, "#define FLOW_STRING_VARIABLE_LENGTH 128 // 128 corresponds to FLOW_STRING_VARIABLE_LENGTH in flow.h\n\n");

		for (unsigned i = 0; i < flow_array.count; ++i) {
			FlowFunction &flow = flow_array.entries[i];
			Function &function = flow.function;

fprintf(output, "namespace %s {\n", *function.name);
fprintf(output, "	// Parameter ids: \n");
			for (int j = 0; j < function.parameter_count; ++j) {
				Parameter &parameter = function.parameters[j];
				unsigned id = to_id32(parameter.name.length, *parameter.name);
fprintf(output, "	static const Id32 %s_id32 = 0x%x;\n", *parameter.name, id);
			}

			if (flow.return_list_count > 0) {
fprintf(output, "\n");
fprintf(output, "	// Return value ids: \n");
				for (int j = 0; j < flow.return_list_count; ++j) {
					Parameter &parameter = flow.return_list[j];

					if (!are_strings_equal(parameter.type, MAKE_STRING("event"))) {
						unsigned id = to_id32(parameter.name.length, *parameter.name);
fprintf(output, "	static const Id32 %s_id32 = 0x%x;\n", *parameter.name, id);
					}
				}
			}

fprintf(output, "\n");
fprintf(output, "	struct ArgumentType {\n");
fprintf(output, "		enum {\n");
			for (int j = 0; j < function.parameter_count; ++j) {
				Parameter &parameter = function.parameters[j];
fprintf(output, "			%s = 1 << %d,\n", *parameter.name, j);
			}
fprintf(output, "		};\n");
fprintf(output, "	};\n");
fprintf(output, "\n");
fprintf(output, "	struct Arguments {\n");
			for (int j = 0; j < function.parameter_count; ++j) {
				Parameter &parameter = function.parameters[j];

				if (are_strings_equal(parameter.type, MAKE_STRING("flow_string"))) {
fprintf(output, "		char %s[FLOW_STRING_VARIABLE_LENGTH]; \n", *parameter.name);
				} else {
fprintf(output, "		%s %s;\n", *parameter.type, *parameter.name);
}
			}
fprintf(output, "		unsigned filled_mask;\n");
fprintf(output, "	};\n");
fprintf(output, "\n");
fprintf(output, "	Arguments *__global_last_arguments;\n");
fprintf(output, "	FlowMessage *__global_last_message;\n");
fprintf(output, "\n");
fprintf(output, "	void read_arguments(const FlowMessage &message, Arguments &result) {\n");
fprintf(output, "		result.filled_mask = 0;\n");
fprintf(output, "		for (unsigned i = 0; i < message.num_arguments; ++i) {\n");
fprintf(output, "			unsigned argument_id = message.argument_ids[i];\n");
fprintf(output, "			switch (argument_id) {\n");
			for (int j = 0; j < function.parameter_count; ++j) {
				Parameter &parameter = function.parameters[j];
fprintf(output, "				case %s_id32 : {\n", *parameter.name);
				if (are_strings_equal(parameter.type, MAKE_STRING("flow_string"))) {
fprintf(output, "					char *cursor = (char*)message.arguments[i];\n");
fprintf(output, "					bool end_found = false;\n");
fprintf(output, "					for (int i = 0; i < ARRAY_COUNT(result.%s) && !end_found; i++) {\n", *parameter.name);
fprintf(output, "						result.%s[i] = *cursor;\n", *parameter.name);
fprintf(output, "						end_found = *cursor == '\\0';\n");
fprintf(output, "						if (end_found)\n");
fprintf(output, "							break; // we just wrote the null terminator to the %s, break!\n", *parameter.name);
fprintf(output, "						cursor++;\n");
fprintf(output, "					};\n");
fprintf(output, "					if (!end_found)\n");
fprintf(output, "						result.%s[ARRAY_COUNT(result.%s)-1] = '\\0';\n", *parameter.name, *parameter.name);
				} else {
fprintf(output, "					result.%s = *(%s*)message.arguments[i];\n", *parameter.name, *parameter.type);
fprintf(output, "					result.filled_mask |= ArgumentType::%s;\n", *parameter.name);
				}
fprintf(output, "				} break;\n");
			}
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
			if (flow.return_list_count > 0) {
fprintf(output, "\n");
fprintf(output, "	void write_return_values(const FlowMessage &message");
				for (int j = 0; j < flow.return_list_count; ++j) {
					Parameter &parameter = flow.return_list[j];
					if (are_strings_equal(parameter.type, MAKE_STRING("flow_string"))) {
						fprintf(output, ", char *%s", *parameter.name);
					} else if (are_strings_equal(parameter.type, MAKE_STRING("event"))) {
					} else {
						fprintf(output, ", %s %s", *parameter.type, *parameter.name);
					}
				}
fprintf(output, ") {\n");
fprintf(output, "		for (unsigned i = 0; i < message.num_return_values; ++i) {\n");
fprintf(output, "			unsigned return_id = message.return_ids[i];\n");
fprintf(output, "			char *p = (char*)message.return_buffer[i];\n");
fprintf(output, "			switch (return_id) {\n");
				for (int j = 0; j < flow.return_list_count; ++j) {
					Parameter &parameter = flow.return_list[j];
					if (!are_strings_equal(parameter.type, MAKE_STRING("event"))) {
fprintf(output, "				case %s_id32 : {\n", *parameter.name);
						if (are_strings_equal(parameter.type, MAKE_STRING("flow_string"))) {
fprintf(output, "					for (int j = 0; j < FLOW_STRING_VARIABLE_LENGTH; ++j) {\n");
fprintf(output, "						p[j] = string[j];\n");
fprintf(output, "						if (string[j] == '\\0')\n");
fprintf(output, "							break;\n");
fprintf(output, "					}\n");
						} else {
fprintf(output, "					*(%s*)p = %s;\n", *parameter.type, *parameter.name);
						}
fprintf(output, "				} break;\n");
					}
				}
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
			}
fprintf(output, "}\n");
fprintf(output, "\n");
		}
		fclose(output);
	}
}

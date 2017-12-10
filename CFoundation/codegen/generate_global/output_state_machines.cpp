void output_function_signature(FILE *output, Function &function) {
	fprintf(output, "%s(", *function.name);
	for (unsigned k = 0; k < function.parameter_count; ++k) {
		Parameter &parameter = function.parameters[k];
		if (parameter.is_pointer)
			fprintf(output, "%s *%s", *parameter.type, *parameter.name);
		else
			fprintf(output, "%s %s", *parameter.type, *parameter.name);

		if (k < function.parameter_count - 1) {
			fprintf(output, ", ");
		}
	}
	fprintf(output, ")");
}
void output_function_call(FILE *output, Function &function) {
	fprintf(output, "%s(", *function.name);
	for (unsigned k = 0; k < function.parameter_count; ++k) {
		Parameter &parameter = function.parameters[k];
		fprintf(output, "%s", *parameter.name);
		if (k < function.parameter_count - 1) {
			fprintf(output, ", ");
		}
	}
	fprintf(output, ")");
}

void output_state_machines(StateMachineArray &state_machine_array, MemoryArena &arena) {
	{
		for (int i = 0; i < state_machine_array.count; ++i) {
			StateMachine &state_machine = state_machine_array.entries[i];

			String name_underscore = make_underscore_case(state_machine.name, arena);
			String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, name_underscore, MAKE_STRING("_state_machine.generated.h"));
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

			for (int j = 0; j < state_machine.state_count; ++j) {
				State *state = state_machine.states[j];
fprintf(output,  "#include \"..%s.h\"\n", *state->path);
			}

fprintf(output, "\n");

			String *short_names = (String *)allocate_memory(arena, state_machine.state_count*sizeof(String));
			String *short_names_underscore = (String *)allocate_memory(arena, state_machine.state_count*sizeof(String));

fprintf(output, "enum %sType {\n", *state_machine.name);
			for (int j = 0; j < state_machine.state_count; ++j) {
				State *state = state_machine.states[j];
				char *short_state_name = state->name.text + state_machine.name.length;
				short_names[j] = make_string(short_state_name, state->name.length - state_machine.name.length);
				short_names_underscore[j] = make_underscore_case(short_names[j], arena);
fprintf(output, "	%sType_%s,\n", *state_machine.name, short_state_name);
			}
fprintf(output, "\n");
fprintf(output, "	%sType_Count,\n", *state_machine.name);
fprintf(output, "};\n\n");

fprintf(output, "struct %sStateMachine {\n", *state_machine.name);
			for (int j = 0; j < state_machine.function_count; ++j) {
				Function &function = state_machine.functions[j];
fprintf(output, "	void ");
				output_function_signature(output, function);
fprintf(output, ";\n");
			}
fprintf(output, "\n");
fprintf(output, "	%sType active_type;\n", *state_machine.name);
			for (int j = 0; j < state_machine.state_count; ++j) {
				State *state = state_machine.states[j];
fprintf(output, "	%s%s %s;\n", *state_machine.name, *short_names[j], *short_names_underscore[j]);
			}
fprintf(output, "};\n");

			fclose(output);
		}
	}
	{
		for (int i = 0; i < state_machine_array.count; ++i) {
			StateMachine &state_machine = state_machine_array.entries[i];

			String name_underscore = make_underscore_case(state_machine.name, arena);
			String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_FOLDER, name_underscore, MAKE_STRING("_state_machine.generated.cpp"));
			MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

			for (int j = 0; j < state_machine.state_count; ++j) {
				State *state = state_machine.states[j];
fprintf(output,  "#include \"..%s.cpp\"\n", *state->path);
			}

fprintf(output, "\n");

			String *short_names = (String *)allocate_memory(arena, state_machine.state_count*sizeof(String));
			String *short_names_underscore = (String *)allocate_memory(arena, state_machine.state_count*sizeof(String));

fprintf(output, "const char *type_str(%sType type) {\n", *state_machine.name);
fprintf(output, "	switch (type) {\n");
			for (int j = 0; j < state_machine.state_count; ++j) {
				State *state = state_machine.states[j];
				char *short_state_name = state->name.text + state_machine.name.length;
				short_names[j] = make_string(short_state_name, state->name.length - state_machine.name.length);
				short_names_underscore[j] = make_underscore_case(short_names[j], arena);
				
fprintf(output, "		case %sType_%s: { return \"%s\"; } break;\n", *state_machine.name, short_state_name, short_state_name);
			}
fprintf(output, "	}\n");
fprintf(output, "	return \"unknown\";\n");
fprintf(output, "}\n\n");

			for (int j = 0; j < state_machine.function_count; ++j) {
				Function &function = state_machine.functions[j];
fprintf(output, "void %sStateMachine::", *state_machine.name);
				output_function_signature(output, function);
fprintf(output, " {\n");
fprintf(output, "	switch (active_type) {\n");
				for (int k = 0; k < state_machine.state_count; ++k) {
					State *state = state_machine.states[k];
fprintf(output, "		case %sType_%s: { %s.", *state_machine.name, *short_names[k], *short_names_underscore[k]);
					output_function_call(output, function);
					fprintf(output, "; } break;\n");
				}
fprintf(output, "	}\n");

fprintf(output, "}\n\n");
			}

			fclose(output);
		}
	}
}
void output_behavior_nodes(BehaviorNodeArray &behavior_node_array, SettingsEnumArray &settings_enum_array, MemoryArena &arena) {
	{
		String output_file_name = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("behavior_autocomplete"), MAKE_STRING(".generated.py"));
		MAKE_OUTPUT_FILE(output, *output_file_name);

		for (unsigned i = 0; i < array_count(behavior_node_array); ++i) {
			BehaviorNode &behavior_node = behavior_node_array[i];

			behavior_node.dependency_parameter_count = 0;
			for (unsigned i = 0; i < behavior_node.dependency_count; ++i) {
				BehaviorNode *n = try_get_behavior_node(behavior_node_array, behavior_node.dependencies[i]);
				if (n->return_type.type_id != void_id) {
					behavior_node.dependency_parameter_count++;
				}
			}
		}

fprintf(output, "enums = {\n");
		for (unsigned i = 0; i < array_count(settings_enum_array); ++i) {
			SettingsEnum &settings_enum = settings_enum_array[i];
fprintf(output, "	\"%s\": [ ", *settings_enum.name);
			unsigned base_length = settings_enum.name.length + 1;
			for (unsigned j = 0; j < array_count(settings_enum.entry_array); ++j) {
				String &value = settings_enum.entry_array[j];
fprintf(output, "\"%.*s\", ", value.length - base_length, value.text + base_length);
			}
fprintf(output, "	],\n");
		}
fprintf(output, "}\n");


fprintf(output, "functions = {\n");
		for (unsigned i = 0; i < array_count(behavior_node_array); ++i) {
			BehaviorNode &behavior_node = behavior_node_array[i];
			if (behavior_node.name[0] == '_')
				continue;

fprintf(output, "	\"%s\": [", *behavior_node.name);
			int default_params = behavior_node.dependency_parameter_count;
			for (unsigned j = default_params; j < array_count(behavior_node.parameters); ++j) {
				Parameter &p = behavior_node.parameters[j];
				fprintf(output, "\"%s\"", *p.type);
				if (j < (array_count(behavior_node.parameters) - 1)) {
					fprintf(output, ", ");
				}
			}
			fprintf(output, "],\n");
		}
fprintf(output, "}\n");


fprintf(output, "node_collection = {\n");
		for (unsigned i = 0; i < array_count(behavior_node_array); ++i) {
			BehaviorNode &behavior_node = behavior_node_array[i];
			if (behavior_node.name[0] == '_')
				continue;

fprintf(output, "	(\"%s.%s(", *behavior_node.collection_name, *behavior_node.name);
			int default_params = behavior_node.dependency_parameter_count;
			for (unsigned j = default_params; j < array_count(behavior_node.parameters); ++j) {
				Parameter &p = behavior_node.parameters[j];
				fprintf(output, "%s", *p.name);
				if (j < (array_count(behavior_node.parameters) - 1)) {
					fprintf(output, ", ");
				}
			}
fprintf(output, ")\", \"%s(", *behavior_node.name);
			for (unsigned j = default_params; j < array_count(behavior_node.parameters); ++j) {
				Parameter &p = behavior_node.parameters[j];
				fprintf(output, "${%u:%s}", j - default_params + 1, *p.name);
				if (j < (array_count(behavior_node.parameters) - 1)) {
					fprintf(output, ", ");
				}
			}
			fprintf(output, ")\"),\n");
		}
fprintf(output, "}\n");
		fclose(output);
	}

	{
		String output_file_name = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("behavior_nodes"), MAKE_STRING(".generated.bin"));
		FILE *output = fopen(*output_file_name, "wb");
		ASSERT(output, "Failed to open file for writing: '%s'", *output_file_name);

		char *buffer = arena.memory + arena.offset;
		char *beginning = buffer;
		intptr_t start = (intptr_t)buffer;

		for (unsigned i = 0; i < array_count(behavior_node_array); ++i) {
			BehaviorNode &behavior_node = behavior_node_array[i];
			behavior_node::write(behavior_node, &buffer);
		}

		size_t size = (size_t)((intptr_t)buffer - start);

		unsigned behavior_node_array_count = array_count(behavior_node_array);

		fwrite(&size, sizeof(size_t), 1, output);
		fwrite(&behavior_node_array_count, sizeof(unsigned), 1, output);
		fwrite(beginning, size, 1, output);

		fclose(output);
	}

	{
		String output_file_name = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("behavior_node_declarations"), MAKE_STRING(".generated.h"));
		FILE *output = fopen(*output_file_name, "wb");
		ASSERT(output, "Failed to open file for writing: '%s'", *output_file_name);

		BehaviorNode *current_collection_node = 0;

fprintf(output, "namespace behavior {\n");
fprintf(output, "	typedef unsigned ScriptEnum;\n");
fprintf(output, "\n");
fprintf(output, "	//! not_reloadable\n");
fprintf(output, "	struct Context {\n");
fprintf(output, "		BehaviorComponent *component;\n");
fprintf(output, "		Instance *instance;\n");
fprintf(output, "		Master *state;\n");
fprintf(output, "\n");
fprintf(output, "		unsigned index;\n");
fprintf(output, "	};\n");
fprintf(output, "	Context make_context(BehaviorComponent *component, unsigned index) {\n");
fprintf(output, "		Context context = {};\n");
fprintf(output, "		context.component = component;\n");
fprintf(output, "\n");
fprintf(output, "		context.instance = component->instances[index];\n");
fprintf(output, "		context.state = component->masters + index;\n");
fprintf(output, "\n");
fprintf(output, "		context.index = index;\n");
fprintf(output, "\n");
fprintf(output, "		return context;\n");
fprintf(output, "	}\n");

		for (unsigned i = 0; i < array_count(behavior_node_array); ++i) {
			BehaviorNode &behavior_node = behavior_node_array[i];

			if (current_collection_node == 0) {
				current_collection_node = &behavior_node;
fprintf(output, "	namespace %s {\n", *current_collection_node->collection_name);
			} else {
				if (behavior_node.collection_name_id != current_collection_node->collection_name_id) {
fprintf(output, "	}\n");
					current_collection_node = &behavior_node;
fprintf(output, "	namespace %s {\n", *current_collection_node->collection_name);
				}
			}

			char type_buffer[256] = {};
			format_type(behavior_node.return_type, type_buffer);

fprintf(output, "		%s%s(Context &c", type_buffer, *behavior_node.name);
			for (unsigned j = 0; j < array_count(behavior_node.parameters); ++j) {
				Parameter &p = behavior_node.parameters[j];
				format_type(p, type_buffer);
				if (p.name.length == 0)
					fprintf(output, ", %s", type_buffer);
				else
					fprintf(output, ", %s%s", type_buffer, *p.name);
			}
fprintf(output, ");\n");
	// );\n", );
		}
fprintf(output, "	}\n");

fprintf(output, "}\n");

		fclose(output);
	}
}
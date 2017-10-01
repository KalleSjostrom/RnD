void output_ability_nodes(AbilityNodeArray &ability_node_array, SettingsEnumArray &settings_enum_array, MemoryArena &arena) {
	{
		String output_file_name = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("ability_autocomplete"), MAKE_STRING(".generated.py"));
		MAKE_OUTPUT_FILE(output, *output_file_name);

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
		for (unsigned i = 0; i < array_count(ability_node_array); ++i) {
			AbilityNode &ability_node = ability_node_array[i];
fprintf(output, "	\"%s\": [", *ability_node.name);
			for (unsigned j = 0; j < array_count(ability_node.parameters); ++j) {
				Parameter &p = ability_node.parameters[j];
				fprintf(output, "\"%s\"", *p.type);
				if (j < (array_count(ability_node.parameters) - 1)) {
					fprintf(output, ", ");
				}
			}
			fprintf(output, "],\n");
		}
fprintf(output, "}\n");


fprintf(output, "node_collection = {\n");
		for (unsigned i = 0; i < array_count(ability_node_array); ++i) {
			AbilityNode &ability_node = ability_node_array[i];
fprintf(output, "	(\"%s.%s(", *ability_node.collection_name, *ability_node.name);
			for (unsigned j = 0; j < array_count(ability_node.parameters); ++j) {
				Parameter &p = ability_node.parameters[j];
				fprintf(output, "%s", *p.name);
				if (j < (array_count(ability_node.parameters) - 1)) {
					fprintf(output, ", ");
				}
			}
fprintf(output, ")\", \"%s(", *ability_node.name);
			for (unsigned j = 0; j < array_count(ability_node.parameters); ++j) {
				Parameter &p = ability_node.parameters[j];
				fprintf(output, "${%u:%s}", j + 1, *p.name);
				if (j < (array_count(ability_node.parameters) - 1)) {
					fprintf(output, ", ");
				}
			}
			fprintf(output, ")\"),\n");
		}
fprintf(output, "}\n");
		fclose(output);
	}

	{
		String output_file_name = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("ability_nodes"), MAKE_STRING(".generated.bin"));
		FILE *output = fopen(*output_file_name, "wb");
		ASSERT(output, "Failed to open file for writing: '%s'", *output_file_name);

		char *buffer = arena.memory + arena.offset;
		char *beginning = buffer;
		intptr_t start = (intptr_t)buffer;

		for (unsigned i = 0; i < array_count(ability_node_array); ++i) {
			AbilityNode &ability_node = ability_node_array[i];
			ability_node::write(ability_node, &buffer);
		}

		size_t size = (size_t)((intptr_t)buffer - start);

		unsigned ability_node_array_count = (unsigned)array_count(ability_node_array);

		fwrite(&size, sizeof(size_t), 1, output);
		fwrite(&ability_node_array_count, sizeof(unsigned), 1, output);
		fwrite(beginning, size, 1, output);

		fclose(output);
	}
}
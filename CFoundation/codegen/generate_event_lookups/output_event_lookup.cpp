void output_event_lookup(EventArray &event_array, String name, String lookup_namespace) {
	{
		char filename[MAX_PATH] = {};
		sprintf(filename, "../../"GAME_CODE_DIR"/generated/%s.generated.h", *lookup_namespace);
		MAKE_OUTPUT_FILE_WITH_HEADER(output, filename);

fprintf(output, "//! network_type\n");
fprintf(output, "enum %s : unsigned {\n", *name);
fprintf(output, "	%s__invalid = 0,\n\n", *name);
		for (int i = 0; i < event_array.count; ++i) {
			Event &event = event_array.entries[i];
fprintf(output, "	%s_%s = 0x%x,\n", *name, *event.name, event.name_id);
		}
fprintf(output, "};\n");

		fclose(output);
	}

	{
		char filename[MAX_PATH] = {};
		sprintf(filename, "../../"GAME_CODE_DIR"/generated/%s.generated.cpp", *lookup_namespace);
		MAKE_OUTPUT_FILE_WITH_HEADER(output, filename);

fprintf(output, "#include \"%s.generated.h\"\n\n", *lookup_namespace);
fprintf(output, "namespace %s {\n", *lookup_namespace);
		for (int i = 0; i < event_array.count; ++i) {
			Event &event = event_array.entries[i];
fprintf(output, "	static unsigned _index_%s = %d;\n", *event.name, i);
		}
fprintf(output, "	static unsigned _index__invalid = UINT_MAX;\n");
fprintf(output, "\n");
fprintf(output, "	// NOTE(kalle): Since this data will be used to write rpc parameters, we need the address to some memory, not just the value..\n");
fprintf(output, "	unsigned *event_to_index(%s event) {\n", *name);
fprintf(output, "		switch (event) {\n");
		for (int i = 0; i < event_array.count; ++i) {
			Event &event = event_array.entries[i];
fprintf(output, "			case %s_%s : { return &_index_%s; } break;\n", *name, *event.name, *event.name);
		}
fprintf(output, "		}\n");
fprintf(output, "		return &_index__invalid;\n");
fprintf(output, "	}\n");
fprintf(output, "\n");
fprintf(output, "	%s index_to_event(unsigned index) {\n", *name);
fprintf(output, "		switch (index) {\n");
		for (int i = 0; i < event_array.count; ++i) {
			Event &event = event_array.entries[i];
fprintf(output, "			case %d : { return %s_%s; } break;\n", i, *name, *event.name);
		}
fprintf(output, "		}\n");
fprintf(output, "		return %s__invalid;\n", *name);
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}
}

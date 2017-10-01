void output_script_nodes(ScriptNodeArray &script_node_array, MemoryArena &arena) {
	{
		String output_file_name = make_filepath(arena, _folder_root, _folder_generated_code, MAKE_STRING("behavior_nodes"), MAKE_STRING(".generated.h"));
		MAKE_OUTPUT_FILE(output, *output_file_name);

		// TODO(kalle): Replace with hashmap?
		unsigned printed_dependency_count = 0;
		uint64_t *printed_dependencies = (uint64_t*) allocate_memory(arena, sizeof(uint64_t)*array_count(script_node_array));
		memset(printed_dependencies, 0, sizeof(uint64_t)*array_count(script_node_array));

fprintf(output, "//! behavior_node_collection\n");
fprintf(output, "namespace fetch {\n");
		for (unsigned i = 0; i < array_count(script_node_array); ++i) {
			ScriptNode &script_node = script_node_array[i];
			Component *component = (Component *)script_node.component;
			Member *member = script_node.member;
			SubCompType sub_component_type = script_node.sub_component_type;

			String sub_comp_name_lc = get_sub_comp_string(sub_component_type, true);
			String sub_comp_name = get_sub_comp_string(sub_component_type, false);

			bool found = false;
			uint64_t id = (uint64_t)sub_component_type << 32 | component->stem_id;
			for (unsigned j = 0; j < printed_dependency_count && !found; ++j) {
				found = printed_dependencies[j] == id;
			}
			if (!found) {
				printed_dependencies[printed_dependency_count++] = id;
				String sub_comp_name_uc = clone_string(sub_comp_name_lc, arena);
				to_upper(sub_comp_name_uc);
fprintf(output, "	%s::%s &_%s_%s(Context &c) {\n", *component->stem, *sub_comp_name, *component->stem, *sub_comp_name);
fprintf(output, "		return GET_%s(%s, c.instance->entity);\n", *sub_comp_name_uc, *component->stem);
fprintf(output, "	}\n");
			}

fprintf(output, "	//! dependencies(fetch._%s_%s)\n", *component->stem, *sub_comp_name);
				if (FLAG_SET(*member, MemberFlag_BehaviorNodePrefix)) {
fprintf(output, "	%s %s%s_%s(Context &c, %s::%s &_%s_%s) {\n", *member->type, FLAG_SET(*member, MemberFlag_IsPointer) ? "*":"", *component->stem, *member->name, *component->stem, *sub_comp_name, *component->stem, *sub_comp_name_lc);
				} else {
fprintf(output, "	%s %s%s(Context &c, %s::%s &_%s_%s) {\n", *member->type, FLAG_SET(*member, MemberFlag_IsPointer) ? "*":"", *member->name, *component->stem, *sub_comp_name, *component->stem, *sub_comp_name_lc);
				}
fprintf(output, "		return _%s_%s.%s;\n", *component->stem, *sub_comp_name_lc, *member->name);
fprintf(output, "	}\n");
		}
fprintf(output, "}\n");

fprintf(output, "//! behavior_node_collection\n");
fprintf(output, "namespace action {\n");
		for (unsigned i = 0; i < array_count(script_node_array); ++i) {
			ScriptNode &script_node = script_node_array[i];
			Component *component = (Component *) script_node.component;
			Member *member = script_node.member;
			SubCompType sub_component_type = script_node.sub_component_type;

			String sub_comp_name_lc = get_sub_comp_string(sub_component_type, true);
			String sub_comp_name = get_sub_comp_string(sub_component_type, false);

fprintf(output, "	//! dependencies(fetch._%s_%s)\n", *component->stem, *sub_comp_name);
				if (FLAG_SET(*member, MemberFlag_BehaviorNodePrefix)) {
fprintf(output, "	void %s_set_%s(Context &c, %s::%s &_%s_%s, %s %s) {\n", *component->stem, *member->name, *component->stem, *sub_comp_name, *component->stem, *sub_comp_name_lc, *member->type, *member->name);
				} else {
fprintf(output, "	void set_%s(Context &c, %s::%s &_%s_%s, %s %s) {\n", *member->name, *component->stem, *sub_comp_name, *component->stem, *sub_comp_name_lc, *member->type, *member->name);
				}
fprintf(output, "		_%s_%s.%s = %s;\n", *component->stem, *sub_comp_name_lc, *member->name, *member->name);
fprintf(output, "	}\n");

				if (FLAG_SET(*member, MemberFlag_BehaviorNodeReset)) {
fprintf(output, "	//! dependencies(fetch._%s_%s)\n", *component->stem, *sub_comp_name);
					if (FLAG_SET(*member, MemberFlag_BehaviorNodePrefix)) {
fprintf(output, "	void %s_reset_%s(Context &c, %s::%s &_%s_%s) {\n", *component->stem, *member->name, *component->stem, *sub_comp_name, *component->stem, *sub_comp_name_lc);
					} else {
fprintf(output, "	void reset_%s(Context &c, %s::%s &_%s_%s) {\n", *member->name, *component->stem, *sub_comp_name, *component->stem, *sub_comp_name_lc);
					}
fprintf(output, "		_%s_%s.%s = %s::get_static_settings(c.instance->entity_id)->%s;\n", *component->stem, *sub_comp_name_lc, *member->name,  *component->stem, *member->name);
fprintf(output, "	}\n");
				}

		}
fprintf(output, "}\n");


		fclose(output);
	}
}
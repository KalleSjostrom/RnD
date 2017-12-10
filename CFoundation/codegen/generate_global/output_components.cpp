// Called by parse_components.cpp directly since we already have this file open with a tokenizer
void insert_generated_info_in_hfile(Component *component, char *filepath, parser::Tokenizer &tok, char *source) {
	#pragma region modify some_component.h
	{
		SubCompStruct *sub_comps = component->sub_comps;

		MAKE_OUTPUT_FILE(tempfile, "__temp.h");

		String name       = component->name;
		String stem       = component->stem;
		String stem_upper = component->stem_upper;

		parser::consume_whitespace(&tok);

		if (tok.at[0] == '\0') { // we are already at the end of the file, which means that we don't have a class declaration.
			fprintf(tempfile, source); // Dump everything we have read and start filling in the generated stuff
fprintf(tempfile,  "\n");
fprintf(tempfile,  "class %s {\n", *name);
		} else {
			size_t file_offset = 0;
			// Search for the where the class begins so we can insert the generated code at the end of the class.
			bool parsing = true;
			while (parsing) {
				parser::Token token = parser::next_token(&tok);
				if (parser::is_equal(token, TOKENIZE("class"))) {
					parser::Token token = parser::next_token(&tok);
					ASSERT(are_strings_equal(token.string, name), "Found class name mismatch! (expected name=%s, actual name=%s)", *name, printable_token(token));
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');

					// Find end of class, i.e. where the matching bracket is.
					int bracket_count = 1;
					while (bracket_count > 0) {
						parser::Token token = parser::next_token(&tok);
						ASSERT(token.type != '\0', "Found premature end of component *.h file! (component=%s)", *name);

						if (token.type == '{')
							bracket_count++;
						else if (token.type == '}')
							bracket_count--;
						else if (token.type == TokenType_CommandMarker) {
							file_offset = (size_t)(tok.at - source) - 2;

							parser::Token command = parser::next_token(&tok);
							bool strings_equal = are_strings_equal(command.string, HFILE_GENERATED_BEGIN_MARKER);
							if (strings_equal) {
								break;
							}
						}
					}
					parsing = false;

					if (bracket_count == 0) { // Did we find the end of the class, or did we hit the BEGIN_GENERATED marker?
						file_offset = (size_t)(tok.at - source);
					}
				}
			}

			ASSERT(file_offset > 0, "No suitable insertion point found for generated code in component %s.\nInsertion point is either in the end of the class or at //! BEGIN_GENERATED", *name);

			// Dump everything we have read and start filling in the generated stuff
			// NOTE(bauer): Use raw dump (fwrite) instead of char output (fprintf), to 'escape' % characters (used in printf() and similar)
			// source[file_offset-1] = '\0';
			// fprintf(tempfile, source);
			fwrite(source, sizeof(char), file_offset-1, tempfile);
		}


fprintf(tempfile,  "//! %s\n", *HFILE_GENERATED_BEGIN_MARKER);
fprintf(tempfile,  "public:\n");
fprintf(tempfile,  "static const unsigned max_instances = %s;\n", *(component->max_instances));
fprintf(tempfile,  "#define %s_HASH_SIZE HASH_SIZE_FOR(%s)\n", *stem_upper, *(component->max_instances));
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	%s() : count(0)", *name);
		if (HAS_SUB_COMP(MASTER))
fprintf(tempfile,  ", master_count(0)");
fprintf(tempfile,  " {\n");
fprintf(tempfile,  "		hash_init(index_map, index_map_storage, ARRAY_COUNT(index_map_storage), INVALID_ENTITY);\n");
fprintf(tempfile,  "	}\n");
fprintf(tempfile,  "	~%s() { }\n", *name);
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	void init(CreationContext &context);\n");
fprintf(tempfile,  "	void deinit();\n");
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	void script_reload();\n");
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	void on_added(Instance *instance, SpawnContext &spawn_context);\n");
fprintf(tempfile,  "	void on_removed(Instance *instance);\n");
		if (HAS_SUB_COMP(SLAVE))
fprintf(tempfile,  "	void update_slave(unsigned index, float dt);\n");
fprintf(tempfile,  "	void update(float dt);\n");
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	void add(Instance *instance, SpawnContext &spawn_context);\n");
fprintf(tempfile,  "	GameObjectId remove(Instance *instance);\n");
		if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(tempfile,  "	void migrated_to_me(Instance *instance);\n");
fprintf(tempfile,  "	void migrated_away(Instance *instance);\n");
fprintf(tempfile,  "	void on_migrated_to_me(Instance *instance);\n");
fprintf(tempfile,  "	void on_migrated_away(Instance *instance);\n");
		}
		if (HAS_SUB_COMP(NETWORK)) {
fprintf(tempfile,  "	void fill_in_default_go_fields(Instance *instance, SpawnContext &spawn_context, GameObjectField *fields, unsigned *field_counter, AhTempAllocator &allocator);\n");
fprintf(tempfile,  "	void update_network_state(GameObjectId go_id, const char *field_buffer, unsigned *offset);\n");
		}
fprintf(tempfile,  "\n");
		if (HAS_SUB_COMP(NETWORK)) {
			SubCompStruct &network = sub_comps[NETWORK];
			for (int i = 0; i < network.member_count; i++) {
				Member &member = network.members[i];
fprintf(tempfile,  "	%s get_%s(EntityRef entity) { \n", *member.type, *member.name);
fprintf(tempfile,  "		unsigned id = get_instance_id(entity);\n");
fprintf(tempfile,  "		return networks[id].%s;\n", *member.name);
fprintf(tempfile,  "	}\n");
			}
		}
		if (HAS_SUB_COMP(MASTER_INPUT)) {
			SubCompStruct &master_input = sub_comps[MASTER_INPUT];
			for (int i = 0; i < master_input.member_count; i++) {
				Member &member = master_input.members[i];
fprintf(tempfile,  "	void input_%s(EntityRef entity, %s %s%s) {\n", *member.name, *member.type, (member.is_pointer?"*":""), *member.name);
fprintf(tempfile,  "		unsigned id = get_instance_id(entity);\n");
fprintf(tempfile,  "		master_inputs[id].%s = %s;\n", *member.name, *member.name);
fprintf(tempfile,  "	}\n");
			}
		}
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	void move_instance(unsigned from, unsigned to);\n");
fprintf(tempfile,  "	__forceinline unsigned get_instance_id(EntityRef entity) { HashEntry *entry = hash_lookup(index_map, entity); if(entry->key == entity) { return entry->value; } else { return INVALID_INSTANCE_ID; } }\n");
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	unsigned count;\n");
		if (HAS_SUB_COMP(MASTER))
fprintf(tempfile,  "	unsigned master_count;\n");

fprintf(tempfile,  "\n");
fprintf(tempfile,  "	HashEntry index_map_storage[%s_HASH_SIZE];\n", *stem_upper);
fprintf(tempfile,  "	HashMap index_map;\n");
fprintf(tempfile,  "\n");
fprintf(tempfile,  "	Instance *instances[max_instances]; //! count count\n");
fprintf(tempfile,  "\n");

		if (HAS_SUB_COMP(MASTER_INPUT))
fprintf(tempfile,  "	%s::MasterInput master_inputs[max_instances]; //! count master_count\n", *stem);
		if (HAS_SUB_COMP(MASTER))
fprintf(tempfile,  "	%s::Master      masters[max_instances]; //! count master_count\n", *stem);
		if (HAS_SUB_COMP(SLAVE_INPUT))
fprintf(tempfile,  "	%s::SlaveInput  slave_inputs[max_instances]; //! count count\n", *stem);
		if (HAS_SUB_COMP(SLAVE))
fprintf(tempfile,  "	%s::Slave       slaves[max_instances]; //! count count\n", *stem);
		if (HAS_SUB_COMP(NETWORK))
fprintf(tempfile,  "	%s::Network     networks[max_instances]; //! count count\n", *stem);

fprintf(tempfile, "};\n");

		fclose(tempfile);
	}
#pragma endregion // Modify component h file

	// Swap the component h file
	{
		int result = rename(filepath, "__backup.h");
		ASSERT(result == 0, "Could not backup the h file! __backup.h already exists? (filepath=%s, GetLastError=%d)", filepath, GetLastError());
		result = rename("__temp.h", filepath);
		if (result == 0) {
			rename("__backup.h", filepath); // try to revert
		}
		ASSERT(result == 0, "Could not replace component h file with the generated one! __temp.h already exists? (filepath=%s, GetLastError=%d)", filepath, GetLastError());
		result = remove("__backup.h");
		ASSERT(result == 0, "Could not remove the backup h file! __backup.h used by another program? (filepath=%s, GetLastError=%d)", filepath, GetLastError());
	}
}


// This generates the component.generated.cpp and creats the component.cpp stub if necessary
void output_component(Component *component, MemoryArena &arena) {
	String name       = component->name;
	String stem       = component->stem;
	String stem_upper = component->stem_upper;

	SubCompStruct *sub_comps = component->sub_comps;

#pragma region some_component.generated.cpp
	{
		// Open output file and insert the standard timestamp
		String output_filepath = make_filepath(arena, ROOT_FOLDER, GENERATED_CODE_FOLDER, stem, GENERATED_CPP_ENDING);
		MAKE_OUTPUT_FILE_WITH_HEADER(output, *output_filepath);

		//////// void SampleComponent::move_instance(unsigned to, unsigned from) ////////
fprintf(output, "void %s::move_instance(unsigned to, unsigned from) {\n", *name);
fprintf(output, "	if (from == to)\n");
fprintf(output, "		return;\n");
fprintf(output, "\n");
fprintf(output, "	instances[to]     = instances[from];\n");
		if (HAS_SUB_COMP(MASTER_INPUT))
fprintf(output, "	master_inputs[to] = master_inputs[from];\n");
		if (HAS_SUB_COMP(MASTER))
fprintf(output, "	masters[to]       = masters[from];\n");
		if (HAS_SUB_COMP(SLAVE_INPUT))
fprintf(output, "	slave_inputs[to]  = slave_inputs[from];\n");
		if (HAS_SUB_COMP(SLAVE))
fprintf(output, "	slaves[to]        = slaves[from];\n");
		if (HAS_SUB_COMP(NETWORK))
fprintf(output, "	networks[to]      = networks[from];\n");
fprintf(output, "\n");
fprintf(output, "	Instance *instance = instances[to];\n");
fprintf(output, "	HashEntry *entry = hash_lookup(index_map, instance->entity);\n");
fprintf(output, "	ASSERT(entry->key == instance->entity, \"Trying to move an unadded entity instance!\");\n");
fprintf(output, "	entry->value = to;\n");
fprintf(output, "}\n\n");

		//////// void SampleComponent::fill_in_default_go_fields(Instance *instance, SpawnContext &spawn_context, GameObjectField *fields, unsigned *field_counter, AhTempAllocator &allocator) ////////
		if (HAS_SUB_COMP(NETWORK)) {
fprintf(output, "void %s::fill_in_default_go_fields(Instance *instance, SpawnContext &spawn_context, GameObjectField *fields, unsigned *field_counter, AhTempAllocator &allocator) {\n", *name);
			SubCompStruct &network = sub_comps[NETWORK];
			if (network.member_count > 0) {
fprintf(output, "	unsigned index = get_instance_id(instance->entity);\n");
fprintf(output, "	char *buffer = (char*)allocator.allocate(");
				for (int i = 0; i < network.member_count; i++) {
					Member &member = network.members[i];
					char *size = field_size(member.type_id);
					ASSERT(size, "No size for field type %s!", *member.type);
fprintf(output, size);
					if (i < network.member_count-1) {
fprintf(output, " + ");
					} else {
fprintf(output, ");\n");
					}
				}
fprintf(output, "\n");
				for (int i = 0; i < network.member_count; i++) {
					Member &member = network.members[i];

					char *size = field_size(member.type_id);
					ASSERT(size, "No size for field type %s!", *member.type);

					// If we have exported data we have defaults of this member!
					char *default_value = 0;
					if (member.exported_line_counter > 0) {
						for (int m = 0; m < member.exported_line_counter; m++) {
							String &line = member.exported_lines[m];
							if (starts_with(line, MAKE_STRING("default "))) {
								default_value = line.text + sizeof("default = ") - 1;
								break;
							}
						}
// fprintf(output, "	*(%s*)buffer = networks[index].%s;\n", *member.type, *member.name);
					} else if (member.default_value.length > 0) {
						default_value = member.default_value.text;
					}

					if (default_value) {
fprintf(output, "	networks[index].%s = %s;\n", *member.name, default_value);
fprintf(output, "	*(%s*)buffer = networks[index].%s;\n", *member.type, *member.name);
					} else {
						char *default_value = default_value_for_type(member.type_id);
						if (default_value) {
fprintf(output, "	*(%s*)buffer = %s; // %s\n", *member.type, default_value, *member.name);
						} else {
fprintf(output, "	*(%s*)buffer = error; // you need to specify a default value for this using //! default_value some_value.\n", *member.type);
							ASSERT(false, "You need to specify a default value for network member '%s', using //! default_value some_value.\n", *member.name);
						}
					}
					String upper_type = make_upper_case(member.type, arena);
fprintf(output, "	GameObjectField go_field_%s = { ID(%s), RPCParameterType::RPC_PARAM_%s_TYPE, buffer };\n", *member.name, *member.name, *upper_type);
fprintf(output, "	fields[(*field_counter)++] = go_field_%s;\n", *member.name);
					if (i < network.member_count-1) {
fprintf(output, "	buffer += %s;\n", size);
fprintf(output, "\n");
					}
				}
			}
fprintf(output, "}\n\n");

		//////// void SampleComponent::update_network_state(EntityRef entity, const char *field_buffer, unsigned *offset) ////////
fprintf(output, "void %s::update_network_state(EntityRef entity, const char *field_buffer, unsigned *offset) {\n", *name);
fprintf(output, "	unsigned id = get_instance_id(entity);\n");
fprintf(output, "	%s::Network& network = networks[id];\n", *stem);
fprintf(output, "\n");

			for (int i = 0; i < network.member_count; i++) {
				Member &member = network.members[i];
				char *size = field_size(member.type_id);
				ASSERT(size, "No size for field type %s!", *member.type);
fprintf(output, "	network.%s = *(%s*)(field_buffer + *offset);\n", *member.name, *member.type);
fprintf(output, "	*offset += %s;\n", size);
				if(i < network.member_count-1) {
fprintf(output, "\n");
				}
			}
fprintf(output, "}\n");
		}

fprintf(output, "\n");

		//////// void SampleComponent::remove(Instance *instance, SpawnContext &spawn_context) ////////
fprintf(output, "void %s::add(Instance *instance, SpawnContext &spawn_context) {\n", *name);
fprintf(output, "	ASSERT((count+1) <= max_instances, \"Too many instances added!\");\n");
fprintf(output, "\n");
fprintf(output, "	unsigned index;\n");
		if (HAS_SUB_COMP(MASTER)) {
fprintf(output, "	if (spawn_context.is_master) {\n");
fprintf(output, "		if (master_count < count) { // If all we have are masters, we don't need to make room for the master!\n");
fprintf(output, "			move_instance(count, master_count); // Make room for the new master, by moving the first predictor to the last slot.\n");
fprintf(output, "		}\n");
fprintf(output, "\n");
fprintf(output, "		index = master_count;\n");
fprintf(output, "		master_count++;\n");
fprintf(output, "\n");
			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(MASTER_INPUT)) {
fprintf(output, "		master_inputs[index] = *%s::get_master_input_settings(instance->entity_id);\n", *stem);
fprintf(output, "\n");
			}
			if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(MASTER)) {
fprintf(output, "		masters[index] = *%s::get_master_settings(instance->entity_id);\n", *stem);
			}
fprintf(output, "	} else {\n");
fprintf(output, "		index = count;\n");
fprintf(output, "	}\n");
		} else {
fprintf(output, "	index = count;\n");
		}
		if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(SLAVE)) {
fprintf(output, "	slaves[index] = *%s::get_slave_settings(instance->entity_id);\n", *stem);
		}
fprintf(output, "\n");
fprintf(output, "	count++;\n");
fprintf(output, "\n");
fprintf(output, "	instances[index] = instance;\n");
fprintf(output, "\n");

// 		if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(NETWORK)) {
// fprintf(output, "	networks[index] = *%s::get_network_settings(instance->entity_id);\n", *stem);
// 		}
// fprintf(output, "\n");

fprintf(output, "	hash_add(index_map, instance->entity, index);\n");
fprintf(output, "\n");
fprintf(output, "	// The custom 'on_added' is called when _every_ component on this entity has been added.\n");
fprintf(output, "}\n");

		//////// GameObjectId SampleComponent::remove(Instance *instance) ////////
fprintf(output, "GameObjectId %s::remove(Instance *instance) {\n", *name);
fprintf(output, "	unsigned index = hash_remove(index_map, instance->entity);\n");
fprintf(output, "\n");
fprintf(output, "	GameObjectId go_id = instance->go_id;\n");
fprintf(output, "\n");
fprintf(output, "	// The custom 'on_removed' is called on _every_ component on this entity before being removed.\n");
fprintf(output, "\n");
		if (HAS_SUB_COMP(MASTER)) {
fprintf(output, "	bool is_master = index < master_count;\n");
fprintf(output, "	if (is_master) {\n");
fprintf(output, "		// put last master in here (if are any more masters) and put this one at the end of everything.\n");
fprintf(output, "		ASSERT(master_count > 0, \"Number of master instances is 0!?\");\n");
fprintf(output, "		master_count--;\n");
fprintf(output, "		if (master_count > 0) { // If the master_count is 0 then this was the last master we removed, so we don't need to move last master into the free slot.\n");
fprintf(output, "			unsigned last_master_index = master_count;\n");
fprintf(output, "			move_instance(index, last_master_index); // Put last master in slot 'index'.\n");
fprintf(output, "		}\n");
fprintf(output, "\n");
fprintf(output, "		// Now there might be a gap at the end of the masters (at master_count)\n");
fprintf(output, "		// Solve this by moving the last slave into this place.\n");
fprintf(output, "		index = master_count;\n");
fprintf(output, "	}\n");
fprintf(output, "\n");
		}
fprintf(output, "	ASSERT(count > 0, \"Number of instances is 0!?\");\n");
fprintf(output, "	count--;\n");
fprintf(output, "	if (count > 0) { // If the count is 0 then this was the last instance we removed, so we don't need to move anything at all!\n");
fprintf(output, "		unsigned last_master_index = count;\n");
fprintf(output, "		move_instance(index, last_master_index); // Put last slave where the removal occured\n");
fprintf(output, "	}\n");
fprintf(output, "	return go_id;\n");
fprintf(output, "}\n");


		//////// GameObjectId SampleComponent::migrated_to_me(Instance *instance) ////////
		if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(output, "void %s::migrated_to_me(Instance *instance) {\n", *name);
fprintf(output, "	unsigned index = get_instance_id(instance->entity);\n");
fprintf(output, "	ASSERT(!IS_MASTER(index), \"Entity migrated to me but is already a master!\");\n");
fprintf(output, "\n");
fprintf(output, "	// Swap with first slave, and increase master_count\n");
fprintf(output, "	if (index == master_count) { // we are already the first slave, we can stay in this slot and just increase the master_count\n");
fprintf(output, "		master_count++;\n");
fprintf(output, "	} else {\n");
fprintf(output, "		unsigned first_slave_index = master_count;\n");
fprintf(output, "\n");
fprintf(output, "		// 1. Get first slave\n");
fprintf(output, "		// 2. Fill first slave slot with our new master\n");
fprintf(output, "		// 3. Move old first slave to where we were\n");
fprintf(output, "		// 4. Increase master count to make this instance into a master\n");
fprintf(output, "\n");
fprintf(output, "		Instance *first_slave_instance = instances[first_slave_index];\n");
fprintf(output, "		instances[first_slave_index] = instances[index];\n");
fprintf(output, "		instances[index] = first_slave_instance;\n");
fprintf(output, "\n");
fprintf(output, "		%s::Master first_slave_master = masters[first_slave_index];\n", *stem);
fprintf(output, "		masters[first_slave_index]= masters[index];\n");
fprintf(output, "		masters[index] = first_slave_master;\n");
fprintf(output, "\n");
fprintf(output, "		%s::Slave first_slave_slave = slaves[first_slave_index];\n", *stem);
fprintf(output, "		slaves[first_slave_index] = slaves[index];\n");
fprintf(output, "		slaves[index] = first_slave_slave;\n");
			if (HAS_SUB_COMP(MASTER_INPUT)) {
fprintf(output, "\n");
fprintf(output, "		%s::MasterInput first_slave_master_input = master_inputs[first_slave_index];\n", *stem);
fprintf(output, "		master_inputs[first_slave_index] = master_inputs[index];\n");
fprintf(output, "		master_inputs[index] = first_slave_master_input;\n");
			}
			if (HAS_SUB_COMP(SLAVE_INPUT)) {
fprintf(output, "\n");
fprintf(output, "		%s::SlaveInput first_slave_slave_input = slave_inputs[first_slave_index];\n", *stem);
fprintf(output, "		slave_inputs[first_slave_index] = slave_inputs[index];\n");
fprintf(output, "		slave_inputs[index] = first_slave_slave_input;\n");
			}
			if (HAS_SUB_COMP(NETWORK)) {
fprintf(output, "\n");
fprintf(output, "		%s::Network first_slave_network = networks[first_slave_index];\n", *stem);
fprintf(output, "		networks[first_slave_index] = networks[index];\n");
fprintf(output, "		networks[index] = first_slave_network;\n");
			}
fprintf(output, "\n");
fprintf(output, "		Instance *new_master_instance = instances[index];\n");
fprintf(output, "		HashEntry *new_master_entry = hash_lookup(index_map, new_master_instance->entity);\n");
fprintf(output, "		new_master_entry->value = index;\n");
fprintf(output, "\n");
fprintf(output, "		Instance *moved_slave_instance = instances[first_slave_index];\n");
fprintf(output, "		HashEntry *moved_slave_entry = hash_lookup(index_map, moved_slave_instance->entity);\n");
fprintf(output, "		moved_slave_entry->value = first_slave_index;\n");
fprintf(output, "\n");
fprintf(output, "		master_count++;\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");
		}

		//////// GameObjectId SampleComponent::migrated_away(Instance *instance) ////////
		if (HAS_SUB_COMP(MASTER) && HAS_SUB_COMP(SLAVE)) {
fprintf(output, "void %s::migrated_away(Instance *instance) {\n", *name);
fprintf(output, "	unsigned index = get_instance_id(instance->entity);\n");
fprintf(output, "	ASSERT(IS_MASTER(index), \"Entity migrated away but wasn't a master!\");\n");
fprintf(output, "\n");
fprintf(output, "	// Swap with last master, and decrease master_count\n");
fprintf(output, "	if (index == master_count-1) { // we are already the last master, we can stay in this slot and just decrease the master_count\n");
fprintf(output, "		master_count--;\n");
fprintf(output, "	} else {\n");
fprintf(output, "		unsigned last_master_index = master_count - 1;\n");
fprintf(output, "\n");
fprintf(output, "		// 1. Get last master\n");
fprintf(output, "		// 2. Fill last master slot with our new slave\n");
fprintf(output, "		// 3. Move old last master to where we were\n");
fprintf(output, "		// 4. Decrease master count to make this instance into a slave\n");
fprintf(output, "\n");
fprintf(output, "		Instance *last_master_instance = instances[last_master_index];\n");
fprintf(output, "		instances[last_master_index] = instances[index];\n");
fprintf(output, "		instances[index] = last_master_instance;\n");
fprintf(output, "\n");
fprintf(output, "		%s::Master last_master_master = masters[last_master_index];\n", *stem);
fprintf(output, "		masters[last_master_index]= masters[index];\n");
fprintf(output, "		masters[index] = last_master_master;\n");
fprintf(output, "\n");
fprintf(output, "		%s::Slave last_master_slave = slaves[last_master_index];\n", *stem);
fprintf(output, "		slaves[last_master_index] = slaves[index];\n");
fprintf(output, "		slaves[index] = last_master_slave;\n");
			if (HAS_SUB_COMP(MASTER_INPUT)) {
fprintf(output, "\n");
fprintf(output, "		%s::MasterInput last_master_master_input = master_inputs[last_master_index];\n", *stem);
fprintf(output, "		master_inputs[last_master_index] = master_inputs[index];\n");
fprintf(output, "		master_inputs[index] = last_master_master_input;\n");
			}
			if (HAS_SUB_COMP(SLAVE_INPUT)) {
fprintf(output, "\n");
fprintf(output, "		%s::SlaveInput last_master_slave_input = slave_inputs[last_master_index];\n", *stem);
fprintf(output, "		slave_inputs[last_master_index] = slave_inputs[index];\n");
fprintf(output, "		slave_inputs[index] = last_master_slave_input;\n");
			}
			if (HAS_SUB_COMP(NETWORK)) {
fprintf(output, "\n");
fprintf(output, "		%s::Network last_master_network = networks[last_master_index];\n", *stem);
fprintf(output, "		networks[last_master_index] = networks[index];\n");
fprintf(output, "		networks[index] = last_master_network;\n");
			}
fprintf(output, "\n");
fprintf(output, "		Instance *new_slave_instance = instances[index];\n");
fprintf(output, "		HashEntry *new_slave_entry = hash_lookup(index_map, new_slave_instance->entity);\n");
fprintf(output, "		new_slave_entry->value = index;\n");
fprintf(output, "\n");
fprintf(output, "		Instance *moved_master_instance = instances[last_master_index];\n");
fprintf(output, "		HashEntry *moved_master_entry = hash_lookup(index_map, moved_master_instance->entity);\n");
fprintf(output, "		moved_master_entry->value = last_master_index;\n");
fprintf(output, "\n");
fprintf(output, "		master_count--;\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");
		}

		fclose(output);
	}
#pragma endregion // some_component.generated.cpp



#pragma region some_component.cpp (if none exists)
	{
		// Open output file and insert the standard timestamp
		String output_filepath = make_filepath(arena, ROOT_FOLDER, COMPONENTS_FOLDER, stem, CPP_ENDING);
		if (!file_exists(*output_filepath)) {
			MAKE_OUTPUT_FILE(output, *output_filepath);
			/// Start outputting the source code ///

fprintf(output,  "#include \"generated/components/%s_component.generated.cpp\"\n", *stem);
fprintf(output,  "\n");

			//////// void SampleComponent::init() ////////
fprintf(output,  "void %s::init(CreationContext &context) {}\n", *name);
fprintf(output,  "void %s::deinit() {}\n", *name);

			//////// void SampleComponent::script_reload() ////////
fprintf(output,  "void %s::script_reload() {}\n", *name);

			//////// void SampleComponent::on_added(Instance *instance, SpawnContext &spawn_context) ////////
fprintf(output,  "void %s::on_added(Instance *instance, SpawnContext &spawn_context) {}\n", *name);
fprintf(output,  "\n");

			//////// void SampleComponent::on_removed(Instance *instance) ////////
fprintf(output,  "void %s::on_removed(Instance *instance) {}\n", *name);
fprintf(output,  "\n");

			//////// void SampleComponent::on_migrated_to_me(Instance *instance) ////////
fprintf(output,  "void %s::on_migrated_to_me(Instance *instance) {}\n", *name);
fprintf(output,  "\n");

			//////// void SampleComponent::on_migrated_away(Instance *instance) ////////
fprintf(output,  "void %s::on_migrated_away(Instance *instance) {}\n", *name);
fprintf(output,  "\n");

			if (HAS_SUB_COMP(SLAVE) && HAS_SUB_COMP(MASTER)) {
			//////// void SampleComponent::update_slave(unsigned index, float dt) ////////
fprintf(output,  "void %s::update_slave(unsigned index, float dt) {\n", *name);
fprintf(output,  "	Instance *instance = instances[index];\n", *stem);
fprintf(output,  "	%s::Slave &slave = slaves[index];\n", *stem);
fprintf(output,  "}\n");
fprintf(output,  "\n");
			}

			//////// void SampleComponent::update(float dt) ////////
fprintf(output,  "void %s::update(float dt) {\n", *name);
			if (HAS_SUB_COMP(MASTER)) {
fprintf(output,  "	// Local instances\n");
fprintf(output,  "	for (unsigned i = 0; i < master_count; ++i) {\n");
fprintf(output,  "		Instance *instance = instances[i];\n", *stem);
				if (HAS_SUB_COMP(NETWORK))
fprintf(output,  "		%s::Network &network = networks[i];\n", *stem);

fprintf(output,  "		%s::Master &master = masters[i];\n", *stem);
fprintf(output,  "\n");
fprintf(output,  "		// Update master \n");
				if (HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "\n");
fprintf(output,  "		update_slave(i, dt); \n");
				}
fprintf(output,  "	}\n");
				if (HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "\n");
fprintf(output,  "	// Remote instances\n");
fprintf(output,  "	for (unsigned i = master_count; i < count; ++i) {\n");
fprintf(output,  "		update_slave(i, dt);\n");
fprintf(output,  "	}\n");
				}
			} else if (HAS_SUB_COMP(SLAVE)) {
fprintf(output,  "	for (unsigned i = 0; i < count; ++i) {\n");
fprintf(output,  "		Instance *instance = instances[i];\n", *stem);
fprintf(output,  "		%s::Slave &slave = slaves[i];\n", *stem);
fprintf(output,  "\n");
fprintf(output,  "		// Update instance \n");
fprintf(output,  "	}\n");
			}
fprintf(output,  "}\n");
fprintf(output,  "\n");
		}
	}
#pragma endregion // some_component.cpp (if none exists)
}

void output_components(ComponentArray &component_array, MemoryArena &temp_arena) {
	for (int i = 0; i < component_array.count; ++i) {
		if (component_array.reparsed[i]) {
			MemoryBlockHandle handle = begin_block(temp_arena);
			output_component(component_array.entries + i, temp_arena);
			end_block(temp_arena, handle);
		}
	}
}
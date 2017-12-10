void parse_component(MemoryArena &temp_arena, MemoryArena &arena, ComponentArray &component_array, char *filepath, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &event_receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
	// Setup parsing
	MAKE_INPUT_FILE(file, filepath);
	size_t filesize = get_filesize(file);

	char *source = allocate_memory(temp_arena, filesize+1);
	filesize = fread(source, 1, filesize, file);
	fclose(file);

	source[filesize] = '\0';

	parser::ParserContext parser_context;
	parser_context.filepath = filepath;
	parser_context.source = source;
	global_last_parser_context = &parser_context;

	parser::Tokenizer tok = parser::make_tokenizer(true, source, filesize);
	parser_context.tokenizer = &tok;
	/////

	ARRAY_CHECK_BOUNDS(component_array);
	Component &component = component_array.entries[component_array.count++];

	String namespace_name = MAKE_STRING("");
	String max_instances = MAKE_STRING("64");
	String icon = MAKE_STRING("cloud");
	bool found_component_spec = false;
	while (!found_component_spec) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_CommandMarker: {
				token = parser::next_token(&tok);
				if (parser::is_equal(token, component_token)) {
					token = parser::next_token(&tok);

					while (!parser::is_equal(token, namespace_token)) {
						if (parser::is_equal(token, after_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');

							while (1) {
								token = parser::next_token(&tok);
								if (token.type == ')')
									break;
								else if (token.type == ',')
									continue;

								ARRAY_CHECK_BOUNDS_STATIC(component.prior_component_array);
								ComponentReference &prior = component.prior_component_array.entries[component.prior_component_array.count++];
								prior.stem = clone_string(token.string, arena);
								prior.stem_id = make_string_id(prior.stem);
							}

							token = parser::next_token(&tok);
						} else if (parser::is_equal(token, depends_on_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');

							while (1) {
								token = parser::next_token(&tok);
								if (token.type == ')')
									break;
								else if (token.type == ',')
									continue;

								ARRAY_CHECK_BOUNDS_STATIC(component.dependency_array);
								ComponentReference &dependency = component.dependency_array.entries[component.dependency_array.count++];
								dependency.stem = clone_string(token.string, arena);
								dependency.stem_id = make_string_id(dependency.stem);
							}

							token = parser::next_token(&tok);
						} else if (parser::is_equal(token, max_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');
							max_instances = clone_string(parser::next_token(&tok).string, arena);
							ASSERT_NEXT_TOKEN_TYPE(tok, ')');
							token = parser::next_token(&tok);
						} else if (parser::is_equal(token, icon_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');
							icon = clone_string(parser::next_token(&tok).string, arena);
							ASSERT_NEXT_TOKEN_TYPE(tok, ')');
							token = parser::next_token(&tok);
						} else {
							PARSER_ASSERT(false, "Unexpected token! Expected 'max', 'after' or 'depends_on' but found %s", printable_token(token));
						}
					}

					ASSERT_TOKEN(tok, token, namespace_token);
					token = parser::next_token(&tok);

					ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
					namespace_name = clone_string(token.string, arena);
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');

					found_component_spec = true;
				} else if (parser::is_equal(token, reloadable_token)) {
				} else {
					null_terminate(token.string);
					PARSER_ASSERT(false, "Unknown command! (command=%s, filename=%s)", *token.string, filepath);
				}
			} break;
			case '\0': {
				PARSER_ASSERT(false, "No component specification found! (filename=%s)", filepath);
			} break;
		}
	}

	PARSER_ASSERT(namespace_name.length > 0, "Could not find a namespace in component! (filepath=%s)", filepath);

	component.stem = namespace_name;
	component.stem_id = make_string_id(component.stem);
	component.name = namespace_to_component_name(namespace_name, arena); // motion -> MotionComponent
	component.stem_upper = make_upper_case(namespace_name, arena);

	component.max_instances = max_instances;
	component.icon = icon;

	u32 sub_comp_count = 0;
	SubCompStruct *sub_comps = component.sub_comps;

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		if (parser::is_equal(token, struct_token)) {
			token = parser::next_token(&tok);
			ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
			String struct_name = clone_string(token.string, arena);

			SubCompType sub_comp_type;
			if (parser::is_equal(token, MasterInput_token))
				sub_comp_type = MASTER_INPUT;
			else if (parser::is_equal(token, Master_token))
				sub_comp_type = MASTER;
			else if (parser::is_equal(token, SlaveInput_token))
				sub_comp_type = SLAVE_INPUT;
			else if (parser::is_equal(token, Slave_token))
				sub_comp_type = SLAVE;
			else if (parser::is_equal(token, Network_token))
				sub_comp_type = NETWORK;
			else if (parser::is_equal(token, Static_token))
				sub_comp_type = STATIC;
			else
				PARSER_ASSERT(false, "Found no component sub type with name=%s", *struct_name);

			ASSERT_NEXT_TOKEN_TYPE(tok, '{');

			SubCompStruct *s = sub_comps + sub_comp_type;
			s->type = sub_comp_type;
			s->member_count = 0;
			token = parser::next_token(&tok);

			parse_struct_members(arena, tok, token, &s->member_count, s->members);

		} else if (token.type == '}') {
			parsing = false;
		} else if (token.type == '\0') {
			PARSER_ASSERT(false, "Reach end of file without the component specification struct closing!");
		}
	}

	String class_name = clone_string(component.name, arena);
	Receiver current_receiver = receiver::make_data(class_name, arena);

	parser::Tokenizer event_tokenizer = parser::make_tokenizer(true, tok.at, filesize - (tok.at - source));
	parser_context.tokenizer = &event_tokenizer;
	search_for_events(arena, event_tokenizer, rpc_array, event_array, flow_array, network_type_array, event_receiver_array, state_array, settings_enum_array, settings_struct_array, current_receiver, &component);
	parser_context.tokenizer = &tok;

	// Look if this component got added to the receiver array, if so store the number of receiver events for it.
	for (int i = event_receiver_array.count-1; i >= 0; i--) {
		Receiver &receiver = event_receiver_array.entries[i];
		if (receiver.name_id == current_receiver.name_id) {
			component.num_received_events = receiver.num_events;
			break;
		}
	}

	insert_generated_info_in_hfile(&component, filepath, tok, source);
}
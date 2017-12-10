void build_function(MemoryArena &arena, parser::Tokenizer &tok, bool is_rpc, Function &function) {
	parser::Token token = parser::next_token(&tok);
	ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
	function.name = clone_string(token.string, arena);
	function.name_id = make_string_id(function.name);

	ASSERT_NEXT_TOKEN_TYPE(tok, '(');

	bool found_end_of_function = false;
	while (!found_end_of_function) {
		Parameter parameter = {};

		token = parser::next_token(&tok);
		if (token.type == ')') {
			break;
		}

		ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
		parameter.is_const = parser::is_equal(token, const_token);
		if (parameter.is_const) {
			token = parser::next_token(&tok);
		}
		parameter.type = clone_string(token.string, arena);
		parameter.type_id = make_string_id(parameter.type);

		token = parser::next_token(&tok);
		parameter.is_ref = token.type == '&';
		if (parameter.is_ref)
			token = parser::next_token(&tok);

		if (is_rpc) {
			ASSERT_TOKEN_TYPE(token, TokenType_Identifier); // Pointers are not allowed in rpcs!
		} else {
			parameter.is_pointer = token.type == '*';
			if (parameter.is_pointer)
				token = parser::next_token(&tok);
		}
		parameter.name = clone_string(token.string, arena);
		parameter.name_id = make_string_id(parameter.name);

		ARRAY_CHECK_BOUNDS_COUNT(function.parameters, function.parameter_count);
		function.parameters[function.parameter_count++] = parameter;

		token = parser::next_token(&tok);
		if (token.type == ',') {

		} else if (token.type == ')') {
			found_end_of_function = true;
		}
	}
}

void parse_member_export_settings(MemoryArena &arena, parser::Tokenizer &tok, Member &m) {
	bool found_end = false;
	while(!found_end) {
		parser::Token line_token = parser::next_line(&tok);
		PARSER_ASSERT(line_token.type != '\0', "Unexpected end of file!");
		PARSER_ASSERT(m.exported_line_counter < ARRAY_COUNT(m.exported_lines), "Too many export lines!");

		String string = clone_string(line_token.string, arena);

		// NOTE(kalle): Stingray components won't allow for ';' to be in the file and the editor just won't startup.
		// You will get a crash if you are running the backend yourself, but not otherwise
		// To fix this, we remove ';' from the exported lines. Not pretty...
		bool found_non_whitespace = false;
		int extra_line_count = 0;
		String extra_lines[32];
		for (int i = string.length-1; i >= 0; i--) {
			if (string.text[i] == ';') {
				if (found_non_whitespace) {
					string.text[i] = '\0';
					char *next_line = string.text + (i+1);
					extra_lines[extra_line_count++] = make_string(next_line, string.length - (i+1));
				} else {
					string.text[i] = string.text[i+1];
					string.text[i+1] = '\0';
					string.length--;
				}
				found_non_whitespace = true;
			}

			if (!found_non_whitespace && !parser::is_whitespace(string.text[i])) {
				found_non_whitespace = true;
			}
		}

		m.exported_lines[m.exported_line_counter++] = string;
		for (int i = extra_line_count-1; i >= 0; i--) {
			m.exported_lines[m.exported_line_counter++] = extra_lines[i];
		}

		found_end = parser::is_block_comment_closing(&tok);
	}
	tok.at+=2; // swallow the closing block comment
}

void parse_struct_members(MemoryArena &arena, parser::Tokenizer &tok, parser::Token &token, unsigned *member_count, Member *members) {
	bool found_struct_end = false;
	while (!found_struct_end) {

		Member m = { 0 };

		if (token.type == TokenType_CommandMarkerBlock) {
			ASSERT_NEXT_TOKEN(tok, export_token);
			parse_member_export_settings(arena, tok, m);
			token = parser::next_token(&tok);
		}
		if (token.type == TokenType_CommandMarker) {
			PARSER_ASSERT(m.exported_line_counter == 0, "Command markers after the export block command is not allowed!");
			token = parser::next_token(&tok);
			if (parser::is_equal(token, export_token)) {
				m.exported_lines[m.exported_line_counter++] = clone_string(parser::next_line(&tok).string, arena);

				token = parser::next_token(&tok);
			} else if (parser::is_equal(token, default_value_token)) {
				m.default_value = clone_string(parser::next_line(&tok).string, arena);

				token = parser::next_token(&tok);
			} else {
				PARSER_ASSERT(false, "Unrecognized command! (command=%s)", printable_token(token));
			}
		}

		if (token.type == '}') {
			found_struct_end = true;
		} else {
			ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
			m.type = clone_string(token.string, arena);
			m.type_id = make_string_id(m.type);

			token = parser::next_token(&tok);
			m.is_pointer = token.type == '*';
			if (m.is_pointer) {
				token = parser::next_token(&tok);
			}
			m.name = clone_string(token.string, arena);
			m.name_id = make_string_id(m.name);

			token = parser::next_token(&tok);
			if (token.type == '[') { // found array
				token = parser::next_token(&tok);
				// m.max_count = clone_string(token.string, arena);
				// if (m.is_pointer) {
				// 	m.is_pointer_pointer = true;
				// }
				m.is_pointer = true;
				// m.is_array = true;
				ASSERT_NEXT_TOKEN_TYPE(tok, ']');
				token = parser::next_token(&tok);
			}

			ASSERT_TOKEN_TYPE(token, ';');

			token = parser::next_token(&tok);
			if (token.type == TokenType_CommandMarker) {
			}

			if (token.type == '}') {
				found_struct_end = true;
			}

			members[(*member_count)++] = m;
		}
	}
}

void add_event(MemoryArena &arena, parser::Tokenizer &tok, EventArray &event_array, Receiver &current_receiver, unsigned current_receiver_index, bool assert_on_void, FunctionType function_type, String *max_entities = 0) {
	if (assert_on_void)
		ASSERT_NEXT_TOKEN(tok, void_token); // Return types not allowed on events!

	Function function = {};
	build_function(arena, tok, false, function);

	ASSERT(current_receiver.name_id != 0, "No valid receiver for event function! (event=%s)", *function.name);
	current_receiver.num_events++;
	function.receiver_name_id = current_receiver.name_id;

	String s = {};
	function.max_entities = max_entities ? *max_entities : s;
	function.function_type = function_type;
	ARRAY_CHECK_BOUNDS(event_array);
	event_array.entries[event_array.count++] = function;
}

void search_for_events(MemoryArena &arena, parser::Tokenizer &tok, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array, Receiver current_receiver = receiver::make_empty(), Component *component = 0) {
	bool parsing = true;
	bool in_generated_block = false;

	unsigned current_receiver_index = receiver_array.count;

	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				if (parser::is_equal(token, class_token) || parser::is_equal(token, struct_token)) {
					token = parser::next_token(&tok);
					String class_name = clone_string(token.string, arena);

					token = parser::peek_next_token(&tok);
					if (token.type == '{') { // To avoid triggering a new receiver on e.g. friend class SomeClass;
						if (current_receiver.num_events > 0) {
							ARRAY_CHECK_BOUNDS(receiver_array);
							receiver_array.entries[receiver_array.count++] = current_receiver;
						}

						current_receiver = receiver::make_data(class_name, arena);
						current_receiver_index = receiver_array.count;
					}
				}
			} break;
			case TokenType_CommandMarker: {
				token = parser::next_token(&tok);

				if (parser::is_equal(token, BEGIN_GENERATED_token)) {
					in_generated_block = true;
					continue;
				}

				if (in_generated_block) {
					if (parser::is_equal(token, END_GENERATED_token)) {
						in_generated_block = false;
					}
					continue;
				}

				///////////// RPC /////////////
				if (parser::is_equal(token, rpc_token)) {
					token = parser::peek_next_token(&tok);
					bool session_bound = true;
					if (parser::is_equal(token, not_session_bound_token)) {
						parser::next_token(&tok);
						session_bound = false;
					}

					ASSERT_NEXT_TOKEN(tok, void_token); // Return types not allowed on rpcs!

					Function function = {};
					build_function(arena, tok, true, function);
					ASSERT(current_receiver.name_id != 0, "No valid receiver for event function! (event=%s)", *function.name);
					current_receiver.num_events++;
					function.receiver_name_id = current_receiver.name_id;
					function.function_type = FunctionType_GlobalReceiver;
					ARRAY_CHECK_BOUNDS(rpc_array);
					RPCFunction &rpc = rpc_array.entries[rpc_array.count++];
					rpc.function = function;
					rpc.session_bound = session_bound;

				///////////// EVENT /////////////
				} else if (parser::is_equal(token, event_token)) {
					add_event(arena, tok, event_array, current_receiver, current_receiver_index, true, FunctionType_GlobalReceiver);

				///////////// ENTITY EVENT TRIGGER /////////////
				} else if (parser::is_equal(token, entity_event_trigger_token)) {
					ASSERT(component, "Entity event triggers are only valid within a component. (this can be changed but would require a specifier for the number of entities that can listen to it)");
					add_event(arena, tok, event_array, current_receiver, current_receiver_index, false, FunctionType_EntitySender, &component->max_instances);

				///////////// FLOW /////////////
				} else if (parser::is_equal(token, flow_token)) {
					FlowFunction flow = {};
					current_receiver.num_events++;
					flow.function.receiver_name_id = current_receiver.name_id;
					flow.function_map_count = 0;

					token = parser::peek_next_token(&tok);
					if (parser::is_equal(token, map_token)) {
						token = parser::next_token(&tok);
						ASSERT_NEXT_TOKEN_TYPE(tok, '(');

						bool found_end = false;
						while (!found_end) {
							token = parser::next_token(&tok);

							ASSERT_TOKEN_TYPE(token, TokenType_Identifier); // We need a name here, map() doesn't make any sense!
							ARRAY_CHECK_BOUNDS_COUNT(flow.function_map, flow.function_map_count);
							flow.function_map[flow.function_map_count++] = clone_string(token.string, arena);

							token = parser::next_token(&tok);
							if (token.type == ',') {
								// we need to search for the next parameter
							} else if (token.type == ')') {
								found_end = true;
							} else {
								PARSER_ASSERT(false, "Unexpected token! (expected=',' or ')', found=%s)", printable_token(token));
							}
						}
					} else if (parser::is_equal(token, return_token) || parser::is_equal(token, delayed_return_token)) {
						bool suppress_default_output_events = parser::is_equal(token, delayed_return_token);

						token = parser::next_token(&tok);
						ASSERT_NEXT_TOKEN_TYPE(tok, '(');

						bool found_end = false;
						while (!found_end) {
							Parameter parameter = {};

							token = parser::next_token(&tok);
							ASSERT_TOKEN_TYPE(token, TokenType_Identifier); // We need a name here, return() doesn't make any sense!
							parameter.type = clone_string(token.string, arena);
							parameter.type_id = make_string_id(parameter.type);

							token = parser::next_token(&tok);
							ASSERT_TOKEN_TYPE(token, TokenType_Identifier); // We need a name here, return() doesn't make any sense!
							parameter.name = clone_string(token.string, arena);
							parameter.name_id = make_string_id(parameter.name);

							ARRAY_CHECK_BOUNDS_COUNT(flow.return_list, flow.return_list_count);
							flow.return_list[flow.return_list_count++] = parameter;

							token = parser::next_token(&tok);
							if (token.type == ',') {
								// we need to search for the next parameter
							} else if (token.type == ')') {
								found_end = true;
							} else {
								PARSER_ASSERT(false, "Unexpected token! (expected=',' or ')', found=%s)", printable_token(token));
							}
						}

						flow.suppress_default_output_events = suppress_default_output_events;
					}

					token = parser::next_token(&tok);
					PARSER_ASSERT(parser::is_equal(token, void_token) || parser::is_equal(token, bool_token), "Only bool or void return types allowed for flow callbacks! (expected=void or bool, found=%s)", printable_token(token));

					build_function(arena, tok, false, flow.function);
					flow.function.function_type = FunctionType_GlobalReceiver;

					// Yuck!!
					for (int i = 0; i < flow.return_list_count; ++i) {
						Parameter &return_parameter = flow.return_list[i];

						bool found_input_with_same_name = false;
						String name;
						for (int k = 0; k < flow.function.parameter_count; ++k) {
							Parameter &in_argument = flow.function.parameters[k];
							if (in_argument.name_id == return_parameter.name_id) {
								name = in_argument.name;
								found_input_with_same_name = true;
								break;
							}
						}
						PARSER_ASSERT(!found_input_with_same_name, "Flow node in arguments cannot have the same name as return values. (name=%s)", *name);
					}

					ARRAY_CHECK_BOUNDS(flow_array);
					flow_array.entries[flow_array.count++] = flow;

				///////////// NETWORK TYPES /////////////
				} else if (parser::is_equal(token, network_type_token)) {
					token = parser::next_token(&tok);
					if (parser::is_equal(token, enum_token)) {
						ARRAY_CHECK_BOUNDS(network_type_array);
						NetworkType &network_type = network_type_array.entries[network_type_array.count++];

						token = parser::next_token(&tok);
						network_type.name = clone_string(token.string, arena);
						network_type.name_id = make_string_id(network_type.name);
						network_type.name_lower_case = make_underscore_case(network_type.name, arena);

						ASSERT_NEXT_TOKEN_TYPE(tok, ':');

						token = parser::next_token(&tok);
						network_type.type = convert_network_type(make_string_id(token.string));

						ASSERT_NEXT_TOKEN_TYPE(tok, '{');

						unsigned count = 0;
						bool found_end = false;
						while (!found_end) {
							token = parser::next_line(&tok);
							if (token.text[0] == '}') {
								found_end = true;
								break;
							}

							if (token.length > 0) {
								count++;
							}
						}

						network_type.count = count;
					} else if (parser::is_equal(token, struct_token)) {
						ARRAY_CHECK_BOUNDS(network_type_array);
						NetworkType &network_type = network_type_array.entries[network_type_array.count++];

						token = parser::next_token(&tok);
						network_type.name = clone_string(token.string, arena);
						network_type.name_id = make_string_id(network_type.name);
						network_type.name_lower_case = make_underscore_case(network_type.name, arena);
						network_type.type = MAKE_STRING("array");
						network_type.is_array = true;

						ASSERT_NEXT_TOKEN_TYPE(tok, '{');

						Member &member = network_type.member;
						token = parser::next_token(&tok);
						if (token.type == TokenType_CommandMarkerBlock) {
							ASSERT_NEXT_TOKEN(tok, export_token);
							parse_member_export_settings(arena, tok, member);
							token = parser::next_token(&tok);
						};
						member.type = clone_string(token.string, arena);
						member.type_id = make_string_id(member.type);

						token = parser::next_token(&tok);
						member.name = clone_string(token.string, arena);
						member.name_id = make_string_id(member.name);

						ASSERT_NEXT_TOKEN_TYPE(tok, '[');
						token = parser::next_token(&tok);
						network_type.max_size = clone_string(token.string, arena);
						ASSERT_NEXT_TOKEN_TYPE(tok, ']');
						ASSERT_NEXT_TOKEN_TYPE(tok, ';');

						ASSERT_NEXT_TOKEN(tok, unsigned_token);
						ASSERT_NEXT_TOKEN(tok, count_token);
					}

				///////////// State (for a state machine) /////////////
				} else if (parser::is_equal(token, state_token)) {
					ARRAY_CHECK_BOUNDS(state_array);
					State &state = state_array.entries[state_array.count++];

					ASSERT_NEXT_TOKEN_TYPE(tok, '(');
					state.machine_name = clone_string(parser::next_token(&tok).string, arena);
					state.machine_name_id = make_string_id(state.machine_name);
					ASSERT_NEXT_TOKEN_TYPE(tok, ')');

					state.path = make_project_path(global_last_parser_context->filepath, arena, CODE_STRING, 2, false);

					char *at = tok.at;

					do {
						token = parser::next_token(&tok);
					} while (!parser::is_equal(token, class_token) && !parser::is_equal(token, struct_token));

					state.name = clone_string(parser::next_token(&tok).string, arena);

					tok.at = at;
				} else if (parser::is_equal(token, settings_token)) {
					token = parser::next_token(&tok);
					if (parser::is_equal(token, struct_token)) {
						token = parser::next_token(&tok);

						ARRAY_CHECK_BOUNDS(settings_struct_array);
						SettingsStruct &settings_struct = settings_struct_array.entries[settings_struct_array.count++];
						settings_struct.name = clone_string(token.string, arena);
						settings_struct.name_id = make_string_id(settings_struct.name);

						ASSERT_NEXT_TOKEN_TYPE(tok, '{')

						token = parser::next_token(&tok);
						parse_struct_members(arena, tok, token, &settings_struct.member_count, settings_struct.members);
					} else if (parser::is_equal(token, enum_token)) {
						token = parser::next_token(&tok);

						ARRAY_CHECK_BOUNDS(settings_enum_array);
						SettingsEnum &enum_entry = settings_enum_array.entries[settings_enum_array.count++];
						enum_entry.name = clone_string(token.string, arena);
						enum_entry.name_id = make_string_id(enum_entry.name);

						ASSERT_NEXT_TOKEN_TYPE(tok, '{')

						while (true) {
							token = parser::next_token(&tok);
							if (token.type == '}') {
								break;
							}

							enum_entry.entries[enum_entry.count++] = clone_string(token.string, arena);

							token = parser::next_token(&tok);
							if (token.type == '=') {
								parser::all_until(&tok, '\n');
							} else if (token.type == ',') {

							}
						}

					} else {
						PARSER_ASSERT(false, "Settings token found but not struct nor enum followed! (token=%s)", printable_token(token));
					}
				} else {
					bool recognized_but_ignored_command =
							parser::is_equal(token, max_token) ||
							parser::is_equal(token, namespace_token)     ||
							parser::is_equal(token, count_token)         ||
							parser::is_equal(token, component_token)     ||
							parser::is_equal(token, default_value_token) ||
							parser::is_equal(token, reloadable_token)    ||
							parser::is_equal(token, not_reloadable_token)||
							parser::is_equal(token, BEGIN_GENERATED_token);
					PARSER_ASSERT(recognized_but_ignored_command, "Unknown command marker! (command=%s)", printable_token(token));
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}

	if (current_receiver.num_events > 0) {
		ARRAY_CHECK_BOUNDS(receiver_array);
		receiver_array.entries[receiver_array.count++] = current_receiver;
	}
}

void parse_game_hfile(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, NetworkTypeArray &network_type_array, ReceiverArray &receiver_array, StateArray &state_array, SettingsEnumArray &settings_enum_array, SettingsStructArray &settings_struct_array) {
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

	search_for_events(arena, tok, rpc_array, event_array, flow_array, network_type_array, receiver_array, state_array, settings_enum_array, settings_struct_array);
}
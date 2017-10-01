void build_function(MemoryArena &arena, parser::Tokenizer &tok, bool is_rpc, Function &function) {
	array_init(function.parameters, 16);
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

		array_push(function.parameters, parameter);

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

void parse_struct_members(MemoryArena &arena, parser::Tokenizer &tok, parser::Token &token, MemberArray &member_array) {
	bool found_struct_end = false;
	while (!found_struct_end) {

		Member m = { 0 };

		if (token.type == TokenType_CommandMarkerBlock) {
			ASSERT_NEXT_TOKEN(tok, export_token);
			parse_member_export_settings(arena, tok, m);
			token = parser::next_token(&tok);
		}
		while (token.type == TokenType_CommandMarker) {
			PARSER_ASSERT(m.exported_line_counter == 0, "Command markers after the export block command is not allowed!");
			token = parser::next_token(&tok);
			if (parser::is_equal(token, export_token)) {
				m.exported_lines[m.exported_line_counter++] = clone_string(parser::next_line(&tok).string, arena);

				token = parser::next_token(&tok);
			} else if (parser::is_equal(token, default_value_token)) {
				m.default_value = clone_string(parser::next_line(&tok).string, arena);

				token = parser::next_token(&tok);
			} else if (parser::is_equal(token, behavior_token)) {
				m.flags |= MemberFlag_IsBehaviorNode;

				token = parser::next_token(&tok);
				if (token.type == '(') {
					while (token.type != ')') {
						token = parser::next_token(&tok);
						if (parser::is_equal(token, reset_token)) {
							m.flags |= MemberFlag_BehaviorNodeReset;
						} else if (parser::is_equal(token, prefix_token)) {
							m.flags |= MemberFlag_BehaviorNodePrefix;
						} else {
							PARSER_ASSERT(false, "Unrecognized behavior command! Available commands are: 'reset'. (command=%s)", printable_token(token));
						}
						token = parser::next_token(&tok);
					}
					token = parser::next_token(&tok);
				}
			} else {
				PARSER_ASSERT(false, "Unrecognized command! (command=%s)", printable_token(token));
			}
		}

		if (token.type == '}') {
			found_struct_end = true;
		} else {
			ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
			if (parser::is_equal(token, const_token) || parser::is_equal(token, static_token)) {
				token = parser::next_token(&tok);
				ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
			}
			m.type = clone_string(token.string, arena);
			m.type_id = make_string_id(m.type);

			token = parser::next_token(&tok);
			if (token.type == '*') {
				SET_FLAG(m, MemberFlag_IsPointer);
				token = parser::next_token(&tok);
			}
			m.name = clone_string(token.string, arena);
			m.name_id = make_string_id(m.name);

			token = parser::next_token(&tok);
			if (token.type == '[') { // found array
				token = parser::next_token(&tok);
				String max_count = clone_string(token.string, arena);
				float number;
				bool found_number = parser::try_parse_number(max_count.text, number);
				ASSERT(found_number, "Only numeric literals supported!")
				m.max_count = number;
				// if (m.is_pointer) {
				// 	m.is_pointer_pointer = true;
				// }
				SET_FLAG(m, MemberFlag_IsPointer);
				SET_FLAG(m, MemberFlag_IsArray);
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

			array_push(member_array, m);
		}
	}
}

void skip_function_body(parser::Tokenizer *tok) {
	unsigned scope_counter = 1;

	parser::Token token;
	while (scope_counter > 0) {
		token = parser::next_token(tok);
		if (token.type == '{') {
			scope_counter++;
		} else if (token.type == '}') {
			scope_counter--;
		}
	}

	// Remove the ; if it's there
	token = parser::peek_next_token(tok);
	if (token.type == ';') {
		parser::next_token(tok);
	}
}

Parameter parse_return_type(MemoryArena &arena, parser::Tokenizer &tok, parser::Token &token) {
	Parameter return_type = {};

	if (parser::is_equal(token, const_token)) {
		return_type.is_const = true;
		token = parser::next_token(&tok);
	}

	return_type.type = token.string;

	token = parser::next_token(&tok);
	if (token.type == TokenType_Namespace) {
		return_type.type.length += 2;
		token = parser::next_token(&tok);
		return_type.type.length += token.length;
		token = parser::next_token(&tok);
	}

	if (token.type == '&') {
		token = parser::next_token(&tok);
		return_type.is_ref = true;
	} else if (token.type == '*') {
		token = parser::next_token(&tok);
		return_type.is_pointer = true;
	}

	return_type.type = clone_string(return_type.type, arena);
	return_type.type_id = make_string_id(return_type.type);

	return return_type;
}

void parse_parameters(MemoryArena &arena, parser::Tokenizer &tok, ParameterArray &parameters) {
	array_init(parameters, 16);

	ASSERT_NEXT_TOKEN_TYPE(tok, '(');

	ASSERT_NEXT_TOKEN(tok, TOKENIZE("Context"));
	ASSERT_NEXT_TOKEN_TYPE(tok, '&');
	ASSERT_NEXT_TOKEN(tok, TOKENIZE("c"));

	parser::Token token;
	while (true) {
		token = parser::next_token(&tok);
		if (token.type == ')') {
			break;
		}

		ASSERT_TOKEN_TYPE(token, ',');
		token = parser::next_token(&tok);

		array_new_entry(parameters);
		Parameter &parameter = array_last(parameters);
		ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
		parameter.is_const = parser::is_equal(token, const_token);
		if (parameter.is_const) {
			token = parser::next_token(&tok);
		}
		parameter.type = clone_string(token.string, arena);

		token = parser::next_token(&tok);

		while (parser::is_equal(token, TOKENIZE("::"))) {
			token = parser::next_token(&tok);
			unsigned new_length = parameter.type.length + 2 + token.string.length + 1;

			String new_type = allocate_string(arena, new_length);
			append_string(new_type, parameter.type);
			append_string(new_type, MAKE_STRING("::"));
			append_string(new_type, token.string);
			null_terminate(new_type);

			parameter.type = new_type;

			token = parser::next_token(&tok);
		}

		parameter.type_id = make_string_id(parameter.type);

		parameter.is_ref = token.type == '&';
		if (parameter.is_ref)
			token = parser::next_token(&tok);

		parameter.is_pointer = token.type == '*';
		if (parameter.is_pointer)
			token = parser::next_token(&tok);

		parameter.name = clone_string(token.string, arena);
		parameter.name_id = make_string_id(parameter.name);
	}

	ASSERT_TOKEN_TYPE(token, ')');
	token = parser::next_token(&tok);
	if (token.type == '{') {
		skip_function_body(&tok);
	}
}

void parse_behavior_node_collection(MemoryArena &arena, parser::Tokenizer &tok, String &collection_name, BehaviorNodeArray &behavior_node_array) {
	while (true) {
		parser::Token token = parser::next_token(&tok);
		if (token.type == '}')
			break;

		array_new_entry(behavior_node_array);
		BehaviorNode &node = array_last(behavior_node_array);
		node.collection_name = collection_name;
		node.collection_name_id = make_string_id(collection_name);

		if (token.type == TokenType_CommandMarker) {
			token = parser::next_token(&tok);

			if (parser::is_equal(token, dependencies_token)) {
				ASSERT_NEXT_TOKEN_TYPE(tok, '(');

				while (token.type != ')') {
					token = parser::next_token(&tok);
					unsigned collection_name_id = make_string_id(token.string);

					ASSERT_NEXT_TOKEN_TYPE(tok, '.');

					token = parser::next_token(&tok);
					unsigned name_id = make_string_id(token.string);

					PARSER_ASSERT(node.dependency_count < ARRAY_COUNT(node.dependencies), "Array index out of bounds!");
					node.dependencies[node.dependency_count++] = (uint64_t)collection_name_id << 32 | name_id;

					token = parser::next_token(&tok);
				}
				token = parser::next_token(&tok);
			}

			PARSER_ASSERT(!parser::is_equal(token, dependencies_token), "The 'dependencies'-command needs to come first (before locals)");
 		}

		node.return_type = parse_return_type(arena, tok, token);

		node.name = clone_string(token.string, arena);
		node.name_id = make_string_id(node.name);

		parse_parameters(arena, tok, node.parameters);
	}
}

void parse_ability_node_collection(MemoryArena &arena, parser::Tokenizer &tok, String &collection_name, AbilityNodeArray &ability_node_array) {
	while (true) {
		parser::Token token = parser::next_token(&tok);
		if (token.type == '}')
			break;

		array_new_entry(ability_node_array);
		AbilityNode &node = array_last(ability_node_array);
		node.collection_name = collection_name;
		node.collection_name_id = make_string_id(collection_name);

		if (token.type == TokenType_CommandMarker) {
			token = parser::next_token(&tok);
			if (parser::is_equal(token, undo_token)) {
				ASSERT_NEXT_TOKEN_TYPE(tok, '(');
				token = parser::next_token(&tok);

				node.undo_name = clone_string(token.string, arena);
				node.undo_name_id = make_string_id(node.undo_name);

				ASSERT_NEXT_TOKEN_TYPE(tok, ')');

				token = parser::next_token(&tok);
			}
		}

		node.return_type = parse_return_type(arena, tok, token);

		node.name = clone_string(token.string, arena);
		node.name_id = make_string_id(node.name);

		parse_parameters(arena, tok, node.parameters);
	}
}

void parse_ability_data_node_collection(MemoryArena &arena, parser::Tokenizer &tok, String &collection_name, AbilityNodeArray &ability_node_array) {
	while (true) {
		parser::Token token = parser::next_token(&tok);
		if (token.type == '}')
			break;

		array_new_entry(ability_node_array);
		AbilityNode &node = array_last(ability_node_array);
		array_init(node.parameters, 32);

		node.collection_name = collection_name;
		node.collection_name_id = make_string_id(collection_name);

		array_new_entry(node.parameters);
		Parameter &parameter = array_last(node.parameters);
		parameter.type = clone_string(token.string, arena);
		parameter.type_id = make_string_id(parameter.type);

		token = parser::next_token(&tok);
		node.name = clone_string(token.string, arena);
		node.name_id = make_string_id(node.name);

		parameter.name = node.name;
		parameter.name_id = node.name_id;

		ASSERT_NEXT_TOKEN_TYPE(tok, ';');
	}
}

void fill_event(MemoryArena &arena, parser::Tokenizer &tok, Event &event, Receiver &current_receiver, bool is_entity_event) {
	if (is_entity_event) {
		parser::Token token = parser::next_token(&tok);
		PARSER_ASSERT(parser::is_equal(token, void_token) || parser::is_equal(token, bool_token), "Return type on entity events must be either void or bool!");
	} else {
		ASSERT_NEXT_TOKEN(tok, void_token); // Return types not allowed on events!
	}

	build_function(arena, tok, false, event.function);

	PARSER_ASSERT(current_receiver.name_id != 0, "No valid receiver for event function! (event=%s)", *event.function.name);
	current_receiver.num_events++;
	event.function.receiver_name_id = current_receiver.name_id;
}

void search_for_events(MemoryArena &arena, parser::Tokenizer &tok, HFileCollection &collection, Receiver current_receiver = receiver::make_empty(), Component *component = 0) {
	bool parsing = true;
	bool in_generated_block = false;

	unsigned current_receiver_index = array_count(collection.receiver_array);

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
							array_push(collection.receiver_array, current_receiver);
						}

						current_receiver = receiver::make_data(class_name, arena);
						current_receiver_index = array_count(collection.receiver_array);
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

					array_new_entry(collection.rpc_array);
					RPCFunction &rpc = array_last(collection.rpc_array);
					rpc.function = function;
					rpc.session_bound = session_bound;

				///////////// EVENT /////////////
				} else if (parser::is_equal(token, event_token)) {
					array_new_entry(collection.event_array);
					Event &event = array_last(collection.event_array);
					fill_event(arena, tok, event, current_receiver, false);

				///////////// ENTITY EVENT /////////////
				} else if (parser::is_equal(token, entity_event_token)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '(');

					array_new_entry(collection.event_array);
					Event &event = array_last(collection.event_array);
					while (true) {
						token = parser::next_token(&tok);
						if (token.type == ')')
							break;
						if (token.type == ',')
							continue;

						unsigned id = make_string_id(token.string);
						event.component_ids[event.component_count++] = id;
					}

					fill_event(arena, tok, event, current_receiver, true);

				///////////// FLOW /////////////
				} else if (parser::is_equal(token, flow_token)) {
					FlowFunction flow = {};
					current_receiver.num_events++;
					flow.function.receiver_name_id = current_receiver.name_id;
					flow.function_map.count = 0;

					array_init(flow.return_list, 8);

					token = parser::peek_next_token(&tok);
					if (parser::is_equal(token, map_token)) {
						token = parser::next_token(&tok);
						ASSERT_NEXT_TOKEN_TYPE(tok, '(');

						bool found_end = false;
						while (!found_end) {
							token = parser::next_token(&tok);

							ASSERT_TOKEN_TYPE(token, TokenType_Identifier); // We need a name here, map() doesn't make any sense!
							flow.function_map.push_back(clone_string(token.string, arena));

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

							array_push(flow.return_list, parameter);

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

					// Yuck!!
					for (int i = 0; i < array_count(flow.return_list); ++i) {
						Parameter &return_parameter = flow.return_list[i];

						bool found_input_with_same_name = false;
						String name;
						for (int k = 0; k < array_count(flow.function.parameters); ++k) {
							Parameter &in_argument = flow.function.parameters[k];
							if (in_argument.name_id == return_parameter.name_id) {
								name = in_argument.name;
								found_input_with_same_name = true;
								break;
							}
						}
						PARSER_ASSERT(!found_input_with_same_name, "Flow node in arguments cannot have the same name as return values. (name=%s)", *name);
					}

					array_push(collection.flow_array, flow);

				///////////// NETWORK TYPES /////////////
				} else if (parser::is_equal(token, network_type_token)) {
					token = parser::next_token(&tok);
					if (parser::is_equal(token, enum_token)) {
						array_new_entry(collection.network_type_array);
						NetworkType &network_type = array_last(collection.network_type_array);

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
						array_new_entry(collection.network_type_array);
						NetworkType &network_type = array_last(collection.network_type_array);

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
					array_new_entry(collection.state_array);
					State &state = array_last(collection.state_array);

					ASSERT_NEXT_TOKEN_TYPE(tok, '(');
					state.machine_name = clone_string(parser::next_token(&tok).string, arena);
					state.machine_name_id = make_string_id(state.machine_name);
					ASSERT_NEXT_TOKEN_TYPE(tok, ')');

					state.path = make_project_path(global_last_parser_context->filepath, arena, _code, 2, false);

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

						array_new_entry(collection.settings_struct_array);
						SettingsStruct &settings_struct = array_last(collection.settings_struct_array);
						array_init(settings_struct.member_array, 32);
						settings_struct.name = clone_string(token.string, arena);
						settings_struct.name_id = make_string_id(settings_struct.name);

						ASSERT_NEXT_TOKEN_TYPE(tok, '{')

						token = parser::next_token(&tok);
						parse_struct_members(arena, tok, token, settings_struct.member_array);
					} else if (parser::is_equal(token, enum_token)) {
						token = parser::next_token(&tok);

						array_new_entry(collection.settings_enum_array);
						SettingsEnum &enum_entry = array_last(collection.settings_enum_array);
						array_init(enum_entry.entry_array, 64);
						enum_entry.name = clone_string(token.string, arena);
						enum_entry.name_id = make_string_id(enum_entry.name);

						token = parser::next_token(&tok);
						if (token.type == ':') {
							parser::next_line(&tok);
						} else if (token.type == '{') {

						} else {
							PARSER_ASSERT(false, "Unexpected token! Expected ':' or '{', but found '%.*s'", token.string.length, *token.string);
						}

						while (true) {
							token = parser::next_token(&tok);
							if (token.type == '}') {
								break;
							}

							array_push(enum_entry.entry_array, clone_string(token.string, arena));

							token = parser::next_token(&tok);
							if (token.type == '=') {
								parser::all_until(&tok, '\n');
							} else if (token.type == ',') {

							} else if (token.type == '}') {
								break;
							}
						}

					} else {
						PARSER_ASSERT(false, "Settings token found but not struct nor enum followed! (token=%s)", printable_token(token));
					}
				} else if (parser::is_equal(token, behavior_node_collection_token)) {
					ASSERT_NEXT_TOKEN(tok, TOKENIZE("namespace"));
					token = parser::next_token(&tok);

					String name = clone_string(token.string, arena);
					to_lower(name);

					ASSERT_NEXT_TOKEN_TYPE(tok, '{');
					parse_behavior_node_collection(arena, tok, name, collection.behavior_node_array);
				} else if (parser::is_equal(token, ability_node_collection_token)) {
					ASSERT_NEXT_TOKEN(tok, TOKENIZE("namespace"));
					token = parser::next_token(&tok);

					String name = clone_string(token.string, arena);
					to_lower(name);

					ASSERT_NEXT_TOKEN_TYPE(tok, '{');
					parse_ability_node_collection(arena, tok, name, collection.ability_node_array);
				} else if (parser::is_equal(token, ability_data_node_collection_token)) {
					String name = {};

					token = parser::peek_next_token(&tok);
					if (token.type == '(') {
						token = parser::next_token(&tok); // Swallow
						token = parser::next_token(&tok);
						name = clone_string(token.string, arena);
						ASSERT_NEXT_TOKEN_TYPE(tok, ')');
					}
					ASSERT_NEXT_TOKEN(tok, TOKENIZE("struct"));
					token = parser::next_token(&tok);

					if (name.length == 0) {
						name = clone_string(token.string, arena);
					}
					to_lower(name);

					ASSERT_NEXT_TOKEN_TYPE(tok, '{');
					parse_ability_data_node_collection(arena, tok, name, collection.ability_node_array);
				} else {
					bool recognized_but_ignored_command =
							parser::is_equal(token, max_token)            ||
							parser::is_equal(token, namespace_token)      ||
							parser::is_equal(token, count_token)          ||
							parser::is_equal(token, component_token)      ||
							parser::is_equal(token, default_value_token)  ||
							parser::is_equal(token, reloadable_token)     ||
							parser::is_equal(token, reloader_token)       ||
							parser::is_equal(token, not_reloadable_token) ||
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
		array_push(collection.receiver_array, current_receiver);
	}
}

void parse_game_hfile(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, HFileCollection &collection) {
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

	search_for_events(arena, tok, collection);
}

void skip_function(parser::Tokenizer *tok) {
	bool found_end_of_parameters = false;
	while(!found_end_of_parameters) {
		parser::Token token = parser::next_token(tok);
		found_end_of_parameters = token.type == ')';
	}

	parser::Token token = parser::peek_next_token(tok);
	if (token.type == ';') {
		parser::next_token(tok);
		return; // This is just a function definition, e.g. void dostuff();
	}

	// We have an implementation of the function, search for the opening bracket and then find the matching closing one.
	bool found_block_start = false;
	while(!found_block_start) {
		parser::Token token = parser::next_token(tok);
		found_block_start = token.type == '{';
	}

	unsigned scope_counter = 1;

	while (scope_counter > 0) {
		parser::Token token = parser::next_token(tok);
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

String coalesce_strings(String *strings, int count, MemoryArena &arena) {
	int total_length = 0;
	for (int i = 0; i < count; i++) {
		total_length += strings[i].length;
	}
	total_length++; // Include null terminator!
	String new_string = make_string(allocate_memory(arena, total_length), total_length-1);

	int index = 0;
	for (int i = 0; i < count; i++) {
		String &string = strings[i];
		for (int j = 0; j < string.length; j++) {
			new_string.text[index++] = string[j];
		}
	}
	new_string.text[index++] = '\0';
	PARSER_ASSERT(index == total_length, "Index out of bounds! Allocation failed? (text=%s)", new_string.text);

	return new_string;
}

String make_full_type_underlined(String &full_type, MemoryArena &arena) {
	String result = clone_string(full_type, arena);
	for (int i = 0; i < result.length; ++i) {
		if (result.text[i] == ':') {
			result.text[i] = '_';
		}
	}
	return result;
}

void add_reloadable_struct(MemoryArena &arena, parser::Tokenizer &tok, ReloadableStruct &reloadable_struct, bool in_private_mode) {
	parser::Token token = parser::next_token(&tok);

	bool allow_private_members = false;
	bool found_end_of_struct = false;
	while (!found_end_of_struct) {
		if (token.type == '}') {
			found_end_of_struct = true;
			break;
		}

		if (token.type == TokenType_CommandMarker) {
			token = parser::next_line(&tok); // swallow line
			token = parser::next_token(&tok); // place us correctly again
			continue;
		}

		if (token.type == TokenType_CommandMarkerBlock) {
			token = parser::next_token(&tok);
			while (!try_consume_block_comment_closing(&tok)) // treat the command block as a comment and skip it
				tok.at++;
			token = parser::next_token(&tok); // place us correctly again
			continue;
		}

		if (token.type == '#') { // found preprocessor directive
			bool end_found = false;
			while (!end_found) {
				token = parser::next_line(&tok); // swallow line
				for (int i = token.length - 1; i >= 0; --i) {
					if (parser::is_whitespace(token.string[i])) // It isn't allowed to have whitspaces after an escaped newline, but it's better that the compiler give the error than us
						continue;
					end_found = token.string[i] != '\\'; // We reached the end of the preprocessor directive if we found no escaped newlines
					break;
				}
			}
			token = parser::next_token(&tok); // place us correctly again
			continue;
		}

		if (token.type == '~') { // found destructor
			token = parser::next_token(&tok); // skip the name
			ASSERT_NEXT_TOKEN_TYPE(tok, '(');
			skip_function(&tok); // skip the function
			token = parser::next_token(&tok); // place us correctly again
			continue;
		}

		PARSER_ASSERT(token.type == TokenType_Identifier, "Unexpected token found '%.*s'", token.length, *token.string);
		PARSER_ASSERT(!parser::is_equal(token, union_token), "'union' not supported in reloadable structs.")

		if (parser::is_equal(token, friend_token)) { // found friend declaration

			ASSERT_NEXT_TOKEN(tok, struct_token);

			token = parser::next_token(&tok);
			if (!allow_private_members) {
				allow_private_members = parser::is_equal(token, Reloader_token);
			}
			ASSERT_NEXT_TOKEN_TYPE(tok, ';');
			token = parser::next_token(&tok);
			continue;
		}

		String member_string_buffer[8];
		unsigned counter = 0;
		member_string_buffer[counter++] = token.string;

		if (parser::is_equal(token, static_token)) {
			token = parser::peek_next_line(&tok);
			if(parser::ends_in(token, ';')) {
				// Ignore static declared thing... variable, function... whatever.
				parser::next_line(&tok);
			} else {
				// Ignore static functions
				skip_function(&tok);
			}
			token = parser::next_token(&tok);
			continue;
		}
		if (parser::is_equal(token, const_token)) {
			parser::all_until(&tok, '\n');
			token = parser::next_token(&tok);
			continue;
		}
		if (parser::is_equal(token, __forceinline_token)) {
			skip_function(&tok);
			token = parser::next_token(&tok);
			continue;
		}
		if (parser::is_equal(token, inline_token)) {
			skip_function(&tok);
			token = parser::next_token(&tok);
			continue;
		}
		if (parser::is_equal(token, enum_token)) {
			parser::all_until(&tok, '}');
			ASSERT_NEXT_TOKEN_TYPE(tok, '}');
			token = parser::next_token(&tok);
			if (token.type == ';') {
				token = parser::next_token(&tok);
			}
			continue;
		}

		String previous_string = token.string;
		token = parser::next_token(&tok);
		if (token.type == '(') { // found constructor
			skip_function(&tok);
			token = parser::next_token(&tok);
			continue;
		} else if (token.type == ':') { // found public:, private:
			if (are_strings_equal(previous_string, MAKE_STRING("private"))) {
				in_private_mode = true;
			} else if (are_strings_equal(previous_string, MAKE_STRING("public"))) {
				in_private_mode = false;
			}
			token = parser::next_token(&tok);
			continue;
		}

		while (token.type == TokenType_Namespace) {
			member_string_buffer[counter++] = token.string;
			PARSER_ASSERT(counter < ARRAY_COUNT(member_string_buffer), "Too many chained namespaces!");

			token = parser::next_token(&tok);
			member_string_buffer[counter++] = token.string;
			PARSER_ASSERT(counter <= ARRAY_COUNT(member_string_buffer), "Too many chained namespaces!");

			token = parser::next_token(&tok);
		}
		bool is_pointer = false;
		bool is_reference = false;

		// TODO(kalle): Support for pointers to pointers
		if (token.type == '*') {
			is_pointer = true;
			token = parser::next_token(&tok);
		} else if (token.type == '&') {
			is_reference = true;
			token = parser::next_token(&tok);
		} else if (parser::is_equal(token, volatile_token)) {
			token = parser::next_token(&tok);
		}

		PARSER_ASSERT(token.type != '*', "No support for pointers to pointers yet.");

		ReloadableMember m = { 0 };
		m.name = clone_string(token.string, arena);
		m.name_id = make_string_id(m.name);

		m.type = clone_string(member_string_buffer[counter-1], arena);
		m.type_id = make_string_id(m.type);

		m.full_type = coalesce_strings(member_string_buffer, counter, arena);
		m.full_type_id = make_string_id(m.full_type);

		m.full_type_underlined = make_full_type_underlined(m.full_type, arena);

		m.is_pointer = is_pointer;

		token = parser::next_token(&tok);
		if (token.type == '(') { // found function
			skip_function(&tok);
			token = parser::next_token(&tok);
			continue;
		} else {
			if (token.type == '[') { // found array
				token = parser::all_until(&tok, ']');
				m.max_count = clone_string(token.string, arena);
				if (m.is_pointer) {
					m.is_pointer_pointer = true;
				}
				m.is_pointer = true;
				m.is_array = true;
				token = parser::next_token(&tok); // Move up to ']'
				token = parser::next_token(&tok); // move past ']'
			}

			ASSERT_TOKEN_TYPE(token, ';');

			token = parser::next_token(&tok);
			if (token.type == TokenType_CommandMarker) {
				token = parser::next_token(&tok);
				if (parser::is_equal(token, count_token)) {
					token = parser::next_token(&tok);
					m.count = clone_string(token.string, arena);
					token = parser::next_token(&tok);
				} else if (parser::is_equal(token, namespace_token)) {
					token = parser::next_token(&tok);

					String new_type = allocate_string(arena, token.length + 2 + m.type.length + 1);
					append_strings(new_type, token.string, MAKE_STRING("::"));
					append_string(new_type, m.type);
					null_terminate(new_type);
					m.type = new_type;

					token = parser::next_token(&tok);
				} else if (parser::is_equal(token, BEGIN_GENERATED_token)) {
					token = parser::next_token(&tok);
				} else if (parser::is_equal(token, TOKENIZE("default_value"))) {
					token = parser::next_line(&tok);
					token = parser::next_token(&tok);
				} else {
					// PARSER_ASSERT(false, "Unrecognized command marker! (command=%s)", printable_token(token));
					token = parser::next_token(&tok);
				}
			}

			if (in_private_mode) {
				PARSER_ASSERT(allow_private_members, "Private members are not allowed in reloadable class without friending the Reloader. Add 'friend struct Reloader' to the class!");
			}

			ARRAY_CHECK_BOUNDS_COUNT(reloadable_struct.members, reloadable_struct.member_count);
			reloadable_struct.members[reloadable_struct.member_count++] = m;
		}

		if (token.type == '}') {
			found_end_of_struct = true;
		}
	}
}

struct NamespaceInfo {
	String name;
	unsigned starting_bracket_count;
};

void parse_hfile(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, ReloadableArray &reloadable_array) {
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

	unsigned brackets = 0;

	NamespaceInfo namespace_stack[16] = {};
	unsigned namespace_count = 0;

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case '{': {
				brackets++;
			} break;
			case '}': {
				brackets--;
				if (namespace_count > 0) {
					NamespaceInfo &info = namespace_stack[namespace_count-1];
					if (brackets == info.starting_bracket_count) {
						namespace_count--;
					}
				}
			} break;
			case TokenType_Identifier: {
				if (parser::is_equal(token, namespace_token)) {
					parser::Token token = parser::next_token(&tok);
					if (token.type == '{') {
						brackets++;
					} else {
						if (parser::peek_next_token(&tok).type != ';') {
							ARRAY_CHECK_BOUNDS_COUNT(namespace_stack, namespace_count);
							NamespaceInfo &info = namespace_stack[namespace_count++];

							info.starting_bracket_count = brackets;
							info.name = clone_string(token.string, arena);
						}
					}
				} else {
					bool is_class = false;
					bool is_struct = parser::is_equal(token, struct_token);
					if (!is_struct) {
						is_class = parser::is_equal(token, class_token);
					}

					if (is_struct || is_class) {
						parser::Token token = parser::next_token(&tok);
						String name = clone_string(token.string, arena);

						token = parser::next_token(&tok);
						if (token.type == ';') // forward declared struct, ignore!
							continue;

						ASSERT_TOKEN_TYPE(token, '{') // We don't support inheritance

						ARRAY_CHECK_BOUNDS(reloadable_array);
						ReloadableStruct &reloadable_struct = reloadable_array.entries[reloadable_array.count++];

						reloadable_struct.namespace_count = namespace_count;

						if (namespace_count > 0) {
							unsigned string_count = namespace_count*2 + 1;

							String *strings = (String *)allocate_memory(temp_arena, string_count * sizeof(String));
							unsigned count = 0;
							for (int i = 0; i < namespace_count; ++i) {
								reloadable_struct.namespaces[i] = namespace_stack[i].name;

								strings[count++] = namespace_stack[i].name;
								strings[count++] = MAKE_STRING("::");
							}
							strings[count++] = name;

							String full_name = coalesce_strings(strings, count, arena);
							reloadable_struct.full_name = full_name;
							reloadable_struct.full_name_underlined = make_full_type_underlined(full_name, arena);
						} else {
							reloadable_struct.full_name = name;
							reloadable_struct.full_name_underlined = name;
						}

						reloadable_struct.name = name;
						reloadable_struct.name_id = make_string_id(name);

						bool in_private_mode = is_class; // classes start off private, while structs start off public
						add_reloadable_struct(arena, tok, reloadable_struct, in_private_mode);
					}
				}
			} break;
			case TokenType_CommandMarker: {
				token = parser::next_token(&tok);
				if (parser::is_equal(token, not_reloadable_token)) {
					token = parser::next_token(&tok); // Swallow the class/struct declaration
				}

			// 	else if (parser::is_equal(token, reloadable_token)) {
			// 		ReloadableStruct &reloadable_struct = reloadable_array.entries[reloadable_array.count++];

			// 		reloadable_struct.added_behavior = CONSTRUCT;

			// 		token = parser::next_token(&tok);
			// 		if (parser::is_equal(token, on_added_token)) {
			// 			ASSERT_NEXT_TOKEN_TYPE(tok, '(');

			// 			token = parser::next_token(&tok);
			// 			if (parser::is_equal(token, zero_token)) {
			// 				reloadable_struct.added_behavior = ZERO;
			// 			} else if (parser::is_equal(token, construct_token)) {
			// 				reloadable_struct.added_behavior = CONSTRUCT;
			// 			} else {
			// 				PARSER_ASSERT(false, "Unrecognized command! (command=%s, filepath=%s)", printable_token(token), filepath);
			// 			}
			// 			ASSERT_NEXT_TOKEN_TYPE(tok, ')');
			// 			token = parser::next_token(&tok);
			// 		}
			// 		bool is_struct = parser::is_equal(token, struct_token);
			// 		bool is_class  = parser::is_equal(token, class_token);
			// 		PARSER_ASSERT(token.type == TokenType_Identifier && (is_struct || is_class), "Can only mark classes or structs with the reloadable command. (found=%s)", printable_token(token));

			// 		parser::Token token = parser::next_token(&tok);
			// 		String name = clone_string(token.string, arena);
			// 		reloadable_struct.full_name = name;
			// 		reloadable_struct.name = name;
			// 		reloadable_struct.name_id = make_string_id(name);

			// 		ASSERT_NEXT_TOKEN_TYPE(tok, '{') // We don't support inheritance

			// 		add_reloadable_struct(arena, tok, reloadable_struct, is_class);
			// 	}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}
}

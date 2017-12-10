void parse_state_machine(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, StateMachineArray &state_machine_array) {
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

	ARRAY_CHECK_BOUNDS(state_machine_array);
	StateMachine &state_machine = state_machine_array.entries[state_machine_array.count++];

	parser::Token token = parser::next_token(&tok);
	state_machine.name = clone_string(token.string, arena);
	state_machine.name_id = make_string_id(token.string);

	ASSERT_NEXT_TOKEN_TYPE(tok, '{');

	token = parser::next_token(&tok);
	while (true) {
		ASSERT_TOKEN(tok, token, void_token); // Return types not allowed on rpcs!
		ARRAY_CHECK_BOUNDS_COUNT(state_machine.functions, state_machine.function_count);
		Function &function = state_machine.functions[state_machine.function_count++];
		build_function(arena, tok, false, function);
		ASSERT_NEXT_TOKEN_TYPE(tok, ';');

		parser::Token token = parser::next_token(&tok);
		if (token.type == '}') {
			break;
		}
	}
	ASSERT_NEXT_TOKEN_TYPE(tok, ';');
}

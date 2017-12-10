static String external_event = MAKE_STRING("external_event");

void parse_for_flow_events(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, EventArray &event_array, EventHashMap &event_hash_map) {
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

	bool parsing = true;
	while (tok.at[0] != '\0') {
		bool mismatch = false;
		int i = 0;
		for (i = 0; i < external_event.length && !mismatch; i++) {
			mismatch = tok.at[i] != external_event[i];
		}
		tok.at += i;

		if (!mismatch) {
			tok.at++;
			parser::consume_whitespace(&tok);
			ASSERT_NEXT_TOKEN(tok, TOKENIZE("variable_values"));
			ASSERT_NEXT_TOKEN_TYPE(tok, '=');
			ASSERT_NEXT_TOKEN_TYPE(tok, '{');
			ASSERT_NEXT_TOKEN(tok, TOKENIZE("event_name"));
			ASSERT_NEXT_TOKEN_TYPE(tok, '=');
			parser::Token token = parser::next_token(&tok);
			add_event(arena, token.string, event_array, event_hash_map);
			ASSERT_TOKEN_TYPE(token, TokenType_String);
		}
	}
}

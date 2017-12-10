static String events = MAKE_STRING("events");

void parse_for_animation_events(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, EventArray &event_array, EventHashMap &event_hash_map) {
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
		for (i = 0; i < events.length && !mismatch; i++) {
			mismatch = tok.at[i] != events[i];
		}
		tok.at += i;

		if (!mismatch) {
			tok.at++;
			parser::consume_whitespace(&tok);
			ASSERT_NEXT_TOKEN_TYPE(tok, '=');
			ASSERT_NEXT_TOKEN_TYPE(tok, '{');

			while (true) {
				parser::Token token = parser::next_token(&tok);
				if (token.type != TokenType_String) {
					add_event(arena, token.string, event_array, event_hash_map);
				}
				ASSERT_NEXT_TOKEN_TYPE(tok, '=');
				ASSERT_NEXT_TOKEN_TYPE(tok, '{');

				while (tok.at[0] != '}')
					tok.at++;

				tok.at++;
				parser::consume_whitespace(&tok);
				if (tok.at[0] == '}') {
					return;
				} else {
					tok.at--;
				}
			}
		}
	}
}

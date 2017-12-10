
void add_game_string(MemoryArena &arena, String &key, String &value, StringFormat format, GameStringArray &game_string_array, GameStringHashMap &game_string_hash_map, bool assert_on_mismatch = true) {
	unsigned id = make_string_id(key);
	HASH_LOOKUP(entry, game_string_hash_map.entries, game_string_hash_map.max_count, id);
	if (entry->key == 0) { // Didn't  exist

		// Add it to the hash map
		entry->key = id;

		// Add it to the array
		ARRAY_CHECK_BOUNDS(game_string_array);
		GameString &game_string = game_string_array.entries[game_string_array.count++];
		game_string.key = clone_string(key, arena);
		game_string.key_id = id;
		game_string.value = clone_string(value, arena);
		uint64_t id64 = make_string_id64(value);

		entry->value_id = id64;
		game_string.value_id = id64;
		game_string.format = format;
	} else if (assert_on_mismatch) {
		uint64_t id64 = make_string_id64(value);
		PARSER_ASSERT(entry->value_id == id64, "'%s' has already been used as an ID marker for a different value! (value=%s)", printable_string(key), printable_string(value));
	}
}

void parse_for_game_strings(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, GameStringArray &game_string_array, GameStringHashMap &game_string_hash_map) {
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
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_IDStringMarker: {
				token = parser::next_token(&tok);
				add_game_string(arena, token.string, token.string, StringFormat_ID32, game_string_array, game_string_hash_map, false);
			} break;
			case TokenType_ID32StringMarker: {
				parser::Token key = parser::next_token(&tok);
				token = parser::next_token(&tok);
				if (token.type == ',') {
					add_game_string(arena, key.string, parser::next_token(&tok).string, StringFormat_ID32, game_string_array, game_string_hash_map);
				}
			} break;
			case TokenType_ID64StringMarker: {
				parser::Token key = parser::next_token(&tok);
				token = parser::next_token(&tok);
				if (token.type == ',') {
					add_game_string(arena, key.string, parser::next_token(&tok).string, StringFormat_ID64, game_string_array, game_string_hash_map);
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}
}

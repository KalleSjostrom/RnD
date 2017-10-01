GameStringArray make_unique(MemoryArena &arena, GameStringArray &game_string_array) {
	unsigned hash_count = (unsigned) (array_count(game_string_array) * 1.5f);
	unsigned hash_size = sizeof(GameStringHashEntry) * hash_count;
	GameStringHashEntry *entries = (GameStringHashEntry *)allocate_memory(arena, hash_size);
	memset(entries, 0, hash_size);
	GameStringHashMap game_string_hash_map = { entries, hash_count, };

	GameStringArray unique_game_strings;
	array_init(unique_game_strings, array_count(game_string_array));

	for (unsigned i = 0; i < array_count(game_string_array); ++i) {
		GameString &game_string = game_string_array[i];
		HASH_LOOKUP(entry, game_string_hash_map.entries, game_string_hash_map.max_count, game_string.key_id);
		if (entry->key == 0) { // Didn't  exist
			entry->key = game_string.key_id;
			entry->value = game_string.value_id;
			array_push(unique_game_strings, game_string);
		} else {
			ASSERT(entry->value == game_string.value_id, "'%s' has already been used as an ID marker for a different value! (value=%s)", printable_string(game_string.key), printable_string(game_string.value));
		}
	}

	return unique_game_strings;
}

void add_game_string(MemoryArena &arena, String &key, String &value, StringFormat format, GameStringArray &game_string_array) {
	array_new_entry(game_string_array);
	GameString &game_string = array_last(game_string_array);
	game_string.key = clone_string(key, arena);
	game_string.key_id = make_string_id64(key);
	game_string.value = clone_string(value, arena);
	game_string.value_id = make_string_id64(value);
	game_string.format = format;
}

void add_game_string(String &key, uint64_t key_id, String &value, uint64_t value_id, StringFormat format, GameStringArray &game_string_array) {
	array_new_entry(game_string_array);
	GameString &game_string = array_last(game_string_array);
	game_string.key = key;
	game_string.key_id = key_id;
	game_string.value = value;
	game_string.value_id = value_id;
	game_string.format = format;
}

void parse_for_game_strings(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, GameStringArray &game_string_array) {
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
				add_game_string(arena, token.string, token.string, StringFormat_ID32, game_string_array);
			} break;
			case TokenType_ID32StringMarker: {
				parser::Token key = parser::next_token(&tok);
				token = parser::next_token(&tok);
				if (token.type == ',') {
					add_game_string(arena, key.string, parser::next_token(&tok).string, StringFormat_ID32, game_string_array);
				}
			} break;
			case TokenType_ID64StringMarker: {
				parser::Token key = parser::next_token(&tok);
				token = parser::next_token(&tok);
				if (token.type == ',') {
					add_game_string(arena, key.string, parser::next_token(&tok).string, StringFormat_ID64, game_string_array);
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}
}

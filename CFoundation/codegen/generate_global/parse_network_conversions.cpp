
void parse_network_conversions(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, NetworkConversionArray &network_conversion_array) { // , NetworkConversionHashMap &network_conversion_map) {
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
			case TokenType_Identifier: {
				ARRAY_CHECK_BOUNDS(network_conversion_array);
				NetworkConversion &network_conversion = network_conversion_array.entries[network_conversion_array.count++];
				network_conversion.name = clone_string(token.string, arena);
				network_conversion.name_id = make_string_id(network_conversion.name);

				// HASH_LOOKUP(entry, network_conversion_map.entries, network_conversion_map.max_count, network_conversion.name_id);
				// entry->key = network_conversion.name_id;

				ASSERT_NEXT_TOKEN_TYPE(tok, ':');

				parser::Token &type = parser::next_token(&tok);
				network_conversion.type = clone_string(type.string, arena);

				parser::Token &from = parser::next_line(&tok);
				parser::Token &to = parser::next_line(&tok);

				network_conversion.from = clone_string(from.string, arena);
				network_conversion.to = clone_string(to.string, arena);
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}
}

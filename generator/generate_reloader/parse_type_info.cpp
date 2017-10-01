void parse_type_info(MemoryArena &arena, String &type_info_path, ReloadableStruct **struct_collection, ReloadableMap &reloadable_map) {

	FILE *file = fopen(*type_info_path, "r");
	if (!file)
		return;

	// Setup parsing
	size_t filesize = get_filesize(file);
	char *source = PUSH_STRING(arena, filesize+1);
	filesize = fread(source, 1, filesize, file);
	fclose(file);

	source[filesize] = '\0';

	parser::ParserContext parser_context;
	parser_context.filepath = *type_info_path;
	parser_context.source = source;
	global_last_parser_context = &parser_context;

	parser::Tokenizer tok = parser::make_tokenizer(true, source, filesize);
	parser_context.tokenizer = &tok;
	/////

	parser::Token token = parser::next_token(&tok);
	if (!parser::is_equal(token, TOKENIZE("enum")))
		return;

	ASSERT_NEXT_TOKEN(tok, TOKENIZE("ReloadType"));
	ASSERT_NEXT_TOKEN_TYPE(tok, '{');

	while (true) {
		token = parser::next_token(&tok);
		if (token.type == '}') {
			token = parser::next_token(&tok); // Swallow the ;
			break;
		}
	}

	while (true) {
		token = parser::next_token(&tok);
		if (token.type == '\0') {
			break;
		} else {

			// Wrong version of the file, just ignore the rest of the file..
			if (token.type != '#')
				return;

			ASSERT_NEXT_TOKEN(tok, TOKENIZE("define"));

			token = parser::next_token(&tok);
			String &string = token.string;

			String current_type;
			unsigned current_type_id;
			ReloadableStruct *current_struct = 0;

			static int reload_offset = sizeof("__RELOAD_") - 1;
			string.text += reload_offset;
			string.length -= reload_offset;
			if (string[0] == 'S') { // __RELOAD_SIZE__
				static int offset = sizeof("SIZE__") - 1;
				string.text += offset;
				string.length -= offset;

				current_type = string;
				current_type_id = make_string_id32(string);
				HashMap_Lookup(entry, reloadable_map, current_type_id, 0);

				current_struct = 0;

				if (entry->key == current_type_id) {
					current_struct = struct_collection[entry->value];
					current_struct->has_typeinfo = true;
				}
			} else if (string[0] == 'O') { // __RELOAD_OFFSET__
				if (current_struct) {
					static int offset = sizeof("OFFSET__") - 1;
					string.text += offset;
					string.length -= offset;

					string.text += current_type.length + 1;
					string.length -= current_type.length + 1;

					for (int i = 0; i < array_count(current_struct->member_array); ++i) {
						ReloadableMember &m = current_struct->member_array[i];
						if (is_equal(m.name, string)) {
							m.existed = true;
							break;
						}
					}
				}
			}

			token = parser::next_token(&tok);
		}
	}
}

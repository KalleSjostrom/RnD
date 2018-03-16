struct PathId {
	u32 *path;
	i32 cursor;
};
PathId make_path_id(String string) {
	PathId id = {};
	array_init(id.path, 4);

	i32 at = 0;
	for (i32 i = 0; i < string.length; ++i) {
		char c = string[i];
		if (c == '/') {
			u32 length = (u32)(i - at);
			unsigned id32 = to_id32(length, string.text + at);
			array_push(id.path, id32);
			at = i+1;
		}
	}

	u32 length = (u32)(string.length - at);
	unsigned id32 = to_id32(length, string.text + at);
	array_push(id.path, id32);

	return id;
}
struct PluginInfo {
	String path;
	PathId *includes;
	PathId *excludes;
};
struct Project {
	String *translation_units; // Dynamically allocate this!
	PluginInfo *plugin_infos;

	String name;
	String asset_path;
	String output_path;
	String root;
	String user_commands;
};

void parse_project(MemoryArena &arena, Project &project, char *project_path) {
	FILE *file;
	fopen_s(&file, project_path, "r");
	ASSERT(file, "Could not find the project file! '%s'", project_path);

	size_t filesize = get_filesize(file);

	MemoryArena transient_arena = {};

	char *source = (char *)PUSH_SIZE(transient_arena, filesize + 1);
	fread(source, filesize, 1, file);
	source[filesize] = '\0';
	fclose(file);

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize, TokenizerFlags_KeepLineComments);
	parser::ParserContext pcontext;
	parser::set_parser_context(pcontext, &tok, (char*)*project_path);

	parser::Token t_translation_units = TOKENIZE("translation_units");
	parser::Token t_plugins = TOKENIZE("plugins");
	parser::Token t_path = TOKENIZE("path");
	parser::Token t_include = TOKENIZE("include");
	parser::Token t_exclude = TOKENIZE("exclude");
	parser::Token t_asset_path = TOKENIZE("asset_path");
	parser::Token t_output_path = TOKENIZE("output_path");
	parser::Token t_user_commands = TOKENIZE("user_commands");

	array_init(project.translation_units, 8);
	array_init(project.plugin_infos, 8);

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				if (parser::is_equal(token, t_translation_units)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');
					token = parser::next_token(&tok);
					while (token.type != '}') {
						array_push(project.translation_units, clone_string(arena, token.string));
						token = parser::next_token(&tok);
					}
				} else if (parser::is_equal(token, t_plugins)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');
					while (token.type != '}') {
						token = parser::next_token(&tok);
						if (token.type == '{') {
							PluginInfo plugin_info = {};
							array_init(plugin_info.includes, 8);
							array_init(plugin_info.excludes, 8);
							token = parser::next_token(&tok);
							while (token.type != '}') {
								if (parser::is_equal(token, t_path)) {
									ASSERT_NEXT_TOKEN_TYPE(tok, '=');
									token = parser::next_token(&tok);
									plugin_info.path = clone_string(arena, token.string);
									token = parser::next_token(&tok);
								} else if (parser::is_equal(token, t_include)) {
									ASSERT_NEXT_TOKEN_TYPE(tok, '=');
									ASSERT_NEXT_TOKEN_TYPE(tok, '{');
									token = parser::next_token(&tok);
									while (token.type != '}') {
										PathId path = make_path_id(token.string);
										array_push(plugin_info.includes, path);
										token = parser::next_token(&tok);
									}
									token = parser::next_token(&tok);
								} else if (parser::is_equal(token, t_exclude)) {
									ASSERT_NEXT_TOKEN_TYPE(tok, '=');
									ASSERT_NEXT_TOKEN_TYPE(tok, '{');
									token = parser::next_token(&tok);
									while (token.type != '}') {
										PathId path = make_path_id(token.string);
										array_push(plugin_info.excludes, path);
										token = parser::next_token(&tok);
									}
									token = parser::next_token(&tok);
								} else {
									PARSER_ASSERT(false, "Invalid plugin tag '%.*s'", token.string.length, *token.string);
								}
							}
							array_push(project.plugin_infos, plugin_info);
							token = parser::peek_next_token(&tok);
						} else {
							ASSERT_TOKEN_TYPE(token, '}');
						}
					}
				} else if (parser::is_equal(token, t_asset_path)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					token = parser::next_token(&tok);
					project.asset_path = clone_string(arena, token.string);
				} else if (parser::is_equal(token, t_output_path)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					token = parser::next_token(&tok);
					project.output_path = clone_string(arena, token.string);
				} else if (parser::is_equal(token, t_user_commands)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					token = parser::next_token(&tok);
					project.user_commands = clone_string(arena, token.string);
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}

	String s_project_path = make_string(project_path);
	project.name = get_filename(s_project_path, true);
	null_terminate(project.name);

	project.root = get_directory(s_project_path);
	null_terminate(project.root);
}
struct PathId {
	u32 *path;
	i32 cursor;
};
PathId make_path_id(Allocator *allocator, String string) {
	PathId id = {};
	array_make(allocator, id.path, 4);

	i32 at = 0;
	for (i32 i = 0; i < (i32)string.length; ++i) {
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

	Allocator allocator; // Used for strings atm

	String name;
	String asset_path;
	String output_path;
	String root;
	String user_commands;
};

void parse_project(Allocator *allocator, Project &project, char *project_path) {
	FILE *file;
	fopen_s(&file, project_path, "r");
	ASSERT(file, "Could not find the project file! '%s'", project_path);

	size_t filesize = get_filesize(file);

	ArenaAllocator transient_arena = {};

	char *source = PUSH(&transient_arena, filesize + 1, char);
	fread(source, filesize, 1, file);
	source[filesize] = '\0';
	fclose(file);

	Tokenizer tok = make_tokenizer(source, filesize, TokenizerFlags_KeepLineComments);
	ParserContext pcontext;
	set_parser_context(pcontext, &tok, (char*)*project_path);

	Token t_translation_units = TOKENIZE("translation_units");
	Token t_plugins = TOKENIZE("plugins");
	Token t_path = TOKENIZE("path");
	Token t_include = TOKENIZE("include");
	Token t_exclude = TOKENIZE("exclude");
	Token t_asset_path = TOKENIZE("asset_path");
	Token t_output_path = TOKENIZE("output_path");
	Token t_user_commands = TOKENIZE("user_commands");

	array_make(allocator, project.translation_units, 8);
	array_make(allocator, project.plugin_infos, 8);

	bool parsing = true;
	while (parsing) {
		Token token = next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				if (is_equal(token, t_translation_units)) {
					assert_next_token_type(&tok, '=');
					assert_next_token_type(&tok, '{');
					token = next_token(&tok);
					while (token.type != '}') {
						array_push(project.translation_units, string(&project.allocator, token.string));
						token = next_token(&tok);
					}
				} else if (is_equal(token, t_plugins)) {
					assert_next_token_type(&tok, '=');
					assert_next_token_type(&tok, '{');
					while (token.type != '}') {
						token = next_token(&tok);
						if (token.type == '{') {
							PluginInfo plugin_info = {};
							array_make(allocator, plugin_info.includes, 8);
							array_make(allocator, plugin_info.excludes, 8);
							token = next_token(&tok);
							while (token.type != '}') {
								if (is_equal(token, t_path)) {
									assert_next_token_type(&tok, '=');
									token = next_token(&tok);
									plugin_info.path = string(&project.allocator, token.string);
									token = next_token(&tok);
								} else if (is_equal(token, t_include)) {
									assert_next_token_type(&tok, '=');
									assert_next_token_type(&tok, '{');
									token = next_token(&tok);
									while (token.type != '}') {
										PathId path = make_path_id(allocator, token.string);
										array_push(plugin_info.includes, path);
										token = next_token(&tok);
									}
									token = next_token(&tok);
								} else if (is_equal(token, t_exclude)) {
									assert_next_token_type(&tok, '=');
									assert_next_token_type(&tok, '{');
									token = next_token(&tok);
									while (token.type != '}') {
										PathId path = make_path_id(allocator, token.string);
										array_push(plugin_info.excludes, path);
										token = next_token(&tok);
									}
									token = next_token(&tok);
								} else {
									PARSER_ASSERT(false, "Invalid plugin tag '%.*s'", (i32)token.string.length, *token.string);
								}
							}
							array_push(project.plugin_infos, plugin_info);
							token = peek_next_token(&tok);
						} else {
							ASSERT_TOKEN_TYPE(token, '}');
						}
					}
				} else if (is_equal(token, t_asset_path)) {
					assert_next_token_type(&tok, '=');
					token = next_token(&tok);
					project.asset_path = string(&project.allocator, token.string);
				} else if (is_equal(token, t_output_path)) {
					assert_next_token_type(&tok, '=');
					token = next_token(&tok);
					project.output_path = string(&project.allocator, token.string);
				} else if (is_equal(token, t_user_commands)) {
					assert_next_token_type(&tok, '=');
					token = next_token(&tok);
					project.user_commands = string(&project.allocator, token.string);
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}

	DynamicString project_path_string = dynamic_string(allocator, project_path);
	project.name = get_filename(string(project_path_string), true);

	project.root = get_directory(string(project_path_string));
}
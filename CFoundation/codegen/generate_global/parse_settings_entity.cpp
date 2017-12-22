static String GENERATED_COMPONENTS_FOLDER = MAKE_STRING("content/generated/components/");

SettingsDataStore *parse_settings_data(MemoryArena &arena, parser::Tokenizer &tok, parser::Token &token) {
	SettingsDataStore *data_store = (SettingsDataStore *)allocate_memory(arena, sizeof(SettingsDataStore));
	memset(data_store, 0, sizeof(SettingsDataStore));

	while (token.type != '}') { // While we haven't reached the end of the struct
		ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
		String name = clone_string(token.string, arena);

		ASSERT_NEXT_TOKEN_TYPE(tok, '=');

		SettingsData *sd = get_new_settings_data(*data_store, name);

		token = parser::peek_next_line(&tok);
		if (token.string[0] == '{') {
			token = parser::next_line(&tok); // Consume the next line. TODO(kalle): Make a consume function instead of reparsing!
			token = parser::next_token(&tok);

			if (parser::is_equal(token, _resource_name_token)) {
				ASSERT_NEXT_TOKEN_TYPE(tok, '=');
				token = parser::next_token(&tok);
				sd->value = clone_string(token.string, arena);
				parser::all_until(&tok, '}');
				token = parser::next_token(&tok);
			} else {
				sd->next = parse_settings_data(arena, tok, token);
			}
			ASSERT_TOKEN_TYPE(token, '}');
		} else if (token.string[0] == '[') {
			token = parser::all_until(&tok, ']');
			token.text++; token.length--; // Swallow '['
			sd->value = clone_string(token.string, arena);
			ASSERT_NEXT_TOKEN_TYPE(tok, ']');
		} else {
			token = parser::next_line(&tok); // Consume the next line. TODO(kalle): Make a consume function instead of reparsing!
			sd->value = clone_string(token.string, arena);
		}

		token = parser::next_token(&tok);
	}

	return data_store;
}

void parse_component(MemoryArena &arena, parser::Tokenizer &tok, ComponentArray &component_array, ComponentSettings *component_settings) {
	parser::Token token = parser::next_token(&tok);
	ASSERT_TOKEN_TYPE(token, TokenType_String);

	String guid = token.string;

	ASSERT_NEXT_TOKEN_TYPE(tok, '=');
	ASSERT_NEXT_TOKEN_TYPE(tok, '{');

	ASSERT_NEXT_TOKEN(tok, _component_token);

	ASSERT_NEXT_TOKEN_TYPE(tok, '=');
	ASSERT_NEXT_TOKEN_TYPE(tok, '{');

	token = parser::next_token(&tok);
	while (!parser::is_equal(token, _resource_name_token)) {
		token = parser::next_token(&tok);
	}

	ASSERT_NEXT_TOKEN_TYPE(tok, '=');
	token = parser::next_token(&tok);
	ASSERT_TOKEN_TYPE(token, TokenType_String);

	int start, stop;
	if (string_split(token.string, GENERATED_COMPONENTS_FOLDER, &start, &stop)) { // We found a match
		String component_stem = make_string(token.string.text + stop, token.string.length - stop);

		for (int i = 0; i < component_array.count; ++i) {
			Component *component = component_array.entries + i;
			if (are_strings_equal(component_stem, component->stem)) {
				component_settings->component_stem_id = component->stem_id;
				component_settings->guid_id = make_string_id(guid);
				break;
			}
		}
	} else {
		component_settings->component_path_id = make_string_id64(token.string);
	}

	while (!parser::is_equal(token, _resource_type_token)) {
		token = parser::next_token(&tok);
	}

	ASSERT_NEXT_TOKEN_TYPE(tok, '=');
	ASSERT_NEXT_TOKEN(tok, component_token);
	ASSERT_NEXT_TOKEN_TYPE(tok, '}');

	token = parser::next_token(&tok);

	component_settings->settings_data_store = parse_settings_data(arena, tok, token);
}

void parse_modified_component(MemoryArena &arena, parser::Tokenizer &tok, ComponentArray &component_array, ComponentSettings *component_settings) {
	parser::Token token = parser::next_token(&tok);
	ASSERT_TOKEN_TYPE(token, TokenType_String); ASSERT_NEXT_TOKEN_TYPE(tok, '='); ASSERT_NEXT_TOKEN_TYPE(tok, '{');

	component_settings->component_stem_id = 0;
	component_settings->guid_id = make_string_id(token.string);

	token = parser::next_token(&tok);

	component_settings->settings_data_store = parse_settings_data(arena, tok, token);
}

static const uint64_t environment_component_id64 = make_string_id64(MAKE_STRING("content/environment_components/environment"));
static const uint64_t transition_tile_component_id64 = make_string_id64(MAKE_STRING("content/environment_components/transition_tile"));
static const uint64_t fill_tile_component_id64 = make_string_id64(MAKE_STRING("content/environment_components/fill_tile"));
static const uint64_t custom_tile_component_id64 = make_string_id64(MAKE_STRING("content/environment_components/custom_tile"));
static const uint64_t level_generation_component_id64 = make_string_id64(MAKE_STRING("content/environment_components/level_generation"));

void insert_settings(AllSettings &all_settings, Settings &settings) {
	if (starts_with(settings.inherit_path, MAKE_STRING("core/"))) {
		return;
	}

	if (settings.component_settings_count == 1) {
		ComponentSettings &component_settings = settings.component_settings[0];
		// Environments
		if (component_settings.component_path_id == environment_component_id64) {
			ARRAY_CHECK_BOUNDS(all_settings.environments);
			all_settings.environments.entries[all_settings.environments.count++] = settings;
		}
		// Tiles
		else if (component_settings.component_path_id == custom_tile_component_id64 || component_settings.component_path_id == fill_tile_component_id64 || component_settings.component_path_id == transition_tile_component_id64) {
			ARRAY_CHECK_BOUNDS(all_settings.tiles);
			all_settings.tiles.entries[all_settings.tiles.count++] = settings;
		}
		else if (component_settings.component_path_id == level_generation_component_id64) {
			ARRAY_CHECK_BOUNDS(all_settings.level_generation);
			all_settings.level_generation.entries[all_settings.level_generation.count++] = settings;
		}
		else if (component_settings.component_stem_id == 0) {
			// Ignore
		} else {
			ARRAY_CHECK_BOUNDS(all_settings.entities);
			all_settings.entities.entries[all_settings.entities.count++] = settings;
		}
	} else if (settings.component_settings_count > 1) {
		if (settings.component_settings[0].component_stem_id) { // We have a valid component in the first slot. Enough to sort out the stingray entities like shading environments?
			ARRAY_CHECK_BOUNDS(all_settings.entities);
			all_settings.entities.entries[all_settings.entities.count++] = settings;
		}
	}
}

// component_stem = t.ex: motion, health
void parse_settings_entity(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, ComponentArray &component_array, AllSettings &all_settings) {
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

	Settings settings = {0};
	settings.path = make_project_path(filepath, arena);
	settings.path_id = make_string_id64(settings.path);

	settings.component_settings_count = 0;

	ComponentSettings *component_settings = 0;

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				if (parser::is_equal(token, components_token)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');

					while (true) {
						ARRAY_CHECK_BOUNDS_COUNT(settings.component_settings, settings.component_settings_count);
						component_settings = &settings.component_settings[settings.component_settings_count++];
						parse_component(arena, tok, component_array, component_settings);
						token = parser::peek_next_token(&tok);
						if (token.type == '}') {
							break;
						}
					}
				} else if (parser::is_equal(token, modified_components_token)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');

					while (true) {
						ARRAY_CHECK_BOUNDS_COUNT(settings.component_settings, settings.component_settings_count);
						component_settings = &settings.component_settings[settings.component_settings_count++];
						parse_modified_component(arena, tok, component_array, component_settings);
						token = parser::peek_next_token(&tok);
						if (token.type == '}') {
							break;
						}
					}
				} else if (parser::is_equal(token, inherit_token)) {
					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');

					token = parser::next_token(&tok);
					while (!parser::is_equal(token, _resource_name_token)) {
						token = parser::next_token(&tok);
					}

					ASSERT_NEXT_TOKEN_TYPE(tok, '=');
					token = parser::next_token(&tok);
					ASSERT_TOKEN_TYPE(token, TokenType_String);

					settings.inherit_path = clone_string(token.string, arena);
					settings.inherit_path_id = make_string_id64(settings.inherit_path);
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}

	insert_settings(all_settings, settings);
}
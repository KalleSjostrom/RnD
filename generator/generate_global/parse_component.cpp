BehaviorNode *try_get_behavior_node(BehaviorNodeArray &behavior_node_array, unsigned id) {
	for (unsigned i = 0; i < array_count(behavior_node_array); ++i) {
		if (behavior_node_array[i].name_id == id)
			return &behavior_node_array[i];
	}
	return 0;
}

void parse_component(MemoryArena &temp_arena, MemoryArena &arena, char *filepath, ComponentArray &component_array, GameStringArray &game_string_array, ScriptNodeArray &script_node_array, HFileCollection &hfile_collection) {
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

	array_new_entry(component_array);
	Component &component = array_last(component_array);

	String namespace_name = MAKE_STRING("");
	String max_instances = MAKE_STRING("64");
	String category = MAKE_STRING("");
	String icon = MAKE_STRING("cloud");
	bool found_component_spec = false;
	while (!found_component_spec) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_CommandMarker: {
				token = parser::next_token(&tok);
				if (parser::is_equal(token, component_token)) {
					token = parser::next_token(&tok);

					while (!parser::is_equal(token, namespace_token)) {
						if (parser::is_equal(token, after_token) || parser::is_equal(token, before_token)) {
							bool is_before = parser::is_equal(token, before_token);
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');

							while (1) {
								token = parser::next_token(&tok);
								if (token.type == ')')
									break;
								else if (token.type == ',')
									continue;

								ComponentReference *ref;
								if (is_before) {
									ref = &component.before_component_array.new_entry();
								} else {
									ref = &component.after_component_array.new_entry();
								}
								ref->stem = clone_string(token.string, arena);
								ref->stem_id = make_string_id(ref->stem);
							}
						} else if (parser::is_equal(token, master_token)) {
							component.sub_component_mask |= ComponentMask_Master;
						} else if (parser::is_equal(token, network_token)) {
							component.sub_component_mask |= ComponentMask_Network;
						} else if (parser::is_equal(token, slave_token)) {
							component.sub_component_mask |= ComponentMask_Slave;
						} else if (parser::is_equal(token, update_token)) {
							component.sub_component_mask |= ComponentMask_Update;
						} else if (parser::is_equal(token, no_update_token)) {
							component.sub_component_mask |= ComponentMask_NoUpdate;
						} else if (parser::is_equal(token, script_reload_token)) {
							component.sub_component_mask |= ComponentMask_ScriptReload;
						} else if (parser::is_equal(token, init_token)) {
							component.sub_component_mask |= ComponentMask_Init;
						} else if (parser::is_equal(token, deinit_token)) {
							component.sub_component_mask |= ComponentMask_Deinit;
						} else if (parser::is_equal(token, on_added_token)) {
							component.sub_component_mask |= ComponentMask_OnAdded;
						} else if (parser::is_equal(token, on_removed_token)) {
							component.sub_component_mask |= ComponentMask_OnRemoved;
						} else if (parser::is_equal(token, migration_events_token)) {
							component.sub_component_mask |= ComponentMask_MigrationEvents;
						} else if (parser::is_equal(token, depends_on_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');

							while (1) {
								token = parser::next_token(&tok);
								if (token.type == ')')
									break;
								else if (token.type == ',')
									continue;

								ComponentReference &dependency = component.dependency_array.new_entry();
								dependency.stem = clone_string(token.string, arena);
								dependency.stem_id = make_string_id(dependency.stem);
							}
						} else if (parser::is_equal(token, max_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');
							max_instances = clone_string(parser::next_token(&tok).string, arena);
							ASSERT_NEXT_TOKEN_TYPE(tok, ')');
						} else if (parser::is_equal(token, category_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');
							category = clone_string(parser::next_token(&tok).string, arena);
							ASSERT_NEXT_TOKEN_TYPE(tok, ')');
						} else if (parser::is_equal(token, icon_token)) {
							ASSERT_NEXT_TOKEN_TYPE(tok, '(');
							icon = clone_string(parser::next_token(&tok).string, arena);
							ASSERT_NEXT_TOKEN_TYPE(tok, ')');
						} else {
							PARSER_ASSERT(false, "Unexpected token! Expected 'max', 'after' or 'depends_on' but found %s", printable_token(token));
						}
						token = parser::next_token(&tok);
					}

					ASSERT_TOKEN(tok, token, namespace_token);
					token = parser::next_token(&tok);

					ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
					namespace_name = clone_string(token.string, arena);
					ASSERT_NEXT_TOKEN_TYPE(tok, '{');

					found_component_spec = true;
				} else if (parser::is_equal(token, reloadable_token)) {
				} else {
					null_terminate(token.string);
					PARSER_ASSERT(false, "Unknown command! (command=%s, filename=%s)", *token.string, filepath);
				}
			} break;
			case '\0': {
				PARSER_ASSERT(false, "No component specification found! (filename=%s)", filepath);
			} break;
		}
	}

	PARSER_ASSERT(namespace_name.length > 0, "Could not find a namespace in component! (filepath=%s)", filepath);

	component.stem = namespace_name;
	component.stem_id = make_string_id(component.stem);
	component.name = namespace_to_component_name(namespace_name, arena); // motion -> MotionComponent
	component.stem_upper = make_upper_case(namespace_name, arena);

	component.max_instances = max_instances;
	component.category = category;
	component.icon = icon;

	SubCompStruct *sub_comps = component.sub_comps;

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		if (parser::is_equal(token, struct_token)) {
			token = parser::next_token(&tok);
			ASSERT_TOKEN_TYPE(token, TokenType_Identifier);
			String struct_name = clone_string(token.string, arena);

			SubCompType sub_comp_type;
			if (parser::is_equal(token, MasterInput_token)) {
				sub_comp_type = SubCompType_MasterInput;
			} else if (parser::is_equal(token, Master_token)) {
				sub_comp_type = SubCompType_Master;
				component.sub_component_mask |= ComponentMask_Master;
				if (!(component.sub_component_mask & ComponentMask_NoUpdate)) {
					component.sub_component_mask |= ComponentMask_Update;
				}
			} else if (parser::is_equal(token, SlaveInput_token)) {
				sub_comp_type = SubCompType_SlaveInput;
			} else if (parser::is_equal(token, Slave_token)) {
				sub_comp_type = SubCompType_Slave;
				component.sub_component_mask |= ComponentMask_Slave;
				if (!(component.sub_component_mask & ComponentMask_NoUpdate)) {
					component.sub_component_mask |= ComponentMask_Update;
				}
			} else if (parser::is_equal(token, Network_token)) {
				sub_comp_type = SubCompType_Network;
				component.sub_component_mask |= ComponentMask_Network | ComponentMask_Slave;
			} else if (parser::is_equal(token, Static_token)) {
				sub_comp_type = SubCompType_Static;
			} else {
				PARSER_ASSERT(false, "Found no component sub type with name=%s", *struct_name);
			}

			ASSERT_NEXT_TOKEN_TYPE(tok, '{');

			SubCompStruct *s = sub_comps + sub_comp_type;
			s->type = sub_comp_type;
			array_init(s->member_array, 32);
			token = parser::next_token(&tok);

			parse_struct_members(arena, tok, token, s->member_array);

			if (sub_comp_type == SubCompType_Network) {
				for (unsigned i = 0; i < array_count(s->member_array); ++i) {
					Member &member = s->member_array[i];
					uint64_t id = make_string_id64(member.name);
					add_game_string(member.name, id, member.name, id, StringFormat_ID32, game_string_array);
				}
			}

			BehaviorNodeArray &behavior_node_array = hfile_collection.behavior_node_array;
			for (unsigned k = 0; k < array_count(s->member_array); ++k) {
				Member &member = s->member_array[k];
				if (FLAG_SET(member, MemberFlag_IsBehaviorNode)) {
					// Check dependencies
					String sub_comp_string = get_sub_comp_string(sub_comp_type, false);
					String name = allocate_string(arena, 1 + component.stem.length + 1 + sub_comp_string.length + 1);
					append_string(name, MAKE_STRING("_"));
					append_string(name, component.stem);
					append_string(name, MAKE_STRING("_"));
					append_string(name, sub_comp_string);
					null_terminate(name);
					unsigned name_id = make_string_id(name);

					BehaviorNode *existing_node = try_get_behavior_node(behavior_node_array, name_id);
					if (!existing_node) {
						array_new_entry(behavior_node_array);
						BehaviorNode &node = array_last(behavior_node_array);
						array_init(node.parameters, 16);
						hfile_collection.behavior_node_array_changed = true;
						
						node.collection_name = fetch_string;
						node.collection_name_id = fetch_id;

						node.return_type.type = allocate_string(arena, component.stem.length + 2 + sub_comp_string.length);
						append_string(node.return_type.type, component.stem);
						append_string(node.return_type.type, MAKE_STRING("::"));
						append_string(node.return_type.type, sub_comp_string);
						null_terminate(node.return_type.type);
						node.return_type.type_id = make_string_id(node.return_type.type);
						node.return_type.is_ref = true;

						node.name = name;
						node.name_id = name_id;
					}

					// Setup the script node
					array_new_entry(script_node_array);
					ScriptNode &script_node = array_last(script_node_array);
					script_node.component_stem_id = component.stem_id;
					script_node.sub_component_type = sub_comp_type;
					script_node.member_index = k;

					// Setup the behavior node for the fetch
					{ // fetch
						array_new_entry(behavior_node_array);
						BehaviorNode &node = array_last(behavior_node_array);
						array_init(node.parameters, 16);
						hfile_collection.behavior_node_array_changed = true;

						unsigned collection_name_id = fetch_id;
						uint64_t dependency = (uint64_t)collection_name_id << 32 | name_id;
						node.dependencies[node.dependency_count++] = dependency;
						
						node.collection_name = fetch_string;
						node.collection_name_id = fetch_id;

						{
							Parameter dep_parameter = {};

							String sub_comp_name = get_sub_comp_string(sub_comp_type, false);
							String dep_type_name = allocate_string(arena, component.stem.length + 2 + sub_comp_name.length + 1);
							append_string(dep_type_name, component.stem);
							append_string(dep_type_name, MAKE_STRING("::"));
							append_string(dep_type_name, sub_comp_name);
							append_string(dep_type_name, MAKE_STRING("&"));

							dep_parameter.type = dep_type_name;
							dep_parameter.type_id = make_string_id(dep_type_name);

							array_push(node.parameters, dep_parameter);
						}

						node.return_type.type = member.type;
						node.return_type.type_id = member.type_id;
						node.return_type.is_pointer = FLAG_SET(member, MemberFlag_IsPointer);

						if (FLAG_SET(member, MemberFlag_BehaviorNodePrefix)) {
							String node_name = allocate_string(arena, component.stem.length + 1 + member.name.length + 1);
							append_string(node_name, component.stem);
							append_string(node_name, MAKE_STRING("_"));
							append_string(node_name, member.name);
							null_terminate(node_name);

							node.name = node_name;
							node.name_id = make_string_id(node_name);
						} else {
							node.name = member.name;
							node.name_id = member.name_id;
						}
					}

					unsigned prefix_count = 0;
					String prefixes[2] = {};
					prefixes[prefix_count++] = set_prefix_string;
					if (FLAG_SET(member, MemberFlag_BehaviorNodeReset)) {
						prefixes[prefix_count++] = reset_prefix_string;
					}

					// Setup the behavior node for the actions
					for (unsigned prefix_index = 0; prefix_index < prefix_count; ++prefix_index) {
						String &prefix = prefixes[prefix_index];

						array_new_entry(behavior_node_array);
						BehaviorNode &node = array_last(behavior_node_array);
						array_init(node.parameters, 16);
						hfile_collection.behavior_node_array_changed = true;

						uint64_t dependency = (uint64_t)fetch_id << 32 | name_id;
						node.dependencies[node.dependency_count++] = dependency;
						
						node.collection_name = action_string;
						node.collection_name_id = action_id;

						{
							Parameter dep_parameter = {};

							String sub_comp_name = get_sub_comp_string(sub_comp_type, false);
							String dep_type_name = allocate_string(arena, component.stem.length + 2 + sub_comp_name.length + 1);
							append_string(dep_type_name, component.stem);
							append_string(dep_type_name, MAKE_STRING("::"));
							append_string(dep_type_name, sub_comp_name);
							append_string(dep_type_name, MAKE_STRING("&"));

							dep_parameter.type = dep_type_name;
							dep_parameter.type_id = make_string_id(dep_type_name);

							array_push(node.parameters, dep_parameter);
						}

						if (prefix_index == 0) {
							array_new_entry(node.parameters);
							Parameter &parameter = array_last(node.parameters);
							parameter.type = member.type;
							parameter.type_id = member.type_id;
							parameter.name = member.name;
							parameter.name_id = member.name_id;
							parameter.is_pointer = FLAG_SET(member, MemberFlag_IsPointer);
						}

						node.return_type.type = void_string;
						node.return_type.type_id = void_id32;

						String node_name = {};
						if (FLAG_SET(member, MemberFlag_BehaviorNodePrefix)) {
							node_name = allocate_string(arena, component.stem.length + 1 + prefix.length + member.name.length + 1);
							append_string(node_name, component.stem);
							append_string(node_name, MAKE_STRING("_"));
						} else {
							node_name = allocate_string(arena, prefix.length + member.name.length + 1);
						}
						append_string(node_name, prefix);
						append_string(node_name, member.name);
						null_terminate(node_name);

						node.name = node_name;
						node.name_id = make_string_id(node_name);
					}
				}
			}
		} else if (token.type == '}') {
			parsing = false;
		} else if (token.type == '\0') {
			PARSER_ASSERT(false, "Reach end of file without the component specification struct closing!");
		}
	}

	String class_name = clone_string(component.name, arena);
	Receiver current_receiver = receiver::make_data(class_name, arena);

	parser::Tokenizer event_tokenizer = parser::make_tokenizer(true, tok.at, filesize - (tok.at - source));
	parser_context.tokenizer = &event_tokenizer;
	search_for_events(arena, event_tokenizer, hfile_collection, current_receiver, &component);
	parser_context.tokenizer = &tok;

	// Look if this component got added to the receiver array, if so store the number of receiver events for it.
	for (int i = array_count(hfile_collection.receiver_array)-1; i >= 0; i--) {
		Receiver &receiver = hfile_collection.receiver_array[i];
		if (receiver.name_id == current_receiver.name_id) {
			component.num_received_events = receiver.num_events;
			break;
		}
	}

	insert_generated_info_in_hfile(component, filepath, tok, source);
}
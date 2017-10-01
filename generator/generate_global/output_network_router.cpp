bool conversion_write(FILE *output, NetworkConversionArray &network_conversion_array, unsigned type_id, String &name) {
	for (int i = 0; i < array_count(network_conversion_array); ++i) {
		NetworkConversion &network_conversion = network_conversion_array[i];
		if (type_id == network_conversion.name_id) {
fprintf(output, "				make_rpc_message_parameter(RPCParameterType::%s, (void*)%s(%s)),\n", *network_conversion.type, *network_conversion.from, *name);
			return true;
		}
	}
	return false;
}

bool conversion_read(FILE *output, NetworkConversionArray &network_conversion_array, unsigned type_id, const char *parameter_index) {
	for (int i = 0; i < array_count(network_conversion_array); ++i) {
		NetworkConversion &network_conversion = network_conversion_array[i];
		if (type_id == network_conversion.name_id) {
fprintf(output, "%s(parameters[%s].data_pointer));\n", *network_conversion.to, parameter_index);
			return true;
		}
	}
	return false;
}
bool conversion_read(FILE *output, NetworkConversionArray &network_conversion_array, Parameter &parameter, unsigned parameter_index) {
	for (int i = 0; i < array_count(network_conversion_array); ++i) {
		NetworkConversion &network_conversion = network_conversion_array[i];
		if (parameter.type_id == network_conversion.name_id) {
fprintf(output, "					%s %s = %s(parameters[%d].data_pointer));\n", *network_conversion.name, *parameter.name, *network_conversion.to, parameter_index);
			return true;
		}
	}
	return false;
}

bool conversion_read(FILE *output, NetworkConversionArray &network_conversion_array, Parameter &parameter, const char *parameter_index) {
	for (int i = 0; i < array_count(network_conversion_array); ++i) {
		NetworkConversion &network_conversion = network_conversion_array[i];
		if (parameter.type_id == network_conversion.name_id) {
fprintf(output, "					%s %s = %s(parameters[%s].data_pointer));\n", *network_conversion.name, *parameter.name, *network_conversion.to, parameter_index);
			return true;
		}
	}
	return false;
}


// FULKOD(kalle): Linear search
NetworkType *find_network_type(NetworkTypeArray &network_type_array, unsigned name_id) {
	for (int i = 0; i < array_count(network_type_array); ++i) {
		if (network_type_array[i].name_id == name_id) {
			return &network_type_array[i];
		}
	}
	return 0;
}

bool is_rpc_dynamic(Function &rpc, NetworkTypeArray &network_type_array) {
	// TODO(kalle): Speed this up!
	for (unsigned i = 1; i < array_count(rpc.parameters); ++i) {
		Parameter &parameter = rpc.parameters[i];
		NetworkType *network_type = find_network_type(network_type_array, parameter.type_id);
		if (network_type) {
			return true;
		}
	}
	return false;
}

void output_network_router(RPCArray &rpc_array, EventArray &event_array, GameObjectArray &go_array, SettingsArray &settings_array, NetworkConversionArray &network_conversion_array, NetworkTypeArray &network_type_array, MemoryArena &arena) {
	{
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../" GAME_CODE_DIR "/generated/network_router.generated.h");

fprintf(output, "#define RPC_ALL UINT64_MAX\n");
fprintf(output, "#define RPC_OTHERS (UINT64_MAX-1)\n");
fprintf(output, "namespace network_router {\n");
fprintf(output, "	// EventDelegate to use when broadcasting. This pointer won't persist so it needs to be set in on_script_reload!\n");
fprintf(output, "	static EventDelegate *event_delegate;\n");
fprintf(output, "\n");
fprintf(output, "	// Game object names:\n");

		for (int i = 0; i < array_count(go_array); ++i) {
			GameObject &game_object = go_array[i];
fprintf(output, "	static const Id32 go_%s_id32 = 0x%x;\n", *game_object.name, game_object.name_id);
		}
fprintf(output, "\n");
fprintf(output, "	// RPC names:\n");
		for (unsigned i = 0; i < array_count(rpc_array); ++i) {
			Function &rpc = rpc_array[i].function;
fprintf(output, "	static const Id32 %s_id32 = 0x%x;\n", *rpc.name, rpc.name_id);
		}
fprintf(output, "\n");

		//////// receive ////////
fprintf(output, "	namespace receive {\n");
fprintf(output, "		void read_game_object_fields(GameSessionPtr session, GameObjectId go_id);\n");
fprintf(output, "	}\n");
fprintf(output, "\n");

		//////// send ////////
fprintf(output, "	namespace send {\n");
		for (unsigned i = 0; i < array_count(rpc_array); ++i) {
			Function &rpc = rpc_array[i].function;
fprintf(output, "		void %s(PeerId receiving_peer", *rpc.name);
			for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
				Parameter &parameter = rpc.parameters[j];
				fprintf(output, ", %s %s", *parameter.type, *parameter.name);
			}
			fprintf(output, ");\n");
		}
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}
	{
		MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../" GAME_CODE_DIR "/generated/network_router.generated.cpp");

fprintf(output, "namespace network_router {\n");

		//////// noop ////////
fprintf(output, "	namespace noop {\n");
fprintf(output, "		void game_object_created(int go_id, PeerId sending_peer) { }\n");
fprintf(output, "		void game_object_destroyed(int go_id, PeerId sending_peer) { }\n");
fprintf(output, "		void game_object_migrated_to_me(int go_id, PeerId sending_peer) { }\n");
fprintf(output, "		void game_object_migrated_away(int go_id, PeerId sending_peer) { }\n");
fprintf(output, "		void game_object_sync_done(PeerId sending_peer) { }\n");
fprintf(output, "		void game_session_disconnect(PeerId sending_peer) { }\n");
fprintf(output, "		void rpc(PeerId sending_peer, Id32 message_id, struct RPCMessageParameter *parameters, unsigned num_parameters) { }\n");
fprintf(output, "	}\n");
fprintf(output, "\n");

		//////// receive ////////
fprintf(output, "	namespace receive {\n");
fprintf(output, "		void game_object_created(int go_id, PeerId sending_peer)        { LOG_INFO(\"Network\", \"game_object_created (go_id=%%d, sender=%%llx)\",        go_id, sending_peer); components::manager->game_object_created(go_id); }\n");
fprintf(output, "		void game_object_destroyed(int go_id, PeerId sending_peer)      { LOG_INFO(\"Network\", \"game_object_destroyed (go_id=%%d, sender=%%llx)\",      go_id, sending_peer); components::manager->game_object_destroyed(go_id); }\n");
fprintf(output, "		void game_object_migrated_to_me(int go_id, PeerId sending_peer) { LOG_INFO(\"Network\", \"game_object_migrated_to_me (go_id=%%d, sender=%%llx)\", go_id, sending_peer); components::manager->game_object_migrated_to_me(go_id); }\n");
fprintf(output, "		void game_object_migrated_away(int go_id, PeerId sending_peer)  { LOG_INFO(\"Network\", \"game_object_migrated_away (go_id=%%d, sender=%%llx)\",  go_id, sending_peer); components::manager->game_object_migrated_away(go_id); }\n");
fprintf(output, "		void game_object_sync_done(PeerId sending_peer) {\n");
fprintf(output, "			event_delegate->trigger_game_object_sync_done(sending_peer);\n");
fprintf(output, "		}\n");
fprintf(output, "		void game_session_disconnect(PeerId sending_peer) { LOG_INFO(\"Network\", \"game_session_disconnect! (sender=%%llx)\", sending_peer); event_delegate->trigger_game_session_disconnect(sending_peer); }\n");
fprintf(output, "\n");
fprintf(output, "		// Forward declare read functions for custom array network types to avoid unordered declarations\n");
		for (int i = 0; i < array_count(network_type_array); ++i) {
			NetworkType &network_type = network_type_array[i];
			if (network_type.is_array) {
				fprintf(output, "		void read_%s(%s &array, RPCMessageParameter *parameters, unsigned &index);\n", *network_type.name_lower_case, *network_type.name);
			}
		}
fprintf(output, "\n");
fprintf(output, "		// Custom array network types\n");
		for (int i = 0; i < array_count(network_type_array); ++i) {
			NetworkType &network_type = network_type_array[i];
			if (network_type.is_array) {
fprintf(output, "		void read_%s(%s &array, RPCMessageParameter *parameters, unsigned &index) {\n", *network_type.name_lower_case, *network_type.name);
fprintf(output, "			ASSERT(parameters[index++].type == RPCParameterType::RPC_PARAM_ARRAY_BEGINS, \"Trying to read an array, but there is no begin marker!\");\n");
fprintf(output, "			for (int i = 0; i < ARRAY_COUNT(array.entries); i++) {\n");
fprintf(output, "				RPCMessageParameter &parameter = parameters[index];\n");
fprintf(output, "				if (parameter.type == RPCParameterType::RPC_PARAM_ARRAY_ENDS) {\n");
fprintf(output, "					index++;\n");
fprintf(output, "					return;\n");
fprintf(output, "				}\n");
				Member &member = network_type.member;
				NetworkType *member_network_type = find_network_type(network_type_array, member.type_id);
				if (member_network_type) {
fprintf(output, "				%s &child_array = array.entries[array.count++];\n", *member_network_type->name);
fprintf(output, "				read_%s(child_array, parameters, index);\n", *member_network_type->name_lower_case);
				} else {
fprintf(output, "				array.entries[array.count++] = ");
					if (!conversion_read(output, network_conversion_array, member.type_id, "index++")) {
						fprintf(output, "*(%s*)(parameters[index++].data_pointer);\n", *member.type, *member.name, *member.type);
					}
				}
fprintf(output, "			}\n");
fprintf(output, "		}\n");
			}
		}

fprintf(output, "\n");
fprintf(output, "		void rpc(PeerId sending_peer, Id32 message_id, struct RPCMessageParameter *parameters, unsigned num_parameters) {\n");
fprintf(output, "			if (event_delegate == 0) {\n");
fprintf(output, "				LOG_WARNING(\"NetworkRouter\", \"event_delegate set to 0. Ignoring rpc!\");\n");
fprintf(output, "				return;\n");
fprintf(output, "			}\n");
fprintf(output, "\n");
fprintf(output, "			switch (message_id) {\n");
		for (unsigned i = 0; i < array_count(rpc_array); ++i) {
			Function &rpc = rpc_array[i].function;
fprintf(output, "				case %s_id32: {\n", *rpc.name);

			if (is_rpc_dynamic(rpc, network_type_array)) {
fprintf(output, "					unsigned index = 0;\n");
				for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
					Parameter &parameter = rpc.parameters[j];
					NetworkType *network_type = find_network_type(network_type_array, parameter.type_id);
					if (network_type) {
fprintf(output, "					%s %s = {};\n", *parameter.type, *parameter.name);
fprintf(output, "					read_%s(%s, parameters, index);\n", *network_type->name_lower_case, *parameter.name);

					} else {
						if (!conversion_read(output, network_conversion_array, parameter, "index++")) {
fprintf(output, "					%s %s = *(%s*)(parameters[index++].data_pointer);\n", *parameter.type, *parameter.name, *parameter.type, j - 1);
						}
					}
				}
			} else {
				for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
					Parameter &parameter = rpc.parameters[j];

					if (!conversion_read(output, network_conversion_array, parameter, j-1)) {
fprintf(output, "					%s %s = *(%s*)(parameters[%d].data_pointer);\n", *parameter.type, *parameter.name, *parameter.type, j - 1);
					}
				}
			}
fprintf(output, "\n");
fprintf(output, "					event_delegate->trigger_%s(sending_peer", *rpc.name);

			for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
				Parameter &parameter = rpc.parameters[j];
				fprintf(output, ", %s", *parameter.name);
			}
				fprintf(output, ");\n");
fprintf(output, "				} break;\n");
		}
fprintf(output, "			}\n");
fprintf(output, "		}\n");

		//////// read_game_object_fields(GameSessionPtr session, GameObjectId go_id) ////////
fprintf(output, "		void read_game_object_fields(GameSessionPtr session, GameObjectId go_id) {\n");
fprintf(output, "			GOReadResult result = {};\n");
fprintf(output, "			bool has_changed = _GameSession.read_changed_fields(session, go_id, result);\n");
fprintf(output, "			if (!has_changed)\n");
fprintf(output, "				return;\n");
fprintf(output, "\n");
fprintf(output, "			ComponentGroup *group = components::group;\n");
fprintf(output, "			Entity entity = components::manager->get_entity(go_id);\n");
fprintf(output, "\n");
fprintf(output, "			unsigned offset = 0;\n");
fprintf(output, "			const char *buffer = result.field_buffer;\n");
fprintf(output, "			switch (result.go_name_id32) {\n");

		for (int i = 0; i < array_count(settings_array); ++i) {
			Settings &settings = settings_array[i];
			if (settings.game_object) {
				GameObject &game_object = *settings.game_object;
fprintf(output, "				case go_%s_id32: {\n", *game_object.name);
				for (int j = 0; j < array_count(settings.component_settings); ++j) {
					ComponentSettings &component_settings = settings.component_settings[j];
					Component &component = *component_settings.component;
					SubCompStruct *sub_comps = component.sub_comps;
					if (HAS_SUB_COMP_AND_NON_ZERO_COUNT(SubCompType_Network)) {
fprintf(output, "					group->%s.update_network_state(entity, buffer, &offset);\n", *component.stem);
					}
				}
fprintf(output, "				} break;\n");
			}
		}
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n\n");

		//////// send ////////
fprintf(output, "	namespace send {\n");
		//////// write ////////
fprintf(output, "		__forceinline RPCMessageParameter make_rpc_message_parameter(RPCParameterType type, void *data) {\n");
fprintf(output, "			RPCMessageParameter p = { type, data };\n");
fprintf(output, "			return p;\n");
fprintf(output, "		}\n");
fprintf(output, "		// Forward declare write functions for custom array network types to avoid unordered declarations\n");
		for (int i = 0; i < array_count(network_type_array); ++i) {
			NetworkType &network_type = network_type_array[i];
			if (network_type.is_array) {
				fprintf(output, "		void write_%s(%s &array, RPCMessageParameter *parameters, unsigned &index);\n", *network_type.name_lower_case, *network_type.name);
			}
		}
fprintf(output, "\n");
fprintf(output, "		// Custom array network types\n");
		for (int i = 0; i < array_count(network_type_array); ++i) {
			NetworkType &network_type = network_type_array[i];
			if (network_type.is_array) {
fprintf(output, "		void write_%s(%s &array, RPCMessageParameter *parameters, unsigned &index) {\n", *network_type.name_lower_case, *network_type.name);
fprintf(output, "			parameters[index++] = make_rpc_message_parameter(RPCParameterType::RPC_PARAM_ARRAY_BEGINS, 0);\n");
fprintf(output, "				for (unsigned i = 0; i < array.count; i++) {\n");
				Member &member = network_type.member;
				NetworkType *member_network_type = find_network_type(network_type_array, member.type_id);
				if (member_network_type) {
fprintf(output, "					write_%s(array.entries[i], parameters, index);\n", *member_network_type->name_lower_case);
				} else {
					if (!conversion_write(output, network_conversion_array, member.type_id, member.name)) {
						String type_upper = clone_string(member.type, arena);
						to_upper(type_upper);
fprintf(output, "					parameters[index++] = make_rpc_message_parameter(RPCParameterType::RPC_PARAM_%s_TYPE, (void*) (array.entries + i));\n", *type_upper);
					}
				}
fprintf(output, "				}\n");
fprintf(output, "			parameters[index++] = make_rpc_message_parameter(RPCParameterType::RPC_PARAM_ARRAY_ENDS, 0);\n");
fprintf(output, "		}\n");
			}
		}
fprintf(output, "		void _send(unsigned id, PeerId receiving_peer, RPCMessageParameter *parameters, unsigned num_parameters) {\n");
fprintf(output, "			if (receiving_peer == RPC_ALL) {\n");
fprintf(output, "				unsigned num_ready_peers = NETWORK_SESSION_HANDLER().get_num_ready_peers();\n");
fprintf(output, "				for (unsigned i = 0; i < num_ready_peers; i++) {\n");
fprintf(output, "					PeerId receiver = NETWORK_SESSION_HANDLER().get_ready_peer(i);\n");
fprintf(output, "					_Network.send_rpc(id, receiver, parameters, num_parameters);\n");
fprintf(output, "				}\n");
fprintf(output, "				_Network.send_rpc(id, _Network.peer_self(), parameters, num_parameters);\n");
fprintf(output, "			} else if (receiving_peer == RPC_OTHERS) {\n");
fprintf(output, "				unsigned num_ready_peers = NETWORK_SESSION_HANDLER().get_num_ready_peers();\n");
fprintf(output, "				for (unsigned i = 0; i < num_ready_peers; i++) {\n");
fprintf(output, "					PeerId receiver = NETWORK_SESSION_HANDLER().get_ready_peer(i);\n");
fprintf(output, "					_Network.send_rpc(id, receiver, parameters, num_parameters);\n");
fprintf(output, "				}\n");
fprintf(output, "			} else {\n");
fprintf(output, "				_Network.send_rpc(id, receiving_peer, parameters, num_parameters);\n");
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "\n");
fprintf(output, "\n");
		for (unsigned i = 0; i < array_count(rpc_array); ++i) {
			Function &rpc = rpc_array[i].function;
fprintf(output, "		void %s(PeerId receiving_peer", *rpc.name);
			for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
				Parameter &parameter = rpc.parameters[j];
				fprintf(output, ", %s %s", *parameter.type, *parameter.name);
			}
			fprintf(output, ") {\n");
			if (array_count(rpc.parameters) > 1) {
				if (is_rpc_dynamic(rpc, network_type_array)) {
fprintf(output, "			AhTempAllocator ta;\n");
fprintf(output, "			// Grab a real chunk of temp memory!\n");
fprintf(output, "			static const unsigned max_rpc_parameters = 1024;\n");
fprintf(output, "			unsigned index = 0;\n");
fprintf(output, "			RPCMessageParameter *parameters = (RPCMessageParameter *)ta.allocate(max_rpc_parameters * sizeof(RPCMessageParameter));\n");
fprintf(output, "\n");
					for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
						Parameter &parameter = rpc.parameters[j];
						NetworkType *network_type = find_network_type(network_type_array, parameter.type_id);
						if (network_type) {
fprintf(output, "			write_%s(%s, parameters, index);\n", *network_type->name_lower_case, *parameter.name);

						} else {
fprintf(output, "			parameters[index++] = ");
							if (!conversion_write(output, network_conversion_array, parameter.type_id, parameter.name)) {
								String type_upper = clone_string(parameter.type, arena);
								to_upper(type_upper);
								fprintf(output, "make_rpc_message_parameter(RPCParameterType::RPC_PARAM_%s_TYPE, (void*)&%s);\n", *type_upper, *parameter.name);
							}
						}
					}
				} else {
fprintf(output, "			RPCMessageParameter parameters[] = {\n");
					for (unsigned j = 1; j < array_count(rpc.parameters); ++j) {
						Parameter &parameter = rpc.parameters[j];

						if (!conversion_write(output, network_conversion_array, parameter.type_id, parameter.name)) {
							String type_upper = clone_string(parameter.type, arena);
							to_upper(type_upper);
fprintf(output, "				make_rpc_message_parameter(RPCParameterType::RPC_PARAM_%s_TYPE, (void*)&%s),\n", *type_upper, *parameter.name);
						}

					}
fprintf(output, "			};\n");
				}
fprintf(output, "			_send(%s_id32, receiving_peer, parameters, %d);\n", *rpc.name, array_count(rpc.parameters) - 1);
			} else {
fprintf(output, "			_send(%s_id32, receiving_peer, 0, 0);\n", *rpc.name, array_count(rpc.parameters) - 1);
			}
fprintf(output, "		}\n");
		}
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}
}

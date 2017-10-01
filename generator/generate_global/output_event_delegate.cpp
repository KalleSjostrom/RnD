#define ARRAY_NAME FunctionReceiverArray
#define ARRAY_TYPE unsigned
#define ARRAY_MAX_SIZE 32
#include "../utils/array_static.cpp"

struct FunctionReceiverLink {
	Function *function;
	FunctionReceiverArray receivers;

	unsigned max_number_of_entities; // If this is > 0 then this is an entity event
};

struct FunctionReceiverHashEntry {
	unsigned value; // index into the function receiver array
	unsigned key;
};

#define FUNCTION_RECEIVER_LINK_SIZE 1024
#define FUNCTION_RECEIVER_MAP_SIZE 2048

// FULKOD(kalle): Linear search
unsigned get_receiver_index_for(unsigned receiver_name_id, ReceiverArray &receiver_array) {
	for (unsigned i = 0; i < array_count(receiver_array); ++i) {
		Receiver &receiver = receiver_array[i];
		if (receiver.name_id == receiver_name_id) {
			return i;
		}
	}
	ASSERT(false, "Couldn't find receiver index for name_id %u", receiver_name_id);
	return 0xFFFFFFFF;
}

void add_function(Function &function, unsigned *cursor, FunctionReceiverLink *functions, FunctionReceiverHashEntry *hashmap, ReceiverArray &receiver_array, unsigned max_number_of_entities = 0) {
	HASH_LOOKUP(entry, hashmap, FUNCTION_RECEIVER_MAP_SIZE, function.name_id);
	FunctionReceiverLink *frl = 0;
	if (entry->key == function.name_id) { // was already here!
		frl = functions + entry->value;
		frl->max_number_of_entities += max_number_of_entities;
	} else {
		entry->key = function.name_id;
		entry->value = *cursor;

		frl = functions + (*cursor)++;
		frl->function = &function;
		frl->max_number_of_entities = max_number_of_entities;
	}

	unsigned receiver_index = get_receiver_index_for(function.receiver_name_id, receiver_array);
	frl->receivers.push_back(receiver_index);
}

void output_event_delegate(RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, ReceiverArray &receiver_array, MemoryArena &arena) {
	{
		FunctionReceiverLink *functions = (FunctionReceiverLink *) allocate_memory(arena, FUNCTION_RECEIVER_LINK_SIZE*sizeof(FunctionReceiverLink));
		FunctionReceiverHashEntry *hashmap = (FunctionReceiverHashEntry *) allocate_memory(arena, FUNCTION_RECEIVER_MAP_SIZE*sizeof(FunctionReceiverHashEntry));

		unsigned cursor = 0;

		for (unsigned i = 0; i < array_count(event_array); ++i) {
			add_function(event_array[i].function, &cursor, functions, hashmap, receiver_array, event_array[i].max_number_of_entities);
		}
		for (unsigned i = 0; i < array_count(rpc_array); ++i) {
			add_function(rpc_array[i].function, &cursor, functions, hashmap, receiver_array);
		}
		for (unsigned i = 0; i < array_count(flow_array); ++i) {
			add_function(flow_array[i].function, &cursor, functions, hashmap, receiver_array);
		}

		for (unsigned i = 0; i < cursor; ++i) {
			FunctionReceiverLink &function_link = functions[i];
			Function &function = *function_link.function;
			if (function_link.max_number_of_entities > 0) {
				array_remove_last(function.parameters); // Remove the last parameter, this is reserved for the user_data
			}
		}

		{
			MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../" GAME_CODE_DIR "/generated/event_delegate.generated.h");

			//////// receivers ////////
fprintf(output, "enum ReceiverType {\n");
			for (unsigned i = 0; i < array_count(receiver_array); ++i) {
				Receiver &receiver = receiver_array[i];
fprintf(output, "	ReceiverType_%s,\n", *receiver.name);
			}
fprintf(output, "};\n");
fprintf(output, "\n");
fprintf(output, "struct Instance;\n");
fprintf(output, "struct Receiver {\n");
fprintf(output, "	void *object;\n");
fprintf(output, "	ReceiverType type;\n");
fprintf(output, "};\n");
fprintf(output, "\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
fprintf(output, "struct EntityReceiverlist_%s {\n", *function.name);
					for (unsigned j = 0; j < function_link.receivers.count; ++j) {
						Receiver &function_receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "	void *%s;\n", *function_receiver.name_underscored);
fprintf(output, "	unsigned %s_user_data;\n", *function_receiver.name_underscored);
					}
fprintf(output, "};\n");
fprintf(output, "\n");
				}
			}
fprintf(output, "struct EventDelegate {\n");
fprintf(output, "\n");
fprintf(output, "	EventDelegate();\n");
fprintf(output, "\n");
fprintf(output, "	unsigned valid_store_index(Entity *entities, unsigned count, unsigned index) {\n");
fprintf(output, "		for (unsigned i = 0; i < count; ++i) {\n");
fprintf(output, "			Entity entity = entities[index];\n");
fprintf(output, "			if (entity == INVALID_ENTITY) {\n");
fprintf(output, "				return index; // this index is free! we are done\n");
fprintf(output, "			}\n");
fprintf(output, "\n");
fprintf(output, "			index = (index == count-1) ? 0 : index+1; // Increment the first_free_store_index\n");
fprintf(output, "		}\n");
fprintf(output, "\n");
fprintf(output, "		ASSERT(false, \"Too many entities registered!\");\n");
fprintf(output, "		return 0;\n");
fprintf(output, "	}\n");
fprintf(output, "\n");
			//////// register ////////
			for (unsigned i = 0; i < array_count(receiver_array); ++i) {
				Receiver &receiver = receiver_array[i];
fprintf(output, "	void register_%s(void *object);\n", *receiver.name_underscored);
fprintf(output, "	void unregister_%s(void *object);\n", *receiver.name_underscored);
			}
fprintf(output, "\n");
fprintf(output, "	// Register for entity events\n");
// fprintf(output, "	void _register_entity_event(Entity entity, void *object, ReceiverType type, HashMap &hash_map, EntityReceiverlist *storage, unsigned storage_count, unsigned *storage_index, unsigned optional_user_data);\n");
// fprintf(output, "	void _unregister_entity_event(Entity entity, void *object, HashMap &hash_map, EntityReceiverlist *storage);\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
fprintf(output, "	void register_%s(Entity entity, void *object, ReceiverType type, unsigned user_data = 0);\n", *function.name);
fprintf(output, "	void unregister_%s(Entity entity, void *object);\n", *function.name);
fprintf(output, "\n");
				}
			}
fprintf(output, "	void assert_unregistered(Instance *instance);\n");
fprintf(output, "\n");

			//////// trigger ////////
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
fprintf(output, "	void trigger_%s(", *function.name);

				for (unsigned j = 0; j < array_count(function.parameters); ++j) {
					Parameter &parameter = function.parameters[j];
					fprintf(output, "%s %s%s", *parameter.type, parameter.is_pointer?"*":parameter.is_ref?"&":"", *parameter.name);
					if (j < array_count(function.parameters)-1)
						fprintf(output, ", ");
				}
				fprintf(output, ");\n");
			}
fprintf(output, "\n");

			//////// receiver storage ////////
fprintf(output, "	// One event per receiver declaration. If we have multiple instances of the same object, this might not work. Dynamically growing array? Constant times this size?\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
					Receiver &receiver = receiver_array[function_link.receivers[0]];
fprintf(output, "	unsigned %s_storage_index;\n", *function.name);
fprintf(output, "	Entity %s_entities[%u];\n", *function.name, function_link.max_number_of_entities);
fprintf(output, "	EntityReceiverlist_%s %s_storage[%u]; // This is the max number of entities that can be registered\n", *function.name, *function.name, function_link.max_number_of_entities);
fprintf(output, "	#define %s_hash_size HASH_SIZE_FOR(%u)\n", *function.name, function_link.max_number_of_entities);
fprintf(output, "	HashEntry %s_map_storage[%s_hash_size];\n", *function.name, *function.name);
fprintf(output, "	HashMap %s_map;\n", *function.name);
				} else {
fprintf(output, "	unsigned %s_count;\n", *function.name);
fprintf(output, "	Receiver %s_receivers[%d]; //! count %s_count\n", *function.name, function_link.receivers.count, *function.name);
				}
			}
fprintf(output, "};\n");

			fclose(output);
		}

		{
			MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../" GAME_CODE_DIR "/generated/event_delegate.generated.cpp");

fprintf(output, "EventDelegate::EventDelegate() : ");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
fprintf(output, "%s_storage_index(0)", *function.name);
				} else {
fprintf(output, "%s_count(0)", *function.name);
				}
				if (i < cursor - 1)
fprintf(output, ", ");
			}
fprintf(output, "\n{\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
fprintf(output, "	hash_init(%s_map, %s_map_storage, ARRAY_COUNT(%s_map_storage), INVALID_ENTITY);\n", *function.name, *function.name, *function.name);
				}
			}
fprintf(output, "}\n");
fprintf(output, "\n");

			//////// register/unregister for entity events ////////
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
fprintf(output, "void EventDelegate::register_%s(Entity entity, void *object, ReceiverType type, unsigned optional_user_data) {\n", *function.name);
fprintf(output, "	if (entity == INVALID_ENTITY) return;\n");
fprintf(output, "	HashEntry *entry = hash_lookup(%s_map, entity);\n", *function.name);
fprintf(output, "	if (entry->key != entity) { // We don't have a storage entry for this, create one!\n");
fprintf(output, "		entry->key = entity;\n");
fprintf(output, "\n");
fprintf(output, "		%s_storage_index = valid_store_index(%s_entities, %u, %s_storage_index);\n", *function.name, *function.name, function_link.max_number_of_entities, *function.name);
fprintf(output, "		%s_entities[%s_storage_index] = entity; // Reserve the slot\n", *function.name, *function.name);
// fprintf(output, "		LOG_INFO(\"EventDelegate\", \"%s_storage_index (entity=%%u, storage_index=%%u)\", entity, %s_storage_index);\n", *function.name, *function.name);
fprintf(output, "		entry->value = %s_storage_index;\n", *function.name);
fprintf(output, "		EntityReceiverlist_%s &receivers = %s_storage[entry->value];\n", *function.name, *function.name);
fprintf(output, "		memset(&receivers, 0, sizeof(receivers));\n");
fprintf(output, "	}\n");
fprintf(output, "\n");
fprintf(output, "	EntityReceiverlist_%s &receivers = %s_storage[entry->value];\n", *function.name, *function.name);
					if (function_link.receivers.count == 1) {
						Receiver &receiver = receiver_array[function_link.receivers[0]];
fprintf(output, "	receivers.%s = object;\n", *receiver.name_underscored);
fprintf(output, "	receivers.%s_user_data = optional_user_data;\n", *receiver.name_underscored);
// fprintf(output, "	LOG_INFO(\"EventDelegate\", \"Register (entity=%%u, user_data=%%u, receiver=%s, storage_index=%%u)\", entity, optional_user_data, %s_storage_index);\n", *receiver.name, *function.name);
					} else {
fprintf(output, "	switch(type) {\n");
						for (unsigned j = 0; j < function_link.receivers.count; ++j) {
							Receiver &receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "		case ReceiverType_%s: {\n", *receiver.name);
fprintf(output, "			receivers.%s = object;\n", *receiver.name_underscored);
fprintf(output, "			receivers.%s_user_data = optional_user_data;\n", *receiver.name_underscored);
// fprintf(output, "			LOG_INFO(\"EventDelegate\", \"Register (entity=%%u, user_data=%%u, receiver=%s, storage_index=%%u)\", entity, optional_user_data, %s_storage_index);\n", *receiver.name, *function.name);
fprintf(output, "		} break;\n");
						}
fprintf(output, "	};\n");
					}
fprintf(output, "}\n");
fprintf(output, "void EventDelegate::unregister_%s(Entity entity, void *object) {\n", *function.name);
fprintf(output, "	HashEntry *entry = hash_lookup(%s_map, entity);\n", *function.name);
fprintf(output, "	if (entry->key != entity)\n");
fprintf(output, "		return;\n");
fprintf(output, "\n");
fprintf(output, "	EntityReceiverlist_%s &receivers = %s_storage[entry->value];\n", *function.name, *function.name);
// fprintf(output, "	LOG_INFO(\"EventDelegate\", \"Unregister %s_storage_index (entity=%%u, storage_index=%%u)\", entity, %s_storage_index);\n", *function.name, *function.name);
					if (function_link.receivers.count == 1) {
						Receiver &receiver = receiver_array[function_link.receivers[0]];
// fprintf(output, "	LOG_INFO(\"EventDelegate\", \"Unregister (entity=%%u, user_data=%%u, receiver=%s, storage_index=%%u)\", entity, receivers.%s_user_data, %s_storage_index);\n", *receiver.name, *receiver.name_underscored, *function.name);
fprintf(output, "	receivers.%s = 0;\n", *receiver.name_underscored);
fprintf(output, "	receivers.%s_user_data = 0;\n", *receiver.name_underscored);
// fprintf(output, "	LOG_INFO(\"EventDelegate\", \"Hash remove (entity=%%u)\", entity);\n");
fprintf(output, "	%s_entities[entry->value] = INVALID_ENTITY; // Give up the slot\n", *function.name);
fprintf(output, "	hash_try_remove(%s_map, entity);\n", *function.name);

					} else {
						for (unsigned j = 0; j < function_link.receivers.count; ++j) {
							Receiver &receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "	if (receivers.%s == object) {\n", *receiver.name_underscored);
// fprintf(output, "		LOG_INFO(\"EventDelegate\", \"Unregister (entity=%%u, user_data=%%u, receiver=%s, storage_index=%%u)\", entity, receivers.%s_user_data, %s_storage_index);\n", *receiver.name, *receiver.name_underscored, *function.name);
fprintf(output, "		receivers.%s = 0;\n", *receiver.name_underscored);
fprintf(output, "		receivers.%s_user_data = 0;\n", *receiver.name_underscored);
fprintf(output, "	}\n");
						}
fprintf(output, "	if (");
						for (unsigned j = 0; j < function_link.receivers.count; ++j) {
							Receiver &receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "receivers.%s == 0", *receiver.name_underscored);
							if (j < function_link.receivers.count-1) {
fprintf(output, " && ");
							}
						}
fprintf(output, ") {\n");
// fprintf(output, "		LOG_INFO(\"EventDelegate\", \"Hash remove (entity=%%u)\", entity);\n");
fprintf(output, "		%s_entities[entry->value] = INVALID_ENTITY; // Give up the slot\n", *function.name);
fprintf(output, "		hash_try_remove(%s_map, entity);\n", *function.name);
fprintf(output, "	}\n");
					}

fprintf(output, "}\n");
				}
			}

fprintf(output, "void EventDelegate::assert_unregistered(Instance *instance) {\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function_link.max_number_of_entities > 0) {
fprintf(output, "	{\n");
fprintf(output, "		HashEntry *entry = hash_lookup(%s_map, instance->entity);\n", *function.name);
fprintf(output, "		ASSERT(entry->key == INVALID_ENTITY, \"Entity is still registered in '%s' (entity_path=%%s, entity_id=%%u)\", ENTITY_ID_STR(instance->entity_id), instance->entity._id);\n", *function.name);
fprintf(output, "	}\n");
				}
			}
fprintf(output, "}\n");

			//////// register ////////
			for (unsigned i = 0; i < array_count(receiver_array); ++i) {
				Receiver &receiver = receiver_array[i];

fprintf(output, "void EventDelegate::register_%s(void *object) {\n", *receiver.name_underscored);
fprintf(output, "	Receiver receiver = { object, ReceiverType_%s };\n", *receiver.name);
				for (unsigned j = 0; j < cursor; ++j) {
					FunctionReceiverLink &function_link = functions[j];
					if (function_link.max_number_of_entities == 0) {
						Function &function = *function_link.function;
						for (unsigned k = 0; k < function_link.receivers.count; ++k) {
							Receiver &function_receiver = receiver_array[function_link.receivers[k]];
							if (function_receiver.name_id == receiver.name_id) {
fprintf(output, "	%s_receivers[%s_count++] = receiver;\n", *function.name, *function.name);
								break;
							}
						}
					}
				}
fprintf(output, "}\n");
			}
fprintf(output, "\n");

			//////// unregister ////////
fprintf(output, "static void _unregister(unsigned &count, Receiver *receivers, void *object) {\n");
fprintf(output, "	for (unsigned i = 0; i < count; ++i) {\n");
fprintf(output, "		Receiver &receiver = receivers[i];\n");
fprintf(output, "		if (receiver.object == object) {\n");
fprintf(output, "			receivers[i] = receivers[--count];\n");
fprintf(output, "			return;\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

			for (unsigned i = 0; i < array_count(receiver_array); ++i) {
				Receiver &receiver = receiver_array[i];

fprintf(output, "void EventDelegate::unregister_%s(void *object) {\n", *receiver.name_underscored);
				for (unsigned j = 0; j < cursor; ++j) {
					FunctionReceiverLink &function_link = functions[j];
					if (function_link.max_number_of_entities == 0) {
						Function &function = *function_link.function;
						for (unsigned k = 0; k < function_link.receivers.count; ++k) {
							Receiver &function_receiver = receiver_array[function_link.receivers[k]];
							if (function_receiver.name_id == receiver.name_id) {
fprintf(output, "	_unregister(%s_count, %s_receivers, object);\n", *function.name, *function.name);
								break;
							}
						}
					}
				}
fprintf(output, "}\n");
			}
fprintf(output, "\n");

			//////// trigger ////////
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
fprintf(output, "void EventDelegate::trigger_%s(", *function.name);

				for (unsigned j = 0; j < array_count(function.parameters); ++j) {
					Parameter &parameter = function.parameters[j];
					fprintf(output, "%s %s%s", *parameter.type, parameter.is_pointer?"*":parameter.is_ref ? "&":"", *parameter.name);
					if (j < array_count(function.parameters)-1)
						fprintf(output, ", ");
				}
				fprintf(output, ") {\n");

				// Since the sender is positioned at index 0, we should begin at index 1 in this case.
				if (function_link.max_number_of_entities > 0) {
					//////////// ENTITY EVENT
fprintf(output, "	HashEntry *entry = hash_lookup(%s_map, entity);\n", *function.name);
fprintf(output, "	if (entry->key != entity) // Could not find any registered events for entity\n");
fprintf(output, "		return;\n");
fprintf(output, "\n");
fprintf(output, "	EntityReceiverlist_%s &receivers = %s_storage[entry->value];\n", *function.name, *function.name);

					for (unsigned j = 0; j < function_link.receivers.count; ++j) {
						Receiver &receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "	%s *%s_object = (%s *)receivers.%s;\n", *receiver.name, *receiver.name_underscored, *receiver.name, *receiver.name_underscored);

fprintf(output, "	if (%s_object && %s_object->%s(", *receiver.name_underscored, *receiver.name_underscored, *function.name);
						for (unsigned k = 0; k < array_count(function.parameters); ++k) {
							Parameter &parameter = function.parameters[k];
							fprintf(output, "%s", *parameter.name);
							if (k < array_count(function.parameters)-1)
								fprintf(output, ", ");
						}
						fprintf(output, ", receivers.%s_user_data)) {\n", *receiver.name_underscored);
fprintf(output, "		receivers.%s = 0;\n", *receiver.name_underscored);
fprintf(output, "		receivers.%s_user_data = 0;\n", *receiver.name_underscored);
fprintf(output, "	}\n");
					}

fprintf(output, "	if (");
					for (unsigned j = 0; j < function_link.receivers.count; ++j) {
						Receiver &receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "receivers.%s == 0", *receiver.name_underscored);
						if (j < function_link.receivers.count-1) {
fprintf(output, " && ");
						}
					}
fprintf(output, ") {\n");
fprintf(output, "		%s_entities[entry->value] = INVALID_ENTITY; // Give up the slot\n", *function.name);
fprintf(output, "		hash_try_remove(%s_map, entity);\n", *function.name);
fprintf(output, "	}\n");
				} else {
fprintf(output, "	for (unsigned i = 0; i < %s_count; ++i) {\n", *function.name);
fprintf(output, "		Receiver &receiver = %s_receivers[i];\n", *function.name);
fprintf(output, "		switch (receiver.type) {\n");
					for (unsigned j = 0; j < function_link.receivers.count; ++j) {
						Receiver &receiver = receiver_array[function_link.receivers[j]];
fprintf(output, "			case ReceiverType_%s: {\n", *receiver.name);
fprintf(output, "				%s *object = (%s *)receiver.object;\n", *receiver.name, *receiver.name);

fprintf(output, "				object->%s(", *function.name);
						for (unsigned k = 0; k < array_count(function.parameters); ++k) {
							Parameter &parameter = function.parameters[k];
							fprintf(output, "%s", *parameter.name);
							if (k < array_count(function.parameters)-1)
								fprintf(output, ", ");
						}
						fprintf(output, ");\n");
fprintf(output, "			} break;\n");
					}
fprintf(output, "		}\n");
fprintf(output, "	}\n");
				}
fprintf(output, "}\n");
			}

			fclose(output);
		}
	}
}

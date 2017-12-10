struct FunctionReceiverLink {
	Function *function;

	unsigned receiver_count;
	unsigned receivers[32];
};

struct FunctionReceiverHashEntry {
	unsigned value; // index into the function receiver array
	unsigned key;
};

#define FUNCTION_RECEIVER_LINK_SIZE 1024
#define FUNCTION_RECEIVER_MAP_SIZE 2048

// FULKOD(kalle): Linear search
unsigned get_receiver_index_for(unsigned receiver_name_id, ReceiverArray &receiver_array) {
	for (unsigned i = 0; i < receiver_array.count; ++i) {
		Receiver &receiver = receiver_array.entries[i];
		if (receiver.name_id == receiver_name_id) {
			return i;
		}
	}
	ASSERT(false, "Couldn't find receiver index for name_id %u", receiver_name_id);
	return 0xFFFFFFFF;
}

void add_function(Function &function, unsigned *cursor, FunctionReceiverLink *functions, FunctionReceiverHashEntry *hashmap, ReceiverArray &receiver_array) {
	HASH_LOOKUP(entry, hashmap, FUNCTION_RECEIVER_MAP_SIZE, function.name_id);
	if (entry->key == function.name_id) { // was already here!
		FunctionReceiverLink &frl = functions[entry->value];

		if (function.function_type == FunctionType_EntitySender) {
			unsigned temp_receiver = frl.receivers[0];
			frl.function = &function;
			frl.receivers[0] = get_receiver_index_for(function.receiver_name_id, receiver_array);
			frl.receivers[frl.receiver_count++] = temp_receiver;
		} else {
			frl.receivers[frl.receiver_count++] = get_receiver_index_for(function.receiver_name_id, receiver_array);
		}
	} else {
		entry->key = function.name_id;
		entry->value = *cursor;

		FunctionReceiverLink &frl = functions[(*cursor)++];
		frl.function = &function;
		frl.receivers[frl.receiver_count++] = get_receiver_index_for(function.receiver_name_id, receiver_array);
	}
}

void output_event_delegate(RPCArray &rpc_array, EventArray &event_array, FlowArray &flow_array, ReceiverArray &receiver_array, MemoryArena &arena) {
	{
		FunctionReceiverLink *functions = (FunctionReceiverLink *) allocate_memory(arena, FUNCTION_RECEIVER_LINK_SIZE*sizeof(FunctionReceiverLink*));
		FunctionReceiverHashEntry *hashmap = (FunctionReceiverHashEntry *) allocate_memory(arena, FUNCTION_RECEIVER_MAP_SIZE*sizeof(FunctionReceiverHashEntry*));

		unsigned cursor = 0;

		for (unsigned i = 0; i < event_array.count; ++i) {
			add_function(event_array.entries[i], &cursor, functions, hashmap, receiver_array);
		}
		for (unsigned i = 0; i < rpc_array.count; ++i) {
			add_function(rpc_array.entries[i].function, &cursor, functions, hashmap, receiver_array);
		}
		for (unsigned i = 0; i < flow_array.count; ++i) {
			add_function(flow_array.entries[i].function, &cursor, functions, hashmap, receiver_array);
		}

		{
			MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/event_delegate.generated.h");

			//////// receivers ////////
fprintf(output, "enum ReceiverType {\n");
			for (unsigned i = 0; i < receiver_array.count; ++i) {
				Receiver &receiver = receiver_array.entries[i];
fprintf(output, "	ReceiverType_%s,\n", *receiver.name);
			}
fprintf(output, "};\n");
fprintf(output, "\n");
fprintf(output, "struct Receiver {\n");
fprintf(output, "	void *object;\n");
fprintf(output, "	ReceiverType type;\n");
fprintf(output, "};\n");
fprintf(output, "\n");
fprintf(output, "struct EntityReceiverlist {\n");
fprintf(output, "	EntityRef entity;\n");
fprintf(output, "	unsigned count;\n");
fprintf(output, "	Receiver entries[8]; // 8 different functions can listen to one event for one entity\n");
fprintf(output, "};\n");
fprintf(output, "\n");
fprintf(output, "struct EventDelegate {\n");
fprintf(output, "\n");
fprintf(output, "	EventDelegate();\n");
fprintf(output, "\n");
fprintf(output, "	unsigned valid_store_index(EntityReceiverlist *store, unsigned count, unsigned index) {\n");
fprintf(output, "		for (unsigned i = 0; i < count; ++i) {\n");
fprintf(output, "			EntityReceiverlist *receivers = store + index;\n");
fprintf(output, "			if (receivers->entity == 0) {\n");
fprintf(output, "				return index; // this index is free! we are done\n");
fprintf(output, "			}\n");
fprintf(output, "\n");
fprintf(output, "			index = (index == count) ? 0 : index+1; // Increment the first_free_store_index\n");
fprintf(output, "		}\n");
fprintf(output, "\n");
fprintf(output, "		ASSERT(false, \"Too many entries registered!\");\n");
fprintf(output, "		return 0;\n");
fprintf(output, "	}\n");
fprintf(output, "\n");


			//////// register ////////
			for (unsigned i = 0; i < receiver_array.count; ++i) {
				Receiver &receiver = receiver_array.entries[i];
fprintf(output, "	void register_%s(void *object);\n", *receiver.name_underscored);
fprintf(output, "	void unregister_%s(void *object);\n", *receiver.name_underscored);
			}
fprintf(output, "\n");
fprintf(output, "	// Register for entity events\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function.function_type == FunctionType_EntitySender) {
fprintf(output, "	void register_%s(EntityRef entity, void *object, ReceiverType type);\n", *function.name);
fprintf(output, "	void unregister_%s(EntityRef entity, void *object);\n", *function.name);
fprintf(output, "\n");
				}
			}

fprintf(output, "\n");

			//////// trigger ////////
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
fprintf(output, "	void trigger_%s(", *function.name);

				for (unsigned j = 0; j < function.parameter_count; ++j) {
					Parameter &parameter = function.parameters[j];
					fprintf(output, "%s %s%s", *parameter.type, parameter.is_pointer?"*":parameter.is_ref?"&":"", *parameter.name);
					if (j < function.parameter_count-1)
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
				if (function.function_type == FunctionType_EntitySender) {
					Receiver &receiver = receiver_array.entries[function_link.receivers[0]];
fprintf(output, "	unsigned %s_storage_index;\n", *function.name);
fprintf(output, "	EntityReceiverlist %s_storage[%s]; // This is the max number of entities that the sender manages. (e.g. HealthComponent::max_instances if health component is the event sender)\n", *function.name, *function.max_entities);
fprintf(output, "	#define %s_HASH_SIZE HASH_SIZE_FOR(%s)\n", *function.name, *function.max_entities);
fprintf(output, "	HashEntry %s_map_storage[%s_HASH_SIZE];\n", *function.name, *function.name);
fprintf(output, "	HashMap %s_map;\n", *function.name);
				} else {
fprintf(output, "	unsigned %s_count;\n", *function.name);
fprintf(output, "	Receiver %s_receivers[%d]; //! count %s_count\n", *function.name, function_link.receiver_count, *function.name);
				}
			}
fprintf(output, "};\n");

			fclose(output);
		}

		{
			MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/event_delegate.generated.cpp");

fprintf(output, "EventDelegate::EventDelegate() : ");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function.function_type == FunctionType_EntitySender) {
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
				if (function.function_type == FunctionType_EntitySender) {
fprintf(output, "	hash_init(%s_map, %s_map_storage, ARRAY_COUNT(%s_map_storage), INVALID_ENTITY);\n", *function.name, *function.name, *function.name);
				}
			}
fprintf(output, "}\n");
fprintf(output, "\n");

			//////// register/unregister for entity events ////////
fprintf(output, "\n// Register for entity events\n");
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function.function_type == FunctionType_EntitySender) {
fprintf(output, "void EventDelegate::register_%s(EntityRef entity, void *object, ReceiverType type) {\n", *function.name);
fprintf(output, "	HashEntry *entry = hash_lookup(%s_map, entity);\n", *function.name);
fprintf(output, "	entry->key = entity;\n");
fprintf(output, "\n");
fprintf(output, "	%s_storage_index = valid_store_index(%s_storage, ARRAY_COUNT(%s_storage), %s_storage_index);\n", *function.name, *function.name, *function.name, *function.name);
fprintf(output, "	entry->value = %s_storage_index;\n", *function.name);
fprintf(output, "\n");
fprintf(output, "	EntityReceiverlist &receivers = %s_storage[entry->value];\n", *function.name);
fprintf(output, "\n");
fprintf(output, "#if DEVELOPMENT\n");
fprintf(output, "	// Validate that this entity event is not already added\n");
fprintf(output, "	for (unsigned i = 0; i < receivers.count; ++i) {\n");
fprintf(output, "		Receiver &receiver = receivers.entries[i];\n");
fprintf(output, "		ASSERT(receiver.object != object, \"Event listener object already registered!\");\n");
fprintf(output, "	}\n");
fprintf(output, "#endif // DEVELOPMENT\n");
fprintf(output, "\n");
fprintf(output, "	Receiver receiver = { object, type };\n");
fprintf(output, "	receivers.entries[receivers.count++] = receiver;\n");
fprintf(output, "}\n");
fprintf(output, "\n");
fprintf(output, "void EventDelegate::unregister_%s(EntityRef entity, void *object) {\n", *function.name);
fprintf(output, "	unsigned index = hash_remove(%s_map, entity);\n", *function.name);
fprintf(output, "\n");
fprintf(output, "	EntityReceiverlist &receivers = %s_storage[index];\n", *function.name);
fprintf(output, "	for (unsigned i = 0; i < receivers.count; ++i) {\n");
fprintf(output, "		Receiver &receiver = receivers.entries[i];\n");
fprintf(output, "		if (receiver.object == object) {\n");
fprintf(output, "			receivers.entries[i] = receivers.entries[--receivers.count];\n");
fprintf(output, "			break;\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");
fprintf(output, "\n");
				}
			}

			//////// register ////////
			for (unsigned i = 0; i < receiver_array.count; ++i) {
				Receiver &receiver = receiver_array.entries[i];

fprintf(output, "void EventDelegate::register_%s(void *object) {\n", *receiver.name_underscored);
fprintf(output, "	Receiver receiver = { object, ReceiverType_%s };\n", *receiver.name);
				for (unsigned j = 0; j < cursor; ++j) {
					FunctionReceiverLink &function_link = functions[j];
					Function &function = *function_link.function;
					if (function.function_type == FunctionType_GlobalReceiver) {
						for (unsigned k = 0; k < function_link.receiver_count; ++k) {
							Receiver &function_receiver = receiver_array.entries[function_link.receivers[k]];
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
			for (unsigned i = 0; i < cursor; ++i) {
				FunctionReceiverLink &function_link = functions[i];
				Function &function = *function_link.function;
				if (function.function_type == FunctionType_GlobalReceiver) {
fprintf(output, "static void _unregister_%s(EventDelegate &event_delegate, void *object) {\n", *function.name);
fprintf(output, "	for (unsigned i = 0; i < event_delegate.%s_count; ++i) {\n", *function.name);
fprintf(output, "		Receiver &receiver = event_delegate.%s_receivers[i];\n", *function.name);
fprintf(output, "		if (receiver.object == object) {\n");
fprintf(output, "			event_delegate.%s_receivers[i] = event_delegate.%s_receivers[--event_delegate.%s_count];\n", *function.name, *function.name, *function.name);
fprintf(output, "			return; // There shouldn't be any more entries with this object.\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");
				}
			}

			for (unsigned i = 0; i < receiver_array.count; ++i) {
				Receiver &receiver = receiver_array.entries[i];

fprintf(output, "void EventDelegate::unregister_%s(void *object) {\n", *receiver.name_underscored);
				for (unsigned j = 0; j < cursor; ++j) {
					FunctionReceiverLink &function_link = functions[j];
					Function &function = *function_link.function;
					if (function.function_type == FunctionType_GlobalReceiver) {
						for (unsigned k = 0; k < function_link.receiver_count; ++k) {
							Receiver &function_receiver = receiver_array.entries[function_link.receivers[k]];
							if (function_receiver.name_id == receiver.name_id) {
fprintf(output, "	_unregister_%s(*this, object);\n", *function.name);
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

				for (unsigned j = 0; j < function.parameter_count; ++j) {
					Parameter &parameter = function.parameters[j];
					fprintf(output, "%s %s%s", *parameter.type, parameter.is_pointer?"*":parameter.is_ref ? "&":"", *parameter.name);
					if (j < function.parameter_count-1)
						fprintf(output, ", ");
				}
				fprintf(output, ") {\n");

				// Since the sender is positioned at index 0, we should begin at index 1 in this case.
				unsigned receiver_start_index = function.function_type == FunctionType_EntitySender ? 1 : 0;
				if (function.function_type == FunctionType_EntitySender) {
fprintf(output, "	HashEntry *entry = hash_lookup(%s_map, entity);\n", *function.name);
fprintf(output, "	if (entry->key != entity) // Could not find any registered events for entity\n");
fprintf(output, "		return;\n");
fprintf(output, "\n");
fprintf(output, "	EntityReceiverlist &receivers = %s_storage[entry->value];\n", *function.name);
fprintf(output, "	for (unsigned i = 0; i < receivers.count; ++i) {\n");
fprintf(output, "		Receiver &receiver = receivers.entries[i];\n");
				} else {
fprintf(output, "	for (unsigned i = 0; i < %s_count; ++i) {\n", *function.name);
fprintf(output, "		Receiver &receiver = %s_receivers[i];\n", *function.name);
				}
fprintf(output, "		switch (receiver.type) {\n");
				for (unsigned j = receiver_start_index; j < function_link.receiver_count; ++j) {
					Receiver &receiver = receiver_array.entries[function_link.receivers[j]];
fprintf(output, "			case ReceiverType_%s: {\n", *receiver.name);
fprintf(output, "				%s *object = (%s *)receiver.object;\n", *receiver.name, *receiver.name);
fprintf(output, "				object->%s(", *function.name);
					for (unsigned k = 0; k < function.parameter_count; ++k) {
						Parameter &parameter = function.parameters[k];
						fprintf(output, "%s", *parameter.name);
						if (k < function.parameter_count-1)
							fprintf(output, ", ");
					}
					fprintf(output, ");\n");
fprintf(output, "			} break;\n");
					}
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");
			}

			fclose(output);
		}
	}
}
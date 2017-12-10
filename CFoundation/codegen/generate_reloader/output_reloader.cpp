#include "predefined_types.h"

static char *entry_point = "game__ReloaderEntryPoint";

unsigned void_id = to_id32(sizeof("void") - 1, "void");

#define RELOADER_OUTPUT_DIR "../../"GAME_CODE_DIR

void output_reloader(ReloadableArray &reloadable_array, MemoryArena &arena) {

	make_sure_directory_exists(arena, MAKE_STRING(RELOADER_OUTPUT_DIR), MAKE_STRING("generated"));

	if (!file_exists(RELOADER_OUTPUT_DIR"/generated/typeinfo.generated.cpp")) {
		MAKE_OUTPUT_FILE_WITH_HEADER(output, RELOADER_OUTPUT_DIR"/generated/typeinfo.generated.cpp");
		fclose(output);
	}

	{
		MAKE_OUTPUT_FILE_WITH_HEADER(output, RELOADER_OUTPUT_DIR"/generated/reloader.generated.cpp");

fprintf(output, "static const intptr_t __RELOAD_TYPE__base_type = %d;\n", CUSTOM_TYPE_BASE);
fprintf(output, "\n");
fprintf(output, "#include \"typeinfo.generated.cpp\"\n");
fprintf(output, "\n");

fprintf(output, "static const intptr_t __RELOAD_TYPE__any = 0;\n");
fprintf(output, "static const intptr_t __RELOAD_TYPE__generic = 1;\n");

for (int i = 0; i < ARRAY_COUNT(predefined_types); ++i) {
	const char *name = predefined_types[i].name;
fprintf(output, "static const intptr_t __RELOAD_TYPE__%s = %d;\n", name, i+2);
fprintf(output, "static const size_t __RELOAD_SIZE__%s = %d;\n", name, predefined_types[i].size);
}
fprintf(output, "// These are here as placeholders for stuff that didn't exist before the reload, i.e. added variables. The real info is in typeinfo.generated.cpp\n");
for (int i = 0; i < reloadable_array.count; ++i) {
	ReloadableStruct &s = reloadable_array.entries[i];
fprintf(output, "#if !defined(__DEFINE_TYPE__%s)\n", *s.full_name_underlined);
fprintf(output, "	static const intptr_t __RELOAD_TYPE__%s = %d;\n", *s.full_name_underlined, make_string_id(s.full_name));
fprintf(output, "	static const size_t __RELOAD_SIZE__%s = 0;\n", *s.full_name_underlined);
fprintf(output, "#endif\n");
	for (int j = 0; j < s.member_count; ++j) {
		ReloadableMember &m = s.members[j];
fprintf(output, "#if !defined(__DEFINE_OFFSET__%s_%s)\n", *s.full_name_underlined, *m.name);
fprintf(output, "	static const intptr_t __RELOAD_OFFSET__%s_%s = -1;\n", *s.full_name_underlined, *m.name);
fprintf(output, "#endif\n");
	}
}
fprintf(output, "#include \"reload/reloader.cpp\"\n");
fprintf(output, "\n");
fprintf(output, "// We need this to be a struct with static functions so that reloadable structs can declare this a friend.\n");
fprintf(output, "// Otherwise, we can't access the private members of such a class.\n");
fprintf(output, "struct Reloader {\n");
fprintf(output, "	static void generate_layout();\n");
fprintf(output, "	static void reload(void *old_mspace, size_t old_size, void *new_mspace, char *temporary_memory);\n");
fprintf(output, "\n");
for (int i = 0; i < reloadable_array.count; ++i) {
	ReloadableStruct &s = reloadable_array.entries[i];
fprintf(output, "	static void set_%s(MemoryMapper *mapper, intptr_t base_old, intptr_t offset_old, intptr_t base_new, char *debug_tag = 0);\n", *s.full_name_underlined);
}
fprintf(output, "};");
fprintf(output, "\n");
fprintf(output, "void Reloader::generate_layout() {\n");
fprintf(output, "	#if PS4\n");
fprintf(output, "		#define TYPEINFO_FILEPATH (\"/host/\" __FILE__)\n");
fprintf(output, "	#else\n");
fprintf(output, "		#define TYPEINFO_FILEPATH (__FILE__)\n");
fprintf(output, "	#endif\n");
fprintf(output, "	#define RELOADER_PATHLENGTH (sizeof(TYPEINFO_FILEPATH)-1)\n");
fprintf(output, "	char typeinfo_filepath[RELOADER_PATHLENGTH+1];\n");
fprintf(output, "	char ending[] = \"typeinfo.generated.cpp\";\n");
fprintf(output, "	unsigned start_of_filename = RELOADER_PATHLENGTH - (sizeof(ending)-1);\n");
fprintf(output, "	for (unsigned i = 0; i < start_of_filename; ++i)\n");
fprintf(output, "		typeinfo_filepath[i] = TYPEINFO_FILEPATH[i];\n");
fprintf(output, "	for (unsigned i = 0; i < (sizeof(ending)-1); ++i)\n");
fprintf(output, "		typeinfo_filepath[i + start_of_filename] = ending[i];\n");
fprintf(output, "	typeinfo_filepath[RELOADER_PATHLENGTH] = '\\0';\n");
fprintf(output, "\n");
fprintf(output, "	FILE *output = fopen(typeinfo_filepath, \"w\");\n");
fprintf(output, "\n");

for (int i = 0; i < reloadable_array.count; ++i) {
	ReloadableStruct &s = reloadable_array.entries[i];
fprintf(output, "	fprintf(output, \"#define __DEFINE_TYPE__%s\\n\");\n", *s.full_name_underlined);
fprintf(output, "	fprintf(output, \"static const intptr_t __RELOAD_TYPE__%s = %d;\\n\");\n", *s.full_name_underlined, make_string_id(s.full_name));
fprintf(output, "	fprintf(output, \"static const size_t __RELOAD_SIZE__%s = %%lu;\\n\", sizeof(%s));\n", *s.full_name_underlined, *s.full_name);
	for (int j = 0; j < s.member_count; ++j) {
		ReloadableMember &m = s.members[j];
fprintf(output, "	fprintf(output, \"#define __DEFINE_OFFSET__%s_%s\\n\");\n", *s.full_name_underlined, *m.name);
fprintf(output, "	fprintf(output, \"static const intptr_t __RELOAD_OFFSET__%s_%s = %%lu;\\n\", offsetof(%s, %s));\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name);
	}
}
fprintf(output, "\n");
fprintf(output, "	fclose(output);\n");
fprintf(output, "}\n");

for (int i = 0; i < reloadable_array.count; ++i) {
	ReloadableStruct &s = reloadable_array.entries[i];
fprintf(output, "void Reloader::set_%s(MemoryMapper *mapper, intptr_t base_old, intptr_t offset_old, intptr_t base_new, char *debug_tag) {\n", *s.full_name_underlined);
	for (int j = 0; j < s.namespace_count; ++j) {
fprintf(output, "	using namespace %s;\n", *s.namespaces[j]);
	}
fprintf(output, "	if (offset_old < 0) { // offset_old of -1 means this didn't exist in the old space, i.e. we have added a variable.\n");
	if (s.added_behavior == ZERO) {
fprintf(output, "		memset((void*)base_new, 0, sizeof(%s));\n", *s.full_name);
	} else if (s.added_behavior == CONSTRUCT) {
fprintf(output, "		// Create a new\n");
fprintf(output, "		*(%s*)base_new = %s();\n", *s.full_name, *s.full_name);
	}
fprintf(output, "		return;\n");
fprintf(output, "	}\n");
fprintf(output, "	base_old += offset_old;\n");
fprintf(output, "	// Add the entire struct as a region too. This is needed to diffentiate memory of the first member vs memory of the beginning of the struct, since these are not necessarily the same when moving around the layout of a struct\n");
fprintf(output, "	add_node_to_lookup(mapper->address_lookup, base_old, __RELOAD_SIZE__%s, base_new, sizeof(%s), __RELOAD_TYPE__%s, \"%s\");\n", *s.full_name_underlined, *s.full_name, *s.full_name_underlined, *s.full_name);
fprintf(output, "\n");
	for (int j = 0; j < s.member_count; ++j) {
		ReloadableMember &m = s.members[j];
		if(m.is_pointer && m.is_array) {
fprintf(output, "	unsigned count;\n");
			break;
		}
	}

	for (int j = 0; j < s.member_count; ++j) {
		ReloadableMember &m = s.members[j];

		bool known_reloadable_struct = false;
		for (int k = 0; k < reloadable_array.count; ++k) {
			if (m.type_id == reloadable_array.entries[k].name_id) {
				known_reloadable_struct = true;
				break;
			}
		}

		if (m.is_pointer) {
			if (m.is_array) {
				if (m.count.length > 0) { // If count is non-empty we need to copy a specific amount. This is the name of another variable in the same struct as this, holding the value of the count
fprintf(output, "	count = *(unsigned*)((base_old + __RELOAD_OFFSET__%s_%s)); // Get the count from the variable.\n", *s.full_name_underlined, *m.count);
				} else {
					if (m.is_array) {
fprintf(output, "	count = %s; // No variable count, use the max count.\n", *m.max_count);
					} else {
fprintf(output, "	count = 1; // We don't have any count information here, assume a pointer to _one_ object, not an array of objects.\n");
					}
				}

				if (m.is_pointer_pointer) { // An array of pointers
					if (known_reloadable_struct) {
fprintf(output, "	set_pointer_array(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), count, __RELOAD_TYPE__%s, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type, *m.type, *s.full_name_underlined, *m.type, *m.name);
					} else {
fprintf(output, "	set_pointer_array(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), count, __RELOAD_TYPE__generic, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type, *s.full_name_underlined, *m.type, *m.name);
					}
				} else {
					if (known_reloadable_struct) {
fprintf(output, "	for (unsigned i = 0; i < count; i++) {\n");
fprintf(output, "		set_%s(mapper, base_old, __RELOAD_OFFSET__%s_%s + i * __RELOAD_SIZE__%s, base_new + offsetof(%s, %s) + i * sizeof(%s), \"%s { %s %s }\");\n", *m.full_type_underlined, *s.full_name_underlined, *m.name, *m.full_type_underlined, *s.full_name, *m.name, *m.full_type, *s.full_name, *m.type, *m.name);
fprintf(output, "	}\n");
					} else {
fprintf(output, "	memcpy_generic(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), count*sizeof(%s), \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.full_type, *s.full_name, *m.full_type, *m.name);
					}
				}
			} else {
				if (known_reloadable_struct) {
fprintf(output, "	set_pointer(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), __RELOAD_TYPE__%s, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type, *s.full_name, *m.type, *m.name);
				} else {
					if (m.type_id == void_id) {
fprintf(output, "	set_pointer(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), __RELOAD_TYPE__any, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *s.full_name, *m.type, *m.name);
					} else {
fprintf(output, "	set_pointer(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), __RELOAD_TYPE__generic, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *s.full_name, *m.type, *m.name);
					}
				}
			}
		} else {
			if (known_reloadable_struct) {
fprintf(output, "	set_%s(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), \"%s { %s %s }\");\n", *m.full_type_underlined, *s.full_name_underlined, *m.name, *s.full_name, *m.name, *s.full_name, *m.type, *m.name);
			} else {
// fprintf(output, "	memcpy_generic(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), sizeof(%s), \"%s { %s %s }\"));\n", *s.full_name, *m.name, *s.full_name, *m.name, *m.type, *s.full_name, *m.type, *m.name);
fprintf(output, "	SET_GENERIC(mapper, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), sizeof(%s), %s, \"%s { %s %s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type, *m.type, *s.full_name, *m.type, *m.name);
			}
		}
	}
fprintf(output, "}\n\n");
}

fprintf(output, "void Reloader::reload(void *old_mspace, size_t old_size, void *new_mspace, char *temporary_memory) {\n");
fprintf(output, "	malloc_state *malloc_state_old = (malloc_state *)(old_mspace);\n");
fprintf(output, "	malloc_state *malloc_state_new = (malloc_state *)(new_mspace);\n");
fprintf(output, "\n");
fprintf(output, "	malloc_chunk *current_malloc_chunk_old = (malloc_chunk*)mem2chunk(old_mspace);\n");
fprintf(output, "	malloc_chunk *current_malloc_chunk_new = (malloc_chunk*)mem2chunk(new_mspace);\n");
fprintf(output, "\n");
fprintf(output, "	intptr_t old_memory = (intptr_t)current_malloc_chunk_old;\n");
fprintf(output, "	intptr_t new_memory = (intptr_t)current_malloc_chunk_new;\n");
fprintf(output, "\n");
fprintf(output, "	MemoryMapper mapper = setup_memory_mapper(temporary_memory, old_memory, old_size);\n");
fprintf(output, "\n");
fprintf(output, "	{\n");
fprintf(output, "		// TODO(kalle): Clone the malloc_state? Should we keep the information about the freed objects?\n");
fprintf(output, "\n");
fprintf(output, "		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);\n");
fprintf(output, "		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);\n");
fprintf(output, "		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "		// We require to know about the \"entry point\", i.e. the layout of where the input memory is pointing.\n");
fprintf(output, "		Pointer cursor = {\n");
fprintf(output, "			base_old, 0, __RELOAD_TYPE__%s\n", entry_point);
fprintf(output, "		};\n");
fprintf(output, "\n");
fprintf(output, "		bool done = false;\n");
fprintf(output, "		while (!done) {\n");
fprintf(output, "			switch (cursor.pointer_type) {\n");
for (int i = 0; i < reloadable_array.count; ++i) {
	ReloadableStruct &s = reloadable_array.entries[i];
fprintf(output, "				case __RELOAD_TYPE__%s: {\n", *s.full_name_underlined);
fprintf(output, "					verify_size(current_malloc_chunk_old, __RELOAD_SIZE__%s);\n", *s.full_name_underlined);
fprintf(output, "					set_%s(&mapper, base_old, 0, base_new, \"root\");\n", *s.full_name_underlined);
fprintf(output, "					base_new += sizeof(%s);\n", *s.full_name);
fprintf(output, "					base_old += __RELOAD_SIZE__%s;\n", *s.full_name_underlined);
fprintf(output, "					set_malloc_chunk_head(current_malloc_chunk_old, current_malloc_chunk_new, sizeof(%s));\n", *s.full_name);
fprintf(output, "					verify_size(current_malloc_chunk_new, sizeof(%s));\n", *s.full_name);
fprintf(output, "				} break;\n");
}
fprintf(output, "				case __RELOAD_TYPE__generic: {\n");
fprintf(output, "					size_t bytes_to_copy = estimate_unpadded_request(chunksize(current_malloc_chunk_old));\n");
fprintf(output, "					memcpy_generic(&mapper, cursor.addr_old, 0, base_new, bytes_to_copy, \"root\");\n");
fprintf(output, "					base_new += bytes_to_copy;\n");
fprintf(output, "					base_old += bytes_to_copy;\n");
fprintf(output, "					set_malloc_chunk_head(current_malloc_chunk_old, current_malloc_chunk_new, bytes_to_copy);\n");
fprintf(output, "				} break;\n");
fprintf(output, "				default: {\n");
fprintf(output, "					ASSERT(false, \"Unknown reload type! (type=%%u)\", cursor.pointer_type);\n");
fprintf(output, "				}\n");
fprintf(output, "			}\n");
fprintf(output, "			malloc_chunk *previous = current_malloc_chunk_old;\n");
fprintf(output, "			// Go to the next chunk\n");
fprintf(output, "			current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);\n");
fprintf(output, "			current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "			base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);\n");
fprintf(output, "			base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "			size_t size_from_chunk = estimate_unpadded_request(chunksize(current_malloc_chunk_old));\n");
fprintf(output, "			if (base_old + size_from_chunk >= old_memory + old_size) {\n");
fprintf(output, "				intptr_t used_space_new = base_new - new_memory;\n");
fprintf(output, "				intptr_t used_space_old = base_old - old_memory;\n");
fprintf(output, "				intptr_t size_diff = used_space_new - used_space_old;\n");
fprintf(output, "				// If we have used more space in the new state, we have less size left to init the top with\n");
fprintf(output, "				current_malloc_chunk_new->head = current_malloc_chunk_old->head - size_diff;\n");
fprintf(output, "				init_top(malloc_state_new, current_malloc_chunk_new, chunksize(current_malloc_chunk_new));\n");
fprintf(output, "				done = true;\n");
fprintf(output, "			} else {\n");
fprintf(output, "				bool found_valid_cursor = false;\n");
fprintf(output, "				while (!found_valid_cursor) {\n");
fprintf(output, "					bool success = next_reloadable_pointer(&cursor, &mapper);\n");
fprintf(output, "					ASSERT(success, \"Could not find another pointer to follow!\")\n");
fprintf(output, "\n");
fprintf(output, "					// Check to see if this pointed in to already mapped memory. If so it's useless to us, and we'll continue to the next one.\n");
fprintf(output, "					intptr_t target_addr_old = *(intptr_t*)cursor.addr_old;\n");
fprintf(output, "					found_valid_cursor = target_addr_old >= base_old; // A valid cursor is memory we _haven't_ seen, i.e. something we want to follow.\n");
fprintf(output, "					if (found_valid_cursor)\n");
fprintf(output, "						*(intptr_t*)(cursor.addr_new) = base_new;\n");
fprintf(output, "					else {\n");
fprintf(output, "						// We have seen this memory before, so it must be a region stored for it!\n");
fprintf(output, "						success = try_set_pointer_value(mapper.address_lookup, target_addr_old, cursor.addr_new, cursor.pointer_type);\n");
fprintf(output, "						ASSERT(success, \"Couldn't set pointer value for memory we have already seen!?\");\n");
fprintf(output, "					}\n");
fprintf(output, "				}\n");
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "		for (unsigned i = 0; i < mapper.pointer_count; ++i) {\n");
fprintf(output, "			Pointer *pointer = mapper.pointers + i;\n");
fprintf(output, "			intptr_t addr_old = pointer->addr_old;\n");
fprintf(output, "\n");
fprintf(output, "			intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");
fprintf(output, "			bool success = try_set_pointer_value(mapper.address_lookup, target_addr_old, pointer->addr_new, pointer->pointer_type);\n");
fprintf(output, "			ASSERT(success, \"Could not patch the pointer after reload!\")\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

		fclose(output);
	}
}
#include "predefined_types.h"

static char *entry_point = "game__ReloaderEntryPoint";
static unsigned void_id = to_id32(sizeof("void") - 1, "void");

void output_reloader(String &type_info_path, String &reloader_path, ReloadableStruct **struct_collection, i32 struct_collection_count, ReloadableMap &reloadable_map) {
	if (!file_exists(*type_info_path)) {
		FILE *output = fopen(*type_info_path, "w");
		ASSERT(output, "Failed to open file for writing: '%s'", *type_info_path);

fprintf(output, "enum ReloadType {\n");
fprintf(output, "	ReloadType_BaseType,\n");
fprintf(output, "	ReloadType_%s,\n", entry_point);
fprintf(output, "};\n");
fprintf(output, "#define __RELOAD_SIZE__game__ReloaderEntryPoint 0\n");


		fclose(output);
	}

	{
		FILE *output = fopen(*reloader_path, "w");
		ASSERT(output, "Failed to open file for writing: '%s'", *reloader_path);

fprintf(output, "#include \"typeinfo.generated.cpp\"\n");
fprintf(output, "\n");

fprintf(output, "#include \"reload/reloader.cpp\"\n");
fprintf(output, "\n");
fprintf(output, "// We need this to be a struct with static functions so that reloadable structs can declare this a friend.\n");
fprintf(output, "// Otherwise, we can't access the private members of such a class.\n");
fprintf(output, "struct Reloader {\n");
// fprintf(output, "	static void generate_layout();\n");
// fprintf(output, "	static void reload(void *old_mspace, size_t old_size, void *new_mspace, char *temporary_memory);\n");
// fprintf(output, "\n");
// 		for (i32 i = 0; i < struct_collection_count; ++i) {
// 			ReloadableStruct &s = *struct_collection[i];
// 			if (s.has_typeinfo && s.contains_pointers) {
// fprintf(output, "	static void ptrset_%s(PointerContext &context, intptr_t base_old, intptr_t base_new, char *debug_tag = 0);\n", *s.full_name_underlined);
// 			}
// 			if (!s.is_chunk) {
// fprintf(output, "	static void set_%s(PointerContext &context, intptr_t base_old, intptr_t base_new);\n", *s.full_name_underlined);
// 			}
// 		}
// fprintf(output, "};");
// fprintf(output, "\n");
fprintf(output, "static void generate_layout() {\n");
fprintf(output, "	#if PS4\n");
fprintf(output, "		#define TYPEINFO_FILEPATH (\"/host/\" __FILE__)\n");
fprintf(output, "	#else\n");
fprintf(output, "		#define TYPEINFO_FILEPATH (__FILE__)\n");
fprintf(output, "	#endif\n");
fprintf(output, "	#define RELOADER_PATHLENGTH (sizeof(TYPEINFO_FILEPATH)-1)\n");
fprintf(output, "	char typeinfo_filepath[RELOADER_PATHLENGTH+1];\n");
fprintf(output, "	char ending[] = \"typeinfo.generated.cpp\";\n");
fprintf(output, "	unsigned start_of_filename = RELOADER_PATHLENGTH - (sizeof(ending)-1);\n");
fprintf(output, "	for (unsigned i = 0; i < start_of_filename; ++i) {\n");
fprintf(output, "		if (TYPEINFO_FILEPATH[i] == '\\\\')\n");
fprintf(output, "			typeinfo_filepath[i] = '/';\n");
fprintf(output, "		else\n");
fprintf(output, "			typeinfo_filepath[i] = TYPEINFO_FILEPATH[i];\n");
fprintf(output, "	}\n");
fprintf(output, "	for (unsigned i = 0; i < (sizeof(ending)-1); ++i)\n");
fprintf(output, "		typeinfo_filepath[i + start_of_filename] = ending[i];\n");
fprintf(output, "	typeinfo_filepath[RELOADER_PATHLENGTH] = '\\0';\n");
fprintf(output, "\n");
fprintf(output, "	FILE *output = fopen(typeinfo_filepath, \"w\");\n");
fprintf(output, "\n");

		{ // Output the ReloadType enum
fprintf(output, "	fprintf(output, \"enum ReloadType {\\n\");\n");
fprintf(output, "	fprintf(output, \"	ReloadType_void,\\n\");\n");
fprintf(output, "	fprintf(output, \"	ReloadType_generic,\\n\");\n");

			for (u32 i = 0; i < ARRAY_COUNT(predefined_types); ++i) {
				const char *name = predefined_types[i].name;
fprintf(output, "	fprintf(output, \"	ReloadType_%s,\\n\");\n", name);
			}

fprintf(output, "	fprintf(output, \"	ReloadType_BaseType,\\n\");\n");

			for (i32 i = 0; i < struct_collection_count; ++i) {
				ReloadableStruct &s = *struct_collection[i];
fprintf(output, "	fprintf(output, \"	ReloadType_%s,\\n\");\n", *s.full_name_underlined);
			}
fprintf(output, "	fprintf(output, \"}; \\n\");\n");
		}

		{ // Output the sizes and offsets
			for (u32 i = 0; i < ARRAY_COUNT(predefined_types); ++i) {
				const char *name = predefined_types[i].name;
fprintf(output, "	fprintf(output, \"#define __RELOAD_SIZE__%s %d\\n\");\n", name, predefined_types[i].size);
			}
			for (i32 i = 0; i < struct_collection_count; ++i) {
				ReloadableStruct &s = *struct_collection[i];
fprintf(output, "	fprintf(output, \"#define __RELOAD_SIZE__%s %%zu\\n\", sizeof(%s));\n", *s.full_name_underlined, *s.full_name);
				for (i32 j = 0; j < array_count(s.member_array); ++j) {
					ReloadableMember &m = s.member_array[j];
fprintf(output, "	fprintf(output, \"#define __RELOAD_OFFSET__%s_%s %%zu\\n\", offsetof(%s, %s));\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name);
				}
			}
		}

fprintf(output, "\n");
fprintf(output, "	fclose(output);\n");
fprintf(output, "}\n");

		for (i32 i = 0; i < struct_collection_count; ++i) {
			ReloadableStruct &s = *struct_collection[i];
			if (!(s.has_typeinfo && s.contains_pointers)) {
// fprintf(output, "	// We didn't exist in the old space, create us.\n");
// 				if (s.added_behavior == ZERO) {
// fprintf(output, "	memset((void*)base_new, 0, sizeof(%s));\n", *s.full_name);
// 				} else if (s.added_behavior == CONSTRUCT) {
// // fprintf(output, "	// Create a new\n");
// fprintf(output, "	*(%s*)base_new = %s();\n", *s.full_name, *s.full_name);
// 				}
// fprintf(output, "}\n");
			} else {
fprintf(output, "static void ptrset_%s(PointerContext &context, intptr_t base_old, intptr_t base_new, char *debug_tag) {\n", *s.full_name_underlined);
				for (i32 j = 0; j < array_count(s.namespace_array); ++j) {
fprintf(output, "	using namespace %s;\n", *s.namespace_array[j]);
				}
// 				for (i32 j = 0; j < array_count(s.member_array); ++j) {
// 					ReloadableMember &m = s.members[j];
// 					ReloadableStruct *known_type = m.known_reloadable_type_index >= 0 ? &reloadable_array[m.known_reloadable_type_index] : 0;
// 					if (m.existed && m.is_array && (m.is_pointer_pointer || (known_type && known_type->has_typeinfo && known_type->contains_pointers))) {
// fprintf(output, "	unsigned count;\n");
// 						break;
// 					}
// 				}

				for (i32 j = 0; j < array_count(s.member_array); ++j) {
					ReloadableMember &m = s.member_array[j];
					ReloadableStruct *known_type = m.known_reloadable_type_index >= 0 ? struct_collection[m.known_reloadable_type_index] : 0;
					bool has_valid_typeinfo = known_type && known_type->has_typeinfo;

					if (!m.existed)
						continue;

					b32 is_pointer = m.flags & MemberFlag_IsPointer;
					b32 is_pointer_pointer = m.flags & MemberFlag_IsPointerPointer;
					b32 is_array = m.flags & MemberFlag_IsArray;

					if (is_pointer) {
						if (is_array) {
							char count_buffer[1024] = {};
							if (is_pointer_pointer || (has_valid_typeinfo && known_type->contains_pointers)) {
								if (m.count.length > 0) { // If count is non-empty we need to copy a specific amount. This is the name of another variable in the same struct as this, holding the value of the count
									snprintf(count_buffer, ARRAY_COUNT(count_buffer), "*(unsigned*)((base_old + __RELOAD_OFFSET__%s_%s))", *s.full_name_underlined, *m.count);
								} else {
									if (is_array) {
										snprintf(count_buffer, ARRAY_COUNT(count_buffer), "%s", *m.max_count);
									} else {
										snprintf(count_buffer, ARRAY_COUNT(count_buffer), "1");
									}
								}
							}

							if (is_pointer_pointer) { // An array of pointers
								if (has_valid_typeinfo) {
fprintf(output, "	set_pointer_array(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), %s, ReloadType_%s, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, count_buffer, *m.type, *s.full_name_underlined, *m.type, *m.name);
								} else {
fprintf(output, "	set_pointer_array(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), %s, ReloadType_generic, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, count_buffer, *s.full_name_underlined, *m.type, *m.name);
								}
							} else {
								if (has_valid_typeinfo && known_type->contains_pointers) {
fprintf(output, "	for (unsigned i = 0; i < (%s); i++) {\n", count_buffer);
fprintf(output, "		ptrset_%s(context, base_old + __RELOAD_OFFSET__%s_%s + i * __RELOAD_SIZE__%s, base_new + offsetof(%s, %s) + i * sizeof(%s), \"%s { %s %s }\");\n", *m.full_type_underlined, *s.full_name_underlined, *m.name, *m.full_type_underlined, *s.full_name, *m.name, *m.full_type, *s.full_name, *m.type, *m.name);
fprintf(output, "	}\n");
								} else {
// fprintf(output, "	memcpy_generic(context, base_old, __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), count*sizeof(%s), \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.full_type, *s.full_name, *m.full_type, *m.name);
								}
							}
						} else {
							if (has_valid_typeinfo) {
fprintf(output, "	try_fill_pointer(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), ReloadType_%s, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type, *s.full_name, *m.type, *m.name);
							} else {
fprintf(output, "	try_fill_pointer(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), ReloadType_generic, \"%s { %s *%s }\");\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *s.full_name, *m.type, *m.name);
							}
						}
					} else {
						if (has_valid_typeinfo && known_type->contains_pointers) {
fprintf(output, "	ptrset_%s(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), \"%s { %s %s }\");\n", *m.full_type_underlined, *s.full_name_underlined, *m.name, *s.full_name, *m.name, *s.full_name, *m.type, *m.name);
						}
					}
				}
fprintf(output, "}\n\n");
			}
		}

		for (i32 i = 0; i < struct_collection_count; ++i) {
			ReloadableStruct &s = *struct_collection[i];
			if (s.is_chunk || !s.has_typeinfo)
				continue;

fprintf(output, "static void set_%s(PointerContext &context, intptr_t base_old, intptr_t base_new) {\n", *s.full_name_underlined);
fprintf(output, "	check_pointer(context, base_old, base_new, __RELOAD_SIZE__%s, ReloadType_%s);\n", *s.full_name_underlined, *s.full_name_underlined);
			if (!s.has_typeinfo) {
// fprintf(output, "	// We didn't exist in the old space, create us.\n");
				// if (s.added_behavior == ZERO) {
fprintf(output, "	memset((void*)base_new, 0, sizeof(%s));\n", *s.full_name);
// 				} else if (s.added_behavior == CONSTRUCT) {
// // fprintf(output, "	// Create a new\n");
// fprintf(output, "	*(%s*)base_new = %s();\n", *s.full_name, *s.full_name);
// 				}
fprintf(output, "}\n");
			} else {
				for (i32 j = 0; j < array_count(s.namespace_array); ++j) {
fprintf(output, "	using namespace %s;\n", *s.namespace_array[j]);
				}

				for (i32 j = 0; j < array_count(s.member_array); ++j) {
					if (s.member_array[j].union_id) {
fprintf(output, "	size_t max_union_member_size = 0;\n");
						break;
					}
				}

				u32 current_union_id = 0;
				for (i32 j = 0; j < array_count(s.member_array); ++j) {
					ReloadableMember &m = s.member_array[j];
					ReloadableStruct *known_type = m.known_reloadable_type_index >= 0 ? struct_collection[m.known_reloadable_type_index] : 0;
					bool has_valid_typeinfo = known_type && known_type->has_typeinfo;

					if (!m.existed)
						continue;

					b32 is_pointer = m.flags & MemberFlag_IsPointer;
					b32 is_pointer_pointer = m.flags & MemberFlag_IsPointerPointer;
					b32 is_array = m.flags & MemberFlag_IsArray;

					bool inserted_union_check = false;
					if (is_pointer) {
						if (is_array) {
							char count_buffer[1024] = {};
							if (m.count.length > 0) { // If count is non-empty we need to copy a specific amount. This is the name of another variable in the same struct as this, holding the value of the count
								snprintf(count_buffer, ARRAY_COUNT(count_buffer), "*(unsigned*)((base_old + __RELOAD_OFFSET__%s_%s))", *s.full_name_underlined, *m.count);
							} else {
								if (is_array) {
									snprintf(count_buffer, ARRAY_COUNT(count_buffer), "%s", *m.max_count);
								} else {
									snprintf(count_buffer, ARRAY_COUNT(count_buffer), "1");
								}
							}

							if (is_pointer_pointer) { // An array of pointers
fprintf(output, "	check_pointer(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), (%s)*sizeof(void*), ReloadType_generic);\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, count_buffer);
							} else {
								if (m.union_id) {
									if (m.union_id == current_union_id) { // All members except the first.
										inserted_union_check = true;
fprintf(output, "if (sizeof(%s) > max_union_member_size) { // Handle unions, pick the larges of the members and clone that \n", *m.full_type);
									}
fprintf(output, "	max_union_member_size = (%s)*sizeof(%s);\n", count_buffer, *m.full_type);
								}
								if (has_valid_typeinfo && !known_type->is_chunk) {
fprintf(output, "	for (unsigned i = 0; i < (%s); i++) {\n", count_buffer);
fprintf(output, "		set_%s(context, base_old + __RELOAD_OFFSET__%s_%s + i * __RELOAD_SIZE__%s, base_new + offsetof(%s, %s) + i * sizeof(%s));\n", *m.full_type_underlined, *s.full_name_underlined, *m.name, *m.full_type_underlined, *s.full_name, *m.name, *m.full_type);
fprintf(output, "	}\n");
								} else {
fprintf(output, "	move_chunk(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), (%s)*sizeof(%s), ReloadType_generic);\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, count_buffer, *m.full_type);
								}
							}
						} else {
							if (has_valid_typeinfo) {
fprintf(output, "	check_pointer(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), sizeof(void*), ReloadType_%s);\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type);
							} else {
fprintf(output, "	check_pointer(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), sizeof(void*), ReloadType_generic);\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name);
							}
						}
					} else {
						if (m.union_id) {
							if (m.union_id == current_union_id) { // All members except the first.
								inserted_union_check = true;
fprintf(output, "if (sizeof(%s) > max_union_member_size) { // Handle unions, pick the larges of the members and clone that \n", *m.full_type);
							}
fprintf(output, "	max_union_member_size = sizeof(%s);\n", *m.full_type);
						}

						if (has_valid_typeinfo && !known_type->is_chunk) {
fprintf(output, "	set_%s(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s));\n", *m.full_type_underlined, *s.full_name_underlined, *m.name, *s.full_name, *m.name);
						} else {
fprintf(output, "	move_chunk(context, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), sizeof(%s), ReloadType_generic);\n", *s.full_name_underlined, *m.name, *s.full_name, *m.name, *m.type);
						}
					}
					if (inserted_union_check) {
fprintf(output, "} // union\n");
					}

					current_union_id = m.union_id;
				}
fprintf(output, "}\n\n");
			}
		}

fprintf(output, "static void reload(void *old_mspace, size_t old_size, void *new_mspace, char *temporary_memory) {\n");
fprintf(output, "	malloc_state *malloc_state_old = (malloc_state *)(old_mspace);\n");
fprintf(output, "	malloc_state *malloc_state_new = (malloc_state *)(new_mspace);\n");
fprintf(output, "\n");
fprintf(output, "	malloc_chunk *current_malloc_chunk_old = (malloc_chunk*)mem2chunk(old_mspace);\n");
fprintf(output, "	malloc_chunk *current_malloc_chunk_new = (malloc_chunk*)mem2chunk(new_mspace);\n");
fprintf(output, "\n");
fprintf(output, "	intptr_t old_memory = (intptr_t)current_malloc_chunk_old;\n");
fprintf(output, "	intptr_t new_memory = (intptr_t)current_malloc_chunk_new;\n");
fprintf(output, "\n");
fprintf(output, "	PointerContext context = setup_pointer_context(temporary_memory, old_memory, old_size);\n");
fprintf(output, "\n");
fprintf(output, "	{ // Gather pointers\n");
fprintf(output, "		// TODO(kalle): Clone the malloc_state? Should we keep the information about the freed objects?\n");
fprintf(output, "\n");
fprintf(output, "		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);\n");
fprintf(output, "		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);\n");
fprintf(output, "		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "		// We require to know about the \"entry point\", i.e. the layout of where the input memory is pointing.\n");
fprintf(output, "		Pointer cursor = make_entry_pointer(base_old, ReloadType_%s);\n", entry_point);
fprintf(output, "\n");
fprintf(output, "		bool done = false;\n");
fprintf(output, "		while (!done) {\n");
		bool found_any_valid_struct = false;
		for (i32 i = 0; i < struct_collection_count; ++i) {
			ReloadableStruct &s = *struct_collection[i];
			if (s.has_typeinfo) {
				if (!found_any_valid_struct) {
					found_any_valid_struct = true;
fprintf(output, "			switch (cursor.pointer_type) {\n");
				}
				if (s.allocateable) {
fprintf(output, "				case ReloadType_%s: {\n", *s.full_name_underlined);
					if (s.contains_pointers) {
fprintf(output, "					ptrset_%s(context, base_old, base_new, \"root\");\n", *s.full_name_underlined);
fprintf(output, "					base_new += sizeof(%s);\n", *s.full_name);
fprintf(output, "					base_old += __RELOAD_SIZE__%s;\n", *s.full_name_underlined);
fprintf(output, "					set_malloc_chunk_head(current_malloc_chunk_old, current_malloc_chunk_new, sizeof(%s));\n", *s.full_name);
					} else {
fprintf(output, "					base_new += sizeof(%s);\n", *s.full_name);
fprintf(output, "					base_old += sizeof(%s);\n", *s.full_name);
fprintf(output, "					set_malloc_chunk_head(current_malloc_chunk_old, current_malloc_chunk_new, sizeof(%s));\n", *s.full_name);
					}
fprintf(output, "				} break;\n");
				}
			}
		}
		if (found_any_valid_struct) {
fprintf(output, "			}\n");
		}
fprintf(output, "			// We have reached the end of a new:ed struct. We need to look at the next newed portion, i.e. the next malloc chunk to find out where to go next.\n");
fprintf(output, "\n");
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
fprintf(output, "					bool success = search_for_next_reloadable_pointer(context, &cursor);\n");
fprintf(output, "					ASSERT(success, \"Could not find another pointer to follow!\");\n");
fprintf(output, "\n");
fprintf(output, "					// Check to see if this pointed in to already mapped memory. If so it's useless to us, and we'll continue to the next one.\n");
fprintf(output, "					intptr_t target_addr_old = *(intptr_t*)cursor.addr_old;\n");
fprintf(output, "					found_valid_cursor = target_addr_old >= base_old; // A valid cursor is memory we _haven't_ seen, i.e. context we want to follow.\n");
fprintf(output, "					ASSERT(found_valid_cursor, \"We should only have valid pointers left! Since it shouldn't be possible to point back\");\n");
fprintf(output, "					*(intptr_t*)(cursor.addr_new) = base_new;\n");
fprintf(output, "				}\n");
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "	// Reset state for the next iteration\n");
fprintf(output, "	current_malloc_chunk_old = (malloc_chunk*)old_memory;\n");
fprintf(output, "	current_malloc_chunk_new = (malloc_chunk*)new_memory;\n");
fprintf(output, "\n");
fprintf(output, "	{ // Copy the data\n");
fprintf(output, "		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);\n");
fprintf(output, "		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);\n");
fprintf(output, "		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);\n");
fprintf(output, "\n");
fprintf(output, "		sort_pointers(context, base_old, old_size);\n");
fprintf(output, "\n");
fprintf(output, "		// We require to know about the \"entry point\", i.e. the layout of where the input memory is pointing.\n");
fprintf(output, "		Pointer cursor = make_entry_pointer(base_old, ReloadType_%s);\n", entry_point);
fprintf(output, "\n");
fprintf(output, "		bool done = false;\n");
fprintf(output, "		while (!done) {\n");
		if (found_any_valid_struct) {
fprintf(output, "			switch (cursor.pointer_type) {\n");
		}
		for (i32 i = 0; i < struct_collection_count; ++i) {
			ReloadableStruct &s = *struct_collection[i];
			if (s.has_typeinfo && s.allocateable) {
fprintf(output, "				case ReloadType_%s: {\n", *s.full_name_underlined);
				if (s.is_chunk) {
fprintf(output, "					move_chunk(context, base_old, base_new, sizeof(%s), ReloadType_generic);\n", *s.full_name);
fprintf(output, "					base_new += sizeof(%s);\n", *s.full_name);
fprintf(output, "					base_old += sizeof(%s);\n", *s.full_name);
				} else {
fprintf(output, "					set_%s(context, base_old, base_new);\n", *s.full_name_underlined);
fprintf(output, "					base_new += sizeof(%s);\n", *s.full_name);
fprintf(output, "					base_old += __RELOAD_SIZE__%s;\n", *s.full_name_underlined);
				}
fprintf(output, "				} break;\n");
			}
		}
		if (found_any_valid_struct) {
fprintf(output, "			}\n");
		}
fprintf(output, "			// We have reached the end of a new:ed struct. We need to look at the next newed portion, i.e. the next malloc chunk to find out where to go next.\n");
fprintf(output, "\n");
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
fprintf(output, "				done = true;\n");
fprintf(output, "			} else {\n");
fprintf(output, "				get_next_reloadable_pointer(context, &cursor);\n");
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");
fprintf(output, "};\n");

		fclose(output);
	}
}

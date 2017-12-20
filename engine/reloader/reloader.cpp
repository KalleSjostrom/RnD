#include "reloader.h"
#ifdef RELOAD_PROFILING
#include "utils/profiler.c"
#endif

// Should we instead allocate the memory here? So that we will expand into system memory if size is larger than the engines default plugin memory size
void set_malloc_chunk_head(malloc_chunk *old_chunk, malloc_chunk *new_chunk, size_t new_size) {
	new_chunk->head = pad_request(new_size) | cinuse(old_chunk) | pinuse(old_chunk);
}
size_t safe_ratio_0(size_t numerator, size_t divisor) {
	return divisor == 0 ? 0 : (numerator / divisor);
}
size_t estimate_unpadded_request(size_t chunksize) {
	/////// TODO ///////
	return chunksize;
}

bool search_for_next_reloadable_pointer(PointerContext &pointer_context, Pointer *out_pointer, intptr_t min_address, intptr_t max_address) {
	// The cursor should always be on the pointer that has the lowest target address
	intptr_t lowest_target_address = INTPTR_MAX;
	int best_pointer_index = -1;
	PointerArray &pointer_array = pointer_context.pointer_array;
	for (u32 i = pointer_context.current_pointer_index; i < pointer_array.count; ++i) {
		Pointer &pointer = pointer_array.entries[i];

		if (pointer.target_addr_old < lowest_target_address && pointer.target_addr_old >= min_address && pointer.target_addr_old < max_address) {
			lowest_target_address = pointer.target_addr_old;
			best_pointer_index = (int)i;
		}
	}

	if (best_pointer_index >= 0) {
		*out_pointer = pointer_array.entries[best_pointer_index];

		Pointer temp = pointer_array.entries[pointer_context.current_pointer_index];
		pointer_array.entries[pointer_context.current_pointer_index] = *out_pointer;
		pointer_array.entries[best_pointer_index] = temp;

		pointer_context.current_pointer_index++;
		return true;
	}

	return false;
}
void sort_pointers(PointerContext &pointer_context, intptr_t base_old, size_t size) {
	pointer_context.current_pointer_index = 0;

	PointerArray &pointer_array = pointer_context.pointer_array;
	int count = 0;
	for (u32 i = 0; i < pointer_array.count; ++i) {
		Pointer &p = pointer_array.entries[i];
		intptr_t offset = p.target_addr_old - base_old;
		if (offset >= 0 && offset < (int)size) { // Pointing into our chunk
			pointer_context.sorted_pointer_indices[count].value = (unsigned)offset;
			pointer_context.sorted_pointer_indices[count].index = i;
			count++;
		} else { // I'm pointing to outside the range, just keep whatever I was pointing at
			*(intptr_t*)(p.addr_new) = p.target_addr_new;
		}
	}
	quick_sort(pointer_context.sorted_pointer_indices, count);
	pointer_context.sorted_pointer_count = count;
}

void check_pointer(PointerContext &pointer_context, intptr_t chunk_start_old, intptr_t chunk_start_new, size_t chunk_size, u32 chunk_type) {
	(void)chunk_type;
	// This chunk is a fixed block of memory (unmodifed), therefore we need to check all the pointers in the address range ([chunk_start_old, chunk_start_old + chunk_size])
	while (pointer_context.current_pointer_index < pointer_context.sorted_pointer_count) {
		SortElement &element = pointer_context.sorted_pointer_indices[pointer_context.current_pointer_index];
		Pointer &pointer = pointer_context.pointer_array.entries[element.index];
		ASSERT(pointer.target_addr_old != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_addr_old < (chunk_start_old + (ptrdiff_t)chunk_size)) { // Am I pointing to moved memory?
																		// I'm pointing into an unmodified chunk. Find out how far in the pointer pointed to, this delta is the same in the old and new space!
			pointer.target_addr_new = chunk_start_new + (pointer.target_addr_old - chunk_start_old);
			ASSERT(pointer.addr_new, "We need to store this for later!");

			*(intptr_t*)(pointer.addr_new) = pointer.target_addr_new;
			pointer_context.current_pointer_index++;
		} else {
			break;
		}
	}
}

void move_chunk(PointerContext &pointer_context, intptr_t addr_old, intptr_t addr_new, size_t size, u32 chunk_type) {
	memmove((void*)addr_new, (void*)addr_old, size);
	check_pointer(pointer_context, addr_old, addr_new, size, chunk_type);
}

bool get_next_reloadable_pointer(PointerContext &pointer_context, Pointer *out_pointer) {
	if (pointer_context.current_pointer_index >= pointer_context.sorted_pointer_count)
		return false;

	SortElement &element = pointer_context.sorted_pointer_indices[pointer_context.current_pointer_index];
	*out_pointer = pointer_context.pointer_array.entries[element.index];
	return true;
}

void try_fill_pointer(PointerContext &pointer_context, intptr_t addr_old, intptr_t addr_new, u32 type) {
	intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");
	if (target_addr_old != 0) { // We don't care about pointers pointing to null!
		ASSERT(pointer_context.pointer_array.count < MAX_POINTERS, "Array index out of bounds!");
		Pointer &p = pointer_context.pointer_array.entries[pointer_context.pointer_array.count++];

		p.addr_old = addr_old;
		p.addr_new = addr_new;
		p.target_addr_old = target_addr_old;
		p.target_addr_new = target_addr_old; // By default, point to wherever it was pointing to
		p.type = type;
	}
}

u32 expand_type(HANDLE process, u64 mod_base, SymbolContext &symbol_context, u32 _type, u32 indentation = 0) {
	NameEntry *name_entry = get_name_hash(symbol_context, _type);
	if (name_entry->type == _type) {
		TypeInfo *type_info = get_recorded_type_info(symbol_context, name_entry->name_id);
		return type_info->mask;
	}

	TI_FINDCHILDREN_PARAMS *children = 0;
	TypeInfo *type_info = make_type_info(symbol_context, _type, SymTagUDT, &children);
	name_entry->name_id = type_info->key;
	name_entry->type = _type;

#ifdef RELOAD_VERBOSE_DEBUGGING
	LOG_INFO("Reloader", "%.*s%S = { // id=0x%016zx, type=%u, size=%d, count=%d, mask=%d\n", indentation, INDENTATION, type_info->name, type_info->key, _type, type_info->size, type_info->count, type_info->mask);
#endif

	// NOTE(kalle): This wastes some memory since Count also contain member functions.
	type_info->members = PUSH_STRUCTS(symbol_context.arena, children->Count, MemberInfo);
	type_info->member_count = 0;
	for (u32 i = 0; i < children->Count; ++i) {
		ULONG child = children->ChildId[i];

		TrimmedType child_type = get_trimmed_type(symbol_context, child);
		if (child_type.tag == SymTagFunctionType) {
			continue;
		}

		MemberInfo &member_info = type_info->members[type_info->member_count++];
		fill_member_info(symbol_context, member_info, child, child_type);

		TrimmedType backing_type = child_type;
		if (backing_type.tag == SymTagPointerType) {
			type_info->mask |= TypeInfoMask_ContainsPointers;
		}
		while (backing_type.tag == SymTagPointerType || backing_type.tag == SymTagArrayType) {
			backing_type = get_trimmed_type(symbol_context, backing_type.type);

			if (backing_type.tag == SymTagPointerType) {
				type_info->mask |= TypeInfoMask_ContainsPointers;
			}
		}

		if (backing_type.tag == SymTagUDT) {
			unsigned child_mask = expand_type(process, mod_base, symbol_context, backing_type.type, indentation + 1);
			type_info->mask |= child_mask;

#ifdef RELOAD_VERBOSE_DEBUGGING
			NameEntry *debug_name_entry = get_name_hash(symbol_context, backing_type.type);
			if (debug_name_entry->type == backing_type.type) {
				TypeInfo *debug_type_info = get_recorded_type_info(symbol_context, debug_name_entry->name_id);
				LOG_INFO("Reloader", "%.*s} // name=%S, mask=%d\n", indentation, INDENTATION, debug_type_info->name, debug_type_info->mask);
			}
		}
		LOG_INFO("Reloader", "%.*s%S // id=0x%016zx, offset=%d, type=%d\n", indentation+1, INDENTATION, member_info.name, member_info.key, member_info.offset, member_info.type.type);
#else
		}
#endif
	}
	return type_info->mask;
}
void expand_pointer(HANDLE process, u64 mod_base, SymbolContext &old_symbol_context, SymbolContext &new_symbol_context, PointerContext &pointer_context, u32 _type, intptr_t addr_old, intptr_t addr_new) {
	// In the array case (which is probably the moste common?), there is probably a lot of stuff here that is redundant..
	NameEntry *name_entry = get_name_hash(new_symbol_context, _type);
	ASSERT(name_entry->type == _type, "Found unknown struct!");

	TypeInfo *type_info = get_recorded_type_info(new_symbol_context, name_entry->name_id);
	ASSERT(type_info->key == name_entry->name_id, "Found unknown struct!");

	if (!(type_info->mask & TypeInfoMask_ContainsPointers)) {
		return;
	}

#ifdef RELOAD_VERBOSE_DEBUGGING
	LOG_INFO("Reloader", "Expanding pointer '%S'\n", type_info->name);
#endif
	
	TypeInfo *old_type_info = get_recorded_type_info(old_symbol_context, name_entry->name_id);

	for (u32 i = 0; i < type_info->member_count; ++i) {
		MemberInfo *new_member_info = type_info->members + i;
		MemberInfo *old_member_info = try_find_member_info(old_type_info->members, old_type_info->member_count, new_member_info->key, i);

		u32 new_offset = new_member_info->offset;
		u32 old_offset = old_member_info ? old_member_info->offset : 0;

		TrimmedType trimmed_type = new_member_info->type;
		ASSERT(trimmed_type.tag != SymTagFunctionType, "Functions are not allowed!");
		switch (trimmed_type.tag) {
			case SymTagUDT: {
				expand_pointer(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, trimmed_type.type, addr_old + old_offset, addr_new + new_offset);
			} break;
			case SymTagPointerType: {
				trimmed_type = get_trimmed_type(new_symbol_context, trimmed_type.type);
				try_fill_pointer(pointer_context, addr_old + old_offset, addr_new + new_offset, trimmed_type.type);
			} break;
			case SymTagArrayType: { // set_pointer_array
				u64 count = get_count(new_symbol_context, trimmed_type.type);

				trimmed_type = get_trimmed_type(new_symbol_context, trimmed_type.type);
				if (trimmed_type.tag == SymTagPointerType) {
					trimmed_type = get_trimmed_type(new_symbol_context, trimmed_type.type);
					for (unsigned j = 0; j < count; ++ j) {
						try_fill_pointer(pointer_context, addr_old + old_offset, addr_new + new_offset, trimmed_type.type);

						old_offset += sizeof(void*); // TODO(kalle): I can't really add to old_offset here unless I _know_ that they are the same size
						new_offset += sizeof(void*);
					}
				} else if (trimmed_type.tag == SymTagUDT) {
					for (unsigned j = 0; j < count; ++ j) {
						expand_pointer(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, trimmed_type.type, addr_old + old_offset, addr_new + new_offset);

						old_offset += sizeof(void*); // TODO(kalle): I can't really add to old_offset here unless I _know_ that they are the same size
						new_offset += sizeof(void*);
					}
				}
			} break;
		}
	}
}

unsigned _patch_type(HANDLE process, u64 mod_base, SymbolContext &old_symbol_context, SymbolContext &new_symbol_context, PointerContext &pointer_context, u32 thing_type, intptr_t addr_old, intptr_t addr_new) {
	NameEntry *name_entry = get_name_hash(new_symbol_context, thing_type);
	ASSERT(name_entry->type == thing_type, "Found unknown struct!");

	TypeInfo *type_info = get_recorded_type_info(new_symbol_context, name_entry->name_id);
	ASSERT(type_info->key == name_entry->name_id, "Found unknown struct!");

	if ((type_info->mask & TypeInfoMask_IsVisited) && !(type_info->mask & TypeInfoMask_IsModified)) {
		move_chunk(pointer_context, addr_old, addr_new, type_info->size, thing_type);
		return type_info->mask;
	}

	check_pointer(pointer_context, addr_old, addr_new, type_info->size, thing_type); // Check pointer to myself
	TypeInfo *old_type_info = get_recorded_type_info(old_symbol_context, name_entry->name_id);

	MemberInfo dummy = {}; // In case we didn't have a given member in the old version, we use this dummy as a placeholder.

	for (u32 i = 0; i < type_info->member_count; ++i) {
		MemberInfo *new_member_info = type_info->members + i;
		MemberInfo *old_member_info = try_find_member_info(old_type_info->members, old_type_info->member_count, new_member_info->key, i);

		// If we have no record of the old member, it is newly added, and this struct is modified!
		if (old_member_info == 0) {
			old_member_info = &dummy;
			type_info->mask |= TypeInfoMask_IsModified;
#ifdef RELOAD_VERBOSE_DEBUGGING
			LOG_INFO("Reloader", "Detected new member. (new=%S)\n", new_member_info->name);
#endif
		}

		u32 new_offset = new_member_info->offset;
		u32 old_offset = old_member_info->offset;

		// If we do have a record, check if the data have changed
		if ((old_member_info->type.type != new_member_info->type.type) || (old_member_info->type.tag != new_member_info->type.tag) || (old_offset != new_offset)) {
			type_info->mask |= TypeInfoMask_IsModified;
		}

#ifdef RELOAD_VERBOSE_DEBUGGING
		if (old_member_info->type.type != new_member_info->type.type) {
			type_info->mask |= TypeInfoMask_IsModified;
			LOG_INFO("Reloader", "Detected missmatch in type (old=%u, new=%u)\n", old_member_info->type.type, new_member_info->type.type);
		} else if (old_member_info->type.tag != new_member_info->type.tag) {
			type_info->mask |= TypeInfoMask_IsModified;
			LOG_INFO("Reloader", "Detected missmatch in tag (old=%u, new=%u)\n", old_member_info->type.tag, new_member_info->type.tag);
		} else if (old_offset != new_offset) {
			type_info->mask |= TypeInfoMask_IsModified;
			LOG_INFO("Reloader", "Detected missmatch in offset (old=%u, new=%u)\n", old_offset, new_offset);
		}
#endif
		
		TrimmedType trimmed_type = new_member_info->type;
		ASSERT(trimmed_type.tag != SymTagFunctionType, "Functions are not allowed!");
		switch (trimmed_type.tag) {
			case SymTagUDT: {
				u32 child_mask = _patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, trimmed_type.type, addr_old + old_offset, addr_new + new_offset);
				type_info->mask |= child_mask;
			} break;
			case SymTagTypedef: {
				ASSERT(false, "Not implemented");
			} break;
			case SymTagPointerType: {
				check_pointer(pointer_context, addr_old + old_offset, addr_new + new_offset, sizeof(void*), trimmed_type.type);
			} break;
			case SymTagArrayType: {
				u64 count = get_count(new_symbol_context, trimmed_type.type);
				u32 size = get_size(new_symbol_context, trimmed_type.type);

				trimmed_type = get_trimmed_type(new_symbol_context, trimmed_type.type);
				if (trimmed_type.tag == SymTagPointerType) {
					trimmed_type = get_trimmed_type(new_symbol_context, trimmed_type.type);
#ifdef RELOAD_VERBOSE_DEBUGGING
					LOG_INFO("Reloader", "Found array of pointers! (count=%zu, type=%u)\n", count, trimmed_type.type);
#endif
					for (unsigned j = 0; j < count; ++ j) {
						try_fill_pointer(pointer_context, addr_old + old_offset, addr_new + new_offset, trimmed_type.type);

						old_offset += sizeof(void*);
						new_offset += sizeof(void*);
					}
				} else if (trimmed_type.tag == SymTagUDT) {
					NameEntry *type_name_entry = get_name_hash(new_symbol_context, trimmed_type.type);
					ASSERT(type_name_entry->type == trimmed_type.type, "Could not find type!");
					TypeInfo *new_child_type_info = get_recorded_type_info(new_symbol_context, type_name_entry->name_id);
					TypeInfo *old_child_type_info = get_recorded_type_info(old_symbol_context, type_name_entry->name_id);

					u32 new_element_size = new_child_type_info->size;
					u32 old_element_size = old_child_type_info->size;
					ASSERT(size / new_element_size == count, "Count does not match size / element size! :o");
					ASSERT(count, "Zero count arrays??");

					int first_entry_set = 0;
					if (!(new_child_type_info->mask & TypeInfoMask_IsVisited)) {
#ifdef RELOAD_VERBOSE_DEBUGGING
						LOG_INFO("Reloader", "Visiting first in array\n");
#endif

						// If I haven't visited the type in the array, visit the first via _patch_type.
						unsigned child_mask = _patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, trimmed_type.type, addr_old + old_offset, addr_new + new_offset);
						first_entry_set = 1;

						TypeInfo *test_new_child_type_info = get_recorded_type_info(new_symbol_context, type_name_entry->name_id);
						ASSERT(test_new_child_type_info == new_child_type_info, "Does this work?");

						type_info->mask |= child_mask;
					}

					ASSERT(new_child_type_info->mask & TypeInfoMask_IsVisited, "The child type needs to have been visited here!");

					if (new_child_type_info->mask & TypeInfoMask_IsModified) {
#ifdef RELOAD_VERBOSE_DEBUGGING
						LOG_INFO("Reloader", "Found array of modified structs! (count=%zu, type=%u)\n", count, trimmed_type.type);
						LOG_WARNING("Reloader", "Performance warning! Array entries have been modifed, move each one! (count=%zu)\n", count);
#endif
						for (unsigned j = first_entry_set; j < count; ++j) {
							_patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, trimmed_type.type, addr_old + old_offset + j * old_element_size, addr_new + new_offset + j * new_element_size);
						}
					} else {
#ifdef RELOAD_VERBOSE_DEBUGGING
						LOG_INFO("Reloader", "Found array of unmodified structs! (count=%zu, type=%u)\n", count, trimmed_type.type);
#endif
						move_chunk(pointer_context, addr_old + old_offset + first_entry_set * old_element_size, addr_new + new_offset + first_entry_set * new_element_size, size - first_entry_set * new_element_size, trimmed_type.type);
					}
				} else if (trimmed_type.tag == SymTagArrayType) {
					ASSERT(false, "Arrays of arrays are not yet implemented!");
				} else if (trimmed_type.tag == SymTagBaseType) {
					move_chunk(pointer_context, addr_old + old_offset, addr_new + new_offset, size, trimmed_type.type);
				} else {
					ASSERT(false, "Unsupported tag '%u'", trimmed_type.tag);
					move_chunk(pointer_context, addr_old + old_offset, addr_new + new_offset, size, trimmed_type.type);
				}
			} break;
			default: {
				u32 size = get_size(new_symbol_context, trimmed_type.type);
				move_chunk(pointer_context, addr_old + old_offset, addr_new + new_offset, size, trimmed_type.type);
			} break;
		}
	}
	type_info->mask |= TypeInfoMask_IsVisited;
	return type_info->mask;
}

SymbolContext _create_symbol_context(HANDLE process, u64 mod_base, SYMBOL_INFOW *entry) {
#ifdef RELOAD_PROFILING
	Stopwatch sw;
	sw.start();
#endif

	SymbolContext symbol_context = setup_symbol_context(process, mod_base);
	expand_type(process, mod_base, symbol_context, entry->TypeIndex);

#ifdef RELOAD_PROFILING
	float time = sw.stop();
	LOG_INFO("Reloader", "Took %g seconds to expand types.\n", time);
#endif

	return symbol_context;
}

void _patch_memory(HANDLE process, u64 mod_base, SYMBOL_INFOW *entry, SymbolContext &old_symbol_context, ReloadHeader &header) {
#ifdef RELOAD_PROFILING
	Stopwatch pointer_sw;
	Stopwatch data_sw;
	Stopwatch total_sw;

	total_sw.start();
	pointer_sw.start();
#endif

	size_t old_memory_size = header.old_memory_size;
	malloc_state *malloc_state_new = (malloc_state *)(header.new_mspace);

	malloc_chunk *current_malloc_chunk_old = (malloc_chunk*)mem2chunk(header.old_mspace);
	malloc_chunk *current_malloc_chunk_new = (malloc_chunk*)mem2chunk(header.new_mspace);

	intptr_t old_memory = (intptr_t)current_malloc_chunk_old;
	intptr_t new_memory = (intptr_t)current_malloc_chunk_new;

	PointerContext pointer_context = setup_pointer_context();
	SymbolContext new_symbol_context = _create_symbol_context(process, mod_base, entry);

	// TODO(kalle): Clone the malloc_state? Should we keep the information about the freed objects?

	{ // Gather pointers
		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

		Pointer current_pointer = make_entry_pointer(base_old, entry);

		bool done = false;
		while (!done) {
			size_t current_chuck_size = estimate_unpadded_request(chunksize(current_malloc_chunk_old));

			expand_pointer(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, current_pointer.type, base_old, base_new);

			NameEntry *name_entry = get_name_hash(new_symbol_context, current_pointer.type);
			TypeInfo *new_type_info = get_recorded_type_info(new_symbol_context, name_entry->name_id);
			size_t new_size = new_type_info->size;

			TypeInfo *old_type_info = get_recorded_type_info(old_symbol_context, name_entry->name_id);
			size_t old_size = old_type_info->key == current_pointer.type ? old_type_info->size : new_size;

			size_t count = safe_ratio_0(current_chuck_size, old_size);

			base_new += new_size * count;
			base_old += old_size * count;

			set_malloc_chunk_head(current_malloc_chunk_old, current_malloc_chunk_new, new_size * count);

			// Go to the next chunk
			current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
			current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

			base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
			base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

			size_t size_from_chunk = estimate_unpadded_request(chunksize(current_malloc_chunk_old));
			if (base_old + size_from_chunk >= old_memory + old_memory_size) {
				intptr_t used_space_new = base_new - new_memory;
				intptr_t used_space_old = base_old - old_memory;
				intptr_t size_diff = used_space_new - used_space_old;
				// If we have used more space in the new state, we have less size left to init the top with
				current_malloc_chunk_new->head = current_malloc_chunk_old->head - size_diff;
				init_top(malloc_state_new, current_malloc_chunk_new, chunksize(current_malloc_chunk_new));
				done = true;
			}
			else {
				bool success = search_for_next_reloadable_pointer(pointer_context, &current_pointer, base_old, old_memory + old_memory_size);
				if (!success) {
#ifdef RELOAD_PROFILING
					LOG_INFO("Reloader", "Could not find another pointer to follow!\n");
#endif
					done = true;
					break;
				}

				// Check to see if this pointed into already mapped memory. If so it's useless to us, and we'll continue to the next one.
				// intptr_t target_addr_old = *(intptr_t*)current_pointer.addr_old;
				*(intptr_t*)(current_pointer.addr_new) = base_new;
			}
		}
	}

#ifdef RELOAD_PROFILING
	LOG_INFO("Reloader", "Took %g seconds to collect pointers.\n", pointer_sw.stop());

	data_sw.start();
#endif

	// Reset state for the next iteration
	current_malloc_chunk_old = (malloc_chunk*)old_memory;
	current_malloc_chunk_new = (malloc_chunk*)new_memory;

	{ // Copy the data
		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

		sort_pointers(pointer_context, base_old, old_memory_size);

		// We require to know about the "entry point", i.e. the layout of where the input memory is pointing.
		Pointer current_pointer = make_entry_pointer(base_old, entry);

		bool done = false;
		while (!done) {
			size_t current_chuck_size = estimate_unpadded_request(chunksize(current_malloc_chunk_old));

			NameEntry *name_entry = get_name_hash(new_symbol_context, current_pointer.type);
			TypeInfo *old_type_info = get_recorded_type_info(old_symbol_context, name_entry->name_id);
			TypeInfo *new_type_info = get_recorded_type_info(new_symbol_context, name_entry->name_id);

			size_t old_size = old_type_info->size;
			size_t new_size = new_type_info->size;

			size_t count = safe_ratio_0(current_chuck_size, old_size);
			for (unsigned i = 0; i < count; i++) {
				_patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, current_pointer.type, base_old, base_new);
				base_new += new_size;
				base_old += old_size;
			}
			// We have reached the end of a new:ed struct. We need to look at the next newed portion, i.e. the next malloc chunk to find out where to go next.

			// Go to the next chunk
			current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
			current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

			base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
			base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

			size_t size_from_chunk = estimate_unpadded_request(chunksize(current_malloc_chunk_old));
			if (base_old + size_from_chunk >= old_memory + old_memory_size) {
				done = true;
			} else {
				get_next_reloadable_pointer(pointer_context, &current_pointer);
			}
		}
	}

	free(pointer_context.memory);

#ifdef RELOAD_PROFILING
	LOG_INFO("Reloader", "Took %g seconds to move data.\n", data_sw.stop());
	LOG_INFO("Reloader", "Took %g seconds to patch memory.\n", total_sw.stop());
#endif
}

void reloader_unload_symbols(WCHAR *entry_point_name, MODULEINFO &module_info, SymbolContext &symbol_context) {
	HANDLE process = GetCurrentProcess();

	SymbolInfoPackage symbol_info_package;
	BOOL result = SymGetTypeFromNameW(process, (DWORD64)module_info.lpBaseOfDll, entry_point_name, &symbol_info_package.si);

	if (result) {
		SYMBOL_INFOW *symbol_info = &symbol_info_package.si;
		symbol_context = _create_symbol_context(process, symbol_info->ModBase, symbol_info);
	}
}
void reloader_load_symbols(WCHAR *entry_point_name, MODULEINFO &module_info, SymbolContext &symbol_context, ReloadHeader &reload_header) {
	HANDLE process = GetCurrentProcess();

	SymbolInfoPackage symbol_info_package;
	BOOL result = SymGetTypeFromNameW(process, (DWORD64)module_info.lpBaseOfDll, entry_point_name, &symbol_info_package.si);

	if (result) {
		SYMBOL_INFOW *symbol_info = &symbol_info_package.si;
		_patch_memory(process, symbol_info->ModBase, symbol_info, symbol_context, reload_header);
	}
}
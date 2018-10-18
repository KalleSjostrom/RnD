#include "reloader.h"
#include "core/utils/stopwatch.h"

#define RELOAD_PROFILING 1

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
	Pointer *pointer_array = pointer_context.pointer_array;
	for (uint32_t i = pointer_context.current_pointer_index; i < array_count(pointer_array); ++i) {
		Pointer &pointer = pointer_array[i];

		if (pointer.target_addr_old < lowest_target_address && pointer.target_addr_old >= min_address && pointer.target_addr_old < max_address) {
			lowest_target_address = pointer.target_addr_old;
			best_pointer_index = (int)i;
		}
	}

	if (best_pointer_index >= 0) {
		*out_pointer = pointer_array[best_pointer_index];

		Pointer temp = pointer_array[pointer_context.current_pointer_index];
		pointer_array[pointer_context.current_pointer_index] = *out_pointer;
		pointer_array[best_pointer_index] = temp;

		pointer_context.current_pointer_index++;
		return true;
	}

	return false;
}
void sort_pointers(PointerContext &pointer_context, intptr_t base_old, size_t size) {
	pointer_context.current_pointer_index = 0;

	Pointer *pointer_array = pointer_context.pointer_array;
	int count = 0;
	for (uint32_t i = 0; i < array_count(pointer_array); ++i) {
		Pointer &p = pointer_array[i];
		intptr_t offset = p.target_addr_old - base_old;
		if (offset >= 0 && offset < (int)size) { // Pointing into our chunk
			pointer_context.sorted_pointer_indices[count].value = (unsigned)offset;
			pointer_context.sorted_pointer_indices[count].index = i;
			count++;
		} else { // I'm pointing to outside the range, just keep whatever I was pointing at
			// *(intptr_t*)(p.addr_new) = p.target_addr_new;
			// ASSERT(*(intptr_t*)(p.addr_new) == p.target_addr_new, "I'm pointing to outside the range, the default should be equal!");
		}
	}
	quick_sort(pointer_context.sorted_pointer_indices, count);
	pointer_context.sorted_pointer_count = count;
}

void _check_pointer_self(PointerContext &context, intptr_t chunk_start_old, intptr_t chunk_start_new, uint32_t chunk_type) {
	// This chunk is of a specific type that is being set (and have possibly been modified), we only need to check pointers equal to chunk_start_old.
	int index_offset = 0;
	while (context.current_pointer_index + index_offset < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index + index_offset];
		Pointer &pointer = context.pointer_array[element.index];
		ASSERT(pointer.target_addr_old != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_addr_old == chunk_start_old) {
			if (pointer.type == chunk_type) { // Also make sure I'm pointing to the correct type (and not the first member of a struct)
				pointer.target_addr_new = chunk_start_new;
				ASSERT(pointer.addr_new, "We need to store this for later!");

				// *(intptr_t*)(pointer.addr_new) = pointer.target_addr_new;

				if (index_offset > 0) { // We have some skipped pointers
					// Swap the current element with the first skipped pointer.
					SortElement temp = context.sorted_pointer_indices[context.current_pointer_index];
					context.sorted_pointer_indices[context.current_pointer_index] = element;
					context.sorted_pointer_indices[context.current_pointer_index + index_offset] = temp;
				}

				context.current_pointer_index++;
			} else {
				// I'm pointing to context inside a possibly modified chunk, it might have moved so this pointer needs to wait
				index_offset++; // Check the next one
			}
		} else {
			break;
		}
	}
}

void _check_pointer_fixed(PointerContext &context, intptr_t chunk_start_old, intptr_t chunk_start_new, size_t chunk_size) {
	// This chunk is a fixed block of memory (unmodifed), therefore we need to check all the pointers in the address range ([chunk_start_old, chunk_start_old + chunk_size])
	while (context.current_pointer_index < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index];
		Pointer &pointer = context.pointer_array[element.index];
		ASSERT(pointer.target_addr_old != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_addr_old < (chunk_start_old + (ptrdiff_t)chunk_size)) { // Am I pointing to moved memory?
			// I'm pointing into an unmodified chunk. Find out how far in the pointer pointed to, this delta is the same in the old and new space!
			pointer.target_addr_new = chunk_start_new + (pointer.target_addr_old - chunk_start_old);
			ASSERT(pointer.addr_new, "We need to store this for later!");

			// *(intptr_t*)(pointer.addr_new) = pointer.target_addr_new;
			context.current_pointer_index++;
		} else {
			break;
		}
	}
}

void move_chunk(PointerContext &pointer_context, intptr_t chunk_start_old, intptr_t chunk_start_new, size_t chunk_size) {
	memmove((void*)chunk_start_new, (void*)chunk_start_old, chunk_size);
	_check_pointer_fixed(pointer_context, chunk_start_old, chunk_start_new, chunk_size);
}

bool get_next_reloadable_pointer(PointerContext &pointer_context, Pointer *out_pointer) {
	if (pointer_context.current_pointer_index >= pointer_context.sorted_pointer_count)
		return false;

	SortElement &element = pointer_context.sorted_pointer_indices[pointer_context.current_pointer_index];
	*out_pointer = pointer_context.pointer_array[element.index];
	return true;
}

void try_fill_pointer(PointerContext &pointer_context, intptr_t addr_old, intptr_t addr_new, uint32_t type) {
	intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");
	if (target_addr_old != 0) { // We don't care about pointers pointing to null!
		ASSERT(pointer_context.array_count(pointer_array) < MAX_POINTERS, "Array index out of bounds!");
		Pointer &p = pointer_context.pointer_array[pointer_context.array_count(pointer_array)++];

		p.addr_old = addr_old;
		p.addr_new = addr_new;
		p.target_addr_old = target_addr_old;
		p.target_addr_new = target_addr_old; // By default, point to wherever it was pointing to
		p.type = type;
	}
}

uint32_t expand_type(SymbolContext &symbol_context, uint32_t type_index) {
	TypeName *type_name = get_type_name(symbol_context, type_index);

	FullType *type_info = get_recorded_type_info(symbol_context, type_name->id);
	if (type_info->key == type_name->id) {
		return type_info->mask;
	}

	type_info->key = type_name->id;
	type_info->type_index = type_index;
	type_info->mask = 0;

	Children *children = get_children(symbol_context, type_index);
	for (uint32_t i = 0; i < children->count; ++i) {
		Type &child = children->types[i];

		CV_typ_t backing_type = get_child_type(symbol_context, child);
		if (backing_type == 0)
			continue;

		SymTag backing_tag = get_tag(symbol_context, backing_type);
		if (backing_tag == SymTagNull)
			continue;

		if (backing_tag == SymTagPointerType) {
			type_info->mask |= TypeInfoMask_ContainsPointers;
		}

		while (backing_tag == SymTagPointerType || backing_tag == SymTagArrayType) {
			backing_type = get_type(symbol_context, backing_type);
			backing_tag = get_tag(symbol_context, backing_type);

			if (backing_tag == SymTagPointerType) {
				type_info->mask |= TypeInfoMask_ContainsPointers;
			}
		}

		if (backing_tag == SymTagUDT) {
			unsigned child_mask = expand_type(symbol_context, backing_type);
			type_info->mask |= child_mask;
		}
	}
	return type_info->mask;
}

void expand_pointer(SymbolContext &old_symbol_context, SymbolContext &new_symbol_context, PointerContext &pointer_context, CV_typ_t type_index, intptr_t addr_old, intptr_t addr_new, TypeName *type_name = 0, FullType *type_info = 0, FullType *old_type_info = 0) {
	type_name = type_name ? type_name : get_type_name(new_symbol_context, type_index);
	ASSERT(type_name->type == type_index, "Found unknown struct!");

	type_info = type_info ? type_info : get_recorded_type_info(new_symbol_context, type_name->id);
	ASSERT(type_info->key == type_name->id, "Found unknown struct!");

	if (!(type_info->mask & TypeInfoMask_ContainsPointers)) {
		return;
	}

	old_type_info = old_type_info ? old_type_info : get_recorded_type_info(old_symbol_context, type_name->id);
	Children *children = get_children(symbol_context, type_index);
	for (uint32_t i = 0; i < children->count; ++i) {
		Type &child = children->types[i];

		TypeTagPair type_pair = get_type_pair(symbol_context, child);
		if (type_pair.tag == SymTagNull)
			continue; // Early out if we have no valid type info

		if (type_pair.tag != SymTagUDT && type_pair.tag != SymTagArrayType && type_pair.tag != SymTagPointerType) {
			continue; // Early out if it's not something that can contain a pointer!
		}

		MemberInfo *old_member_info = try_find_member_info(old_type_info->members, old_type_info->member_count, new_member_info->key, i);

		uint32_t new_offset = new_member_info->offset;
		uint32_t old_offset = old_member_info ? old_member_info->offset : 0;

		switch (type_pair.tag) {
			case SymTagUDT: {
				expand_pointer(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_pair.type_index, addr_old + old_offset, addr_new + new_offset);
			} break;
			case SymTagPointerType: {
				type_pair = get_type_pair(new_symbol_context, type_pair.type_index);
				try_fill_pointer(pointer_context, addr_old + old_offset, addr_new + new_offset, type_pair.type_index);
			} break;
			case SymTagArrayType: { // set_pointer_array
				uint32_t size = get_size(new_symbol_context, type_pair.type_index);

				type_pair = get_type_pair(new_symbol_context, type_pair.type_index);
				if (type_pair.tag == SymTagPointerType) {
					type_pair = get_type_pair(new_symbol_context, type_pair.type_index);
					uint32_t count = size / sizeof(void*);
					for (unsigned j = 0; j < count; ++ j) {
						try_fill_pointer(pointer_context, addr_old + old_offset, addr_new + new_offset, type_pair.type_index);

						old_offset += sizeof(void*); // TODO(kalle): I can't really add to old_offset here unless I _know_ that they are the same size
						new_offset += sizeof(void*);
					}
				} else if (type_pair.tag == SymTagUDT) {
					// In the array case (which is probably the most common?), there is probably a lot of stuff here that is redundant..
					TypeName *_type_name = get_type_name(new_symbol_context, type_pair.type_index);
					ASSERT(_type_name->type == type_pair.type_index, "Found unknown struct!");

					FullType *_type_info = get_recorded_type_info(new_symbol_context, _type_name->name_id);
					ASSERT(_type_info->key == _type_name->name_id, "Found unknown struct!");

					if (_type_info->mask & TypeInfoMask_ContainsPointers) {
						FullType *_old_type_info = get_recorded_type_info(old_symbol_context, _type_name->name_id);
						uint32_t count = size / _type_info->size;
						for (unsigned j = 0; j < count; ++ j) {
							expand_pointer(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_pair.type_index, addr_old + old_offset, addr_new + new_offset, _type_name, _type_info, _old_type_info);

							old_offset += _old_type_info->size;
							new_offset += _type_info->size;
						}
					}
				}
			} break;
		}
	}
}

unsigned _patch_type(SymbolContext &old_symbol_context, SymbolContext &new_symbol_context, PointerContext &pointer_context, uint32_t thing_type, intptr_t addr_old, intptr_t addr_new);

void _patch_array(SymbolContext &old_symbol_context, SymbolContext &new_symbol_context, PointerContext &pointer_context, FullType *type_info, uint32_t thing_type, intptr_t addr_old, intptr_t addr_new) {
	uint32_t size = get_size(new_symbol_context, thing_type);

	uint32_t old_offset = 0;
	uint32_t new_offset = 0;

	TrimmedType type_pair = get_type_pair(new_symbol_context, thing_type);
	if (type_pair.tag == SymTagUDT) {
		TypeName *type_type_name = get_type_name(new_symbol_context, type_pair.type_index);
		ASSERT(type_type_name->type == type_pair.type_index, "Could not find type!");
		FullType *new_child_type_info = get_recorded_type_info(new_symbol_context, type_type_name->name_id);
		FullType *old_child_type_info = get_recorded_type_info(old_symbol_context, type_type_name->name_id);

		uint32_t new_element_size = new_child_type_info->size;
		uint32_t old_element_size = old_child_type_info->size;
		uint32_t count = size / new_element_size;
		ASSERT(count, "Zero count arrays??");

		int first_entry_set = 0;
		if (!(new_child_type_info->mask & TypeInfoMask_IsVisited)) {
#if defined(RELOAD_VERBOSE_DEBUGGING)
			log_warning("Reloader", "Visiting first in array");
#endif

			// If I haven't visited the type in the array, visit the first via _patch_type.
			unsigned child_mask = _patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_pair.type_index, addr_old + old_offset, addr_new + new_offset);
			first_entry_set = 1;

			FullType *test_new_child_type_info = get_recorded_type_info(new_symbol_context, type_type_name->name_id);
			ASSERT(test_new_child_type_info == new_child_type_info, "Does this work?");

			type_info->mask |= child_mask;
		}

		ASSERT(new_child_type_info->mask & TypeInfoMask_IsVisited, "The child type needs to have been visited here!");

		if (new_child_type_info->mask & TypeInfoMask_IsModified) {
#if defined(RELOAD_VERBOSE_DEBUGGING)
			log_warning("Reloader", "Found array of modified structs! (count=%zu, type=%u)", count, type_pair.type_index);
			log_warning("Reloader", "Performance warning! Array entries have been modifed, move each one! (count=%zu)", count);
#endif
			for (unsigned j = first_entry_set; j < count; ++j) {
				_patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_pair.type_index, addr_old + old_offset + j * old_element_size, addr_new + new_offset + j * new_element_size);
			}
		}
		else {
#if defined(RELOAD_VERBOSE_DEBUGGING)
			log_warning("Reloader", "Found array of unmodified structs! (count=%zu, type=%u)", count, type_pair.type_index);
#endif
			move_chunk(pointer_context, addr_old + old_offset + first_entry_set * old_element_size, addr_new + new_offset + first_entry_set * new_element_size, size - first_entry_set * new_element_size);
		}
	}
	else if (type_pair.tag == SymTagArrayType) {
		// ASSERT(false, "Arrays of arrays are not yet implemented!");
		_patch_array(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_info, type_pair.type_index, addr_old + old_offset, addr_new + new_offset);
	}
	else if (type_pair.tag == SymTagBaseType || type_pair.tag == SymTagEnum || type_pair.tag == SymTagPointerType) {
		move_chunk(pointer_context, addr_old + old_offset, addr_new + new_offset, size);
	}
	else {
		ASSERT(false, "Unsupported tag '%u'", type_pair.tag);
		move_chunk(pointer_context, addr_old + old_offset, addr_new + new_offset, size);
	}
}

unsigned _patch_type(HANDLE process, uint64_t mod_base, SymbolContext &old_symbol_context, SymbolContext &new_symbol_context, PointerContext &pointer_context, uint32_t thing_type, intptr_t addr_old, intptr_t addr_new) {
	TypeName *type_name = get_type_name(new_symbol_context, thing_type);
	ASSERT(type_name->type == thing_type, "Found unknown struct!");

	FullType *type_info = get_recorded_type_info(new_symbol_context, type_name->name_id);
	ASSERT(type_info->key == type_name->name_id, "Found unknown struct!");

	// In order to move this as a chunk, it cannot contain pointers, it cannot be modified and we need to have visited it so that we are sure we know these two pieces of info.
	if ((type_info->mask & TypeInfoMask_IsVisited) && !(type_info->mask & TypeInfoMask_IsModified)) {
		move_chunk(pointer_context, addr_old, addr_new, type_info->size);
		return type_info->mask;
	}

	_check_pointer_self(pointer_context, addr_old, addr_new, thing_type); // Check pointer to myself
	FullType *old_type_info = get_recorded_type_info(old_symbol_context, type_name->name_id);

	MemberInfo dummy = {}; // In case we didn't have a given member in the old version, we use this dummy as a placeholder.

	for (uint32_t i = 0; i < type_info->member_count; ++i) {
		MemberInfo *new_member_info = type_info->members + i;
		MemberInfo *old_member_info = try_find_member_info(old_type_info->members, old_type_info->member_count, new_member_info->key, i);

		// If we have no record of the old member, it is newly added, and this struct is modified!
		if (old_member_info == 0) {
			old_member_info = &dummy;
			type_info->mask |= TypeInfoMask_IsModified;
#if defined(RELOAD_VERBOSE_DEBUGGING)
			log_warning("Reloader", "Detected new member. (new=%S)", new_member_info->name);
#endif
		}

		uint32_t new_offset = new_member_info->offset;
		uint32_t old_offset = old_member_info->offset;

		// If we do have a record, check if the data have changed
		if ((old_member_info->type.tag != new_member_info->type.tag) || (old_offset != new_offset)) {
			type_info->mask |= TypeInfoMask_IsModified;
		}

#if defined(RELOAD_VERBOSE_DEBUGGING)
		if (old_member_info->type.tag != new_member_info->type.tag) {
			type_info->mask |= TypeInfoMask_IsModified;
			log_warning("Reloader", "Detected missmatch in tag (old=%u, new=%u)", old_member_info->type.tag, new_member_info->type.tag);
		} else if (old_offset != new_offset) {
			type_info->mask |= TypeInfoMask_IsModified;
			log_warning("Reloader", "Detected missmatch in offset (old=%u, new=%u)", old_offset, new_offset);
		}
#endif

		TrimmedType type_pair = new_member_info->type;
		ASSERT(type_pair.tag != SymTagFunctionType, "Functions are not allowed!");
		switch (type_pair.tag) {
			case SymTagUDT: {
				uint32_t child_mask = _patch_type(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_pair.type_index, addr_old + old_offset, addr_new + new_offset);
				type_info->mask |= child_mask;
			} break;
			case SymTagTypedef: {
				ASSERT(false, "Not implemented");
			} break;
			case SymTagPointerType: {
				_check_pointer_fixed(pointer_context, addr_old + old_offset, addr_new + new_offset, sizeof(void*));
			} break;
			case SymTagArrayType: {
				_patch_array(process, mod_base, old_symbol_context, new_symbol_context, pointer_context, type_info, type_pair.type_index, addr_old + old_offset, addr_new + new_offset);
			} break;
			default: {
				uint32_t size = get_size(new_symbol_context, type_pair.type_index);
				move_chunk(pointer_context, addr_old + old_offset, addr_new + new_offset, size);
			} break;
		}
	}
	type_info->mask |= TypeInfoMask_IsVisited;
	return type_info->mask;
}

SymbolContext _create_symbol_context(Allocator &a, HANDLE process, uint64_t mod_base, SYMBOL_INFOW *entry) {
#if defined(RELOAD_PROFILING)
	Stopwatch sw;
	sw.start();
#endif

	SymbolContext symbol_context = setup_symbol_context(process, mod_base);
	expand_type(process, mod_base, a, symbol_context, entry->TypeIndex);

#if defined(RELOAD_PROFILING)
	float time = sw.stop();
	log_warning("Reloader", "Took %g seconds to expand types.", time);
#endif

	return symbol_context;
}

void _patch_memory(Allocator &a, HANDLE process, uint64_t mod_base, SYMBOL_INFOW *entry, SymbolContext &old_symbol_context, ReloadHeader &header) {
#if defined(RELOAD_PROFILING)
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
	SymbolContext new_symbol_context = _create_symbol_context(a, process, mod_base, entry);

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

			TypeName *type_name = get_type_name(new_symbol_context, current_pointer.type);
			FullType *new_type_info = get_recorded_type_info(new_symbol_context, type_name->name_id);
			size_t new_size = new_type_info->size;

			FullType *old_type_info = get_recorded_type_info(old_symbol_context, type_name->name_id);
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
#if defined(RELOAD_PROFILING)
					log_warning("Reloader", "Could not find another pointer to follow!");
#endif
					done = true;
					break;
				}
			}
		}
	}

#if defined(RELOAD_PROFILING)
	log_warning("Reloader", "Took %g seconds to collect pointers.", pointer_sw.stop());

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

			TypeName *type_name = get_type_name(new_symbol_context, current_pointer.type);
			FullType *old_type_info = get_recorded_type_info(old_symbol_context, type_name->name_id);
			FullType *new_type_info = get_recorded_type_info(new_symbol_context, type_name->name_id);

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

	{ // Update all pointers!
		Pointer *pointer_array = pointer_context.pointer_array;
		for (unsigned i = 0; i < array_count(pointer_array); ++i) {
			Pointer &p = pointer_array[i];
			*(intptr_t*)(p.addr_new) = p.target_addr_new;

			bool in_old_space = p.target_addr_new >= old_memory && p.target_addr_new < (intptr_t)(old_memory + old_memory_size);
			ASSERT(!in_old_space, "Pointer in new memory chunk still points to old memory!");
		}
	}

	free(pointer_context.memory);

#if defined(RELOAD_PROFILING)
	log_warning("Reloader", "Took %g seconds to move data.", data_sw.stop());
	log_warning("Reloader", "Took %g seconds to patch memory.", total_sw.stop());
#endif
}

void reloader_unload_symbols(Allocator &a, WCHAR *entry_point_name, MODULEINFO &module_info, SymbolContext &symbol_context) {
	HANDLE process = GetCurrentProcess();

	SymbolInfoPackage symbol_info_package;
	BOOL result = SymGetTypeFromNameW(process, (DWORD64)module_info.lpBaseOfDll, entry_point_name, &symbol_info_package.si);

	if (result) {
		SYMBOL_INFOW *symbol_info = &symbol_info_package.si;
		symbol_context = _create_symbol_context(a, process, symbol_info->ModBase, symbol_info);
	}
}
void reloader_load_symbols(Allocator &a, WCHAR *entry_point_name, MODULEINFO &module_info, SymbolContext &symbol_context, ReloadHeader &reload_header) {
	HANDLE process = GetCurrentProcess();

	SymbolInfoPackage symbol_info_package;
	BOOL result = SymGetTypeFromNameW(process, (DWORD64)module_info.lpBaseOfDll, entry_point_name, &symbol_info_package.si);

	if (result) {
		SYMBOL_INFOW *symbol_info = &symbol_info_package.si;
		_patch_memory(a, process, symbol_info->ModBase, symbol_info, symbol_context, reload_header);
	}
}
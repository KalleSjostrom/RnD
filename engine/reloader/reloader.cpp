////////////////////////////////////////////////////////////
///////////////////////// POINTERS /////////////////////////
////////////////////////////////////////////////////////////
struct Pointer {
	AddressPair address;
	AddressPair target_address;
	CV_typ_t type_index;
};
inline Pointer make_pointer(intptr_t addr_old, CV_typ_t type_index) {
	Pointer p = {};

	p.address.o = addr_old;
	p.address.n = 0;
	p.target_address.o = *(intptr_t*)(addr_old);
	p.target_address.n = 0;
	p.type_index = type_index;

	return p;
}

struct PointerContext {
	void *memory;

	Pointer *pointer_array;
	SortElement *sorted_pointer_indices;

	unsigned current_pointer_index;
	unsigned sorted_pointer_count;
};
PointerContext setup_pointer_context(Allocator &allocator) {
	PointerContext pointer_context = {};

	// Set up the address lookup.
	array_make(&allocator, pointer_context.pointer_array, (1 << 16));
	array_make(&allocator, pointer_context.sorted_pointer_indices, (1 << 16))

	return pointer_context;
}

/**
 * Goes through the pointers in the given context and searches for the one with the lowest target address (in the old range).
 * This is because we want to handle the memory sequentially from start of the memory block to the end.
 */
bool search_for_next_reloadable_pointer(PointerContext &pointer_context, Pointer *out_pointer, AllocatorBackendHeader *backend) {
	// The cursor should always be on the pointer that has the lowest target address
	intptr_t lowest_target_address = INTPTR_MAX;
	int best_pointer_index = -1;
	Pointer *pointer_array = pointer_context.pointer_array;
	for (int32_t i = pointer_context.current_pointer_index; i < array_count(pointer_array); ++i) {
		Pointer &pointer = pointer_array[i];

		if (pointer.target_address.o < lowest_target_address && is_address_in_old_range(backend, pointer.target_address.o)) {
			lowest_target_address = pointer.target_address.o;
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

/**
 * Sort the pointers from lowest target address to highest.
 * This is to make sure we always look at the most relevant pointers first.
 */
void sort_pointers(PointerContext &pointer_context, intptr_t base_old, size_t size) {
	pointer_context.current_pointer_index = 0;

	Pointer *pointer_array = pointer_context.pointer_array;
	int count = 0;
	for (int32_t i = 0; i < array_count(pointer_array); ++i) {
		Pointer &p = pointer_array[i];
		intptr_t offset = p.target_address.o - base_old;
		if (offset >= 0 && offset < (int)size) { // Pointing into our chunk
			pointer_context.sorted_pointer_indices[count].value = (uint32_t)offset;
			pointer_context.sorted_pointer_indices[count].index = i;
			count++;
		} else { // I'm pointing to outside the range, just keep whatever I was pointing at
			// *(intptr_t*)(p.address.n) = p.target_address.n;
			// ASSERT(*(intptr_t*)(p.address.n) == p.target_address.n, "I'm pointing to outside the range, the default should be equal!");
		}
	}
	quick_sort(pointer_context.sorted_pointer_indices, count);
	pointer_context.sorted_pointer_count = count;
}

void check_pointer_self(PointerContext &context, AddressPair chunk, uint32_t chunk_type) {
	// This chunk is of a specific type that is being set (and have possibly been modified), we only need to check pointers equal to chunk.o.
	int index_offset = 0;
	while (context.current_pointer_index + index_offset < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index + index_offset];
		Pointer &pointer = context.pointer_array[element.index];
		ASSERT(pointer.target_address.o != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_address.o == chunk.o) {
			if (pointer.type_index == chunk_type) { // Also make sure I'm pointing to the correct type (and not the first member of a struct)
				pointer.target_address.n = chunk.n;
				ASSERT(pointer.address.n, "We need to store this for later!");

				// *(intptr_t*)(pointer.address.n) = pointer.target_address.n;

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

void check_pointer_fixed(PointerContext &context, AddressPair chunk, size_t chunk_size) {
	// This chunk is a fixed block of memory (unmodifed), therefore we need to check all the pointers in the address range ([chunk.o, chunk.o + chunk_size])
	while (context.current_pointer_index < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index];
		Pointer &pointer = context.pointer_array[element.index];
		ASSERT(pointer.target_address.o != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_address.o < (chunk.o + (ptrdiff_t)chunk_size)) { // Am I pointing to moved memory?
			// I'm pointing into an unmodified chunk. Find out how far in the pointer pointed to, this delta is the same in the old and new space!
			pointer.target_address.n = chunk.n + (pointer.target_address.o - chunk.o);
			ASSERT(pointer.address.n, "We need to store this for later!");

			// *(intptr_t*)(pointer.address.n) = pointer.target_address.n;
			context.current_pointer_index++;
		} else {
			break;
		}
	}
}

void try_add_pointer(PointerContext &pointer_context, AddressPair address, CV_typ_t type_index) {
	AddressPair target_address = {};
	target_address.o = *(intptr_t*)(address.o); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");
	if (target_address.o != 0) { // We don't care about pointers pointing to null!
		array_expand(pointer_context.pointer_array, 1);
		Pointer &p = array_peek(pointer_context.pointer_array);

		p.address = address;
		p.target_address.o = target_address.o;
		p.target_address.n = target_address.o; // By default, point to wherever it was pointing to
		p.type_index = type_index;
	}
}

void move_chunk(PointerContext &pointer_context, AddressPair chunk, size_t chunk_size) {
	memmove((void*)chunk.n, (void*)chunk.o, chunk_size);
	check_pointer_fixed(pointer_context, chunk, chunk_size);
}

int check_if_new_member(TypeContextPair *type_context, MemberListPair *member_list, TypeRecord *type_record, AddressPair address, uint32_t index, OffsetPair *out_offset) {
	Type *new_child = member_list->n->types + index;

	TypeName *child_type_name = member_list->n->names + index;
	Type *old_child = try_find_member(member_list->o, child_type_name);

	out_offset->n = get_offset(new_child);
	out_offset->o = get_offset(old_child);

	TypeTag new_typetag = get_child_type_tag(type_context->n, new_child);

	int is_new_variable = old_child == 0;
	if (!is_new_variable) {
		TypeTag old_typetag = get_child_type_tag(type_context->o, old_child);

		while (new_typetag.tag == SymTagArrayType && old_typetag.tag == SymTagArrayType) {
			new_typetag = get_type_tag(type_context->n, new_typetag.type_index);
			old_typetag = get_type_tag(type_context->o, old_typetag.type_index);
		}

		// If the tags are different, the variable is new
		is_new_variable = old_typetag.tag != new_typetag.tag;
		if (!is_new_variable) {
			// NOTE(kalle):
			// If this variable have changed its type, we treat it as a new varible.
			// Not sure what would be the best approach here... If we change from int a to int *a, it feels strange to copy the value of a..
			if (new_typetag.tag == SymTagBaseType) { // If we are dealing with a base type, then it is new if the type_index has changed
				is_new_variable = new_typetag.type_index != old_typetag.type_index;
			} else { // If it's _not_ a base type, meaning it's a user defined thing, we should have a record of it, check if the type name has changed
				TypeName *new_type_name = get_type_name(type_context->n, new_typetag.type_index);
				TypeName *old_type_name = get_type_name(type_context->o, old_typetag.type_index);

				is_new_variable = old_type_name == 0 || new_type_name->id != old_type_name->id;
			}
		}

		if (out_offset->n != out_offset->o) {
			type_record->flags |= TypeRecordFlags_IsModified;
		}
	}

	// If we are dealing with a new variable, then just clear the memory to 0!
	if (is_new_variable) {
		type_record->flags |= TypeRecordFlags_IsModified;
		uint32_t size = get_size(type_context->n, new_typetag.type_index);
		memset((void*)(address.n + out_offset->n), 0, size);
		member_list->n->flags[index] |= TypeRecordFlags_IsNew;
	}

	return is_new_variable;
}


////////////////////////////////////////////////////////////
///////////////////// GATHER POINTERS  /////////////////////
////////////////////////////////////////////////////////////
void gather_pointers(TypeContextPair *type_context, PointerContext &pointer_context, CV_typ_t type_index, AddressPair address, TypeRecordPair *existing_type_record = 0) {
	TypeRecordPair type_record;
	if (existing_type_record) {
		type_record = *existing_type_record;
	} else {
		TypeName *type_name = get_type_name(type_context->n, type_index);
		type_record = get_type_record_pair(type_context, type_name);
	}

	if (!(type_record.n->flags & TypeRecordFlags_ContainsPointers)) {
		return;
	}

	MemberListPair member_lists = get_member_list_pair(type_context, &type_record);
	for (int i = 0; i < array_count(member_lists.n->types); ++i) {
		OffsetPair offset;
		int is_new_member = check_if_new_member(type_context, &member_lists, type_record.n, address, i, &offset);
		if (is_new_member) { continue; }

		Type *new_child = member_lists.n->types + i;
		TypeTag typetag = get_child_type_tag(type_context->n, new_child);

		bool base_type_pointer = typetag.tag == SymTagBaseType && CV_TYP_IS_PTR(typetag.type_index);
		if (!base_type_pointer && typetag.tag != SymTagUDT && typetag.tag != SymTagArrayType && typetag.tag != SymTagPointerType) {
			continue; // Early out if it's not something that can contain a pointer!
		}

		switch (typetag.tag) {
			case SymTagUDT: {
				gather_pointers(type_context, pointer_context, typetag.type_index, address + offset);
			} break;
			case SymTagBaseType: {
				ASSERT(base_type_pointer, "Not a pointer!");
				CV_typ_t base_type_index = get_base_pointer_type(typetag.type_index);
				try_add_pointer(pointer_context, address + offset, base_type_index);
			} break;
			case SymTagPointerType: {
				typetag = get_type_tag(type_context->n, typetag.type_index);
#if 0
				if (typetag.tag == SymTagFunctionType) {
					TypeName *function_name = get_type_name(type_context->n, typetag.type_index);

					intptr_t target_address.n = get_symbol_address(function_name);
					intptr_t target_address.o = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");

					array_expand(pointer_context.pointer_array, 1);
					Pointer &p = array_peek(pointer_context.pointer_array);
					p.addr_old = addr_old + old_offset;
					p.address.n = address.n + new_offset;
					p.target_address.o = target_address.o;
					p.target_address.n = target_address.n;
					p.type_index = typetag.type_index;
				} else {
#endif
					try_add_pointer(pointer_context, address + offset, typetag.type_index);
			} break;
			case SymTagArrayType: { // set_pointer_array
				uint32_t array_size = get_size(type_context->n, typetag.type_index);
				while (typetag.tag == SymTagArrayType) {
					typetag = get_type_tag(type_context->n, typetag.type_index);
				}

				if (typetag.tag == SymTagUDT) {
					// In the array case (which is probably the most common?), there is probably a lot of stuff here that is redundant..
					TypeName *_type_name = get_type_name(type_context->n, typetag.type_index);

					TypeRecordPair _type_record = get_type_record_pair(type_context, _type_name);

					if (_type_record.n->flags & TypeRecordFlags_ContainsPointers) {
						OffsetPair element_size = get_size_pair(type_context, &_type_record);
						size_t count = array_size / element_size.n;
						for (size_t j = 0; j < count; ++ j) {
							gather_pointers(type_context, pointer_context, typetag.type_index, address + offset, &_type_record);
							offset += element_size;
						}
					}
				} else {
					int is_pointer = (typetag.tag == SymTagBaseType && CV_TYP_IS_PTR(typetag.type_index)) || (typetag.tag == SymTagPointerType);
					if (is_pointer) {
						uint32_t element_size = get_size(type_context->n, typetag.type_index);
						uint32_t count = array_size / element_size;
						for (uint32_t j = 0; j < count; ++ j) {
							try_add_pointer(pointer_context, address + offset, typetag.type_index);

							offset.o += element_size;
							offset.n += element_size;
						}
					}
				}
			} break;
		}
	}
}

////////////////////////////////////////////////////////////
///////////////////// GATHER TYPE INFO /////////////////////
////////////////////////////////////////////////////////////
uint32_t gather_type_information(TypeContext *type_context, CV_typ_t type_index) {
	TypeName *type_name = get_type_name(type_context, type_index);

	TypeRecord *type_record = get_type_record(type_context, type_name->id);
	if (type_record->flags & TypeRecordFlags_CheckedForPointers) {
		return type_record->flags;
	}

	MemberList *member_list = get_member_list(type_context, type_record);
	for (int i = 0; i < array_count(member_list->types); ++i) {
		Type *child = member_list->types + i;

		TypeTag typetag = get_child_type_tag(type_context, child);
		if (typetag.tag == SymTagNull)
			continue; // Early out if we have no valid type info

		while(typetag.tag == SymTagArrayType) {
			// If array of X, get the type of X through potentially multi-dimensional arrays.
			typetag = get_type_tag(type_context, typetag.type_index);
		}

		bool base_type_pointer = typetag.tag == SymTagBaseType && CV_TYP_IS_PTR(typetag.type_index);
		if (!base_type_pointer && typetag.tag != SymTagUDT && typetag.tag != SymTagPointerType) {
			continue; // Early out if it's not something that can contain a pointer!
		}

		bool contains_pointers = false;
		switch (typetag.tag) {
			case SymTagBaseType: {
				ASSERT(base_type_pointer, "Not a pointer!");
				contains_pointers = true;
			} break;
			case SymTagUDT: {
				uint32_t child_flags = gather_type_information(type_context, typetag.type_index);
				if (child_flags & TypeRecordFlags_ContainsPointers) {
					type_record->flags |= TypeRecordFlags_ContainsPointers;
				}
			} break;
			case SymTagPointerType: {
				contains_pointers = true;
			} break;
		}

		if (contains_pointers) {
			type_record->flags |= TypeRecordFlags_ContainsPointers;
		}
	}

	type_record->flags |= TypeRecordFlags_CheckedForPointers;
	return type_record->flags;
}

////////////////////////////////////////////////////////////
///////////////////// PATCH DATA TYPES /////////////////////
////////////////////////////////////////////////////////////
uint32_t patch_type(TypeContextPair *type_context, PointerContext &pointer_context, uint32_t type_index, AddressPair address);
uint32_t patch_array(TypeContextPair *type_context, PointerContext &pointer_context, TypeTag &typetag, AddressPair address) {
	// Get the total size of the array
	uint32_t array_size = get_size(type_context->n, typetag.type_index);
	while (typetag.tag == SymTagArrayType) {
		// Get the backing typetag (could also be an array in case of multi-dimensional arrays, therefore we need a while loop).
		typetag = get_type_tag(type_context->n, typetag.type_index);
	}

	uint32_t result_flags = 0;
	switch (typetag.tag) {
		case SymTagUDT: {
			TypeName *type_name = get_type_name(type_context->n, typetag.type_index);
			TypeRecordPair type_record = get_type_record_pair(type_context, type_name);

			int first_entry_set = 0;
			if (!(type_record.n->flags & TypeRecordFlags_Patched)) {
				// If I haven't visited the type in the array, visit the first via patch_type.
				uint32_t child_flags = patch_type(type_context, pointer_context, typetag.type_index, address);
				first_entry_set = 1;

				if (child_flags & TypeRecordFlags_IsModified) {
					result_flags |= TypeRecordFlags_IsModified;
				}
			}

			ASSERT(type_record.n->flags & TypeRecordFlags_Patched, "The child type needs to have been visited here!");

			OffsetPair element_size = get_size_pair(type_context, &type_record);
			size_t count = array_size / element_size.n;
			ASSERT(count, "Zero count arrays??");

			OffsetPair offset = {};
			if (first_entry_set) { offset = element_size; };

			if (type_record.n->flags & TypeRecordFlags_IsModified) {
#if defined(RELOAD_VERBOSE_DEBUGGING)
				log_warning("Reloader", "Found array of modified structs! (count=%zu, type=%u)", count, typetag.type_index);
				log_warning("Reloader", "Performance warning! Array entries have been modifed, move each one! (count=%zu)", count);
#endif

				for (size_t j = first_entry_set; j < count; ++j) {
					patch_type(type_context, pointer_context, typetag.type_index, address + offset);
					offset += element_size;
				}
			} else {
#if defined(RELOAD_VERBOSE_DEBUGGING)
				log_warning("Reloader", "Found array of unmodified structs! (count=%zu, type=%u)", count, typetag.type_index);
#endif
				move_chunk(pointer_context, address + offset, array_size - first_entry_set * element_size.n);
			}
		} break;
		case SymTagBaseType:
		case SymTagEnum:
		case SymTagPointerType: {
			move_chunk(pointer_context, address, array_size);
		} break;
		default: {
			ASSERT(false, "Unsupported tag '%u'", typetag.tag);
			move_chunk(pointer_context, address, array_size);
		}
	}

	return result_flags;
}

uint32_t patch_type(TypeContextPair *type_context, PointerContext &pointer_context, uint32_t type_index, AddressPair address) {
	TypeName *type_name = get_type_name(type_context->n, type_index);

	TypeRecordPair type_record = get_type_record_pair(type_context, type_name);

	// In order to move this as a chunk, it cannot contain pointers, it cannot be modified and we need to have visited it so that we are sure we know these two pieces of info.
	if ((type_record.n->flags & TypeRecordFlags_Patched) && !(type_record.n->flags & TypeRecordFlags_IsModified)) {
		uint32_t size = get_size(type_context->n, type_record.n->type_index);
		move_chunk(pointer_context, address, size);
		return type_record.n->flags;
	}

	check_pointer_self(pointer_context, address, type_index); // Check pointer to myself

	MemberListPair member_lists = get_member_list_pair(type_context, &type_record);
	for (int i = 0; i < array_count(member_lists.n->types); ++i) {
		unsigned member_flags = member_lists.n->flags[i];
		if (member_flags & TypeRecordFlags_IsNew)
			continue;

		OffsetPair offset;
		int is_new = check_if_new_member(type_context, &member_lists, type_record.n, address, i, &offset);
		if (is_new)
			continue;

		Type *new_child = member_lists.n->types + i;
		TypeTag new_typetag = get_child_type_tag(type_context->n, new_child);

		unsigned tag = new_typetag.tag;
		bool base_type_pointer = tag == SymTagBaseType && CV_TYP_IS_PTR(new_typetag.type_index);
		if (base_type_pointer) { // Pretend that the tag is SymTagPointerType if we have a pointer to base type.
			tag = SymTagPointerType;
		}

		switch (tag) {
			case SymTagUDT: {
				uint32_t child_flags = patch_type(type_context, pointer_context, new_typetag.type_index, address + offset);
				if (child_flags & TypeRecordFlags_IsModified) {
					type_record.n->flags |= TypeRecordFlags_IsModified;
				}
			} break;
			case SymTagTypedef: {
				ASSERT(false, "Not implemented");
			} break;
			case SymTagPointerType: {
				check_pointer_fixed(pointer_context, address + offset, sizeof(void*));
			} break;
			case SymTagArrayType: {
				uint32_t child_flags = patch_array(type_context, pointer_context, new_typetag, address + offset);
				if (child_flags & TypeRecordFlags_IsModified) {
					type_record.n->flags |= TypeRecordFlags_IsModified;
				}
			} break;
			default: {
				uint32_t size = get_size(type_context->n, new_typetag.type_index);
				move_chunk(pointer_context, address + offset, size);
			} break;
		}
	}
	type_record.n->flags |= TypeRecordFlags_Patched;
	return type_record.n->flags;
}

void patch_memory(Allocator &a, const char *pdb_name, uint64_t entry_name_id, TypeContext &old_type_context, TypeContext &new_type_context, Allocator *new_allocator, Allocator *old_allocator, size_t memory_size) {
#if defined(RELOAD_PROFILING)
	Stopwatch pointer_sw;
	Stopwatch data_sw;
	Stopwatch total_sw;

	total_sw.start();
	pointer_sw.start();
#endif

	size_t old_memory_size = memory_size;
	AllocatorBackendHeader *backend = init_allocator_backend(&a, old_allocator, new_allocator, memory_size);

	PointerContext pointer_context = setup_pointer_context(a);

	// TODO(kalle): Clone the malloc_state? Should we keep the information about the freed objects?

	TypeContextPair type_context = {};
	type_context.n = &new_type_context;
	type_context.o = &old_type_context;
	TypeRecord *entry_record = get_type_record(type_context.n, entry_name_id);

	{ // Gather pointers
		AddressPair base_address;
		bool done = next(backend, &base_address);
		ASSERT(!done, "Reload backend was empty! No base address to follow.");

		Pointer current_pointer = make_pointer(base_address.o, entry_record->type_index);
		gather_pointers(&type_context, pointer_context, current_pointer.type_index, base_address);

		while (!done) {
			TypeName *type_name = get_type_name(type_context.n, current_pointer.type_index);
			TypeRecordPair type_record = get_type_record_pair(&type_context, type_name);
			OffsetPair size = get_size_pair(&type_context, &type_record);

			size_t count = get_count(backend, size.o);
			set_head(backend, size.n, count);

			done = next(backend, &base_address);
			if (done) {
				init_top(backend, &base_address);
			} else {
				bool success = search_for_next_reloadable_pointer(pointer_context, &current_pointer, backend);
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
	reset(backend);

	{ // Copy the data
		AddressPair base_address;
		next(backend, &base_address);
		sort_pointers(pointer_context, base_address.o, old_memory_size);

		// We require to know about the "entry point", i.e. the layout of where the input memory is pointing.
		Pointer current_pointer = make_pointer(base_address.o, entry_record->type_index);

		bool done = false;
		while (!done) {
			TypeName *type_name = get_type_name(type_context.n, current_pointer.type_index);
			TypeRecordPair type_record = get_type_record_pair(&type_context, type_name);
			OffsetPair sizes = get_size_pair(&type_context, &type_record);

			size_t count = get_count(backend, sizes.o);
			for (uint32_t i = 0; i < count; i++) {
				patch_type(&type_context, pointer_context, current_pointer.type_index, base_address);
				base_address += sizes;
			}
			// We have reached the end of a new:ed struct. We need to look at the next newed portion, i.e. the next malloc chunk to find out where to go next.

			if (next(backend, &base_address)) {
				done = true;
			} else {
				bool has_valid_pointer = pointer_context.current_pointer_index < pointer_context.sorted_pointer_count;
				ASSERT(has_valid_pointer, "No valid pointer found!");

				SortElement &element = pointer_context.sorted_pointer_indices[pointer_context.current_pointer_index];
				current_pointer = pointer_context.pointer_array[element.index];
			}
		}
	}

	{ // Update all pointers!
		Pointer *pointer_array = pointer_context.pointer_array;
		for (int32_t i = 0; i < array_count(pointer_array); ++i) {
			Pointer &p = pointer_array[i];
			*(intptr_t*)(p.address.n) = p.target_address.n;

			bool in_old_space = is_address_in_old_range(backend, p.target_address.n);
			ASSERT(!in_old_space, "Pointer in new memory chunk still points to old memory!");
		}
	}

	// free(pointer_context.memory);

#if defined(RELOAD_PROFILING)
	log_warning("Reloader", "Took %g seconds to move data.", data_sw.stop());
	log_warning("Reloader", "Took %g seconds to patch memory.", total_sw.stop());
#endif
}

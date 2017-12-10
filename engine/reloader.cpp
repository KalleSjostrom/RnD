#define MAX_POINTERS (1<<17)
#define MAX_RECORDED_ENTRIES (1<<17)

#define VERBOSE_DEBUGGING 1
#define USE_TYPE_CACHE 1

#include "utils/quick_sort.cpp"
#include "utils/profiler.c"
#include "symbol_info.cpp"

struct Pointer {
	intptr_t addr_old;
	intptr_t addr_new;
	intptr_t target_addr_old;
	intptr_t target_addr_new;
	u32 type;
	WCHAR *debug_tag;
};
inline Pointer make_entry_pointer(intptr_t addr_old, SYMBOL_INFOW *symbol) {
	Pointer p = {};
	p.addr_old = addr_old;
	p.addr_new = 0;
	p.target_addr_old = *(intptr_t*)(addr_old);
	p.target_addr_new = 0;
	p.type = symbol->TypeIndex;
	p.debug_tag = symbol->Name;
	return p;
}

struct PointerArray {
	Pointer *entries;
	int count;
};

struct PointerContext {
	void *memory;

	PointerArray pointer_array;
	int current_pointer_index;

	unsigned *record_hashset;

	SortElement *sorted_pointer_indices;
	int sorted_pointer_count;
};
PointerContext setup_pointer_context() {
	char *memory = (char*)malloc(sizeof(Pointer) * (MAX_POINTERS) + sizeof(unsigned) * (MAX_RECORDED_ENTRIES) + sizeof(SortElement) * (MAX_POINTERS));

	PointerContext context = {};

	context.memory = memory;

	// Set up the address lookup.
	context.pointer_array.entries = (Pointer *)memory;
	memory += sizeof(Pointer) * MAX_POINTERS;

	context.record_hashset = (unsigned *)memory;
	memory += sizeof(unsigned) * MAX_RECORDED_ENTRIES;

	context.sorted_pointer_indices = (SortElement*)memory;
	memory += sizeof(SortElement) * MAX_POINTERS;

	return context;
}
bool search_for_next_reloadable_pointer(PointerContext &context, Pointer *out_pointer, intptr_t min_address, intptr_t max_address) {
	// The cursor should always be on the pointer that has the lowest target address
	intptr_t lowest_target_address = INTPTR_MAX;
	int best_pointer_index = -1;
	PointerArray &pointer_array = context.pointer_array;
	for (int i = context.current_pointer_index; i < pointer_array.count; ++i) {
		Pointer &pointer = pointer_array.entries[i];

		if (pointer.target_addr_old < lowest_target_address && pointer.target_addr_old >= min_address && pointer.target_addr_old < max_address) {
			lowest_target_address = pointer.target_addr_old;
			best_pointer_index = (int)i;
		}
	}

	if (best_pointer_index >= 0) {
		*out_pointer = pointer_array.entries[best_pointer_index];

		Pointer temp = pointer_array.entries[context.current_pointer_index];
		pointer_array.entries[context.current_pointer_index] = *out_pointer;
		pointer_array.entries[best_pointer_index] = temp;

		context.current_pointer_index++;
		return true;
	}

	return false;
}
void set_malloc_chunk_head(malloc_chunk *old_chunk, malloc_chunk *new_chunk, size_t new_size) {
	new_chunk->head = pad_request(new_size) | cinuse(old_chunk) | pinuse(old_chunk);
}

// DATA COPY STEP //
void sort_pointers(PointerContext &context, intptr_t base_old, size_t size) {
	context.current_pointer_index = 0;

	PointerArray &pointer_array = context.pointer_array;
	int count = 0;
	for (int i = 0; i < pointer_array.count; ++i) {
		Pointer &p = pointer_array.entries[i];
		intptr_t offset = p.target_addr_old - base_old;
		if (offset >= 0 && offset < (int)size) { // Pointing into our chunk
			context.sorted_pointer_indices[count].value = (unsigned)offset;
			context.sorted_pointer_indices[count].index = i;
			count++;
		}
		else { // I'm pointing to outside the range, just keep whatever I was pointing at
			*(intptr_t*)(p.addr_new) = p.target_addr_new;
		}
	}
	quick_sort(context.sorted_pointer_indices, count);
	context.sorted_pointer_count = count;
}

void check_pointer(PointerContext &context, intptr_t chunk_start_old, intptr_t chunk_start_new, size_t chunk_size, u32 chunk_type) {
	(void)chunk_type;
	// This chunk is a fixed block of memory (unmodifed), therefore we need to check all the pointers in the address range ([chunk_start_old, chunk_start_old + chunk_size])
	while (context.current_pointer_index < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index];
		Pointer &pointer = context.pointer_array.entries[element.index];
		ASSERT(pointer.target_addr_old != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_addr_old < (chunk_start_old + (ptrdiff_t)chunk_size)) { // Am I pointing to moved memory?
																		// I'm pointing into an unmodified chunk. Find out how far in the pointer pointed to, this delta is the same in the old and new space!
			pointer.target_addr_new = chunk_start_new + (pointer.target_addr_old - chunk_start_old);
			ASSERT(pointer.addr_new, "We need to store this for later!");

			*(intptr_t*)(pointer.addr_new) = pointer.target_addr_new;
			context.current_pointer_index++;
		}
		else {
			break;
		}
	}
}

void move_chunk(PointerContext &context, intptr_t addr_old, intptr_t addr_new, size_t size, u32 chunk_type) {
	memmove((void*)addr_new, (void*)addr_old, size);
	check_pointer(context, addr_old, addr_new, size, chunk_type);
}

bool get_next_reloadable_pointer(PointerContext &context, Pointer *out_pointer) {
	if (context.current_pointer_index >= context.sorted_pointer_count)
		return false;

	SortElement &element = context.sorted_pointer_indices[context.current_pointer_index];
	*out_pointer = context.pointer_array.entries[element.index];
	return true;
}

// POINTER GATHERING STEP //
void try_fill_pointer(PointerContext &context, intptr_t addr_old, intptr_t addr_new, u32 type, WCHAR *debug_tag = 0) {
	intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");
	if (target_addr_old != 0) { // We don't care about pointers pointing to null!
		ASSERT(context.pointer_array.count < MAX_POINTERS, "Array index out of bounds!");
		Pointer &p = context.pointer_array.entries[context.pointer_array.count++];

		p.addr_old = addr_old;
		p.addr_new = addr_new;
		p.target_addr_old = target_addr_old;
		p.target_addr_new = target_addr_old; // By default, point to wherever it was pointing to
		p.type = type;
		p.debug_tag = debug_tag;
	}
}

void set_pointer_array(PointerContext &context, intptr_t addr_old, intptr_t addr_new, u64 count, u32 type, WCHAR *debug_tag = 0) {
	for (unsigned i = 0; i < count; ++i) {
		try_fill_pointer(context, addr_old, addr_new, type, debug_tag);

		addr_old += sizeof(void*);
		addr_new += sizeof(void*);
	}
}

void record_type(PointerContext &context, unsigned type) {
	unsigned hash_mask = MAX_RECORDED_ENTRIES - 1;
	unsigned key = type & hash_mask;
	for (unsigned offset = 0; offset < MAX_RECORDED_ENTRIES; offset++) {
		unsigned index = (key + offset) & hash_mask;

		unsigned &entry = context.record_hashset[index];
		if (entry == ~0u) {
			entry = key;
			break;
		}
	}
}

bool has_record(PointerContext &context, unsigned type) {
	unsigned hash_mask = MAX_RECORDED_ENTRIES - 1;
	unsigned key = type & hash_mask;
	for (unsigned offset = 0; offset < MAX_RECORDED_ENTRIES; offset++) {
		unsigned index = (key + offset) & hash_mask;

		unsigned entry = context.record_hashset[index];
		if (entry == ~0u) {
			return false;
		}
		if (entry == key) {
			return true;
		}
	}
	return false;
}

bool expand_pointer(HANDLE process, u64 mod_base, SymbolContext &symbol_context, PointerContext &context, Pointer &pointer, intptr_t addr_old, intptr_t addr_new) {
	u32 type = pointer.type;
	if (has_record(context, type)) { // Have we recorded this type?
#if VERBOSE_DEBUGGING
		// LOG_INFO("Reloader", "Ignoring expansion because it doesn't contains pointers! (name=%S).\n", pointer.debug_tag));
#endif
		return false;
	}

	bool _contains_pointers = false;
	WCHAR *name = 0;

	TI_FINDCHILDREN_PARAMS *children = get_children(symbol_context, type);
	if (children) {
#if VERBOSE_DEBUGGING
		LOG_INFO("Reloader", "Expanding pointer! (name=%S, num_children=%ld).\n", pointer.debug_tag, children->Count);
#endif
		for (u32 i = 0; i < children->Count; ++i) {
			ULONG curChild = children->ChildId[i];

			type = get_type(symbol_context, curChild);
			if (has_record(context, type)) {
#if VERBOSE_DEBUGGING
				name = get_symbol_name(symbol_context, type);
				LOG_INFO("Reloader", "Ignoring expansion because it doesn't contain pointers! (name=%S).\n", name);
#endif
				continue;
			}

			u32 tag = get_tag(symbol_context, type);

#if VERBOSE_DEBUGGING
			WCHAR *symbol_name = get_symbol_name(symbol_context, curChild);
#endif
			if (tag == SymTagFunctionType) {
				LOG_INFO("Reloader", "Ignoring function! (name=%S).\n", symbol_name);
				continue;
			}

			switch (tag) {
				case SymTagUDT: {
					Pointer p = {};
					p.type = type;

#if VERBOSE_DEBUGGING
					name = get_symbol_name(symbol_context, type);
					p.debug_tag = name;
#endif

					u32 offset = get_offset(symbol_context, curChild);

					u32 old_offset = offset;
					u32 new_offset = offset;

					if (expand_pointer(process, mod_base, symbol_context, context, p, addr_old + old_offset, addr_new + new_offset)) {
						_contains_pointers = true;
					}
				} break;
				case SymTagPointerType: {
					while (tag == SymTagPointerType || tag == SymTagArrayType) {
						type = get_type(symbol_context, type);
						tag = get_tag(symbol_context, type);
					}

					u32 offset = get_offset(symbol_context, curChild);

					u32 old_offset = offset;
					u32 new_offset = offset;

#if VERBOSE_DEBUGGING
					name = get_symbol_name(symbol_context, type);
#endif
					try_fill_pointer(context, addr_old + old_offset, addr_new + new_offset, type, name);

					_contains_pointers = true;
				} break;
				case SymTagArrayType: { // set_pointer_array
					// u64 length = get_length(symbol_context, type);
					u64 count = get_count(symbol_context, type);

					type = get_type(symbol_context, type);
					tag = get_tag(symbol_context, type);

#if VERBOSE_DEBUGGING
					name = get_symbol_name(symbol_context, type);
#endif

					if (tag == SymTagPointerType) {
#if VERBOSE_DEBUGGING
						LOG_INFO("Reloader", "Found pointer array! (name=%S, count=%zd).\n", symbol_name, count);
#endif
						u32 offset = get_offset(symbol_context, curChild);

						u32 old_offset = offset;
						u32 new_offset = offset;

						while (tag == SymTagPointerType) {
							type = get_type(symbol_context, type);
							tag = get_tag(symbol_context, type);
						}

						_contains_pointers = true;

#if VERBOSE_DEBUGGING
						name = get_symbol_name(symbol_context, type);
#endif
						set_pointer_array(context, addr_old + old_offset, addr_new + new_offset, count, type, name);
					} else if (tag == SymTagUDT) {
						Pointer p = {};
						p.type = type;

#if VERBOSE_DEBUGGING
						p.debug_tag = name;
#endif

						u32 offset = get_offset(symbol_context, curChild);

						u32 old_offset = offset;
						u32 new_offset = offset;

						for (unsigned j = 0; j < count; ++ j) {
							if (expand_pointer(process, mod_base, symbol_context, context, p, addr_old + old_offset, addr_new + new_offset)) {
								_contains_pointers = true;
							}

							old_offset += sizeof(void*);
							new_offset += sizeof(void*);
						}
					}
				} break;
			}
		}

		// TODO(kalle): Cleanup!
		// delete[] children;
	}

	if (!_contains_pointers) {
#if VERBOSE_DEBUGGING
		LOG_INFO("Reloader", "Type didn't contain pointers, make a record of this! (name=%S).\n", pointer.debug_tag);
#endif
		record_type(context, pointer.type);
	}

	return _contains_pointers;
}

bool set_thing(HANDLE process, u64 mod_base, SymbolContext &symbol_context, PointerContext &context, Pointer &pointer, intptr_t addr_old, intptr_t addr_new) {
	u32 type = pointer.type;

	if (has_record(context, type)) {
#if VERBOSE_DEBUGGING
		LOG_INFO("Reloader", "Found record of type, copy it! (name=%S).\n", pointer.debug_tag);
#endif

		u64 size = get_size(symbol_context, type);

		move_chunk(context, addr_old, addr_new, size, type);
		return false;
	}

	bool _is_modified = false;

	WCHAR *name = 0; // The user must free the buffer!
	u64 size;

	size = get_size(symbol_context, type);
	check_pointer(context, addr_old, addr_new, size, type); // Check pointer to myself

	TI_FINDCHILDREN_PARAMS *children = get_children(symbol_context, type);
	if (children) {
#if VERBOSE_DEBUGGING
		LOG_INFO("Reloader", "Checking member! (name=%S, num_children=%ld).\n", pointer.debug_tag, children->Count);
#endif
		for (u32 i = 0; i < children->Count; ++i) {
			ULONG curChild = children->ChildId[i];
			type = get_type(symbol_context, curChild);

			if (has_record(context, type)) {
#if VERBOSE_DEBUGGING
				name = get_symbol_name(symbol_context, type);
				LOG_INFO("Reloader", "Found record of type, copy it! (name=%S).\n", name);
#endif

				u32 offset = get_offset(symbol_context, curChild);
				size = get_size(symbol_context, type);

				u32 old_offset = offset;
				u32 new_offset = offset;

				move_chunk(context, addr_old + old_offset, addr_new + new_offset, size, type);
				continue;
			}

			u32 tag = get_tag(symbol_context, type);

#if VERBOSE_DEBUGGING
			WCHAR *symbol_name = get_symbol_name(symbol_context, curChild);
#endif
			if (tag == SymTagFunctionType) {
				continue;
			}

			switch (tag) {
				case SymTagUDT: {
					Pointer p = {};
					p.type = type;

#if VERBOSE_DEBUGGING
					name = get_symbol_name(symbol_context, type);
					p.debug_tag = name;
#endif

					u32 offset = get_offset(symbol_context, curChild);

					u32 old_offset = offset;
					u32 new_offset = offset;

					if (set_thing(process, mod_base, symbol_context, context, p, addr_old + old_offset, addr_new + new_offset)) {
						_is_modified = true;
					}
				} break;
				case SymTagPointerType: {
					while (tag == SymTagPointerType || tag == SymTagArrayType) {
						type = get_type(symbol_context, type);
						tag = get_tag(symbol_context, type);
					}

#if VERBOSE_DEBUGGING
					name = get_symbol_name(symbol_context, type);
#endif

					u32 offset = get_offset(symbol_context, curChild);

					u32 old_offset = offset;
					u32 new_offset = offset;

					check_pointer(context, addr_old + old_offset, addr_new + new_offset, sizeof(void*), type);
				} break;
				case SymTagArrayType: { // set_pointer_array
					size = get_size(symbol_context, type);
					// u64 count = get_count(symbol_context, type);

					type = get_type(symbol_context, type);
					tag = get_tag(symbol_context, type);

					u64 element_size = get_size(symbol_context, type);

					u64 new_element_size = element_size;
					u64 old_element_size = element_size;

#if VERBOSE_DEBUGGING
					name = get_symbol_name(symbol_context, type);
#endif

					u32 offset = get_offset(symbol_context, curChild);

					u32 old_offset = offset;
					u32 new_offset = offset;

					if (has_record(context, type)) {
#if VERBOSE_DEBUGGING
						LOG_INFO("Reloader", "Found record of type, copy it! (name=%S).\n", name);
#endif
						move_chunk(context, addr_old + old_offset, addr_new + new_offset, size, type);
					} else {
						if (tag == SymTagPointerType) {
							check_pointer(context, addr_old + old_offset, addr_new + new_offset, sizeof(void*), type);
						} else if (tag == SymTagUDT) {
							size_t c = size / element_size;
							Pointer p = {};
							p.debug_tag = name;
							p.type = type;

							if (set_thing(process, mod_base, symbol_context, context, p, addr_old + old_offset, addr_new + new_offset)) {
								_is_modified = true;
							}

							// If the set_thing above caused this type to be recorded, then copy the rest
							if (has_record(context, type)) {
#if VERBOSE_DEBUGGING
								LOG_INFO("Reloader", "Found record of type in array, copy the rest of the array! (name=%S, count=%zu).\n", name, c);
#endif
								move_chunk(context, addr_old + old_offset + old_element_size, addr_new + new_offset + new_element_size, size - old_element_size, type);
							} else {
								for (size_t j = 1; j < c; j++) {
									if (set_thing(process, mod_base, symbol_context, context, p, addr_old + old_offset + j * old_element_size, addr_new + new_offset + j * new_element_size)) {
										_is_modified = true;
									}
								}
							}
						} else {
#if VERBOSE_DEBUGGING
							LOG_INFO("Reloader", "Found chunk array, copy it! (name=%S, symbol_name=%S).\n", name, symbol_name);
#endif
							move_chunk(context, addr_old + old_offset, addr_new + new_offset, size, type);
						}
					}
				} break;
				default: {
					u32 offset = get_offset(symbol_context, curChild);
					size = get_size(symbol_context, type);

					u32 old_offset = offset;
					u32 new_offset = offset;

#if VERBOSE_DEBUGGING
					LOG_INFO("Reloader", "Found chunk, copy it! (name=%S).\n", symbol_name);
#endif
					move_chunk(context, addr_old + old_offset, addr_new + new_offset, size, type);
				} break;
			}
		}
		// TODO(kalle): Cleanup!
		// delete[] children;
	}

	if (!_is_modified) {
#if VERBOSE_DEBUGGING
		LOG_INFO("Reloader", "Recording type, since it's not modified! (name=%S).\n", pointer.debug_tag);
#endif
		record_type(context, pointer.type);
	}
	return _is_modified;
}

size_t safe_ratio_0(size_t numerator, size_t divisor) {
	return divisor == 0 ? 0 : (numerator / divisor);
}
size_t estimate_unpadded_request(size_t chunksize) {
	/////// TODO ///////
	return chunksize;
}

void symbased_memory_patching(HANDLE process, u64 mod_base, SYMBOL_INFOW *entry, ReloadHeader &header) {
	Stopwatch sw;
	sw.start();

	void *old_mspace = header.old_mspace;
	void *new_mspace = header.new_mspace;

	size_t old_memory_size = header.old_memory_size;

	// malloc_state *malloc_state_old = (malloc_state *)(old_mspace);
	malloc_state *malloc_state_new = (malloc_state *)(new_mspace);

	malloc_chunk *current_malloc_chunk_old = (malloc_chunk*)mem2chunk(old_mspace);
	malloc_chunk *current_malloc_chunk_new = (malloc_chunk*)mem2chunk(new_mspace);

	intptr_t old_memory = (intptr_t)current_malloc_chunk_old;
	intptr_t new_memory = (intptr_t)current_malloc_chunk_new;

	PointerContext context = setup_pointer_context();
	SymbolContext symbol_context = {};

	symbol_context.process = process;
	symbol_context.mod_base = mod_base;
#ifdef USE_TYPE_CACHE
	symbol_context.recorded_types = (TypeInfo*)malloc(sizeof(TypeInfo) * MAX_RECORDED_TYPES);
	memset(symbol_context.recorded_types, 0, sizeof(TypeInfo) * MAX_RECORDED_TYPES);
#endif // USE_TYPE_CACHE

	// TODO(kalle): Clone the malloc_state? Should we keep the information about the freed objects?

	{ // Gather pointers
		memset(context.record_hashset, 0xffffffff, sizeof(unsigned) * MAX_RECORDED_ENTRIES);
		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

		Pointer current_pointer = make_entry_pointer(base_old, entry);

		bool done = false;
		while (!done) {
			size_t current_chuck_size = estimate_unpadded_request(chunksize(current_malloc_chunk_old));

			size_t old_size = get_size(symbol_context, current_pointer.type); // get_old_size(cursor);
			size_t new_size = get_size(symbol_context, current_pointer.type);

			size_t count = safe_ratio_0(current_chuck_size, old_size);

			expand_pointer(process, mod_base, symbol_context, context, current_pointer, base_old, base_new);

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
				bool success = search_for_next_reloadable_pointer(context, &current_pointer, base_old, old_memory + old_memory_size);
				if (!success) {
					LOG_INFO("Reloader", "Could not find another pointer to follow!");
					done = true;
					break;
				}

				// Check to see if this pointed into already mapped memory. If so it's useless to us, and we'll continue to the next one.
				// intptr_t target_addr_old = *(intptr_t*)current_pointer.addr_old;
				*(intptr_t*)(current_pointer.addr_new) = base_new;
			}
		}
	}

	// Reset state for the next iteration
	current_malloc_chunk_old = (malloc_chunk*)old_memory;
	current_malloc_chunk_new = (malloc_chunk*)new_memory;

	{ // Copy the data
		memset(context.record_hashset, 0xffffffff, sizeof(unsigned) * MAX_RECORDED_ENTRIES);

		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

		sort_pointers(context, base_old, old_memory_size);

		// We require to know about the "entry point", i.e. the layout of where the input memory is pointing.
		Pointer current_pointer = make_entry_pointer(base_old, entry);

		bool done = false;
		while (!done) {
			size_t current_chuck_size = estimate_unpadded_request(chunksize(current_malloc_chunk_old));

			size_t old_size = get_size(symbol_context, current_pointer.type); // get_old_size(cursor);
			size_t new_size = get_size(symbol_context, current_pointer.type);

			size_t count = safe_ratio_0(current_chuck_size, old_size);
			for (unsigned i = 0; i < count; i++) {
				set_thing(process, mod_base, symbol_context, context, current_pointer, base_old, base_new);
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
				get_next_reloadable_pointer(context, &current_pointer);
			}
		}
	}

	free(context.memory);

	float time = sw.stop();
	LOG_INFO("Reloader", "Took %g seconds to patch memory.\n", time);
}
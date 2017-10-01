#define MAX_POINTERS 1<<12

#ifndef offsetof
#define offsetof(st, m) ((size_t)(&((st *)0)->m))
#endif

#include "quick_sort.cpp"

struct Pointer {
	intptr_t addr_old;
	intptr_t addr_new;
	intptr_t target_addr_old;
	intptr_t target_addr_new;
	char *debug_tag;
	int pointer_type;
	int _padding;
};
inline Pointer make_entry_pointer(intptr_t addr_old, int pointer_type) {
	Pointer p = {};
	p.addr_old = addr_old;
	p.addr_new = 0;
	p.target_addr_old = *(intptr_t*)(addr_old);
	p.target_addr_new = 0;
	p.pointer_type = pointer_type;
	p.debug_tag = "Entry Point";
	return p;
}

struct PointerArray {
	Pointer *entries;
	int count;
	int _padding;
};

struct PointerContext {
	PointerArray pointer_array;
	SortElement *sorted_pointer_indices;

	int current_pointer_index;
	int sorted_pointer_count;
};
PointerContext setup_pointer_context(char *temporary_memory, intptr_t old_memory, size_t old_size) {
	intptr_t start = (intptr_t)temporary_memory;

	PointerContext context = {};

	// Set up the address lookup.
	context.pointer_array.entries = (Pointer *)temporary_memory;
	temporary_memory += sizeof(Pointer) * MAX_POINTERS;

	context.sorted_pointer_indices = (SortElement*)temporary_memory;
	temporary_memory += sizeof(SortElement) * MAX_POINTERS;

	intptr_t diff = (intptr_t)temporary_memory - start;
	ASSERT(diff < SCRATCH_SPACE_MEMORY_SIZE, "No room in scratch space to allocate pointer context");

	return context;
}

// POINTER GATHERING STEP //
inline void try_fill_pointer(PointerContext &context, intptr_t addr_old, intptr_t addr_new, int pointer_type, char *debug_tag = 0) {
	intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.\n");
	if (target_addr_old != 0) { // We don't care about pointers pointing to null!
		ASSERT(context.pointer_array.count < MAX_POINTERS, "Array index out of bounds!");
		Pointer &p = context.pointer_array.entries[context.pointer_array.count++];

		p.addr_old = addr_old;
		p.addr_new = addr_new;
		p.target_addr_old = target_addr_old;
		p.target_addr_new = target_addr_old; // By default, point to wherever it was pointing to
		p.pointer_type = pointer_type;
		p.debug_tag = debug_tag;
	}
}

inline void set_pointer_array(PointerContext &context, intptr_t addr_old, intptr_t addr_new, unsigned count, intptr_t pointer_type, char *debug_tag = 0) {
	for (unsigned i = 0; i < count; ++i) {
		try_fill_pointer(context, addr_old, addr_new, pointer_type, debug_tag);

		addr_old += sizeof(void*);
		addr_new += sizeof(void*);
	}
}

bool search_for_next_reloadable_pointer(PointerContext &context, Pointer *out_pointer) {
	// The cursor should always be on the pointer that has the lowest target address
	intptr_t lowest_target_address = INTPTR_MAX;
	int best_pointer_index = -1;
	PointerArray &pointer_array = context.pointer_array;
	for (int i = context.current_pointer_index; i < pointer_array.count; ++i) {
		Pointer &pointer = pointer_array.entries[i];

		if (pointer.target_addr_old < lowest_target_address && pointer.pointer_type > ReloadType_BaseType) {
			lowest_target_address = pointer.target_addr_old;
			best_pointer_index = (int) i;
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
inline void set_malloc_chunk_head(malloc_chunk *old_chunk, malloc_chunk *new_chunk, size_t new_size) {
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
		} else { // I'm pointing to outside the range, just keep whatever I was pointing at
			*(intptr_t*)(p.addr_new) = p.target_addr_new;
		}
	}
	quick_sort(context.sorted_pointer_indices, count);
	context.sorted_pointer_count = count;
}

void _check_pointer_modified(PointerContext &context, intptr_t chunk_start_old, intptr_t chunk_start_new, int chunk_type) {
	// This chunk is of a specific type that is being set (and have possibly been modified), we only need to check pointers equal to chunk_start_old.
	int index_offset = 0;
	while (context.current_pointer_index + index_offset < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index + index_offset];
		Pointer &pointer = context.pointer_array.entries[element.index];
		ASSERT(pointer.target_addr_old != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_addr_old == chunk_start_old) {
			if (pointer.pointer_type == chunk_type) { // Also make sure I'm pointing to the correct type (and not the first member of a struct)
				pointer.target_addr_new = chunk_start_new;
				ASSERT(pointer.addr_new, "We need to store this for later!");

				*(intptr_t*)(pointer.addr_new) = pointer.target_addr_new;

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

void _check_pointer_fixed(PointerContext &context, intptr_t chunk_start_old, intptr_t chunk_start_new, int chunk_size) {
	// This chunk is a fixed block of memory (unmodifed), therefore we need to check all the pointers in the address range ([chunk_start_old, chunk_start_old + chunk_size])
	while (context.current_pointer_index < context.sorted_pointer_count) {
		SortElement &element = context.sorted_pointer_indices[context.current_pointer_index];
		Pointer &pointer = context.pointer_array.entries[element.index];
		ASSERT(pointer.target_addr_old != 0, "We should already have removed pointers pointing to null");

		if (pointer.target_addr_old < (chunk_start_old + chunk_size)) { // Am I pointing to moved memory?
			// I'm pointing into an unmodified chunk. Find out how far in the pointer pointed to, this delta is the same in the old and new space!
			pointer.target_addr_new = chunk_start_new + (pointer.target_addr_old - chunk_start_old);
			ASSERT(pointer.addr_new, "We need to store this for later!");

			*(intptr_t*)(pointer.addr_new) = pointer.target_addr_new;
			context.current_pointer_index++;
		} else {
			break;
		}
	}
}

void check_pointer(PointerContext &context, intptr_t chunk_start_old, intptr_t chunk_start_new, int chunk_size, int chunk_type) {
	if (chunk_type > ReloadType_BaseType) {
		_check_pointer_modified(context, chunk_start_old, chunk_start_new, chunk_type);
	} else {
		_check_pointer_fixed(context, chunk_start_old, chunk_start_new, chunk_size);
	}
}

void move_chunk(PointerContext &context, intptr_t addr_old, intptr_t addr_new, size_t size, int chunk_type) {
	memmove((void*)addr_new, (void*)addr_old, size);
	check_pointer(context, addr_old, addr_new, (int)size, chunk_type);
}

bool get_next_reloadable_pointer(PointerContext &context, Pointer *out_pointer) {
	if (context.current_pointer_index >= context.sorted_pointer_count)
		return false;

	SortElement &element = context.sorted_pointer_indices[context.current_pointer_index];
	*out_pointer = context.pointer_array.entries[element.index];
	return true;
}

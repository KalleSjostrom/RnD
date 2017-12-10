#define DEBUG_RELOAD 1

#define MAX_ADDRESS_NODES 16384*8
#define MAX_MAPPED_REGIONS 16384*8 // 2^17
#define MAX_POINTERS 4096

#define BUCKET_MEMORY_RANGE 512

#include "address_lookup.cpp"

#ifndef offsetof
#define offsetof(st, m) ((size_t)(&((st *)0)->m))
#endif

struct Pointer {
	intptr_t addr_old;
	intptr_t addr_new;
	intptr_t pointer_type;
	char *debug_tag;
};

// Stores information about seen and unseen memory locations.
// "address_lookup" can be used to see if we have a mapping between a point in old memory to a point in old memory.
struct MemoryMapper {
	AddressLookup *address_lookup;

	Pointer *pointers;
	unsigned pointer_count;
};

MemoryMapper setup_memory_mapper(char *temporary_memory, intptr_t old_memory, size_t old_size) {
	intptr_t start = (intptr_t)temporary_memory;

	// Set up the address lookup.
	AddressLookup *lookup = (AddressLookup *)temporary_memory;
	temporary_memory += sizeof(AddressLookup);

	lookup->node_storage = (AddressNode *)temporary_memory;
	lookup->node_storage_count = 0;
	temporary_memory += MAX_ADDRESS_NODES * sizeof(AddressNode);

	lookup->region_storage = (MappingRegion *)temporary_memory;
	lookup->region_storage_count = 0;
	temporary_memory += MAX_MAPPED_REGIONS * sizeof(MappingRegion);

	unsigned nr_buckets = (unsigned)ceil(old_size / (float)BUCKET_MEMORY_RANGE);
	lookup->buckets = (AddressNode **)temporary_memory;
	temporary_memory += nr_buckets * sizeof(AddressNode*);

	lookup->memory_base = old_memory;
	lookup->memory_size = old_size;

	// Set up the memory mapper. This has lookup between the old adress to the new.
	MemoryMapper mapper = {};

	mapper.address_lookup = lookup;

	mapper.pointers = (Pointer *)temporary_memory;
	mapper.pointer_count = 0;
	temporary_memory += MAX_POINTERS * sizeof(Pointer);

	intptr_t stop = (intptr_t)temporary_memory;
	ASSERT((stop-start) < SCRATCH_SPACE_MEMORY_SIZE, "Memory allocated for reload is greater than the memory allotted for it!");

	return mapper;
}

void verify_size(malloc_chunk *chunk, size_t size) {
	size_t size_from_chunk = estimate_unpadded_request(chunksize(chunk));
	size_t diff = size_from_chunk - size;
	ASSERT(diff >= 0 && diff < MALLOC_ALIGNMENT, "Size mismatch! (size_from_chunk=%lu, size=%lu)", size_from_chunk, size);
}

inline void set_malloc_chunk_head(malloc_chunk *old_chunk, malloc_chunk *new_chunk, size_t new_size) {
	new_chunk->head = pad_request(new_size) | cinuse(old_chunk) | pinuse(old_chunk);
}

// Adds a mapping from addr_old to addr_new (if none exist) and copies size bytes of memory from addr_old to addr_new.
// TODO(kalle): Fix so that addr_old, and addr_new could have different sizes. This can happen if one changes the size of an array between reloads
inline void memcpy_generic(MemoryMapper *mapper, intptr_t base_old, intptr_t offset_old, intptr_t addr_new, size_t size, char *debug_tag = 0) {
	if (offset_old >= 0) { // If offset_old is -1 then this "thing" didn't exist in the old space.
		intptr_t addr_old = base_old + offset_old;
		add_node_to_lookup(mapper->address_lookup, addr_old, size, addr_new, size, 1, debug_tag);

		// Copy whatever was in the old memory into the new.
		memcpy((void*)addr_new, (void*)addr_old, size);
	}
}

// Works as memcpy_generic but is a macro instead, so that we can copy without memcpy, don't know if it makes any diffence though..
// If offset_old is -1 then this "thing" didn't exist in the old space.
#define SET_GENERIC(mapper, base_old, offset_old, addr_new, size, type, debug_tag) { \
	if (offset_old >= 0) { \
		intptr_t addr_old = base_old + offset_old; \
		add_node_to_lookup((mapper)->address_lookup, addr_old, size, addr_new, size, 1, debug_tag); \
		*(type*)(addr_new) = *(type*)(addr_old); \
	} \
}

inline bool try_set_pointer_value(AddressLookup *address_lookup, intptr_t target_addr_old, intptr_t addr_new, intptr_t pointer_type) {
	if (target_addr_old == 0) {
		*(intptr_t*)(addr_new) = 0;
		return true;
	} else {
		uintptr_t diff = target_addr_old - address_lookup->memory_base;
		if (diff < 0 || diff >= address_lookup->memory_size) {
			// points outside of our memory, perhaps into scratch space
			// Keep pointing to whatever it was, if scratch space it doesn't matter but if it is to another plugin or the engine it will stay the same.
			*(intptr_t*)(addr_new) = target_addr_old;
			return false;
		}

		intptr_t target_addr_new;

		// Check the fast hash map first incase this pointer points to the head of a registered object.
		target_addr_new = get_address_to_block(address_lookup, target_addr_old, pointer_type);
		if (target_addr_new != 0) {
			*(intptr_t*)(addr_new) = target_addr_new;
			return true;
		}

		// Check the slower structure for pointers inside registered objects. e.g. a pointer to arbitrary entry in an array.
		target_addr_new = get_address_inside_block(address_lookup, target_addr_old, pointer_type);
		if (target_addr_new != 0) {
			*(intptr_t*)(addr_new) = target_addr_new;
			return true;
		}
	}

	return false;
}

bool next_reloadable_pointer(Pointer *pointer, MemoryMapper *mapper) {
	// The cursor should always be on the pointer that has the lowest target address
	intptr_t lowest_target_address = INTPTR_MAX;
	int best_pointer_index = -1;
	for (unsigned i = 0; i < mapper->pointer_count; ++i) {
		intptr_t addr_old = mapper->pointers[i].addr_old;
		intptr_t target_addr_old = *(intptr_t*)addr_old;
		if (target_addr_old < lowest_target_address && mapper->pointers[i].pointer_type > __RELOAD_TYPE__base_type) {
			lowest_target_address = target_addr_old;
			best_pointer_index = (int) i;
		}
	}
	if (best_pointer_index >= 0) {
		*pointer = mapper->pointers[best_pointer_index];
		mapper->pointers[best_pointer_index] = mapper->pointers[--mapper->pointer_count];
		return true;
	}

	return false;
}

//////////// POINTERS ////////////
void set_pointer(MemoryMapper *mapper, intptr_t base_old, intptr_t offset_old, intptr_t addr_new, intptr_t pointer_type, char *debug_tag = 0) {
	if (offset_old < 0) { // If offset_old is -1 then this "thing" didn't exist in the old space.
		*(intptr_t*)(addr_new) = 0; // Initiate the pointer in new space to point to null. TODO(kalle): Point to something else, like 0xDEADBEEF?
		return;
	}

	intptr_t addr_old = base_old + offset_old;

	AddressLookup *lookup = mapper->address_lookup;
	// Insert the pointer in the lookup, in case someone is pointing to this pointer. A pointer value is generic, i.e. it's just an address which can't change internally.
	add_node_to_lookup(lookup, addr_old, sizeof(void*), addr_new, sizeof(void*), __RELOAD_TYPE__generic, debug_tag);

	intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.
	bool success = try_set_pointer_value(lookup, target_addr_old, addr_new, pointer_type);
	if (!success) {
		intptr_t diff = target_addr_old - lookup->memory_base;
		if (diff < 0|| diff >= (intptr_t)lookup->memory_size) {
			return; // The pointer points to something outside our game memory, perhaps the scratch space?
		}

		if (mapper->pointer_count == MAX_POINTERS) {
			for (unsigned i = 0; i < mapper->pointer_count; ++i) {
				Pointer *pointer = mapper->pointers + i;
				intptr_t addr_old = pointer->addr_old;

				intptr_t target_addr_old = *(intptr_t*)(addr_old); // Chase the pointer to find whatever was pointed to in the old space, i.e. our target.
				bool success = try_set_pointer_value(lookup, target_addr_old, addr_new, pointer->pointer_type);
				if (success) {
					mapper->pointers[i--] = mapper->pointers[--mapper->pointer_count];
				}
			}
		}

		ASSERT(mapper->pointer_count < MAX_POINTERS, "Too many unmapped pointers!");

		// This pointer could be the _only_ thing that knows what it points to, so store the pointer type in case we need it.
		Pointer pointer = {addr_old, addr_new, pointer_type, debug_tag};
		mapper->pointers[mapper->pointer_count++] = pointer;
	}
}
inline void set_pointer_array(MemoryMapper *mapper, intptr_t base_old, intptr_t offset_old, intptr_t addr_new, u64 count, intptr_t pointer_type, char *debug_tag = 0) {
	if (offset_old < 0) { // If offset_old is -1 then this "thing" didn't exist in the old space.
		for (unsigned i = 0; i < count; ++i) {
			*(intptr_t*)(addr_new) = 0; // Initiate the pointer in new space to point to null. TODO(kalle): Point to something else, like 0xDEADBEEF?
			addr_new++;
		}
	} else {
		intptr_t addr_old = base_old + offset_old;
		for (unsigned i = 0; i < count; ++i) {
			set_pointer(mapper, addr_old, 0, addr_new, pointer_type, debug_tag);

			addr_old += sizeof(void*);
			addr_new += sizeof(void*);
		}
	}
}

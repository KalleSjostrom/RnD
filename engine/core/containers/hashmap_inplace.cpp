#include "hashmap_inplace.h"

#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

#define invalid_key 0

void _inplace_move_into_place(void *hashmap, uint64_t key, int offset) {
	InplaceHashHeader &header = _inplace_hash_header(hashmap);

	int capacity = header.capacity;
	int bucket_size = _inplace_hash_bucket_size(hashmap);

	size_t value_offset = header.value_offset;
	size_t element_size = header.element_size;
	size_t value_size = element_size - value_offset;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	uint64_t target_index = (lookup_key + offset) & hash_mask; // Where it was actually stored, i.e. our target to fill

	// Start at i == 1, since the thing we are removing is at i == 0.
	for (int i = 1; i < capacity; ++i) {
		// Look at our next neighbors
		uint64_t index = (lookup_key + offset + i) & hash_mask;
		char *entry = (char*)hashmap + index * element_size;

		uint64_t *entry_key = (uint64_t*)entry;
		if (*entry_key == invalid_key)
			return; // Found empty slot, we are done.

		uint64_t optimal_index = (*entry_key * bucket_size) & hash_mask; // Where it wants to be stored

		// We need to make sure that this entry is found when doing a lookup.
		// The slot index, or (target index) can't be further 'back' than my optimal index!
		// For example: if my optimal index is 3, than I shouldn't be able to move from 4 to 2.
		// This must also be true at the boundaries, if my optimal index is 3, than I shouldn't be able to move from 4 to 127.
		// After this move, the thing moved should always be closer to its optimal index than before!
		// This is solved by comparing the distance the entry would need to jump to get to its optimal position versus the target position.

		int64_t max_jump_distance = ((index - optimal_index) + capacity) & hash_mask;
		int64_t jump_distance     = ((index - target_index)  + capacity) & hash_mask;

		if (jump_distance <= max_jump_distance) {
			// Move to the place where the removed one was
			memmove((char*)hashmap + target_index * value_size, (char*)hashmap + index * value_size, value_size);
			// Clear out the old entry.
			*entry_key = invalid_key;
			// Now we might have created a new 'hole', so lets keep on searching with this index being the new target.
			target_index = index;
		}
	}
}

bool _inplace_hash_remove(void *hashmap, uint64_t key, void *outvalue) {
	ASSERT(key != invalid_key, "can't remove with invalid key!");

	InplaceHashHeader &header = _inplace_hash_header(hashmap);

	int capacity = header.capacity;
	int bucket_size = _inplace_hash_bucket_size(hashmap);

	size_t value_offset = header.value_offset;
	size_t element_size = header.element_size;
	size_t value_size = element_size - value_offset;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	for (int i = 0; i < capacity; i++) {
		uint64_t index = (lookup_key + i) & hash_mask;
		char *entry = (char*)hashmap + index * element_size;

		uint64_t *entry_key = (uint64_t*)entry;
		if (*entry_key == key) {
			memmove(outvalue, (char*)entry + value_offset, value_size);
			*entry_key = invalid_key;
			_inplace_move_into_place(hashmap, key, i);
			return true;
		}
	}
	return false;
}

void *_inplace_hash_lookup(void *hashmap, uint64_t key) {
	ASSERT(key != invalid_key, "can't lookup with invalid key!");

	InplaceHashHeader &header = _inplace_hash_header(hashmap);

	int capacity = header.capacity;
	int bucket_size = _inplace_hash_bucket_size(hashmap);

	size_t value_offset = header.value_offset;
	size_t element_size = header.element_size;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	for (int i = 0; i < capacity; i++) {
		uint64_t index = (lookup_key + i) & hash_mask;
		char *entry = (char*)hashmap + index * element_size;

		uint64_t *entry_key = (uint64_t*)entry;
		if (*entry_key == invalid_key || *entry_key == key) {
			return entry;
		}
	}

	return 0;
}

void _inplace_hash_add(void *hashmap, uint64_t key, void *value) {
	ASSERT(key != invalid_key, "can't add with invalid key!");

	InplaceHashHeader &header = _inplace_hash_header(hashmap);

	size_t value_offset = header.value_offset;
	size_t element_size = header.element_size;
	size_t value_size = element_size - value_offset;

	char *entry = (char*)_inplace_hash_lookup(hashmap, key);
	// ASSERT(entry, "Critical! Hash map shouldn't every be completly full!");
	_inplace_hash_header(hashmap).count++;

	uint64_t *entry_key = (uint64_t*)entry;
	*entry_key = key;
	memmove(entry + value_offset, value, value_size);
}

void *_inplace_hash_make(Allocator *allocator, int capacity, size_t value_offset, size_t element_size) {
	InplaceHashHeader *header = (InplaceHashHeader*) allocate(allocator, capacity * element_size + sizeof(InplaceHashHeader));
	if (header) {
		header->allocator = allocator;
		header->count = 0;
		header->capacity = capacity;

		header->value_offset = value_offset;
		header->element_size = element_size;

		char *base = (char*)(header + 1);
		for (int i = 0; i < capacity; ++i) {
			uint64_t *entry_key = (uint64_t *)(base + i * element_size);
			*entry_key = invalid_key;
		}
		return base;
	}
	return 0;
}

void *_inplace_hash_grow(void *hashmap, int increment) {
	InplaceHashHeader &header = _inplace_hash_header(hashmap);

	int previous_capacity = header.capacity;
	int next_capacity = next_power_of_2(previous_capacity);
	int needed = previous_capacity + increment;
	int capacity = next_capacity > needed ? next_capacity : next_power_of_2(needed + 1);

	Allocator *allocator = header.allocator;
	InplaceHashHeader *new_header = (InplaceHashHeader*) allocate(allocator, capacity * header.element_size + sizeof(InplaceHashHeader));
	if (new_header) {
		*new_header = header;

		new_header->count = 0;
		new_header->capacity = capacity;

		size_t value_offset = new_header->value_offset;
		size_t element_size = new_header->element_size;

		char *base = (char*)(new_header + 1);
		for (int i = 0; i < capacity; ++i) {
			uint64_t *entry_key = (uint64_t *)(base + i * element_size);
			*entry_key = invalid_key;
		}

		for (int i = 0; i < previous_capacity; ++i) {
			char *entry = (char*)hashmap + i * element_size;
			uint64_t *entry_key = (uint64_t*)entry;
			if (*entry_key != invalid_key) {
				_inplace_hash_add(base, *entry_key, entry + value_offset);
			}
		}

		deallocate(allocator, _inplace_hash_raw_pointer(hashmap));
		return base;
	}
	return 0;
}

void _inplace_hash_destroy(void *hashmap) {
	Allocator *allocator = _inplace_hash_header(hashmap).allocator;
	deallocate(allocator, _inplace_hash_raw_pointer(hashmap));
}
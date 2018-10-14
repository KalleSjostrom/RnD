#include "hashmap.h"

#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

// TODO(kalle): Add to header? Always force 2?
#define _hash_bucket_size(h) 2

#define invalid_key 0

void _move_into_place(HashEntry *hashmap, uint64_t key, int offset) {
	int bucket_size = _hash_bucket_size(hashmap);
	int capacity = _hash_header(hashmap).capacity;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	uint64_t target_index = (lookup_key + offset) & hash_mask; // Where it was actually stored, i.e. our target to fill

	// Start at i == 1, since the thing we are removing is at i == 0.
	for (int i = 1; i < capacity; ++i) {
		// Look at our next neighbors
		uint64_t index = (lookup_key + offset + i) & hash_mask;
		HashEntry *entry = hashmap + index;

		if (entry->key == invalid_key)
			return; // Found empty slot, we are done.

		uint64_t optimal_index = (entry->key * bucket_size) & hash_mask; // Where it wants to be stored

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
			hashmap[target_index] = hashmap[index];
			// Clear out the old entry.
			hashmap[index].key = invalid_key;
			// Now we might have created a new 'hole', so lets keep on searching with this index being the new target.
			target_index = index;
		}
	}
}

HashEntry *_hash_grow(HashEntry *hashmap, int increment) {
	HashHeader *header = &_hash_header(hashmap);

	int previous_capacity = header->capacity;
	int next_capacity = next_power_of_2(previous_capacity);
	int needed = previous_capacity + increment;
	int capacity = next_capacity > needed ? next_capacity : next_power_of_2(needed + 1);

	Allocator *allocator = header->allocator;
	HashHeader *new_header = (HashHeader*) allocate(allocator, capacity * sizeof(HashEntry) + sizeof(HashHeader));
	if (new_header) {
		*new_header = *header; // Clone the previous header
		new_header->count = 0;
		new_header->capacity = capacity;

		uint64_t _invalid_key = invalid_key;
		HashEntry *base = (HashEntry *)(new_header + 1);
		for (int i = 0; i < capacity; ++i) {
			base[i].key = invalid_key;
		}

		for (int i = 0; i < previous_capacity; ++i) {
			HashEntry *entry = hashmap + i;
			if (entry->key != invalid_key) {
				hash_add(base, entry->key, entry->value);
			}
		}

		deallocate(allocator, header);
		return base;
	}
	return 0;
}


HashEntry *hash_make(Allocator *allocator, int capacity) {
	HashHeader *header = (HashHeader*) allocate(allocator, capacity * sizeof(HashEntry) + sizeof(HashHeader));
	if (header) {
		header->allocator = allocator;
		header->count = 0;
		header->capacity = capacity;

		uint64_t _invalid_key = invalid_key;
		HashEntry *base = (HashEntry*)(header + 1);
		for (int i = 0; i < capacity; ++i) {
			base[i].key = invalid_key;
		}
		return base;
	}
	return 0;
}
void hash_destroy(HashEntry *hashmap) {
	Allocator *allocator = _hash_header(hashmap).allocator;
	deallocate(allocator, &_hash_header(hashmap));
}

void _hash_add(HashEntry *hashmap, uint64_t key, uint64_t value) {
	HashEntry *entry = hash_lookup(hashmap, key);
	// ASSERT(entry, "Critical! Hash map shouldn't every be completly full!");
	_hash_header(hashmap).count++;
	entry->key = key;
	entry->value = value;
}
HashEntry *hash_lookup(HashEntry *hashmap, uint64_t key) {
	int bucket_size = _hash_bucket_size(hashmap);
	int capacity = _hash_header(hashmap).capacity;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	for (int i = 0; i < capacity; i++) {
		uint64_t index = (lookup_key + i) & hash_mask;
		HashEntry *entry = hashmap + index;

		if (entry->key == invalid_key || entry->key == key) {
			return entry;
		}
	}

	return 0;
}
bool hash_remove(HashEntry *hashmap, uint64_t key, uint64_t *outvalue) {
	int bucket_size = _hash_bucket_size(hashmap);
	int capacity = _hash_header(hashmap).capacity;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	for (int i = 0; i < capacity; i++) {
		uint64_t index = (lookup_key + i) & hash_mask;
		HashEntry *entry = hashmap + index;
		if (entry->key == key) {
			if (outvalue)
				*outvalue = entry->value;
			entry->key = invalid_key;
			_move_into_place(hashmap, key, i);
			return true;
		}
	}
	return false;
}

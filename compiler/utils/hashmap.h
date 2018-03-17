#pragma once

#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

#define _hash_raw_pointer(h) ((unsigned *) (h) - 2)

#define _hash_bucket_size(h) 2
#define _hash_capacity(h)    _hash_raw_pointer(h)[0]
#define _hash_count(h)       _hash_raw_pointer(h)[1]

#define hash_capacity(h)    ((h)==0 ? 0 : _hash_capacity(h))
#define hash_count(h)       ((h)==0 ? 0 : _hash_count(h))

static void _move_into_place(void *hashmap, uint64_t key, size_t key_size, unsigned offset, size_t value_size) {
	unsigned bucket_size = _hash_bucket_size(hashmap);
	unsigned capacity = _hash_capacity(hashmap);

	uint64_t element_size = key_size + value_size;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	uint64_t target_index = (lookup_key + offset) & hash_mask; // Where it was actually stored, i.e. our target to fill

	// Start at i == 1, since the thing we are removing is at i == 0.
	for (unsigned i = 1; i < capacity; ++i) {
		// Look at our next neighbors
		uint64_t index = (lookup_key + offset + i) & hash_mask;
		char *entry = (char*)hashmap + index * element_size;

		uint64_t _invalid_key = invalid_key;
		if (memcmp(entry, &_invalid_key, key_size) == 0)
			return; // Found empty slot, we are done.

		uint64_t *entry_key = (uint64_t*)entry;
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

void *_hash_remove(void *hashmap, uint64_t key, size_t key_size, void *value, size_t value_size) {
	unsigned bucket_size = _hash_bucket_size(hashmap);
	unsigned capacity = _hash_capacity(hashmap);

	size_t element_size = key_size + value_size;

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	for (unsigned i = 0; i < capacity; i++) {
		uint64_t index = (lookup_key + i) & hash_mask;
		char *entry = (char*)hashmap + index * element_size;

		uint64_t *entry_key = (uint64_t*)entry;
		if (memcmp(entry, &key, key_size) == 0) {
			memmove(value, (char*)entry + key_size, value_size);
			*entry_key = invalid_key;
			_move_into_place(hashmap, key, key_size, i, value_size);
			return value;
		}
	}
	return 0;
}

void *_hash_lookup(void *hashmap, uint64_t key, size_t key_size, size_t element_size) {
	unsigned bucket_size = _hash_bucket_size(hashmap);
	unsigned capacity = _hash_capacity(hashmap);

	uint64_t lookup_key = key * bucket_size;
	uint64_t hash_mask = capacity-1;

	for (unsigned i = 0; i < capacity; i++) {
		uint64_t index = (lookup_key + i) & hash_mask;
		char *entry = (char*)hashmap + index * element_size;

		uint64_t _invalid_key = invalid_key;
		if (memcmp(entry, &_invalid_key, key_size) == 0 || memcmp(entry, &key, key_size) == 0) {
			return entry;
		}
	}

	return 0;
}

void _hash_add(void *h, uint64_t key, size_t key_size, void *value, size_t value_size) {
	void *entry = _hash_lookup(h, key, key_size, key_size + value_size);
	assert(entry && "Critical! Hash map shouldn't every be completly full!");
	_hash_count(h)++;
	memmove(entry, &key, key_size);
	memmove((char*)entry + key_size, value, value_size);
}

// TODO(kalle): round the load factor? Make it settable
#define _hash_need_to_grow(h, n)    ((h)==0 || _hash_count(h)+(n) > (unsigned)(_hash_capacity(h) * 0.7f))
#define _hash_ensure_space(h, k, n) (_hash_need_to_grow((h), (n)) ? (h) = _hash_grow((h), (n), sizeof(k), sizeof(*(h))) : 0)

#define hash_init(h, k, n) (_hash_ensure_space((h), k, n))
#define hash_add(h, k, v) (_hash_ensure_space((h), k, 1), _hash_add((h), (k), sizeof(k), &(v), sizeof(v)))
#define hash_lookup(h, k) ((h) == 0 ? 0 : _hash_lookup((h), (k), sizeof(k), sizeof(*(h))))

static void *_hash_grow(void *h, unsigned increment, size_t key_size, size_t itemsize) {
	unsigned previous_capacity = hash_capacity(h);
	unsigned next_capacity = next_power_of_2(previous_capacity);
	unsigned needed = previous_capacity + increment;
	unsigned capacity = next_capacity > needed ? next_capacity : next_power_of_2(needed + 1);
	// unsigned *p = (unsigned*) realloc(h ? _hash_raw_pointer(h) : 0, capacity * itemsize + sizeof(unsigned)*2);
	unsigned *p = (unsigned*) malloc(capacity * itemsize + sizeof(unsigned)*2);
	if (p) {
		p[1] = 0;
		p[0] = capacity;
		
		uint64_t _invalid_key = invalid_key;
		char *base = (char*)(p+2);
		for (uint64_t i = 0; i < capacity; ++i) {
			char *entry = base + i * itemsize;
			memmove(entry, &_invalid_key, key_size);
		}

		for (uint64_t i = 0; i < previous_capacity; ++i) {
			char *entry = (char*)h + i * itemsize;
			if (memcmp(entry, &_invalid_key, key_size) != 0) {
				_hash_add(base, *(uint64_t*)entry, key_size, entry + key_size, itemsize - key_size);
			}
		}
	}
	return p + 2;
}

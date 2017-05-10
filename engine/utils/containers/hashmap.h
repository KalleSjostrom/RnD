// TODO(kalle): Could the key, value be packed into 32 bits for less memory? Is it even beneficial?
struct HashEntry {
	TKEY key; // e.g. EntityRef. key == 0 means invalid
	TVALUE value; // e.g. index into the component instance array.
};

struct HashMap {
	HashEntry *entries;
	unsigned count;
	TKEY invalid_key;
};

// Returns the entry with the given key, if previously added or a free slot in which to insert something.
HashEntry *lookup(HashMap &hashmap, TKEY key) {
	TKEY lookup_key = key * BUCKET_SIZE;
	unsigned hash_mask = hashmap.count-1;

	for (unsigned offset = 0; offset < hashmap.count; offset++) {
		unsigned index = (lookup_key + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if ((entry->key == hashmap.invalid_key) || (entry->key == key)) {
			return entry;
		}
	}

	return 0;
}
// Associates the value with the given key in the hashmap.
HashEntry *add(HashMap &hashmap, TKEY key, TVALUE value) {
	HashEntry *entry = lookup(hashmap, key);
	entry->key = key; entry->value = value;
	return entry;
}

inline void _move_into_place(HashMap &hashmap, TKEY key, unsigned offset) {
	TKEY lookup_key = key * BUCKET_SIZE;
	unsigned hash_mask = hashmap.count-1;

	unsigned target_hash = (lookup_key + offset) & hash_mask; // Where it was actually stored, i.e. our target to fill

	// Start at i == 1, since the thing we are removing is at i == 0.
	for (unsigned i = 1; i < hashmap.count; ++i) {
		// Look at our next neighbors
		unsigned hash = (lookup_key + offset + i) & hash_mask;
		HashEntry *entry = hashmap.entries + hash;

		if (entry->key == hashmap.invalid_key)
			return; // Found empty slot, we are done.

		unsigned optimal_hash = (entry->key * BUCKET_SIZE) & hash_mask; // Where it wants to be stored

		// We need to make sure that this entry is found when doing a lookup.
		// The slot hash, or (target hash) can't be further 'back' than my optimal hash!
		// For example: if my optimal hash is 3, than I shouldn't be able to move from 4 to 2.
		// This must also be true at the boundaries, if my optimal hash is 3, than I shouldn't be able to move from 4 to 127.
		// After this move, the thing moved should always be closer to its optimal hash than before!
		// This is solved by comparing the distance the entry would need to jump to get to its optimal position versus the target position.

		int max_jump_distance = ((hash - optimal_hash) + hashmap.count) & hash_mask;
		int jump_distance     = ((hash - target_hash)  + hashmap.count) & hash_mask;

		// ASSERT(max_jump_distance >= 0);
		// ASSERT(jump_distance >= 0);

		if (jump_distance <= max_jump_distance) {
			// Move to the place where the removed one was
			hashmap.entries[target_hash] = hashmap.entries[hash];
			// Clear out the old entry.
			hashmap.entries[hash].key = hashmap.invalid_key;
			// Now we might have created a new 'hole', so lets keep on searching with this hash being the new target.
			target_hash = hash;
		}
	}
}
TVALUE remove(HashMap &hashmap, TKEY key) {
	TKEY lookup_key = key * BUCKET_SIZE;
	unsigned hash_mask = hashmap.count-1;
	TVALUE value;

	for (unsigned offset = 0; offset < hashmap.count; ++offset) {
		unsigned index = (lookup_key + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if (entry->key == key) {
			value = entry->value;
			entry->key = hashmap.invalid_key;
			_move_into_place(hashmap, key, offset);
			return value;
		}
	}
	ASSERT(false); // Key not in hashmap
	return value;
}
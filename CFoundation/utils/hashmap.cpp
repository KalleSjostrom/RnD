/*
Based on the "Hacker's Delight" version:
unsigned clp2(unsigned x) {
   x = x - 1;
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   x = x | (x >>16);
   return x + 1;
}*/
#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

#define DEFAULT_BUCKET_SIZE 2
#define HASH_SIZE_FOR(size) (next_power_of_2((size) * DEFAULT_BUCKET_SIZE)) // NOTE(kalle): We use a default of 2 as bucket_size!
#define HASH_SIZE_USING_BUCKET_SIZE(size, bucket_size) (next_power_of_2((size) * bucket_size))

/*
This is a simple hash map with open addressing (internally chained) and linear probing.

It is designed for storing a link between an entity and a component instance index.
NOTE(kalle): If using for anything else, make sure the usage agrees with the design.

The hash function is a simple & operation which requires the hash size to be a power of two.

There is an option to use a "bucket size", which will increase the memory footprint but lower risk of hash collisions (load factor).
If bucket size is 2, the storage array will be twice as large and have the following format
hash:      1     2     3     4
bucket: | 0 1 | 2 3 | 4 5 | 6 7 |
This means that we have one extra slot in place for one collision. Since we use this for entities which (for the most part) have monotonically increasing ids, having more than 2 collisions is unlikely.

NOTE(kalle): Remove is by far the most expensive operation to do. However, since remove is a rare operation (in contrast to add and lookup), this shouldn't matter much.
Remove is slow (or potentially slow) because we might need to shift entries that should have taken the removed entry's place.
NOTE(kalle): Right now it moves them in a bubble up sort of way, i.e. one at a time, no searching for the best fit, and moving that directly.
*/

// TODO(kalle): Could the key & value be packed into 32 bits for less memory? Is it even beneficial?
struct HashEntry {
	union {
		unsigned key; // e.g. EntityRef.  key == 0 means invalid
		unsigned hash; // Used when hashing outside of the hashmap, e.g. for broadphases
	};
	union {
		unsigned value; // e.g. index into the component instance array.
		unsigned handle; // used by broadphase
	};
};
struct HashMap {
	HashEntry *entries;
	// NOTE(kalle): Must be a power of two! Use HASH_SIZE_FOR above
	unsigned count;
	union {
		unsigned invalid_key;
		unsigned invalid_hash;
	};
	unsigned bucket_size;
};

// NOTE(kalle): We want to avoid high load factor so, by default, we use a bucket_size > 1
void hash_init(HashMap &hashmap, HashEntry *entries, unsigned count, unsigned invalid_key, unsigned bucket_size = DEFAULT_BUCKET_SIZE) {
	hashmap.entries = entries;
	hashmap.count = count;
	hashmap.invalid_key = invalid_key;
	hashmap.bucket_size = bucket_size;

	if (invalid_key) { // Assumes that the memory pointed to by entries is already zeroed
		for (unsigned i = 0; i < count; ++i) {
			entries[i].key = invalid_key;
		}
	}
}

// Returns the entry with the given key, if previously added or a free slot in which to insert something.
HashEntry *hash_lookup(HashMap &hashmap, unsigned key) {
	unsigned lookup_key = key * hashmap.bucket_size;
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
HashEntry *hash_add(HashMap &hashmap, unsigned key, unsigned value) {
	HashEntry *entry = hash_lookup(hashmap, key);
	ASSERT(entry, "No space in hashmap! (key=%u, value=%u)", key, value);
	entry->key = key; entry->value = value;
	return entry;
}

HashEntry *hash_add_handle(HashMap &hashmap, unsigned hash, unsigned handle) {
	unsigned lookup_hash = hash * hashmap.bucket_size;
	unsigned hash_mask = hashmap.count-1;

	for (unsigned offset = 0; offset < hashmap.count; offset++) {
		unsigned index = (lookup_hash + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if (entry->hash == hashmap.invalid_hash) {
			entry->hash = hash;
			entry->handle = handle;
			return entry;
		}
	}

	ASSERT(false, "No space in hashmap! (hash=%u, handle=%u)", hash, handle);
	return 0;
}

inline void _move_into_place(HashMap &hashmap, unsigned key, unsigned offset) {
	unsigned lookup_key = key * hashmap.bucket_size;
	unsigned hash_mask = hashmap.count-1;

	unsigned target_hash = (lookup_key + offset) & hash_mask; // Where it was actually stored, i.e. our target to fill

	// Start at i == 1, since the thing we are removing is at i == 0.
	for (unsigned i = 1; i < hashmap.count; ++i) {
		// Look at our next neighbors
		unsigned hash = (lookup_key + offset + i) & hash_mask;
		HashEntry *entry = hashmap.entries + hash;

		if (entry->key == hashmap.invalid_key)
			return; // Found empty slot, we are done.

		unsigned optimal_hash = (entry->key * hashmap.bucket_size) & hash_mask; // Where it wants to be stored

		// We need to make sure that this entry is found when doing a lookup.
		// The slot hash, or (target hash) can't be further 'back' than my optimal hash!
		// For example: if my optimal hash is 3, than I shouldn't be able to move from 4 to 2.
		// This must also be true at the boundaries, if my optimal hash is 3, than I shouldn't be able to move from 4 to 127.
		// After this move, the thing moved should always be closer to its optimal hash than before!
		// This is solved by comparing the distance the entry would need to jump to get to its optimal position versus the target position.

		int max_jump_distance = ((hash - optimal_hash) + hashmap.count) & hash_mask;
		int jump_distance     = ((hash - target_hash)  + hashmap.count) & hash_mask;

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
unsigned hash_remove(HashMap &hashmap, unsigned key) {
	unsigned lookup_key = key * hashmap.bucket_size;
	unsigned hash_mask = hashmap.count-1;
	unsigned value;

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
	ASSERT(false, "Key not found! (key=%u)", key);
	return 0XFFFFFFFF;
}

// Removes a non-unique prehashed key by comparing the handle value for uniqueness
// Used by broadphase atm, where the positional hash value is non-unique
void hash_remove_handle(HashMap &hashmap, unsigned hash, unsigned handle) {
	unsigned lookup_hash = hash * hashmap.bucket_size;
	unsigned hash_mask = hashmap.count-1;

	for (unsigned offset = 0; offset < hashmap.count; ++offset) {
		unsigned index = (lookup_hash + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if (entry->handle == handle) {
			entry->hash = hashmap.invalid_hash;
			_move_into_place(hashmap, hash, offset);
			return;
		}
	}
	ASSERT(false, "Hash not found! (hash=%u, handle=%u)", hash, handle);
}
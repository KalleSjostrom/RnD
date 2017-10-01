#include "hashmap.h"

// NOTE(kalle): We want to avoid high load factor so, by default, we use a bucket_size > 1
void hash_init(HashMap &hashmap, HashEntry *entries, u32 count, u32 invalid_key, u32 bucket_size = DEFAULT_BUCKET_SIZE) {
	hashmap.entries = entries;
	hashmap.count = count;
	hashmap.invalid_key = invalid_key;
	hashmap.bucket_size = bucket_size;

	if (invalid_key) { // Assumes that the memory pointed to by entries is already zeroed
		for (u32 i = 0; i < count; ++i) {
			entries[i].key = invalid_key;
		}
	}
}

// Returns the entry with the given key, if previously added or a free slot in which to insert something.
HashEntry *hash_lookup(HashMap &hashmap, u32 key, bool check_invalid = true) {
	u32 lookup_key = key * hashmap.bucket_size;
	u32 hash_mask = hashmap.count-1;

	for (u32 offset = 0; offset < hashmap.count; offset++) {
		u32 index = (lookup_key + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if ((check_invalid && entry->key == hashmap.invalid_key) || (entry->key == key)) {
			return entry;
		}
	}

	return 0;
}
// Associates the value with the given key in the hashmap.
HashEntry *hash_add(HashMap &hashmap, u32 key, u32 value) {
	HashEntry *entry = hash_lookup(hashmap, key);
	ASSERT(entry, "No space in hashmap! (key=%u, value=%u)", key, value);
	entry->key = key; entry->value = value;
	return entry;
}

HashEntry *hash_add_handle(HashMap &hashmap, u32 hash, u32 handle) {
	u32 lookup_hash = hash * hashmap.bucket_size;
	u32 hash_mask = hashmap.count-1;

	for (u32 offset = 0; offset < hashmap.count; offset++) {
		u32 index = (lookup_hash + offset) & hash_mask;

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

inline void _move_into_place(HashMap &hashmap, u32 key, u32 offset) {
	u32 lookup_key = key * hashmap.bucket_size;
	u32 hash_mask = hashmap.count-1;

	u32 target_hash = (lookup_key + offset) & hash_mask; // Where it was actually stored, i.e. our target to fill

	// Start at i == 1, since the thing we are removing is at i == 0.
	for (u32 i = 1; i < hashmap.count; ++i) {
		// Look at our next neighbors
		u32 hash = (lookup_key + offset + i) & hash_mask;
		HashEntry *entry = hashmap.entries + hash;

		if (entry->key == hashmap.invalid_key)
			return; // Found empty slot, we are done.

		u32 optimal_hash = (entry->key * hashmap.bucket_size) & hash_mask; // Where it wants to be stored

		// We need to make sure that this entry is found when doing a lookup.
		// The slot hash, or (target hash) can't be further 'back' than my optimal hash!
		// For example: if my optimal hash is 3, than I shouldn't be able to move from 4 to 2.
		// This must also be true at the boundaries, if my optimal hash is 3, than I shouldn't be able to move from 4 to 127.
		// After this move, the thing moved should always be closer to its optimal hash than before!
		// This is solved by comparing the distance the entry would need to jump to get to its optimal position versus the target position.

		i32 max_jump_distance = (i32)(((hash - optimal_hash) + hashmap.count) & hash_mask);
		i32 jump_distance     = (i32)(((hash - target_hash)  + hashmap.count) & hash_mask);

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

u32 hash_remove(HashMap &hashmap, u32 key) {
	u32 lookup_key = key * hashmap.bucket_size;
	u32 hash_mask = hashmap.count-1;
	u32 value;

	for (u32 offset = 0; offset < hashmap.count; ++offset) {
		u32 index = (lookup_key + offset) & hash_mask;

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

bool hash_try_remove(HashMap &hashmap, u32 key, u32 *value = 0) {
	u32 lookup_key = key * hashmap.bucket_size;
	u32 hash_mask = hashmap.count-1;

	for (u32 offset = 0; offset < hashmap.count; ++offset) {
		u32 index = (lookup_key + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if (entry->key == hashmap.invalid_key) {
			return false; // The given key was not in the hashmap
		}

		if (entry->key == key) {
			if (value)
				*value = entry->value;

			entry->key = hashmap.invalid_key;
			_move_into_place(hashmap, key, offset);
			return true;
		}
	}
	return false;
}

// Removes a non-unique prehashed key by comparing the handle value for uniqueness
// Used by broadphase atm, where the positional hash value is non-unique
void hash_remove_handle(HashMap &hashmap, u32 hash, u32 handle) {
	u32 lookup_hash = hash * hashmap.bucket_size;
	u32 hash_mask = hashmap.count-1;

	for (u32 offset = 0; offset < hashmap.count; ++offset) {
		u32 index = (lookup_hash + offset) & hash_mask;

		HashEntry *entry = hashmap.entries + index;
		if (entry->handle == handle) {
			entry->hash = hashmap.invalid_hash;
			_move_into_place(hashmap, hash, offset);
			return;
		}
	}
	ASSERT(false, "Hash not found! (hash=%u, handle=%u)", hash, handle);
}

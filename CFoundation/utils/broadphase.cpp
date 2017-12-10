#define BROADPHASE_INVALID_HASH 0xFFFFFFFF
#define BROADPHASE_INVALID_HANDLE 0xFFFFFFFF

/*
This is a broadphase used only by the interact/interactable system right now. TODO(kalle): Make it more general

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
struct BroadphaseEntry {
	Vector3 position;
	float radius;
	unsigned hash;
	unsigned handle;
};
struct Broadphase {
	HashMap hashmap; // Used to map handles with positional hashes
	BroadphaseEntry *entries; // All the BroadphaseEntry in a linear array. An entry in here will keep it's position (index) throughout it's lifetime, so this array might have holes in it.
	unsigned next_free_handle; // The next free index in the above array.
};

inline unsigned _calculate_hash(unsigned hash_mask, int x, int y) {
	static int xprime = 492876863; /* arbitrary large prime */
	static int yprime = 633910099; /* arbitrary large prime */
	return ((x * xprime) ^ (y * yprime)) & hash_mask;
}
inline unsigned _calculate_hash(unsigned hash_mask, Vector3 &position) {
	int x = (int)floor(position.x);
	int y = (int)floor(position.y);
	return _calculate_hash(hash_mask, x, y);
}

void broadphase_init(Broadphase &broadphase, BroadphaseEntry *entries, HashEntry *hash_entries, unsigned count, unsigned invalid_hash) {
	hash_init(broadphase.hashmap, hash_entries, count, invalid_hash, 1);

	broadphase.entries = entries;
	broadphase.next_free_handle = 0;

	if (invalid_hash) { // Assumes that the memory pointed to by entries is already zeroed
		for (unsigned i = 0; i < count; ++i) {
			entries[i].hash = invalid_hash;
		}
	}
}

void broadphase_remove(Broadphase &broadphase, unsigned handle) {
	BroadphaseEntry *entry = broadphase.entries + handle;
	hash_remove_handle(broadphase.hashmap, entry->hash, handle);
	broadphase.entries[handle].hash = broadphase.hashmap.invalid_key;
}

unsigned broadphase_add(Broadphase &broadphase, Vector3 position, float radius) {
	unsigned handle = broadphase.next_free_handle;

	HashMap &hashmap = broadphase.hashmap;
	{ // Update the next free handle
		unsigned index = broadphase.next_free_handle;
		for (unsigned i = 0; i < hashmap.count; ++i) {
			index++;
			if (index == hashmap.count) {
				index = 0;
			}

			BroadphaseEntry *entry = broadphase.entries + index;
			if (entry->hash == hashmap.invalid_key) {
				broadphase.next_free_handle = index;
				break;
			}
		}
	}

	BroadphaseEntry *entry = broadphase.entries + handle;
	entry->position = position;
	entry->radius = radius;
	entry->hash = _calculate_hash(hashmap.count - 1, position);
	entry->handle = handle;

	hash_add_handle(hashmap, entry->hash, handle);

	return handle;
}

void broadphase_move(Broadphase &broadphase, unsigned handle, Vector3 position) {
	BroadphaseEntry *entry = broadphase.entries + handle;
	HashMap &hashmap = broadphase.hashmap;

	entry->position = position; // Update the position

	unsigned hash = _calculate_hash(hashmap.count-1, position);
	if (hash == entry->hash) { // Still in same cell
		return;
	}

	hash_remove_handle(broadphase.hashmap, entry->hash, handle); // Remove the binding between old hash and our handle
	entry->hash = hash; // Update to the new hash
	hash_add_handle(broadphase.hashmap, entry->hash, handle); // Add the entry with the new hash
}

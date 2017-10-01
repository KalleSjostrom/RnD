#include "broadphase.h"

inline unsigned broadphase_calculate_hash(unsigned hash_mask, int x, int y) {
	static int xprime = 492876863; /* arbitrary large prime */
	static int yprime = 633910099; /* arbitrary large prime */
	return ((x * xprime) ^ (y * yprime)) & hash_mask;
}
inline unsigned broadphase_calculate_hash(unsigned hash_mask, Vector3 &position) {
	int x = (int)floor(position.x);
	int y = (int)floor(position.y);
	return broadphase_calculate_hash(hash_mask, x, y);
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
	entry->hash = broadphase_calculate_hash(hashmap.count - 1, position);
	entry->handle = handle;

	hash_add_handle(hashmap, entry->hash, handle);

	return handle;
}

void broadphase_move(Broadphase &broadphase, unsigned handle, Vector3 position) {
	BroadphaseEntry *entry = broadphase.entries + handle;
	HashMap &hashmap = broadphase.hashmap;

	entry->position = position; // Update the position

	unsigned hash = broadphase_calculate_hash(hashmap.count-1, position);
	if (hash == entry->hash) { // Still in same cell
		return;
	}

	hash_remove_handle(broadphase.hashmap, entry->hash, handle); // Remove the binding between old hash and our handle
	entry->hash = hash; // Update to the new hash
	hash_add_handle(broadphase.hashmap, entry->hash, handle); // Add the entry with the new hash
}

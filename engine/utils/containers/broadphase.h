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

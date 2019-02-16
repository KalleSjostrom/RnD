#pragma once

#include <stdint.h>
#include "core/memory/allocator.h"

struct HashEntry *_hash_grow(struct HashEntry *hashmap, int increment);
void _hash_add(struct HashEntry *hashmap, uint64_t key, uint64_t value);
void _hash_add(struct HashEntry *hashmap, struct HashEntry *entry, uint64_t key, uint64_t value);

#define _hash_header(h) (((HashHeader *)(h))[-1])
// TODO(kalle): round the load factor? Make it settable
#define _hash_need_to_grow(h, n) (_hash_header(h).count+(n) > (int)(_hash_header(h).capacity * 0.7f))
#define _hash_ensure_space(h, n) (_hash_need_to_grow((h), (n)) ? (h) = _hash_grow((h), (n)) : 0)

/** \addtogroup Hashmap
 * Hashmap is based on the concept of 'strechy_buffer'.
 * The idea is to store a header before the pointer to the hashmap containing count and capacity.
 * We also store the allocator used when we need to realloc.
 *
 * Some characteristics:
 * - Open addressing (internally chained).
 * - Linear probing.
 * - Hash function is the '&' operator, (capacity will always be rounded up to the nearest power of two.)
 * - Load-factor of 70%.
 * - Fixed bucket size of 2. (see below)
 *
 * This hashmap will only store a 8 byte key and 8 byte value.
 *
 * Usage examples:
 * \code {.cpp}
 * // Initialization:
 * HashEntry *hashmap = hash_init(allocator, hashmap, 32);
 * \endcode
 * \code {.cpp}
 * // Adding:
 * hash_add(hashmap, key, value);
 * \endcode
 * \code {.cpp}
 * // Lookup:
 * HashEntry *entry = hash_lookup(hashmap, key);
 * \endcode
 * \code {.cpp}
 * // Remove:
 * uint64_t removed_value;
 * if (hash_removed(hashmap, key, &removed_value)) { printf("successfully removed"); }
 * \endcode
 *
 * There is an option to use a "bucket size", which will increase the memory footprint but lower risk of hash collisions (load factor).
 * If bucket size is 2, the storage array will be twice as large and have the following format:
 * \code {.cpp}
 * hash:      1     2     3     4
 * bucket: | 0 1 | 2 3 | 4 5 | 6 7 |
 * \endcode
 * This means that we have one extra slot in place for one collision.
 *
 * NOTE: Remove is by far the most expensive operation to do. It is slow (or potentially slow) because we might need to shift entries that should have taken the removed entry's place.<br>
 * NOTE: Right now it moves them in a bubble up sort of way, i.e. one at a time, no searching for the best fit, and moving that directly.
 *  @{
 */
/// Key/value pair to use as a hashmap
struct HashEntry {
	uint64_t key;
	uint64_t value;
};

/// The header of the hashmap.
struct HashHeader {
	/// Allocator used when allocating memory
	Allocator *allocator;
	/// Returns the number of elements we can store before we need to allocate more memory
	int capacity;
	/// Returns the number of elements currently stored in the hashmap
	int count;
};

/// Creates a new hashmap. The capacity will get rounded up to the nearest power of two.
HashEntry *hash_make(Allocator *allocator, int capacity);
/// Deallocates the hashmap memory
void hash_destroy(HashEntry *hashmap);
/// Adds a new key/value pair
#define hash_add(hashmap, key, value) (_hash_ensure_space(hashmap, 1), _hash_add(hashmap, key, value))
/// Adds a previously looked up entry
#define hash_add_entry(hashmap, entry, key, value) (_hash_ensure_space(hashmap, 1), _hash_add(hashmap, entry, key, value))
/// Returns the pair mapped to key
HashEntry *hash_lookup(HashEntry *hashmap, uint64_t key);
/// Tries to removes the value mapped to key if it exists. If so, it stores a copy of it in outvalue and returns true.
bool hash_remove(HashEntry *hashmap, uint64_t key, uint64_t *outvalue = 0);
/** @}*/

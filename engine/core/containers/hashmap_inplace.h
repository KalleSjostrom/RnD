#pragma once

#include <stdint.h>
#include "core/memory/allocator.h"

#define DECLARE_HASH_ENTRY(v) uint64_t key; v value;

void *_inplace_hash_grow(void *h, int increment);
void _inplace_hash_add(void *h, uint64_t key, void *value);
void *_inplace_hash_lookup(void *hashmap, uint64_t key);
bool _inplace_hash_remove(void *hashmap, uint64_t key, void *outvalue);
void _inplace_move_into_place(void *hashmap, uint64_t key, int offset);

void *_inplace_hash_make(Allocator *allocator, int capacity, size_t value_offset, size_t element_size);
void _inplace_hash_destroy(void *hashmap);

#define _inplace_hash_header(h) (((InplaceHashHeader *)(h))[-1])
#define _inplace_hash_raw_pointer(h) ((char*)(h) - sizeof(InplaceHashHeader))
// TODO(kalle): round the load factor? Make it settable
#define _inplace_hash_need_to_grow(h, n)       ((h)==0 || _inplace_hash_header(h).count+(n) > (int)(_inplace_hash_header(h).capacity * 0.7f))
#define _inplace_hash_ensure_space(h, v, n, a) (_inplace_hash_need_to_grow((h), (n)) ? (h) = (decltype(h))(_inplace_hash_grow((h), (n))) : 0)
// TODO(kalle): Add to header? Always force 2?
#define _inplace_hash_bucket_size(h) 2

/** \addtogroup InplaceHashmap
 * Inplace Hashmap is similar to Hashmap, but consists of an uint64_t key and an arbitrary value.
 * It does so by recording the offset to the value and size of the entry struct.
 * Once recorded it uses memmove and memcmp to move the value around and function as the regular hashmap.
 *
 * The entry struct must be declared using the macro DECLARE_HASH_ENTRY.
 * Usage examples:
 * \code {.cpp}
 * // Declare the hash entry struct.
 * struct SomeHashEntry {
 *    DECLARE_HASH_ENTRY(SomeStruct);
 * };
 * \endcode
 * \code {.cpp}
 * // Initialization:
 * SomeHashEntry *hashmap = 0; // Declare the hash map.
 * inplace_hash_init(allocator, hashmap, SomeStruct, initial_capacity);
 * \endcode
 * \code {.cpp}
 * // Adding:
 * inplace_hash_add(hashmap, key, value);
 * \endcode
 * \code {.cpp}
 * // Lookup:
 * SomeHashEntry *entry = inplace_hash_lookup(hashmap, key);
 * \endcode
 * \code {.cpp}
 * // Remove:
 * SomeStruct removed_value;
 * if (inplace_hash_remove(hashmap, key, removed_value)) { printf("successfully removed"); }
 * \endcode
 *  @{
 */
/// The header of the inplace hashmap.
struct InplaceHashHeader {
	/// Allocator used when allocating memory
	Allocator *allocator;
	/// Returns the number of elements we can store before we need to allocate more memory
	int capacity;
	/// Returns the number of elements currently stored in the hashmap
	int count;
	/// The offset from the beginning of the key/value struct to the value
	size_t value_offset;
	/// The size of one key/value pair
	size_t element_size;
};

/// Initializes the hashmap. This needs to be called before any other hashmap calls, since it sets up the allocator to use. The capacity will get rounded up to the nearest power of two.
#define inplace_hash_make(allocator, hashmap, type, capacity) ((hashmap) = (type*)_inplace_hash_make(allocator, capacity, offsetof(type, value), sizeof(type)))
/// Deallocates the hashmap
#define inplace_hash_destroy(hashmap)                         (_inplace_hash_destroy(hashmap))
/// Adds a new key/value pair
#define inplace_hash_add(hashmap, key, value)                 (_inplace_hash_ensure_space(hashmap, key, 1, 0), _inplace_hash_add((hashmap), (key), &(value)))
/// Returns the pair mapped to key
#define inplace_hash_lookup(hashmap, key)                     (decltype(hashmap))_inplace_hash_lookup((hashmap), (key))
/// Tries to removes the value mapped to key if it exists. If so, it stores a copy of it in outvalue and returns true.
#define inplace_hash_remove(hashmap, key, outvalue)           (_inplace_hash_remove((hashmap), (key), &(outvalue)))
/** @}*/

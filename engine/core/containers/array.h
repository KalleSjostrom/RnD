#pragma once

#include <stdint.h>
#include "core/memory/allocator.h"

void *_array_grow(void *a, size_t increment, size_t itemsize, Allocator *allocator); // Default to some global arena?

#define _array_header(a) (((ArrayHeader *)(a))[-1])
#define _array_raw_pointer(a) ((char*)(a) - sizeof(ArrayHeader))
#define _array_need_to_grow(a, n)        ((a)==0 || (size_t)_array_header(a).count+(n) > (size_t)_array_header(a).capacity)
// #define _array_ensure_space(a, n, alloc) (_array_need_to_grow((a), (n)) ? (a) = (decltype(a))(_array_grow((a), (n), sizeof(a[0]), alloc)) : 0)
#define _array_ensure_space(a, n, alloc) do { \
	auto _tmp = a; \
	if (_array_need_to_grow((a), (n))) { \
		(a) = (decltype(_tmp))(_array_grow((a), (n), sizeof(a[0]), alloc)); \
	} \
} while(0);
#define _array_push(a, v) do { \
	_array_ensure_space(a, 1, 0); \
	(a)[array_count(a)++] = (v); \
} while(0);
#define _array_expand(a, n) do { \
	_array_ensure_space(a, n, 0); \
	array_count(a) += n; \
} while(0);

/** \addtogroup Array
 * Array is based on the concept of 'strechy_buffer'.
 * The idea is to store a header before the pointer to the array containing count and capacity.
 * We also store the allocator used when we need to realloc.
 *  @{
 */
/// The header of the array.
struct ArrayHeader {
	/// Allocator used when allocating memory
	Allocator *allocator;
	/// Returns the number of elements we can store before we need to allocate more memory
	int capacity;
	/// Returns the number of elements currently stored in the array
	int count;
};

/// Initializes the array. This needs to be called before any other array calls, since it sets up the allocator to use.
#define array_make(alloc, a, n)  _array_ensure_space(a, n, alloc)
/// Deallocates the array
#define array_destroy(a)         (deallocate(_array_header(a).allocator, &_array_header(a)))
/// Push a new element on the array, will grow if needed
#define array_push(a, v)         _array_push(a, v)
/// Removes the entry by swapping in the last
#define array_remove(a, index)	 ((a)[index] = (a)[--array_count(a)])
/// Makes room for 'n' elements on the array, will grow if needed
#define array_expand(a, n)       _array_expand(a, n)
/// Removes the last element from the list
#define array_pop(a)             ((a)[--array_count(a)])
/// Peeks at the last element from the list without removing it
#define array_peek(a)            ((a)[array_count(a) - 1])
/// Returns the number of elements we can store before we need to allocate more memory
#define array_capacity(a)        (_array_header(a).capacity)
/// Returns the number of elements currently stored in the array
#define array_count(a)           (_array_header(a).count)
/** @}*/

// TODO(kalle): Should we expose this?
typedef int* IntArray;

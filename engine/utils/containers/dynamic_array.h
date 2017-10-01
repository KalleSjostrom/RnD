// Based on https://github.com/nothings/stb/blob/master/stretchy_buffer.h
#pragma once

#define array_raw_pointer(a) ((int *) (a) - 2)
#define array_capacity(a)    ((a)==0 ? 0 : array_raw_pointer(a)[0])
#define array_count(a)       ((a)==0 ? 0 : array_raw_pointer(a)[1])

#define _array_capacity(a)    array_raw_pointer(a)[0]
#define _array_count(a)       array_raw_pointer(a)[1]

#define array_free(array) free(array_raw_pointer(array))
#define array_push(a, v) do { \
	array_try_grow(a, 1); \
	(a)[_array_count(a)++] = (v); \
} while (0)

#define array_new_entry(a) do { \
	array_try_grow(a, 1); \
	_array_count(a)++; \
} while (0)

#define array_last(a) ((a)[array_count(a)-1])

#define array_need_to_grow(a,n)  ((a)==0 || _array_count(a)+(n) > _array_capacity(a))
#define array_try_grow(a,n)      if (array_need_to_grow(a,(n))) { array_grow(a,n); }
#define array_grow(a,n)          _array_grow((a), (n), sizeof(*(a)))

#define array_remove_last(a) 	(_array_count(a)--)
#define array_remove(a, index)	((a)[index] = (a)[--_array_count(a)])
#define array_reset(a)       	(_array_count(a) = 0)

#define array_init(a, n) do { \
	*(intptr_t*)&(a) = 0; \
	_array_grow((a), (n), sizeof(*(a))); \
} while (0)

#define array_set_count(a, n) (_array_count(a) = n)

// if (a != 0) {
// 	fprintf(stderr, "Re-allocating array (prev_count=%d, new_count=%d)\n", needed - increment, capacity);
// 	if ((needed - increment) == 1) { ASSERT(false, "Re-allocating array"); }
// }

#define _array_grow(a, increment, itemsize) do { \
	int doubled_capacity = 2*array_capacity(a); \
	int needed = array_count(a) + increment; \
	int capacity = doubled_capacity > needed ? doubled_capacity : needed; \
	int *p = (int *) realloc(a ? array_raw_pointer(a) : 0, (u32)capacity * itemsize + sizeof(int)*2); \
	if (p) { \
		if (!a) \
			p[1] = 0; \
		p[0] = capacity; \
		*(intptr_t*)&a = (intptr_t)(p+2); \
		int prev_count = needed - increment; \
		memset((a) + prev_count, 0, (size_t)(capacity - prev_count) * sizeof(*(a))); \
	} \
} while (0)

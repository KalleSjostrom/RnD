#include "array.h"

void *_array_grow(void *a, size_t increment, size_t itemsize, Allocator *allocator) { // Default to some global arena?
	size_t doubled_capacity = a ? 2*array_capacity(a) : 0;
	size_t needed = (a ? array_count(a) : 0) + increment;
	size_t capacity = doubled_capacity > needed ? doubled_capacity : needed;

	allocator = a ? _array_header(a).allocator : allocator;
	void *p = realloc(allocator, a ? _array_raw_pointer(a) : 0, capacity * itemsize + sizeof(ArrayHeader));
	if (p) {
		ArrayHeader *header = (ArrayHeader*)p;
		header->capacity = (int)capacity;
		if (!a) {
			header->allocator = allocator;
			header->count = 0;
		}
		return (char*)p + sizeof(ArrayHeader);
	}
	return 0;
}
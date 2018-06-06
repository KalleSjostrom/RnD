#pragma once

#define _array_raw_pointer(a) ((int *) (a) - 2)

#define _array_capacity(a)    _array_raw_pointer(a)[0]
#define _array_count(a)       _array_raw_pointer(a)[1]

#define _array_need_to_grow(a, n) ((a)==0 || _array_count(a)+(n) > _array_capacity(a))
#define _array_ensure_space(a, n) (_array_need_to_grow((a), (n)) ? (a) = _array_grow((a), (n), sizeof(*(a))) : 0)

#define array_init(a, n)     (_array_ensure_space((a), n))
#define array_push(a, v)     (_array_ensure_space((a), 1), (a)[_array_count(a)++] = (v))
#define array_capacity(a)    ((a)==0 ? 0 : _array_capacity(a))
#define array_count(a)       ((a)==0 ? 0 : _array_count(a))

static void *_array_grow(void *a, int increment, size_t itemsize) {
	int doubled_capacity = 2*array_capacity(a);
	int needed = array_count(a) + increment;
	int capacity = doubled_capacity > needed ? doubled_capacity : needed;
	int *p = (int *) realloc(a ? _array_raw_pointer(a) : 0, (unsigned)capacity * itemsize + sizeof(int)*2);
	if (p) {
		if (!a)
			p[1] = 0;
		p[0] = capacity;
	}
	return p + 2;
}

void array_test() {
	int *list = 0;
	_array_ensure_space(list, 16);
	array_push(list, 5);
	array_push(list, 3);
	array_push(list, 8);
	array_push(list, 1);
	array_push(list, 9);
}
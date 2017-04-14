#include <stdio.h>
#include <string.h>

#define ASSERT(cond, text, ...) if (!(cond)) { printf("%s\n", text); }

#define Array(Type) \
struct Type##_Array { \
	unsigned count; \
	unsigned size; \
	Type *data; \
	Type& operator [](unsigned i) { ASSERT(i < count, "Array Index out of bounds."); return data[i]; };\
}

#define ConstArray(Type) \
struct Const##Type##_Array { \
	unsigned count; \
	unsigned size; \
	const Type *data; \
	const Type& operator [](unsigned i) { ASSERT(i < count, "Array Index out of bounds."); return data[i]; };\
}

typedef Array(float) Float_Array;
typedef ConstArray(float) ConstFloat_Array;

void _array_check_size(unsigned count, unsigned size) { ASSERT(count < size, "Array size out of index!"); }
#define array_init(Data, Count, Size) { Count, Size, Data }
#define array_index(ARRAY, pointer) (pointer - ARRAY.data)
#define array_pushback(ARRAY, DATA) ( ARRAY.data[ARRAY.count++] = DATA, _array_check_size(ARRAY.count, ARRAY.size) )
// #define array_insert(ARRAY, DATA, INDEX) ( memmove(&ARRAY.data[INDEX], &ARRAY.data[INDEX+1], sizeof(*ARRAY.data) * (ARRAY.count++)-INDEX), ARRAY.data[INDEX] = DATA, _array_check_size(ARRAY.count, ARRAY.size))
#define array_insert(ARRAY, DATA, INDEX) ( memmove(&ARRAY.data[INDEX+1], &ARRAY.data[INDEX], sizeof(*ARRAY.data) * (ARRAY.count++)-INDEX), ARRAY.data[INDEX] = DATA, _array_check_size(ARRAY.count, ARRAY.size))

#if 0
example:

int lol[64];
float lol2[128];

void my_func2(Float_Array &arr) {
	array_pushback(arr, 3.f);
}

void my_func() {
	Array(int) my_array = array_init(lol, 0, 64);
	Float_Array my_array2 = array_init(lol2, 0, 128);

	array_insert(my_array2, 1.f, 0);
	my_func2(my_array2);
}
#endif


int lol[64];
float lol2[128];

int main() {
	Array(int) test = array_init(lol, 0, 64);

	array_pushback(test, 10);
	array_pushback(test, 20);
	array_pushback(test, 30);
	array_pushback(test, 40);

	array_insert(test, 3, 4);
	// array_insert(test, 4, 0);
	// array_insert(test, 5, 0);

	printf("count: %d\n", test.count);
	printf("size: %d\n", test.size);

	int *p = &test[3];
	printf("array_index: %d\n", (int)array_index(test, p));

	for (int i = 0; i < test.count; ++i) {
		printf("%d\n", test[i]);
	}
}







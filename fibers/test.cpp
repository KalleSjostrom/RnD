#pragma warning(disable : 4255)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4189) // local variable is initialized but not referenced
#pragma warning(disable : 4820) // 'x' bytes padding added after data member 'name'
#pragma warning(disable : 4514) // warning C4514: 'sprintf_s': unreferenced inline function has been removed
#pragma warning(disable : 4710) // warning C4710: 'int printf(const char *const ,...)': function not inlinedc:\program files (x86)\windows kits\10\include\10.0.16299.0\ucrt\stdio.h(946): note: see declaration of 'printf'

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "immintrin.h"

unsigned align(unsigned value, unsigned alignment) {
	
	unsigned alignment_mask = alignment - 1;
	return (value + alignment_mask) & ~alignment_mask;
	/*if (value & alignment_mask) {
		value = alignment - (value & alignment_mask);
	}
	return value;*/
}

unsigned insert(unsigned at, unsigned size, bool is_primitive, unsigned &largest_primitive_data_size) {
	if (is_primitive) {
		largest_primitive_data_size = size > largest_primitive_data_size ? size : largest_primitive_data_size;
	}
	at = align(at, size);
	at += size;
	printf("insert at=%u, size=%u, next=%u\n", at - size, size, at);
	return at;
}

int main(int argc, char const *argv[]) {
	{
		struct Blah {
			float a;
			uint64_t b;
			bool c;
			bool d;
			int e;
			bool f;
		};
		Blah b = {};

		unsigned at = 0;
		unsigned largest_primitive_data_size = 0;
		bool is_primitive = true;

		at = insert(at, sizeof(b.a), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.b), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.c), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.d), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.e), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.f), is_primitive, largest_primitive_data_size);
		at = insert(at, largest_primitive_data_size, false, largest_primitive_data_size);
		printf("\n");
	}

	{
		struct Blah {
			float a;
			float b;
			bool c;
			bool d;
			int e;
			bool f;
		};
		Blah b = {};

		unsigned at = 0;
		unsigned largest_primitive_data_size = 0;
		bool is_primitive = true;

		at = insert(at, sizeof(b.a), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.b), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.c), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.d), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.e), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.f), is_primitive, largest_primitive_data_size);
		at = insert(at, largest_primitive_data_size, false, largest_primitive_data_size);
		printf("\n");
	}

	{
		struct Blah {
			int a;
			int b;
			bool c;
		};
		Blah b = {};

		unsigned at = 0;
		unsigned largest_primitive_data_size = 0;
		bool is_primitive = true;

		at = insert(at, sizeof(b.a), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.b), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.c), is_primitive, largest_primitive_data_size);
		at = insert(at, largest_primitive_data_size, false, largest_primitive_data_size);
		printf("\n");
	}

	{
		struct Blah {
			int64_t a;
			bool b;
		};
		Blah b = {};

		unsigned at = 0;
		unsigned largest_primitive_data_size = 0;
		bool is_primitive = true;

		at = insert(at, sizeof(b.a), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.b), is_primitive, largest_primitive_data_size);
		at = insert(at, largest_primitive_data_size, false, largest_primitive_data_size);
		printf("\n");
	}

	{
		struct Blah {
			__m128 a;
			bool b;
		};
		Blah b = {};

		unsigned at = 0;
		unsigned largest_primitive_data_size = 0;
		bool is_primitive = true;

		at = insert(at, sizeof(b.a), is_primitive, largest_primitive_data_size);
		at = insert(at, sizeof(b.b), is_primitive, largest_primitive_data_size);
		at = insert(at, largest_primitive_data_size, false, largest_primitive_data_size);
		printf("\n");
	}

	return 0;
}

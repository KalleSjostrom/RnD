#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define KB 1024
#define MB 1024 * KB
#define GB 1024 * MB

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))
#define ASSERT_IN_BOUNDS(array, count) ASSERT(ARRAY_COUNT(array) > count, "Array index out of bounds!");

// #define __forceinline __attribute__((always_inline))

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef short i16;
typedef int b32;

#define ASSERT(arg, format, ...) { \
	if (!(arg)) { \
		printf("###### ASSERTION FAILED: " format " [%s:%d]\n", ##__VA_ARGS__, __FILE__, __LINE__); \
		*(volatile int*)0 = 5; \
	} \
}

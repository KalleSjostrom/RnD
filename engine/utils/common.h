#pragma once

#include "engine/utils/platform.h"

#define KB 1024
#define MB 1024 * KB
#define GB 1024 * MB

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))
#define ASSERT_IN_BOUNDS(array, count) ASSERT(ARRAY_COUNT(array) > count, "Array index out of bounds!");

#define FORCE_INLINE __attribute__((always_inline))
// #if defined _WIN32
// 	#define thread_local __declspec(thread)
// #elif defined __GNUC__
// 	#define thread_local __thread
// #endif

typedef size_t u64;
typedef int64_t i64;
typedef double f64;

typedef unsigned int u32;
typedef int i32;
typedef float f32;

typedef unsigned short u16;
typedef short i16;

typedef unsigned char u8;
typedef char i8;

typedef int b32;

#define STATIC_ASSERT(condition, msg) typedef i32 my_static_assert[(condition) ? 1 : -1]
#define ASSERT(condition, format, ...) { \
	if (!(condition)) { \
		fflush(stdout); \
		fprintf(stderr, "###### ASSERTION FAILED ######\n%s:%d:0: error: " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
		*(volatile int*)0 = 5; \
	} \
}

#ifdef OS_WINDOWS
#define snprintf _snprintf
#endif

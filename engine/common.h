#pragma once

#include "core/ignored_warnings.h"

#include <stdio.h>
#include <stdint.h>
// #include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
// #include <float.h>

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

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

#define FORCE_INLINE _forceinline
#define ALIGN(x) __declspec(align(x))
#define ALIGNED_(x) __declspec(align(x))
// #define fminf(x, y) ((x) < (y) ? (x) : (y))
// #define fmaxf(x, y) ((x) < (y) ? (y) : (x))
#define EXPORT extern "C" __declspec(dllexport)

#define ALIGNED_TYPE(t,x) typedef t ALIGNED_(x)

#define OS_WINDOWS
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

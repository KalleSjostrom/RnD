#pragma once

#ifdef OS_WINDOWS
	#define _USE_MATH_DEFINES 1
#endif

#include <math.h>
#ifndef USE_INTRINSICS
#define USE_INTRINSICS 0
#endif

#if USE_INTRINSICS
#include <immintrin.h>
#endif

static const f32 DEGREES_TO_RANDIANS = (f32)M_PI/180.0f;

FORCE_INLINE f32 sigmoidal(f32 activation) {
	// 1 / (1 + e^(-a/p)) where p = 1, a is the activation.
	f32 p = 1.0f;
	return 1.0f / (1.0f + expf(-activation/p));
}

FORCE_INLINE f32 lerp(f32 a, f32 b, f32 t) {
	return a + (b-a)*t;
}

FORCE_INLINE f32 safe_divide(f32 numerator, f32 divisior, f32 default_value) {
	return divisior != 0.0f ? (numerator / divisior) : default_value;
}

FORCE_INLINE b32 float_equal(f32 a, f32 b, f32 epsilon = 0.00001f) {
	return fabsf(a - b) < epsilon;
}

FORCE_INLINE f32 clamp(f32 value, f32 low, f32 high) {
	if (value < low) return low;
	if (value > high) return high;
	return value;
}

FORCE_INLINE f32 saturate(f32 value) {
	return clamp(value, 0, 1);
}

FORCE_INLINE void sincosf(float x, float *sinx, float *cosx) {
	__sincosf(x, sinx, cosx);
}

// FORCE_INLINE void sincosf(float x, float *sinx, float *cosx) {
// 	*sinx = sinf(x);
// 	*cosx = sqrtf(1 - *sinx);
// }

#define ELEMENT_TYPE f32
#include "math_v2.h"
typedef v2_f32 v2;

#define ELEMENT_TYPE f64
#include "math_v2.h"

struct v3 {
	union {
		struct {
			f32 x, y, z;
		};
		struct {
			f32 r, g, b;
		};
	};
	f32 &operator[](int index) {
		return *((f32*)this + index);
	}
};

FORCE_INLINE v3 V3(f32 x, f32 y, f32 z) {
	v3 result = {x, y, z};
	return result;
}

FORCE_INLINE v3 operator*(f32 a, v3 b) {
	return V3(a * b.x, a * b.y, a * b.z);
}
FORCE_INLINE v3 operator*(v3 a, f32 b) {
	return b * a;
}
FORCE_INLINE v3 & operator*=(v3 &a, f32 b) {
	a = a * b;
	return a;
}

FORCE_INLINE v3 operator/(v3 a, f32 b) {
	return V3(a.x/b, a.y/b, a.z/b);
}

FORCE_INLINE v3 & operator/=(v3 &a, f32 b) {
	a = a/b;
	return a;
}

FORCE_INLINE v3 operator+(v3 a, v3 b) {
	return V3(a.x + b.x, a.y + b.y, a.z + b.z);
}
FORCE_INLINE v3 & operator+=(v3 &a, v3 b) {
	a = a + b;
	return a;
}

FORCE_INLINE v3 operator-(v3 a) {
	return V3(-a.x, -a.y, -a.z);
}
FORCE_INLINE v3 operator-(v3 a, v3 b) {
	return V3(a.x - b.x, a.y - b.y, a.z - b.z);
}
FORCE_INLINE v3 & operator-=(v3 &a, v3 b) {
	a = a - b;
	return a;
}

FORCE_INLINE v3 hadamard(v3 a, v3 b) {
	return V3(a.x * b.x, a.y * b.y, a.z * b.z);
}
FORCE_INLINE f32 dot(v3 a, v3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
FORCE_INLINE f32 length_squared(v3 a) {
	return dot(a, a);
}
FORCE_INLINE f32 length(v3 a) {
	return sqrtf(length_squared(a));
}
FORCE_INLINE v3 normalize(v3 a) {
	f32 l = length(a);
	return (1.0f/l) * a;
}
FORCE_INLINE v3 normalize_or_zero(v3 a) {
	f32 l = length(a);
	return l > 0 ? (1.0f/l) * a : V3(0, 0, 0);
}
FORCE_INLINE v3 cross(v3 a, v3 b) {
	f32 x = a.y*b.z - a.z*b.y;
	f32 y = a.z*b.x - a.x*b.z;
	f32 z = a.x*b.y - a.y*b.x;
	return V3(x, y, z);
}

FORCE_INLINE v3 lerp(v3 a, v3 b, f32 t) {
	return V3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

struct v4 {
	union {
		struct {
			f32 x, y, z, w;
		};
		struct {
			f32 r, g, b, a;
		};
	};
};

FORCE_INLINE v4 V4(f32 x, f32 y, f32 z, f32 w) {
	v4 result = {x, y, z, w};
	return result;
}
FORCE_INLINE v4 V4(v3 a, f32 w) {
	v4 result = {a.x, a.y, a.z, w};
	return result;
}

FORCE_INLINE v4 operator*(f32 a, v4 b) {
	return V4(a * b.x, a * b.y, a * b.z, a * b.w);
}
FORCE_INLINE v4 operator*(v4 a, f32 b) {
	return b * a;
}
FORCE_INLINE v4 & operator*=(v4 &a, f32 b) {
	a = a * b;
	return a;
}

FORCE_INLINE v4 operator/(v4 a, f32 b) {
	return V4(a.x/b, a.y/b, a.z/b, a.w/b);
}
FORCE_INLINE v4 & operator/=(v4 &a, f32 b) {
	a = a/b;
	return a;
}

FORCE_INLINE v4 operator+(v4 a, v4 b) {
	return V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
FORCE_INLINE v4 & operator+=(v4 &a, v4 b) {
	a = a + b;
	return a;
}

FORCE_INLINE v4 operator-(v4 a) {
	return V4(-a.x, -a.x, -a.z, -a.w);
}
FORCE_INLINE v4 operator-(v4 a, v4 b) {
	return V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
FORCE_INLINE v4 & operator-=(v4 &a, v4 b) {
	a = a - b;
	return a;
}

FORCE_INLINE v4 hadamard(v4 a, v4 b) {
	return V4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
FORCE_INLINE f32 dot(v4 a, v4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
FORCE_INLINE f32 length_squared(v4 a) {
	return dot(a, a);
}
FORCE_INLINE f32 length(v4 a) {
	return sqrtf(length_squared(a));
}
FORCE_INLINE v4 normalize(v4 a) {
	f32 l = length(a);
	return (1.0f/l) * a;
}

#include "matrix.h"
#include "quaternion.h"

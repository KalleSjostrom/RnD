#include <math.h>
#if USE_INTRINSICS
#include <immintrin.h>
#endif

static const f32 DEGREES_TO_RANDIANS = (f32)M_PI/180.0f;

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

struct v2 {
	f32 x, y;
};

FORCE_INLINE v2 V2(f32 x, f32 y) {
	v2 result = {x, y};
	return result;
}
FORCE_INLINE v2 random_v2() {
	f32 x = (f32)random()/RAND_MAX;
	x = x*2.0f - 1.0f;
	f32 y = (f32)random()/RAND_MAX;
	y = y*2.0f - 1.0f;
	return V2(x, y);
}

FORCE_INLINE v2 operator*(f32 a, v2 b) {
	return V2(a * b.x, a * b.y);
}
FORCE_INLINE v2 operator*(v2 a, f32 b) {
	return b * a;
}
FORCE_INLINE v2 & operator*=(v2 &a, f32 b) {
	a = a * b;
	return a;
}

FORCE_INLINE v2 operator/(v2 a, f32 b) {
	return V2(a.x/b, a.y/b);
}
FORCE_INLINE v2 & operator/=(v2 &a, f32 b) {
	a = a/b;
	return a;
}

FORCE_INLINE v2 operator+(v2 a, v2 b) {
	return V2(a.x + b.x, a.y + b.y);
}
FORCE_INLINE v2 & operator+=(v2 &a, v2 b) {
	a = a + b;
	return a;
}

FORCE_INLINE v2 operator-(v2 a) {
	return V2(-a.x, -a.y);
}
FORCE_INLINE v2 operator-(v2 a, v2 b) {
	return V2(a.x - b.x, a.y - b.y);
}
FORCE_INLINE v2 & operator-=(v2 &a, v2 b) {
	a = a - b;
	return a;
}

FORCE_INLINE v2 hadamard(v2 a, v2 b) {
	return V2(a.x * b.x, a.y * b.y);
}
FORCE_INLINE f32 dot(v2 a, v2 b) {
	return a.x * b.x + a.y * b.y;
}
FORCE_INLINE f32 length_squared(v2 a) {
	return dot(a, a);
}
FORCE_INLINE f32 length(v2 a) {
	return sqrtf(length_squared(a));
}
FORCE_INLINE v2 normalize(v2 a) {
	f32 l = length(a);
	return (1.0f/l) * a;
}
FORCE_INLINE v2 normalize_or_zero(v2 a) {
	f32 l = length(a);
	return l > 0 ? (1.0f/l) * a : V2(0,0);
}
FORCE_INLINE f32 cross(v2 a, v2 b) {
	return (a.x*b.y) - (a.y*b.x);
}
FORCE_INLINE v2 cross(v2 a) {
	return V2(-a.y, a.x);
}


struct v3 {
	f32 x, y, z;
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
	return V3(-a.x, -a.x, -a.z);
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
FORCE_INLINE v3 cross(v3 a, v3 b) {
	f32 x = a.y*b.z - a.z*b.y;
	f32 y = a.z*b.x - a.x*b.z;
	f32 z = a.y*b.x - a.x*b.y;
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

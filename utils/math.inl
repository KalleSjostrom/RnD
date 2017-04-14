#include <math.h>
#include <immintrin.h>

static const float DEGREES_TO_RANDIANS = M_PI/180.0f;

__forceinline float lerp(float a, float b, float t) {
	return a + (b-a)*t;
}

__forceinline float safe_divide(float numerator, float divisior, float default_value) {
	return divisior != 0.0f ? (numerator / divisior) : default_value;
}

__forceinline float clamp(float value, float low, float high) {
	if (value < low) return low;
	if (value > high) return high;
	return value;
}

struct v2 {
	float x, y;
};

__forceinline v2 V2(float x, float y) {
	v2 result = {x, y};
	return result;
}
__forceinline v2 random_v2() {
	float x = (float)random()/RAND_MAX;
	x = x*2.0f - 1.0f;
	float y = (float)random()/RAND_MAX;
	y = y*2.0f - 1.0f;
	return V2(x, y);
}

__forceinline v2 operator*(float a, v2 b) {
	return V2(a * b.x, a * b.y);
}
__forceinline v2 operator*(v2 a, float b) {
	return b * a;
}
__forceinline v2 & operator*=(v2 &a, float b) {
	a = a * b;
	return a;
}

__forceinline v2 operator/(v2 a, float b) {
	return V2(a.x/b, a.y/b);
}
__forceinline v2 & operator/=(v2 &a, float b) {
	a = a/b;
	return a;
}

__forceinline v2 operator+(v2 a, v2 b) {
	return V2(a.x + b.x, a.y + b.y);
}
__forceinline v2 & operator+=(v2 &a, v2 b) {
	a = a + b;
	return a;
}

__forceinline v2 operator-(v2 a) {
	return V2(-a.x, -a.y);
}
__forceinline v2 operator-(v2 a, v2 b) {
	return V2(a.x - b.x, a.y - b.y);
}
__forceinline v2 & operator-=(v2 &a, v2 b) {
	a = a - b;
	return a;
}

__forceinline v2 hadamard(v2 a, v2 b) {
	return V2(a.x * b.x, a.y * b.y);
}
__forceinline float dot(v2 a, v2 b) {
	return a.x * b.x + a.y * b.y;
}
__forceinline float length_squared(v2 a) {
	return dot(a, a);
}
__forceinline float length(v2 a) {
	return sqrtf(length_squared(a));
}
__forceinline v2 normalize(v2 a) {
	float l = length(a);
	return (1.0f/l) * a;
}
__forceinline v2 normalize_or_zero(v2 a) {
	float l = length(a);
	return l > 0 ? (1.0f/l) * a : V2(0,0);
}
__forceinline float cross(v2 a, v2 b) {
	return (a.x*b.y) - (a.y*b.x);
}
__forceinline v2 cross(v2 a) {
	return V2(-a.y, a.x);
}


struct v3 {
	float x, y, z;
};

__forceinline v3 V3(float x, float y, float z) {
	v3 result = {x, y, z};
	return result;
}

__forceinline v3 operator*(float a, v3 b) {
	return V3(a * b.x, a * b.y, a * b.z);
}
__forceinline v3 operator*(v3 a, float b) {
	return b * a;
}
__forceinline v3 & operator*=(v3 &a, float b) {
	a = a * b;
	return a;
}

__forceinline v3 operator/(v3 a, float b) {
	return V3(a.x/b, a.y/b, a.z/b);
}

__forceinline v3 & operator/=(v3 &a, float b) {
	a = a/b;
	return a;
}

__forceinline v3 operator+(v3 a, v3 b) {
	return V3(a.x + b.x, a.y + b.y, a.z + b.z);
}
__forceinline v3 & operator+=(v3 &a, v3 b) {
	a = a + b;
	return a;
}

__forceinline v3 operator-(v3 a) {
	return V3(-a.x, -a.x, -a.z);
}
__forceinline v3 operator-(v3 a, v3 b) {
	return V3(a.x - b.x, a.y - b.y, a.z - b.z);
}
__forceinline v3 & operator-=(v3 &a, v3 b) {
	a = a - b;
	return a;
}

__forceinline v3 hadamard(v3 a, v3 b) {
	return V3(a.x * b.x, a.y * b.y, a.z * b.z);
}
__forceinline float dot(v3 a, v3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
__forceinline float length_squared(v3 a) {
	return dot(a, a);
}
__forceinline float length(v3 a) {
	return sqrtf(length_squared(a));
}
__forceinline v3 normalize(v3 a) {
	float l = length(a);
	return (1.0f/l) * a;
}
__forceinline v3 cross(v3 a, v3 b) {
	float x = a.y*b.z - a.z*b.y;
	float y = a.z*b.x - a.x*b.z;
	float z = a.y*b.x - a.x*b.y;
	return V3(x, y, z);
}
__forceinline v3 lerp(v3 a, v3 b, float t) {
	return V3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

struct v4 {
	union {
		struct {
			float x, y, z, w;
		};
		struct {
			float r, g, b, a;
		};
	};
};

__forceinline v4 V4(float x, float y, float z, float w) {
	v4 result = {x, y, z, w};
	return result;
}
__forceinline v4 V4(v3 a, float w) {
	v4 result = {a.x, a.y, a.z, w};
	return result;
}

__forceinline v4 operator*(float a, v4 b) {
	return V4(a * b.x, a * b.y, a * b.z, a * b.w);
}
__forceinline v4 operator*(v4 a, float b) {
	return b * a;
}
__forceinline v4 & operator*=(v4 &a, float b) {
	a = a * b;
	return a;
}

__forceinline v4 operator/(v4 a, float b) {
	return V4(a.x/b, a.y/b, a.z/b, a.w/b);
}
__forceinline v4 & operator/=(v4 &a, float b) {
	a = a/b;
	return a;
}

__forceinline v4 operator+(v4 a, v4 b) {
	return V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
__forceinline v4 & operator+=(v4 &a, v4 b) {
	a = a + b;
	return a;
}

__forceinline v4 operator-(v4 a) {
	return V4(-a.x, -a.x, -a.z, -a.w);
}
__forceinline v4 operator-(v4 a, v4 b) {
	return V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
__forceinline v4 & operator-=(v4 &a, v4 b) {
	a = a - b;
	return a;
}

__forceinline v4 hadamard(v4 a, v4 b) {
	return V4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
__forceinline float dot(v4 a, v4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
__forceinline float length_squared(v4 a) {
	return dot(a, a);
}
__forceinline float length(v4 a) {
	return sqrtf(length_squared(a));
}
__forceinline v4 normalize(v4 a) {
	float l = length(a);
	return (1.0f/l) * a;
}

#include "matrix.inl"

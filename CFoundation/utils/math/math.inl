#pragma once

#define FLT_MAX 3.402823466e+38F

namespace math {
	const float pi = 3.141592654f;
	const float tau = 6.28318530718f;
	const float to_degrees = 180.0f/pi;
	const float to_radians = pi/180.0f;
	const float epsilon = 0.000001f;
	const float phi = 1.6180339f;

	__forceinline float rad(float degree) { return degree * to_radians; }
	__forceinline float deg(float radians) { return radians * to_degrees; }
	__forceinline bool is_pow2(unsigned x) { return (x & (x - 1)) == 0; }
	__forceinline float square(float f) { return f*f; }
	__forceinline float square_root(float f) { return sqrtf(f); }
	__forceinline unsigned branchless_max(unsigned a, unsigned b) { return a - ((a-b) & (a-b)>>31); }
	__forceinline unsigned branchless_min(unsigned a, unsigned b) { return b + ((a-b) & (a-b)>>31); }
	__forceinline unsigned div_ceil(unsigned a, unsigned b) { return (a+b-1)/b; }
	__forceinline unsigned div_round(unsigned a, unsigned b) { return (a+b/2)/b; }
	__forceinline float clamp(float a, float lower, float upper) {
		if (a < lower)
			return lower;
		if (a > upper)
			return upper;
		return a;
	}
	__forceinline unsigned clamp(unsigned a, unsigned lower, unsigned upper) {
		if (a < lower)
			return lower;
		if (a > upper)
			return upper;
		return a;
	}

	__forceinline float lerp(float a, float b, float t) { return a * (1-t) + b * t; }
	__forceinline float lerp_clamped(float a, float b, float t) { return lerp(a, b, clamp(t, 0.0f, 1.0f)); }
	__forceinline float saturate(float a) {
		return clamp(a, 0.0f, 1.0f);
	}
	__forceinline float sign(float f) {
		return f<0 ? -1.0f : 1.0f;
	}
	__forceinline float min(float a, float b) {
		return a<b ? a : b;
	}
	__forceinline unsigned min(unsigned a, unsigned b) {
		return a<b ? a : b;
	}
	__forceinline float max(float a, float b) {
		return a>b ? a : b;
	}
	__forceinline unsigned max(unsigned a, unsigned b) {
		return a>b ? a : b;
	}

	__forceinline unsigned round_down_to_align(unsigned x, unsigned align)	{ return x - x%align; }
	__forceinline unsigned round_up_to_align(unsigned x, unsigned align)	{ return round_down_to_align(x+align-1, align); }

	__forceinline unsigned log2(unsigned a) {
		unsigned log2 = 0;
		while(a >>= 1) log2++;
		return log2;
	}

	__forceinline float scale_to_01(float x, float lower_bound, float upper_bound) {
		ASSERT(upper_bound > lower_bound, "Upper bound needs to be greater than lower bound! (lower_bound=%.3f, upper_bound=%.3f)", lower_bound, upper_bound);
		float diff = upper_bound - lower_bound;
		return (x - lower_bound) / diff;
	}
}

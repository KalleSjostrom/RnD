#pragma once

#include <math.h>
#include <float.h>
#define M_PI 3.14159265358979323846264338327950288

__forceinline float sigmoidal(float activation) {
	// (1 / (1 + e^(-a/p))) where p = 1, a is the activation.
	float p = 1.0f;
	return 1.0f / (1.0f + expf(-activation/p));
}

__forceinline float lerp(float a, float b, float t) {
	return a + (b-a)*t;
}

__forceinline float safe_divide(float numerator, float divisior, float default_value = 0.0f) {
	return divisior != 0.0f ? (numerator / divisior) : default_value;
}

__forceinline bool float_equal(float a, float b, float epsilon = 0.00001f) {
	return fabsf(a - b) < epsilon;
}

__forceinline float clamp(float value, float low, float high) {
	if (value < low) return low;
	if (value > high) return high;
	return value;
}

__forceinline float saturate(float value) {
	return clamp(value, 0, 1);
}

__forceinline void sincosf(float x, float *sinx, float *cosx) {
	// TODO(kalle): Which is faster?
	// *sinx = sinf(x);
	// *cosx = cosf(x);

	*sinx = sinf(x);
 	*cosx = sqrtf(1 - *sinx);

	// __sincosf(x, sinx, cosx);
	// sincosf(x, sinx, cosx);
}


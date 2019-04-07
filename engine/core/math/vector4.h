#pragma once

#include "core/math/vector2.h"
#include "core/math/vector3.h"

struct Vector4 {
	union {
		struct {
			float x, y, z, w;
		};
		struct {
			float r, g, b, a;
		};
	};
	float &operator[](int index) {
		return *((float*)this + index);
	}
};

__forceinline Vector4 vector4(float x, float y, float z, float w) {
	Vector4 result = {x, y, z, w};
	return result;
}
__forceinline Vector4 vector4(Vector2 a, Vector2 b) {
	Vector4 result = {a.x, a.y, b.x, b.y};
	return result;
}
__forceinline Vector4 vector4(Vector3 a, float w) {
	Vector4 result = {a.x, a.y, a.z, w};
	return result;
}

__forceinline Vector4 operator*(float a, Vector4 b) {
	return vector4(a * b.x, a * b.y, a * b.z, a * b.w);
}
__forceinline Vector4 operator*(Vector4 a, float b) {
	return b * a;
}
__forceinline Vector4 & operator*=(Vector4 &a, float b) {
	a = a * b;
	return a;
}

__forceinline Vector4 operator/(Vector4 a, float b) {
	return vector4(a.x/b, a.y/b, a.z/b, a.w/b);
}
__forceinline Vector4 & operator/=(Vector4 &a, float b) {
	a = a/b;
	return a;
}

__forceinline Vector4 operator+(Vector4 a, Vector4 b) {
	return vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
__forceinline Vector4 & operator+=(Vector4 &a, Vector4 b) {
	a = a + b;
	return a;
}

__forceinline Vector4 operator-(Vector4 a) {
	return vector4(-a.x, -a.x, -a.z, -a.w);
}
__forceinline Vector4 operator-(Vector4 a, Vector4 b) {
	return vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
__forceinline Vector4 & operator-=(Vector4 &a, Vector4 b) {
	a = a - b;
	return a;
}

__forceinline Vector4 hadamard(Vector4 a, Vector4 b) {
	return vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
__forceinline float dot(Vector4 a, Vector4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
__forceinline float length_squared(Vector4 a) {
	return dot(a, a);
}
__forceinline float length(Vector4 a) {
	return sqrtf(length_squared(a));
}
__forceinline Vector4 normalize(Vector4 a) {
	float l = length(a);
	return (1.0f/l) * a;
}

#pragma once

struct Vector3 {
	union {
		struct {
			float x, y, z;
		};
		struct {
			float r, g, b;
		};
	};
	float &operator[](int index) {
		return *((float*)this + index);
	}
};

__forceinline Vector3 vector3(float x, float y, float z) {
	Vector3 result = {x, y, z};
	return result;
}

__forceinline Vector3 operator*(float a, Vector3 b) {
	return vector3(a * b.x, a * b.y, a * b.z);
}
__forceinline Vector3 operator*(Vector3 a, float b) {
	return b * a;
}
__forceinline Vector3 & operator*=(Vector3 &a, float b) {
	a = a * b;
	return a;
}

__forceinline Vector3 operator/(Vector3 a, float b) {
	return vector3(a.x/b, a.y/b, a.z/b);
}

__forceinline Vector3 & operator/=(Vector3 &a, float b) {
	a = a/b;
	return a;
}

__forceinline Vector3 operator+(Vector3 a, Vector3 b) {
	return vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
__forceinline Vector3 & operator+=(Vector3 &a, Vector3 b) {
	a = a + b;
	return a;
}

__forceinline Vector3 operator-(Vector3 a) {
	return vector3(-a.x, -a.y, -a.z);
}
__forceinline Vector3 operator-(Vector3 a, Vector3 b) {
	return vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
__forceinline Vector3 & operator-=(Vector3 &a, Vector3 b) {
	a = a - b;
	return a;
}

__forceinline Vector3 hadamard(Vector3 a, Vector3 b) {
	return vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}
__forceinline float dot(Vector3 a, Vector3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
__forceinline float length_squared(Vector3 a) {
	return dot(a, a);
}
__forceinline float length(Vector3 a) {
	return sqrtf(length_squared(a));
}
__forceinline Vector3 normalize(Vector3 a) {
	float l = length(a);
	return (1.0f/l) * a;
}
__forceinline Vector3 normalize_or_zero(Vector3 a) {
	float l = length(a);
	return l > 0 ? (1.0f/l) * a : vector3(0, 0, 0);
}
__forceinline Vector3 cross(Vector3 a, Vector3 b) {
	float x = a.y*b.z - a.z*b.y;
	float y = a.z*b.x - a.x*b.z;
	float z = a.x*b.y - a.y*b.x;
	return vector3(x, y, z);
}

__forceinline Vector3 lerp(Vector3 a, Vector3 b, float t) {
	return vector3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

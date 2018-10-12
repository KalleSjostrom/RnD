#pragma once

struct Vector2 {
	union {
		struct {
			float x, y;
		};
		struct {
			float u, v;
		};
	};
	float &operator[](int index) {
		return *((float*)this + index);
	}
};

__forceinline Vector2 vector2(float x, float y) {
	Vector2 result = {x, y};
	return result;
}

__forceinline Vector2 operator*(float a, Vector2 b) {
	return vector2(a * b.x, a * b.y);
}
__forceinline Vector2 operator*(Vector2 a, float b) {
	return b * a;
}
__forceinline Vector2 & operator*=(Vector2 &a, float b) {
	a = a * b;
	return a;
}

__forceinline Vector2 operator/(Vector2 a, float b) {
	return vector2(a.x/b, a.y/b);
}

__forceinline Vector2 & operator/=(Vector2 &a, float b) {
	a = a/b;
	return a;
}

__forceinline Vector2 operator+(Vector2 a, Vector2 b) {
	return vector2(a.x + b.x, a.y + b.y);
}
__forceinline Vector2 & operator+=(Vector2 &a, Vector2 b) {
	a = a + b;
	return a;
}

__forceinline Vector2 operator-(Vector2 a) {
	return vector2(-a.x, -a.y);
}
__forceinline Vector2 operator-(Vector2 a, Vector2 b) {
	return vector2(a.x - b.x, a.y - b.y);
}
__forceinline Vector2 & operator-=(Vector2 &a, Vector2 b) {
	a = a - b;
	return a;
}

__forceinline Vector2 hadamard(Vector2 a, Vector2 b) {
	return vector2(a.x * b.x, a.y * b.y);
}
__forceinline float dot(Vector2 a, Vector2 b) {
	return a.x * b.x + a.y * b.y;
}
__forceinline float length_squared(Vector2 a) {
	return dot(a, a);
}
__forceinline float length(Vector2 a) {
	return sqrtf(length_squared(a));
}
__forceinline Vector2 normalize(Vector2 a) {
	float l = length(a);
	return (1.0f/l) * a;
}
__forceinline Vector2 normalize_or_zero(Vector2 a) {
	float l = length(a);
	return l > 0 ? (1.0f/l) * a : vector2(0, 0);
}
__forceinline Vector2 lerp(Vector2 a, Vector2 b, float t) {
	return vector2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

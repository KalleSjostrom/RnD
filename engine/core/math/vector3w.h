#pragma once

struct Vector3w {
	__m128 xmm;
	float &operator[](int index) {
		return xmm.m128_f32[index];
	}
};

// Construct
__forceinline Vector3w vector3w(__m128 v) {
	Vector3w result = {};
	result.xmm = v;
	return result;
}
__forceinline Vector3w vector3w() {
	Vector3w result = {};
	return result;
}
__forceinline Vector3w vector3w(float x) {
	return vector3w(_mm_broadcast_ss(&x));
}
__forceinline Vector3w vector3w(float x, float y, float z) {
	return vector3w(_mm_set_ps(x, y, z, 0));
}
__forceinline Vector3w vector3w(Vector3 v) {
	return vector3w(v.x, v.y, v.z);
}

// Hadamard
__forceinline Vector3w operator*(Vector3w a, Vector3w b) {
	return vector3w(_mm_mul_ps(a.xmm, b.xmm));
}
__forceinline Vector3w &operator*=(Vector3w &a, Vector3w b) {
	a = vector3w(_mm_mul_ps(a.xmm, b.xmm));
	return a;
}

// Scalar math
__forceinline Vector3w operator*(float a, Vector3w b) {
	Vector3w temp = vector3w(_mm_broadcast_ss(&a));
	return temp * b;
}
__forceinline Vector3w operator*(Vector3w a, float b) {
	return b * a;
}
__forceinline Vector3w & operator*=(Vector3w &a, float b) {
	a = a * b;
	return a;
}
__forceinline Vector3w operator/(Vector3w a, float b) {
	__m128 temp = _mm_broadcast_ss(&b);
	return vector3w(_mm_div_ps(a.xmm, temp));
}
__forceinline Vector3w & operator/=(Vector3w &a, float b) {
	a = a / b;
	return a;
}

// Plus & minus
__forceinline Vector3w operator+(Vector3w a, Vector3w b) {
	return vector3w(_mm_add_ps(a.xmm, b.xmm));
}
__forceinline Vector3w & operator+=(Vector3w &a, Vector3w b) {
	a = a + b;
	return a;
}
__forceinline Vector3w operator-(Vector3w a, Vector3w b) {
	return vector3w(_mm_sub_ps(a.xmm, b.xmm));
}
__forceinline Vector3w & operator-=(Vector3w &a, Vector3w b) {
	a = a - b;
	return a;
}
__forceinline Vector3w operator-(Vector3w a) {
	Vector3w temp = vector3w() - a;
	return vector3w(temp.xmm);
}

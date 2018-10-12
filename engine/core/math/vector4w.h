#pragma once

struct Vector4w {
	__m128 xmm;
	float &operator[](int index) {
		return xmm.m128_f32[index];
	}
};

// Construct
__forceinline Vector4w vector4w(__m128 v) {
	Vector4w result = {};
	result.xmm = v;
	return result;
}
__forceinline Vector4w vector4w() {
	Vector4w result = {};
	return result;
}
__forceinline Vector4w vector4w(float x) {
	return vector4w(_mm_broadcast_ss(&x));
}
__forceinline Vector4w vector4w(float x, float y, float z, float w) {
	return vector4w(_mm_set_ps(x, y, z, w));
}
__forceinline Vector4w vector4w(Vector4 v) {
	return vector4w(v.x, v.y, v.z, v.w);
}

// Hadamard
__forceinline Vector4w operator*(Vector4w a, Vector4w b) {
	return vector4w(_mm_mul_ps(a.xmm, b.xmm));
}
__forceinline Vector4w &operator*=(Vector4w &a, Vector4w b) {
	a = vector4w(_mm_mul_ps(a.xmm, b.xmm));
	return a;
}

// Scalar math
__forceinline Vector4w operator*(float a, Vector4w b) {
	Vector4w temp = vector4w(_mm_broadcast_ss(&a));
	return temp * b;
}
__forceinline Vector4w operator*(Vector4w a, float b) {
	return b * a;
}
__forceinline Vector4w & operator*=(Vector4w &a, float b) {
	a = a * b;
	return a;
}
__forceinline Vector4w operator/(Vector4w a, float b) {
	__m128 temp = _mm_broadcast_ss(&b);
	return vector4w(_mm_div_ps(a.xmm, temp));
}
__forceinline Vector4w & operator/=(Vector4w &a, float b) {
	a = a / b;
	return a;
}

// Plus & minus
__forceinline Vector4w operator+(Vector4w a, Vector4w b) {
	return vector4w(_mm_add_ps(a.xmm, b.xmm));
}
__forceinline Vector4w & operator+=(Vector4w &a, Vector4w b) {
	a = a + b;
	return a;
}
__forceinline Vector4w operator-(Vector4w a, Vector4w b) {
	return vector4w(_mm_sub_ps(a.xmm, b.xmm));
}
__forceinline Vector4w & operator-=(Vector4w &a, Vector4w b) {
	a = a - b;
	return a;
}
__forceinline Vector4w operator-(Vector4w a) {
	Vector4w temp = vector4w() - a;
	return vector4w(temp.xmm);
}

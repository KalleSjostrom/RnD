#include <math.h>
#include <immintrin.h>

struct vec4
{
	__m128 xmm;

	vec4 (__m128 v) : xmm (v) {}

	vec4 (float v) { xmm = _mm_broadcast_ss(&v); }

	vec4 (float x, float y, float z, float w)
	{ xmm = _mm_set_ps(w,z,y,x); }

	vec4 (const float *v) { xmm = _mm_load_ps(v); }

	vec4 operator* (const vec4 &v) const
	{ return vec4(_mm_mul_ps(xmm, v.xmm)); }

	vec4 operator+ (const vec4 &v) const
	{ return vec4(_mm_add_ps(xmm, v.xmm)); }

	vec4 operator- (const vec4 &v) const
	{ return vec4(_mm_sub_ps(xmm, v.xmm)); }

	vec4 operator/ (const vec4 &v) const
	{ return vec4(_mm_div_ps(xmm, v.xmm)); }

	void operator*= (const vec4 &v)
	{ xmm = _mm_mul_ps(xmm, v.xmm); }

	void operator+= (const vec4 &v)
	{ xmm = _mm_add_ps(xmm, v.xmm); }

	void operator-= (const vec4 &v)
	{ xmm = _mm_sub_ps(xmm, v.xmm); }

	void operator/= (const vec4 &v)
	{ xmm = _mm_div_ps(xmm, v.xmm); }

	void operator>> (float *v)
	{ _mm_store_ps(v, xmm); }
};

struct v4 {
	float x, y, z, w;

	v4(float px, float py, float pz, float pw) {
		x = px;
		y = py;
		z = pz;
		w = pw;
	}
};

inline v4 operator*(float a, v4 b) {
	return v4(a * b.x, a * b.y, a * b.z, a * b.w);
}
inline v4 operator*(v4 a, v4 b) {
	return v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline v4 operator*(v4 a, float b) {
	return b * a;
}
inline v4 & operator*=(v4 &a, float b) {
	a = a * b;
	return a;
}

inline v4 operator/(v4 a, float b) {
	return v4(a.x/b, a.y/b, a.z/b, a.w/b);
}
inline v4 & operator/=(v4 &a, float b) {
	a = a/b;
	return a;
}

inline v4 operator+(v4 a, v4 b) {
	return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline v4 & operator+=(v4 &a, v4 b) {
	a = a + b;
	return a;
}

inline v4 operator-(v4 a) {
	return v4(-a.x, -a.x, -a.z, -a.w);
}
inline v4 operator-(v4 a, v4 b) {
	return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline v4 & operator-=(v4 &a, v4 b) {
	a = a - b;
	return a;
}

inline v4 hadamard(v4 a, v4 b) {
	return v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline float dot(v4 a, v4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
inline float length_squared(v4 a) {
	return dot(a, a);
}
inline float length(v4 a) {
	return sqrtf(length_squared(a));
}
inline v4 normalize(v4 a) {
	float l = length(a);
	return (1.0f/l) * a;
}

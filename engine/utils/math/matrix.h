#pragma once
#include "quaternion.h"

ALIGNED_TYPE_(struct, 16) {
	float m[16];
	FORCE_INLINE float operator[](int index) { return m[index]; }
// 	FORCE_INLINE float operator[](int index, float rhs) { return m[index] = rhs; }
	// FORCE_INLINE float &operator&() { return &m; }
} m4;

#if USE_INTRINSICS
void mm_matrix_mul(float *am, float *bm, float *cm) {
	__m128 col_a0 = _mm_load_ps(&am[0]);
	__m128 col_a1 = _mm_load_ps(&am[4]);
	__m128 col_a2 = _mm_load_ps(&am[8]);
	__m128 col_a3 = _mm_load_ps(&am[12]);

	__m128 col_c0 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c0 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c0);
	col_c0 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c0);
	col_c0 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c0);

	__m128 col_c1 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c1 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c1);
	col_c1 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c1);
	col_c1 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c1);

	__m128 col_c2 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c2 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c2);
	col_c2 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c2);
	col_c2 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c2);

	__m128 col_c3 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c3 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c3);
	col_c3 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c3);
	col_c3 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c3);

	_mm_store_ps(cm + 0, col_c0);
	_mm_store_ps(cm + 4, col_c1);
	_mm_store_ps(cm + 8, col_c2);
	_mm_store_ps(cm + 12, col_c3);
}
#endif // USE_INTRINSICS

FORCE_INLINE void fill_m4(float *mem,
						float m11, float m12, float m13, float m14,
						float m21, float m22, float m23, float m24,
						float m31, float m32, float m33, float m34,
						float m41, float m42, float m43, float m44) {

	mem[0]  = m11; mem[1]  = m21; mem[2]  = m31; mem[3]  = m41; // column 1
	mem[4]  = m12; mem[5]  = m22; mem[6]  = m32; mem[7]  = m42; // column 2
	mem[8]  = m13; mem[9]  = m23; mem[10] = m33; mem[11] = m43; // column 3
	mem[12] = m14; mem[13] = m24; mem[14] = m34; mem[15] = m44; // column 4
}

FORCE_INLINE m4 Matrix4x4(
						float m11, float m12, float m13, float m14,
						float m21, float m22, float m23, float m24,
						float m31, float m32, float m33, float m34,
						float m41, float m42, float m43, float m44) {

	m4 m = {
		m11, m21, m31, m41, // column 1
		m12, m22, m32, m42, // column 2
		m13, m23, m33, m43, // column 3
		m14, m24, m34, m44, // column 4
	};
	return m;
}

FORCE_INLINE m4 operator*(m4 a, m4 b) {
	m4 c = {};
#if USE_INTRINSICS
	mm_matrix_mul(a.m, b.m, c.m);
#else
	(void)a;
	(void)b;
	ASSERT(false, "Not implemented"); // Not implemented
#endif
	return c;
}

FORCE_INLINE m4 & operator*=(m4 &a, m4 b) {
	m4 c = {};
#if USE_INTRINSICS
	mm_matrix_mul(a.m, b.m, c.m);
#else
	(void)a;
	(void)b;
	ASSERT(false, "Not implemented"); // Not implemented
#endif
	memcpy(&a, &c, sizeof(m4));
	return a;
}

#define INDEX(row, col) col*4+row

FORCE_INLINE v4 operator*(m4 a, v4 b) {
	v4 c;

	c.x =
		a.m[INDEX(0, 0)] * b.x +
		a.m[INDEX(0, 1)] * b.y +
		a.m[INDEX(0, 2)] * b.z +
		a.m[INDEX(0, 3)] * b.w;

	c.y =
		a.m[INDEX(1, 0)] * b.x +
		a.m[INDEX(1, 1)] * b.y +
		a.m[INDEX(1, 2)] * b.z +
		a.m[INDEX(1, 3)] * b.w;

	c.z =
		a.m[INDEX(2, 0)] * b.x +
		a.m[INDEX(2, 1)] * b.y +
		a.m[INDEX(2, 2)] * b.z +
		a.m[INDEX(2, 3)] * b.w;

	c.w =
		a.m[INDEX(3, 0)] * b.x +
		a.m[INDEX(3, 1)] * b.y +
		a.m[INDEX(3, 2)] * b.z +
		a.m[INDEX(3, 3)] * b.w;

	return c;
}

FORCE_INLINE v3 &right_axis(m4 &m) {
	return *(v3*)(m.m + 0);
}
FORCE_INLINE v3 &forward_axis(m4 &m) {
	return *(v3*)(m.m + 8);
}
FORCE_INLINE v3 &up_axis(m4 &m) {
	return *(v3*)(m.m + 4);
}
FORCE_INLINE v3 &translation(m4 &m) {
	return *(v3*)(m.m + 12);
}

FORCE_INLINE v3 multiply_perspective(m4 a, v3 b) {
	v4 c = a * V4(b, 1.0f);
	float t = 1.0f/c.w;
	return V3(c.x, c.y, c.z) * t;
}

FORCE_INLINE m4 Matrix4x4(v3 right, v3 forward, v3 up, v3 position) {
	return Matrix4x4(
			right.x,   right.y,   right.z,   position.x,
			up.x,      up.y,      up.z,      position.y,
			forward.x, forward.y, forward.z, position.z,
			0,         0,         0,         1);
}

FORCE_INLINE m4 Matrix4x4(float v) {
	return Matrix4x4(
			v, v, v, v,
			v, v, v, v,
			v, v, v, v,
			v, v, v, v);
}

FORCE_INLINE m4 identity() {
	return Matrix4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
}

void print_matrix(float *m) {
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%7.4f  ", (double) m[col * 4 + row]);
		}
		printf("\n");
	}
	printf("\n");
}

FORCE_INLINE m4 look_at(v3 eye, v3 l, v3 up) {
	v3 f = normalize(l - eye); // Look direction (Z-axis)
	v3 s = normalize(cross(f, up)); // Horizontal direction (X-axis)
	v3 u = cross(s, f); // Up direction (Y-axis)

	m4 m = Matrix4x4(
		s.x, s.y, s.z, -dot(s, eye),
		u.x, u.y, u.z, -dot(u, eye),
		-f.x, -f.y, -f.z, -dot(-f, eye),
		0.0f, 0.0f, 0.0f, 1.0f);
	return m;
}

FORCE_INLINE m4 view_from_pose(m4 &pose) {
	v3 &f = forward_axis(pose); // Look direction (Z-axis)
	v3 &s = right_axis(pose); // Horizontal direction (X-axis)
	v3 &u = up_axis(pose); // Up direction (Y-axis)
	v3 eye = translation(pose);

	m4 m = Matrix4x4(
		s.x, s.y, s.z, -dot(s, eye),
		u.x, u.y, u.z, -dot(u, eye),
		-f.x, -f.y, -f.z, -dot(-f, eye),
		0.0f, 0.0f, 0.0f, 1.0f);
	return m;
}

FORCE_INLINE m4 perspective(float width, float height, float _near, float _far) {
	return Matrix4x4(
		(2*_near)/width, 0, 0, 0,
		0, (2*_near/height), 0, 0,
		0, 0, -(_far + _near) / (_far - _near), (-2*_far*_near) / (_far-_near),
		0, 0, -1, 0); // This comes from the fact that we should divide by -z to go from homogenous to clip coordinates.
}
FORCE_INLINE m4 perspective(float right, float left, float top, float bottom, float _near, float _far) {
	return Matrix4x4(
		(2*_near) / (right-left), 0, (right+left)/(right-left), 0,
		0, (2*_near) / (top-bottom), (top+bottom)/(top-bottom), 0,
		0, 0, -(_far+_near) / (_far-_near), (-2*_far*_near) / (_far-_near),
		0, 0, -1, 0);
}

FORCE_INLINE m4 perspective_fov(float v_fov, float aspect, float _near, float _far) {
	float angle = (v_fov * DEGREES_TO_RANDIANS) / 2.0f;
	float half_height = tanf(angle) * _near;
	float height = half_height * 2;
	float width = aspect * height;
	printf("%g %g\n", width, height);
	return perspective(width, height, _near, _far);
}

FORCE_INLINE m4 orthographic(float width, float height, float _near, float _far) {
	return Matrix4x4(
		2/width, 0, 0, 0,
		0, 2/height, 0, 0,
		0, 0, -2/(_far-_near), -(_far+_near)/(_far-_near),
		0, 0, 0, 1); // No homogenous coordinates, w is not needed.
}

FORCE_INLINE m4 orthographic(float right, float left, float top, float bottom, float _near, float _far) {
	return Matrix4x4(
		2/(right-left), 0, 0, -(right+left)/(right-left),
		0, 2/(top-bottom), 0, -(top+bottom)/(top-bottom),
		0, 0, -2/(_far-_near), -(_far+_near)/(_far-_near),
		0, 0, 0, 1); // No homogenous coordinates, w is not needed.
}


FORCE_INLINE void rotate_around(m4 &pose, q4 q) {
	v3 &x = *(v3*)(pose.m + 0);
	v3 &y = *(v3*)(pose.m + 4);
	v3 &z = *(v3*)(pose.m + 8);

	x = ::rotate_around(q, x);
	y = ::rotate_around(q, y);
	z = ::rotate_around(q, z);
}

FORCE_INLINE void rotate_around(m4 &pose, float angle, float x, float y) {
	float ca = cosf(angle);
	float sa = sinf(angle);

	float rx = -ca*x + sa*y + x;
	float ry = -sa*x - ca*y + y;

	m4 rotation = identity();

	rotation.m[INDEX(0, 3)] = rx;
	rotation.m[INDEX(1, 3)] = ry;

	rotation.m[INDEX(0, 0)] = ca;
	rotation.m[INDEX(0, 1)] = -sa;
	rotation.m[INDEX(1, 0)] = sa;
	rotation.m[INDEX(1, 1)] = ca;

	pose *= rotation;
}

FORCE_INLINE void rotate(m4 &pose, float angle) {
	float ca = cosf(angle);
	float sa = sinf(angle);

	m4 rotation = identity();

	rotation.m[INDEX(0, 0)] = ca;
	rotation.m[INDEX(0, 1)] = -sa;
	rotation.m[INDEX(1, 0)] = sa;
	rotation.m[INDEX(1, 1)] = ca;

	pose *= rotation;
}

FORCE_INLINE void set_rotation(m4 &pose, float angle) {
	float ca = cosf(angle);
	float sa = sinf(angle);

	pose.m[INDEX(0, 0)] = ca;
	pose.m[INDEX(0, 1)] = -sa;
	pose.m[INDEX(1, 0)] = sa;
	pose.m[INDEX(1, 1)] = ca;
}

FORCE_INLINE void set_scale(m4 &pose, v3 scale) {
	pose.m[INDEX(0, 0)] = scale.x;
	pose.m[INDEX(1, 1)] = scale.y;
	pose.m[INDEX(2, 2)] = scale.z;
}

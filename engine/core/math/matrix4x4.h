#pragma once

ALIGNED_TYPE(struct, 16) {
	float m[16];
	__forceinline float operator[](int index) { return m[index]; }
} Matrix4x4;

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

__forceinline void fill_m4(float *mem,
						float m11, float m12, float m13, float m14,
						float m21, float m22, float m23, float m24,
						float m31, float m32, float m33, float m34,
						float m41, float m42, float m43, float m44) {

	mem[0]  = m11; mem[1]  = m21; mem[2]  = m31; mem[3]  = m41; // column 1
	mem[4]  = m12; mem[5]  = m22; mem[6]  = m32; mem[7]  = m42; // column 2
	mem[8]  = m13; mem[9]  = m23; mem[10] = m33; mem[11] = m43; // column 3
	mem[12] = m14; mem[13] = m24; mem[14] = m34; mem[15] = m44; // column 4
}

__forceinline Matrix4x4 matrix4x4(
						float m11, float m12, float m13, float m14,
						float m21, float m22, float m23, float m24,
						float m31, float m32, float m33, float m34,
						float m41, float m42, float m43, float m44) {

	Matrix4x4 m = {
		m11, m21, m31, m41, // column 1
		m12, m22, m32, m42, // column 2
		m13, m23, m33, m43, // column 3
		m14, m24, m34, m44, // column 4
	};
	return m;
}

__forceinline Matrix4x4 operator*(Matrix4x4 a, Matrix4x4 b) {
	Matrix4x4 c = {};
	mm_matrix_mul(a.m, b.m, c.m);
	return c;
}

__forceinline Matrix4x4 & operator*=(Matrix4x4 &a, Matrix4x4 b) {
	Matrix4x4 c = {};
	mm_matrix_mul(a.m, b.m, c.m);
	memcpy(&a, &c, sizeof(Matrix4x4));
	return a;
}

__forceinline int matrix4x4_index(int row, int col) {
	return col*4 + row;
}

__forceinline Vector4 operator*(Matrix4x4 a, Vector4 b) {
	Vector4 c;
	c.x =
		a.m[matrix4x4_index(0, 0)] * b.x +
		a.m[matrix4x4_index(0, 1)] * b.y +
		a.m[matrix4x4_index(0, 2)] * b.z +
		a.m[matrix4x4_index(0, 3)] * b.w;

	c.y =
		a.m[matrix4x4_index(1, 0)] * b.x +
		a.m[matrix4x4_index(1, 1)] * b.y +
		a.m[matrix4x4_index(1, 2)] * b.z +
		a.m[matrix4x4_index(1, 3)] * b.w;

	c.z =
		a.m[matrix4x4_index(2, 0)] * b.x +
		a.m[matrix4x4_index(2, 1)] * b.y +
		a.m[matrix4x4_index(2, 2)] * b.z +
		a.m[matrix4x4_index(2, 3)] * b.w;

	c.w =
		a.m[matrix4x4_index(3, 0)] * b.x +
		a.m[matrix4x4_index(3, 1)] * b.y +
		a.m[matrix4x4_index(3, 2)] * b.z +
		a.m[matrix4x4_index(3, 3)] * b.w;

	return c;
}

__forceinline Vector3 &right_axis(Matrix4x4 &m) {
	return *(Vector3*)(m.m + 0);
}
__forceinline Vector3 &forward_axis(Matrix4x4 &m) {
	return *(Vector3*)(m.m + 8);
}
__forceinline Vector3 &up_axis(Matrix4x4 &m) {
	return *(Vector3*)(m.m + 4);
}
__forceinline Vector3 &translation(Matrix4x4 &m) {
	return *(Vector3*)(m.m + 12);
}

__forceinline Vector3 multiply_perspective(Matrix4x4 a, Vector3 b) {
	Vector4 c = a * vector4(b, 1.0f);
	float t = 1.0f/c.w;
	return vector3(c.x, c.y, c.z) * t;
}

__forceinline Matrix4x4 matrix4x4(Vector3 right, Vector3 forward, Vector3 up, Vector3 position) {
	return matrix4x4(
			right.x,   right.y,   right.z,   position.x,
			up.x,      up.y,      up.z,      position.y,
			forward.x, forward.y, forward.z, position.z,
			0,         0,         0,         1);
}

__forceinline Matrix4x4 matrix4x4(float v) {
	return matrix4x4(
			v, v, v, v,
			v, v, v, v,
			v, v, v, v,
			v, v, v, v);
}

__forceinline Matrix4x4 identity() {
	return matrix4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
}

void matrix4x4_tostring(float *m) {
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%7.4f  ", (double) m[col * 4 + row]);
		}
		printf("\n");
	}
	printf("\n");
}

__forceinline Matrix4x4 look_at(Vector3 eye, Vector3 l, Vector3 up) {
	Vector3 f = normalize(l - eye); // Look direction (Z-axis)
	Vector3 s = normalize(cross(f, up)); // Horizontal direction (X-axis)
	Vector3 u = cross(s, f); // Up direction (Y-axis)

	Matrix4x4 m = matrix4x4(
		s.x, s.y, s.z, -dot(s, eye),
		u.x, u.y, u.z, -dot(u, eye),
		-f.x, -f.y, -f.z, -dot(-f, eye),
		0.0f, 0.0f, 0.0f, 1.0f);
	return m;
}

__forceinline Matrix4x4 view_from_pose(Matrix4x4 &pose) {
	Vector3 &f = forward_axis(pose); // Look direction (Z-axis)
	Vector3 &s = right_axis(pose); // Horizontal direction (X-axis)
	Vector3 &u = up_axis(pose); // Up direction (Y-axis)
	Vector3 eye = translation(pose);

	Matrix4x4 m = matrix4x4(
		s.x, s.y, s.z, -dot(s, eye),
		u.x, u.y, u.z, -dot(u, eye),
		-f.x, -f.y, -f.z, -dot(-f, eye),
		0.0f, 0.0f, 0.0f, 1.0f);
	return m;
}

__forceinline Matrix4x4 perspective(float width, float height, float _near, float _far) {
	return matrix4x4(
		(2*_near)/width, 0, 0, 0,
		0, (2*_near/height), 0, 0,
		0, 0, -(_far + _near) / (_far - _near), (-2*_far*_near) / (_far-_near),
		0, 0, -1, 0); // This comes from the fact that we should divide by -z to go from homogenous to clip coordinates.
}
__forceinline Matrix4x4 perspective(float right, float left, float top, float bottom, float _near, float _far) {
	return matrix4x4(
		(2*_near) / (right-left), 0, (right+left)/(right-left), 0,
		0, (2*_near) / (top-bottom), (top+bottom)/(top-bottom), 0,
		0, 0, -(_far+_near) / (_far-_near), (-2*_far*_near) / (_far-_near),
		0, 0, -1, 0);
}

__forceinline Matrix4x4 perspective_fov(float v_fov, float aspect, float _near, float _far) {
	float degrees_to_randians = (float)M_PI/180.0f;
	float angle = (v_fov * degrees_to_randians) / 2.0f;
	float half_height = tanf(angle) * _near;
	float height = half_height * 2;
	float width = aspect * height;
	printf("%g %g\n", width, height);
	return perspective(width, height, _near, _far);
}

__forceinline Matrix4x4 orthographic(float width, float height, float _near, float _far) {
	return matrix4x4(
		2/width, 0, 0, 0,
		0, 2/height, 0, 0,
		0, 0, -2/(_far-_near), -(_far+_near)/(_far-_near),
		0, 0, 0, 1); // No homogenous coordinates, w is not needed.
}

__forceinline Matrix4x4 orthographic(float right, float left, float top, float bottom, float _near, float _far) {
	return matrix4x4(
		2/(right-left), 0, 0, -(right+left)/(right-left),
		0, 2/(top-bottom), 0, -(top+bottom)/(top-bottom),
		0, 0, -2/(_far-_near), -(_far+_near)/(_far-_near),
		0, 0, 0, 1); // No homogenous coordinates, w is not needed.
}

__forceinline void rotate_around(Matrix4x4 &pose, Quaternion q) {
	Vector3 &x = *(Vector3*)(pose.m + 0);
	Vector3 &y = *(Vector3*)(pose.m + 4);
	Vector3 &z = *(Vector3*)(pose.m + 8);

	x = ::rotate_around(q, x);
	y = ::rotate_around(q, y);
	z = ::rotate_around(q, z);
}

__forceinline void rotate_around(Matrix4x4 &pose, float angle, float x, float y) {
	float ca = cosf(angle);
	float sa = sinf(angle);

	float rx = -ca*x + sa*y + x;
	float ry = -sa*x - ca*y + y;

	Matrix4x4 rotation = identity();

	rotation.m[matrix4x4_index(0, 3)] = rx;
	rotation.m[matrix4x4_index(1, 3)] = ry;

	rotation.m[matrix4x4_index(0, 0)] = ca;
	rotation.m[matrix4x4_index(0, 1)] = -sa;
	rotation.m[matrix4x4_index(1, 0)] = sa;
	rotation.m[matrix4x4_index(1, 1)] = ca;

	pose *= rotation;
}

__forceinline void rotate(Matrix4x4 &pose, float angle) {
	float ca = cosf(angle);
	float sa = sinf(angle);

	Matrix4x4 rotation = identity();

	rotation.m[matrix4x4_index(0, 0)] = ca;
	rotation.m[matrix4x4_index(0, 1)] = -sa;
	rotation.m[matrix4x4_index(1, 0)] = sa;
	rotation.m[matrix4x4_index(1, 1)] = ca;

	pose *= rotation;
}

__forceinline void set_rotation(Matrix4x4 &pose, float angle) {
	float ca = cosf(angle);
	float sa = sinf(angle);

	pose.m[matrix4x4_index(0, 0)] = ca;
	pose.m[matrix4x4_index(0, 1)] = -sa;
	pose.m[matrix4x4_index(1, 0)] = sa;
	pose.m[matrix4x4_index(1, 1)] = ca;
}

__forceinline void set_scale(Matrix4x4 &pose, Vector3 scale) {
	pose.m[matrix4x4_index(0, 0)] = scale.x;
	pose.m[matrix4x4_index(1, 1)] = scale.y;
	pose.m[matrix4x4_index(2, 2)] = scale.z;
}

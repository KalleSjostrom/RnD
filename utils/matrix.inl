struct m4 {
	float m[16] __attribute__((aligned(16)));
	__forceinline float operator[](int index) { return m[index]; }
// 	__forceinline float operator[](int index, float rhs) { return m[index] = rhs; }
	// __forceinline float &operator&() { return &m; }
};

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

__forceinline m4 Matrix4x4(
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

__forceinline m4 operator*(m4 a, m4 b) {
	m4 c = {};
	mm_matrix_mul(a.m, b.m, c.m);
	return c;
}

#define INDEX(row, col) col*4+row

__forceinline v4 operator*(m4 a, v4 b) {
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

__forceinline v3 multiply_perspective(m4 a, v3 b) {
	v4 c = a * V4(b, 1.0f);
	float t = 1.0f/c.w;
	return V3(c.x, c.y, c.z) * t;
}

__forceinline m4 Matrix4x4(float v) {
	return Matrix4x4(
			v, v, v, v,
			v, v, v, v,
			v, v, v, v,
			v, v, v, v);
}

__forceinline m4 identity() {
	return Matrix4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
}

__forceinline void set_translation(m4 &m, v3 translation) {
	m.m[12] = translation.x;
	m.m[13] = translation.y;
	m.m[14] = translation.z;
}
__forceinline v3 translation(m4 &m) {
	v3 t = { m.m[12], m.m[13], m.m[14] };
	return t;
}

void print_matrix(float *m) {
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%7.4f  ", m[col * 4 + row]);
		}
		printf("\n");
	}
	printf("\n");
}

__forceinline m4 look_at(v3 eye, v3 l, v3 up) {
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

__forceinline m4 perspective(float width, float height, float near, float far) {
	// Use similar triangles.
	// xp/xw = -n/zw -- solve for xp -> (n*xw)/-zw
	// Map xw into normalized device coordinates (NDC) - > xn = (1 - (-1))xw / (r-1) + beta. Since this only supports a symmetric viewport, beta is 0. (r+l)/(r-l)
	return Matrix4x4(
		(2*near)/width, 0, 0, 0,
		0, (2*near/height), 0, 0,
		0, 0, -(far + near) / (far - near), (-2*far*near) / (far-near),
		0, 0, -1, 0); // This comes from the fact that we should divide by -z to go from homogenous to clip coordinates.
}
__forceinline m4 perspective(float right, float left, float top, float bottom, float near, float far) {
	return Matrix4x4(
		(2*near) / (right-left), 0, (right+left)/(right-left), 0,
		0, (2*near) / (top-bottom), (top+bottom)/(top-bottom), 0,
		0, 0, -(far+near) / (far-near), (-2*far*near) / (far-near),
		0, 0, -1, 0);
}

__forceinline m4 perspective_fov(float v_fov, float aspect, float near, float far) {
	float angle = (v_fov * DEGREES_TO_RANDIANS) / 2.0f;
	float half_height = tanf(angle) * near;
	float height = half_height * 2;
	float width = aspect * height;
	return perspective(width, height, near, far);
}

__forceinline m4 orthographic(float width, float height, float near, float far) {
	return Matrix4x4(
		2/width, 0, 0, 0,
		0, 2/height, 0, 0,
		0, 0, -2/(far-near), -(far+near)/(far-near),
		0, 0, 0, 1); // No homogenous coordinates, w is not needed.
}

__forceinline m4 orthographic(float right, float left, float top, float bottom, float near, float far) {
	return Matrix4x4(
		2/(right-left), 0, 0, -(right+left)/(right-left),
		0, 2/(top-bottom), 0, -(top+bottom)/(top-bottom),
		0, 0, -2/(far-near), -(far+near)/(far-near),
		0, 0, 0, 1); // No homogenous coordinates, w is not needed.
}

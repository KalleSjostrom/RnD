struct q4 {
	union {
		struct {
			f32 x, y, z, w;
		};
		struct {
			v3 xyz;
		};
		struct {
			v4 xyzw;
		};
	};
};

FORCE_INLINE q4 operator*(q4 a, q4 b) {
    q4 c;

    c.x = (a.x * b.w) + (a.y * b.z) - (a.z * b.y) + (a.w * b.x);
    c.y = (-a.x * b.z) + (a.y * b.w) + (a.z * b.x) + (a.w * b.y);
    c.z = (a.x * b.y) - (a.y * b.x) + (a.z * b.w) + (a.w * b.z);
    c.w = (-a.x * b.x) - (a.y * b.y) - (a.z * b.z) + (a.w * b.w);

    return c;
}

q4 Quaternion(v3 axis, float angle) {
	float half_angle = angle / 2.0f;
	float s, c;
	sincosf(half_angle, &s, &c);

	q4 q = {};
	q.x = axis.x * s;
	q.y = axis.y * s;
	q.z = axis.z * s;
	q.w = c;
	return q;
}

void to_axis_angle(q4 q, v3 &out_axis, float &out_angle) {
	float s, c;
	c = q.w;
	s = sqrtf(1 - c*c);

	float half_angle = acosf(c);

	// TODO(kalle): Risk of div by 0
	out_axis.x = q.x / s;
	out_axis.y = q.y / s;
	out_axis.z = q.z / s;
	out_angle = half_angle * 2.0f;
}

v3 rotate_around(q4 q, v3 v) {
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

q4 normalize(q4 q) {
	q4 qn = {};
	qn.xyzw = normalize(q.xyzw);
	return qn;
}

q4 inverse(q4 b) { // conjugate
	q4 a = { -b.x, -b.y, -b.z, b.w };
	return a;
}

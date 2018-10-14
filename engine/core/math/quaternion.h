#pragma once

struct Quaternion {
	union {
		struct {
			float x, y, z, w;
		};
		struct {
			Vector3 xyz;
		};
		struct {
			Vector4 xyzw;
		};
	};
	float &operator[](int index) {
		return *((float*)this + index);
	}
};

__forceinline Quaternion operator*(Quaternion a, Quaternion b) {
    Quaternion c;

    c.x = (a.x * b.w) + (a.y * b.z) - (a.z * b.y) + (a.w * b.x);
    c.y = (-a.x * b.z) + (a.y * b.w) + (a.z * b.x) + (a.w * b.y);
    c.z = (a.x * b.y) - (a.y * b.x) + (a.z * b.w) + (a.w * b.z);
    c.w = (-a.x * b.x) - (a.y * b.y) - (a.z * b.z) + (a.w * b.w);

    return c;
}

Quaternion quaternion(Vector3 axis, float angle) {
	float half_angle = angle / 2.0f;
	float s, c;
	sincosf(half_angle, &s, &c);

	Quaternion q = {};
	q.x = axis.x * s;
	q.y = axis.y * s;
	q.z = axis.z * s;
	q.w = c;
	return q;
}

void to_axis_angle(Quaternion q, Vector3 &out_axis, float &out_angle) {
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

Vector3 rotate_around(Quaternion q, Vector3 v) {
	return v + 2.0f * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

Quaternion normalize(Quaternion q) {
	Quaternion qn = {};
	qn.xyzw = normalize(q.xyzw);
	return qn;
}

Quaternion inverse(Quaternion b) { // conjugate
	Quaternion a = { -b.x, -b.y, -b.z, b.w };
	return a;
}

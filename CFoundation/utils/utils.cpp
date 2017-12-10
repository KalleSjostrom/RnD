#define ENUM_WITH_STRING_VALUE(value) value,
#define ENUM_WITH_STRING_STRING(value) #value,
#define ENUM_WITH_STRINGS(typeName, values)\
	enum typeName { values(ENUM_WITH_STRING_VALUE) typeName##_Count, };\
	const char* typeName##_EnumStr[] = { values(ENUM_WITH_STRING_STRING) };

#define BOOL_STR(b) (b ? "true" : "false")

namespace vec {
	static const Vector3 right = vector3(1, 0, 0);
	static const Vector3 forward = vector3(0, 1, 0);
	static const Vector3 up = vector3(0, 0, 1);
	static const Vector3 left = vector3(-1, 0, 0);
	static const Vector3 backward = vector3(0, -1, 0);
	static const Vector3 down = vector3(0, 0, -1);

	static const Vector3 zero = vector3(0, 0, 0);
	static const Vector3 ones = vector3(1, 1, 1);
}

namespace utils {
	float wrap_norm_angle(float angle) {
		angle = fmod(angle, 1.0f);
		if (angle <= -0.5f) {
			return angle + 1.0f;
		} else if (angle > 0.5f) {
			return angle - 1.0f;
		}
		return angle;
	}
	float rotate_norm_angle_towards(float source, float destination, float speed, float dt) {
		float direction = math::sign(destination - source);
		float distance = abs(destination - source);
		if (distance > 0.5f) {
			direction = -direction;
		}

		float next_step = wrap_norm_angle(source + direction * speed * dt);
		float distance_before = distance;
		float distance_after = abs(destination - next_step);

		if (distance_before > 0.5f)
			distance_before = 1.0f - distance_before;

		if (distance_after > 0.5)
			distance_after = 1.0f - distance_after;

		// If updated forward is further away than from source, then we are done.
		bool was_closer_before = distance_before < distance_after;
		return was_closer_before ? destination : next_step;
	}

	float wrap_angle(float angle) {
		angle = fmod(angle, math::tau);
		if (angle <= -math::pi) {
			return angle + math::tau;
		} else if (angle > math::pi) {
			return angle - math::tau;
		}
		return angle;
	}

	float rotate_angle_towards(float source, float destination, float speed, float dt) {
		float direction = math::sign(destination - source);
		float distance = abs(destination - source);
		if (distance > math::pi) {
			direction = -direction;
		}

		float next_step = wrap_angle(source + direction * speed * dt);
		float distance_before = distance;
		float distance_after = abs(destination - next_step);

		if (distance_before > math::pi)
			distance_before = math::tau - distance_before;

		if (distance_after > math::pi)
			distance_after = math::tau - distance_after;

		// If updated forward is further away than from source, then we are done.
		bool was_closer_before = distance_before < distance_after;
		return was_closer_before ? destination : next_step;
	}

	float move_towards(float source, float destination, float speed, float dt) {
		float next_step = source + math::sign(destination - source) * speed * dt;
		// If updated forward is further away than from source, then we are done.
		bool was_closer_before = abs(destination - source) < abs(destination - next_step);
		return was_closer_before ? destination : next_step;
	}

	Vector3 move_towards_v3(Vector3 source, Vector3 destination, float speed, float dt) {
		Vector3 next_step = source + normalize(destination - source) * speed * dt;
		// If updated forward is further away than from source, then we are done.
		bool was_closer_before = length_squared(destination - source) < length_squared(destination - next_step);
		return was_closer_before ? destination : next_step;
	}

	_forceinline float quat_quat_angle(Quaternion left, Quaternion right){
		float quat_dot = dot(left, right);
		quat_dot = math::clamp(quat_dot, -1, 1); // we could get 1.000001
		quat_dot = abs(quat_dot); // for when the directions of the quaternions point in separate directions
		float angle = abs(2 * acos(quat_dot));
		return angle;
	}

	_forceinline Quaternion rotate_quaternion_towards(Quaternion source, Quaternion destination, float speed, float dt){
		if (dt == 0) {
			return source;
		}

		Quaternion source_norm      = normalize(source);
		Quaternion destination_norm = normalize(destination);

		float angle = quat_quat_angle(source_norm, destination_norm);
		float delta = speed * dt;
		if (angle <= delta) {
			return destination_norm;
		}

		float t = delta / angle;
		Quaternion ret = slerp(source_norm, destination_norm, t);
		return ret;
	}
	__forceinline Quaternion normalized_angle_to_quaternion(float angle) {
		angle = (angle - 0.5f) * math::tau;
		return quaternion(vector3(0, 0, 1), -angle);
	}
	__forceinline float normalized_angle_from_direction(Vector3 direction) {
		return atan2(direction.x, direction.y) / (math::tau) + 0.5f;
	}
	__forceinline Vector3 forward(Quaternion q) {
		return forward_axis(matrix4x4(q));
	}
	__forceinline Vector3 right(Quaternion q) {
		return right_axis(matrix4x4(q));
	}
	__forceinline Vector3 up(Quaternion q) {
		return up_axis(matrix4x4(q));
	}
	__forceinline Vector3 forward(UnitRef unit, unsigned node) {
		return forward_axis(*_Unit.world_pose(unit, node));
	}
	__forceinline Vector3 right(UnitRef unit, unsigned node) {
		return right_axis(*_Unit.world_pose(unit, node));
	}
	__forceinline Vector3 up(UnitRef unit, unsigned node) {
		return up_axis(*_Unit.world_pose(unit, node));
	}
	__forceinline Vector3 transform_forward(const LocalTransform &lt) {
		return forward(quaternion(lt.rot));
	}
	__forceinline Matrix4x4 from_quaternion_translation(Quaternion q, Vector3 t) {
		Matrix4x4 tm = matrix4x4(q);
		translation(tm) = t;
		return tm;
	}
	__forceinline Matrix4x4 look(Vector3 direction, Vector3 up = vector3(0, 0, 1)) {
		Matrix4x4 m = matrix4x4_identity();
		forward_axis(m) = normalize(direction);
		up_axis(m) = orthonormalize(up, forward_axis(m));
		right_axis(m) = cross(forward_axis(m), up_axis(m));
		return m;
	}
	__forceinline Quaternion look_quat(Vector3 direction, Vector3 up = vector3(0, 0, 1)) {
		return quaternion_orthogonal(look(direction, up));
	}

	__forceinline float pitch(Quaternion &q) {
		return atan2(2*(q.y*q.z + q.w*q.x), q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);
	}
	__forceinline float roll(Quaternion &q) {
		return asin(-2*(q.x*q.z - q.w*q.y));
	}
	__forceinline float yaw(Quaternion &q) {
		return atan2(2*(q.x*q.y + q.w*q.z), q.w*q.w + q.x*q.x - q.y*q.y - q.z*q.z);
	}

	__forceinline Quaternion from_yaw_pitch_roll(float yaw, float pitch, float roll) {
		Quaternion a = quaternion(vec::up, yaw);
		Quaternion b = quaternion(vec::right, pitch);
		Quaternion c = quaternion(vec::forward, roll);

		return (a * b) * c;
	}

	__forceinline Vector3 rotate_around(const Vector3 &axis, const Vector3 &vector, float angle) {
		// Uses Rodrigues formula - split the input vector into parallel and perpendicular components relative the axis of rotation.
		Vector3 parallel = axis * dot(axis, vector); // project direction onto k
		float c = cos(angle);
		float s = sin(angle);
		return vector * c + cross(axis, vector) * s + parallel*(1-c);
	}

	__forceinline DynamicScriptDataItem make_script_data_item(void *pointer, DynamicScriptDataType type, unsigned size = 0) { // 0 means "figure it out"
		DynamicScriptDataItem item = {};
		switch (type) {
			case D_DATA_NIL_TYPE : { item.pointer = 0; item.type = type; item.size = 0; }; break;
			case D_DATA_BOOLEAN_TYPE : { item.pointer = pointer; item.type = type; item.size = sizeof(unsigned); }; break;
			case D_DATA_NUMBER_TYPE : { item.pointer = pointer; item.type = type; item.size = sizeof(float); }; break;
			case D_DATA_STRING_TYPE : { item.pointer = pointer; item.type = type; item.size = size; }; break;
			case D_DATA_CUSTOM_TVECTOR2 : { item.pointer = pointer; item.type = type; item.size = sizeof(Vector2); }; break;
			case D_DATA_CUSTOM_TVECTOR3 : { item.pointer = pointer; item.type = type; item.size = sizeof(Vector3); }; break;
			case D_DATA_CUSTOM_TVECTOR4 : { item.pointer = pointer; item.type = type; item.size = sizeof(Vector4); }; break;
			case D_DATA_CUSTOM_TMATRIX4X4 : { item.pointer = pointer; item.type = type; item.size = sizeof(Matrix4x4); }; break;
			case D_DATA_CUSTOM_TUNITREFERENCE : { item.pointer = pointer; item.type = type; item.size = sizeof(UnitRef); }; break;
			case D_DATA_CUSTOM_TPOINTER : { item.pointer = pointer; item.type = type; item.size = size; }; break;
			default : { ASSERT(false, "Uncrecognized script data type! (type=%d)", type); }
		};
		return item;
	}
}

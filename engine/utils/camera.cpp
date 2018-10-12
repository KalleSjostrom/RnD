struct Camera {
	Matrix4x4 pose;
	Matrix4x4 view;
	Matrix4x4 projection;
};

void update_view(Camera &camera) {
	camera.view = view_from_pose(camera.pose);
}

void set_position(Camera &camera, Vector3 position) {
	translation(camera.pose) = position;
	update_view(camera);
}

void setup_camera(Camera &camera, Vector3 position, float fov, float aspect_ratio) {
	camera.pose = identity();
	forward_axis(camera.pose) = V3(0.0f, 0.0f, -1.0f);
	set_position(camera, position);

	camera.projection = perspective_fov(fov, aspect_ratio, 0.1f, 10000.0f);
}

void begin_frame(Camera &camera, GLint projection_location, GLint view_location, GLint camera_location) {
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat*)(camera.projection.m));
	glUniformMatrix4fv(view_location, 1, GL_FALSE, (GLfloat*)(camera.view.m));

	// TODO(kalle): Better to extract the position from the view matrix inside the shader??
	Vector3 camera_position = translation(camera.pose);
	glUniform3fv(view_location, 1, (GLfloat*)(&camera_position));
}

bool move(Camera &camera, InputData &input, float translation_speed, float rotation_speed, float dt) {
	bool moved = false;

	v2 m = {};
	if (IS_HELD(input, InputKey_A)) {
		m.x = -1;
		moved = true;
	}
	if (IS_HELD(input, InputKey_S)) {
		m.y = -1;
		moved = true;
	}
	if (IS_HELD(input, InputKey_D)) {
		m.x = 1;
		moved = true;
	}
	if (IS_HELD(input, InputKey_W)) {
		m.y = 1;
		moved = true;
	}

	if (IS_HELD(input, InputKey_MouseRight)) {
		translation_speed *= 8;
	}

	Matrix4x4 &pose = camera.pose;

	Vector3 &x = *(Vector3*)(pose.m + 0);
	Vector3 &y = *(Vector3*)(pose.m + 4);
	Vector3 &z = *(Vector3*)(pose.m + 8);
	Vector3 &position = translation(pose);

	position += x * (m.x * dt * translation_speed);
	position += z * (m.y * dt * translation_speed);

	if (IS_HELD(input, InputKey_MouseLeft)) {
		Vector3 world_up = V3(0, 1, 0);
		q4 qx = Quaternion(world_up, -input.mouse_xrel * dt * rotation_speed);
		q4 qy = Quaternion(x, -input.mouse_yrel * dt * rotation_speed);
		q4 q = qx * qy;

		x = ::rotate_around(q, x);
		y = ::rotate_around(q, y);
		z = ::rotate_around(q, z);

		if (input.mouse_xrel != 0 || input.mouse_yrel != 0) {
			moved = true;
		}
	}

	update_view(camera);
	return moved;
}
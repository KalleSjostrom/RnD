struct Camera {
	m4 pose;
	m4 view;
	m4 projection;
};

void update_view(Camera &camera) {
	camera.view = view_from_pose(camera.pose);
}

void set_position(Camera &camera, v3 position) {
	translation(camera.pose) = position;
	update_view(camera);
}

void setup_camera(Camera &camera, v3 position, float aspect_ratio) {
	camera.pose = identity();
	forward_axis(camera.pose) = V3(0.0f, 0.0f, -1.0f);
	set_position(camera, position);

	camera.projection = perspective_fov(60, aspect_ratio, 0.1f, 10000.0f);
}

void begin_frame(Camera &camera, GLint projection_location, GLint view_location) {
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat*)(camera.projection.m));
	glUniformMatrix4fv(view_location, 1, GL_FALSE, (GLfloat*)(camera.view.m));
}

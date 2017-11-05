struct Camera {
	v3 position;
	i32 __padding;

	m4 view;
	m4 projection;
	m4 view_projection;
};

inline void set_position(Camera &camera, v3 position) {
	camera.view = look_at(position, V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
	camera.position = position;
}

void set_projection(Camera &camera, float aspect_ratio) {
	camera.projection = perspective_fov(90, aspect_ratio, 1.0f, 1000.0f);
}

void setup_camera(Camera &camera, v3 position, float aspect_ratio) {
	set_position(camera, position);
	camera.projection = perspective_fov(90, aspect_ratio, 1.0f, 1000.0f);
}

void set_projection(Camera &camera, float width, float height) {
	camera.projection = orthographic(width, height, 0.1f, 100.0f);
}
void setup_camera(Camera &camera, v3 position, float width, float height) {
	set_position(camera, position);
	camera.projection = orthographic(width, height, 0.1f, 100.0f);
}

void begin_frame(Camera &camera, GLint view_projection_location) {
	camera.view_projection = camera.projection * camera.view;
	glUniformMatrix4fv(view_projection_location, 1, GL_FALSE, (GLfloat*)(camera.view_projection.m));
}

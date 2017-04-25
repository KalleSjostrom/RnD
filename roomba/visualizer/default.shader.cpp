namespace shader_default {
static const char *vertex = GLSL(
	uniform mat4 model;
	uniform mat4 view_projection;

	layout(location = 0) in vec3 positions;

	void main(){
		gl_Position = view_projection * model * vec4(positions, 1.0f);
	}
);

static const char *fragment = GLSL(
	out vec4 color;
	void main(){
		color = vec4(0.8f, 0.1f, 0.1f, 1.0f);
	}
);
}

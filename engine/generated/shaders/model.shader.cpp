namespace shader_model {
static const char *vertex = GLSL(
	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 model;

	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 normal;
	layout(location = 2) in vec2 uv;

	out vec4 fpos;
	out vec4 fnormal;
	out vec4 flight;
	out vec2 fuv;

	void main() {
		mat4 mv = view * model;
		mat4 m = projection * mv;

		gl_Position = m * vec4(position, 1.0f);
		fpos = mv * vec4(position, 1.0f);

		// Read up on why the transpose of the inverse works!
		vec3 light_direction = normalize(vec3(1, -1, 0));
		fnormal = transpose(inverse(mv)) * vec4(normal, 1.0f);
		flight = transpose(inverse(mv)) * vec4(light_direction, 1.0f);
		fuv = uv;
	}
);

static const char *fragment = GLSL(
	uniform sampler2D diffuse;
	uniform sampler2D specular;
	uniform sampler2D emissive;
	uniform sampler2D bump;
	uniform sampler2D translucency;

	uniform vec3 camera;

	in vec4 fpos;
	in vec4 fnormal;
	in vec4 flight;
	in vec2 fuv;
	out vec4 color;

	void main() {
		vec3 normal = fnormal.xyz;

		color = texture(diffuse, fuv);
		color.a = texture(translucency, fuv).x;

		float d = dot(-flight.xyz, normal);
		color.xyz *= clamp(d, 0.3f, 1.0f);
	}
);
}

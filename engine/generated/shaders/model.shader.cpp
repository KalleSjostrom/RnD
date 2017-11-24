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
	out vec2 fuv;

	void main() {
		mat4 mv = view * model;
		mat4 m = projection * mv;

		gl_Position = m * vec4(position, 1.0f);
		fpos = mv * vec4(position, 1.0f);

		// Read up on why the transpose of the inverse works!
		fnormal = transpose(inverse(mv)) * vec4(normal, 1.0f);
		fuv = uv;
	}
);

static const char *fragment = GLSL(
	in vec4 fpos;
	in vec4 fnormal;
	in vec2 fuv;
	out vec4 color;

	uniform sampler2D diffuse;

	void main() {
		vec3 n = (fnormal.xyz + vec3(1)) * 0.5f;

		vec3 p = vec3(0, 0, 0); // fpos.xyz
		vec3 v = vec3(0, 0, 10);
		vec3 l = vec3(5, 10, 10);

		vec3 light_to_frag = normalize(p - l);
		vec3 r = 2 * dot(light_to_frag, n) * n - light_to_frag; //  reflect(light_to_frag, n);
		vec3 view_to_frag = normalize(p - v);
		float spec = dot(r, view_to_frag);

		vec3 c = vec3(1, 1, 1);
		c *= (spec+1)*0.5f;

		// color = vec4(c, 1);
		color = texture(diffuse, fuv);
	}
);
}

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
		color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
);
}

namespace shader_sphere {
static const char *vertex = GLSL(
	uniform mat4 model;

	layout(location = 0) in vec3 positions;

	void main(){
		gl_Position = model * vec4(positions, 1.0f);
	}
);

static const char *geometry = GLSL(
	uniform mat4 view_projection;
	out vec2 center;
	out vec2 pos;

	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;

	void main() {
		gl_Position = vec4(0, 0, 0, 0);

		center = gl_in[0].gl_Position.xy;
		vec2 zw = gl_in[0].gl_Position.zw;

		float radius = 100;

		pos = vec2(center.x - radius, center.y - radius);
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		pos = vec2(center.x + radius, center.y - radius);
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		pos = vec2(center.x - radius, center.y + radius);
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		pos = vec2(center.x + radius, center.y + radius);
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		EndPrimitive();
	}
);

static const char *fragment = GLSL(
	in vec2 center;
	in vec2 pos;

	out vec4 color;

	void main() {
		float l = distance(pos, center);
		color = vec4(0.8f, 0.1f, 0.1f, 1-smoothstep(50, 51, l));
	}
);
}

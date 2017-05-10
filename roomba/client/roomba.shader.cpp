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

namespace shader_roomba {
static const char *vertex = R"BLAH(
	#version 410
	uniform mat4 model;
	out vec2 directions;

	layout(location = 0) in vec3 positions;

	void main() {
		directions = vec2(model[0][0], model[0][1]);
		gl_Position = model * vec4(positions, 1.0f);
	}
	)BLAH";

static const char *geometry = GLSL(
	uniform mat4 view_projection;
	out vec2 center;
	out vec2 pos;

	in vec2 directions[];
	out vec2 direction;

	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;

	void main() {
		gl_Position = vec4(0, 0, 0, 0);

		center = gl_in[0].gl_Position.xy;
		vec2 zw = gl_in[0].gl_Position.zw;

		direction = directions[0];

		float radius = 0.35f / 2.0; // 350 mm is the diameter

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
	in vec2 direction;

	out vec4 color;

	void main() {
		float l = distance(pos, center);
		vec2 to_pos = normalize(pos - center);

		vec2 perp = vec2(-direction.y, direction.x);
		perp = perp * (0.258/2.0); // vec2(direction.y, direction.x);
		if (distance(center + perp, pos) < 0.02) {
			color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			return;
		}

		perp = -perp;
		if (distance(center + perp, pos) < 0.02) {
			color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			return;
		}

		float radius = 0.35f / 2.0;
		float d = dot(to_pos, direction);
		float a = acos(d);
		if (abs(a) < 0.1) {
			color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		} else {
			color = vec4(0.8f, 0.1f, 0.1f, l > radius ? 0.0f : 1.0f);
		}
	}
);
}

namespace shader_line {
static const char *vertex = GLSL(
	uniform mat4 model;
	layout(location = 0) in vec3 positions;

	void main() {
		gl_Position = model * vec4(positions, 1.0f);
	}
);

static const char *geometry = GLSL(
	uniform mat4 view_projection;
	out vec2 pos;
	out vec2 start;

	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;

	void main() {
		gl_Position = vec4(0, 0, 0, 0);

		vec2 a = gl_in[0].gl_Position.xy;
		vec2 b = gl_in[1].gl_Position.xy;

		vec2 zw = gl_in[0].gl_Position.zw;

		float radius = 0.01;

		vec2 to_b = normalize(b-a) * radius;
		vec2 perp = vec2(-to_b.y, to_b.x);

		a -= to_b;
		b += to_b;

		start = a;

		pos = a - perp;
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		pos = b - perp;
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		pos = a + perp;
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		pos = b + perp;
		gl_Position = view_projection * vec4(pos, zw);
		EmitVertex();

		EndPrimitive();
	}
);

static const char *fragment = GLSL(
	in vec2 pos;
	in vec2 start;

	out vec4 color;

	void main() {
		float d = distance(pos, start) * 100.0f;
		if (mod(floor(d), 2.0) == 0.0) {
			color = vec4(0, 0, 1, 1);
		} else {
			color = vec4(0, 1, 0, 1);
		}
	}
);
}

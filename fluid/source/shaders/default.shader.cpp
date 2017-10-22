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

namespace shader_light {
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

		float radius = 400;

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

	layout(location = 0) out vec4 color;
	layout(location = 1) out vec4 info;

	void main() {

		// float lsq = pos.x*center.x + pos.y*center.y;
		float r = distance(pos, center);
		// float light_a = r < 100.0f ? 1.0f : 0.0f;
		// float light_a = r < 100.0f ? 1.0f : 0.0f;

		float t = clamp(r / 400.f, 0.0, 1.0);
		t = 1-t;
		t = t*t*t;
		// t = pow(t, 0.4f);
    	float light_a = t * t * (3.0 - 2.0 * t);

		color = vec4(0.1, 0.1, 0.1, 0) * light_a;
		info = vec4(light_a, 0.01f*light_a, 0, 0);
		// info += vec4(light_a, sum*light_a, 0, 0);
	}
);
}

namespace shader_ray {
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

	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;

	void main() {
		gl_Position = vec4(0, 0, 0, 0);

		vec2 a = gl_in[0].gl_Position.xy;
		vec2 b = gl_in[1].gl_Position.xy;

		vec2 zw = gl_in[0].gl_Position.zw;

		float radius = 2;

		vec2 to_b = normalize(b-a) * radius;
		vec2 perp = vec2(-to_b.y, to_b.x);

		a -= to_b;
		b += to_b;

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
	out vec4 color;

	void main() {
		color = vec4(1, 1, 1, 1);
	}
);
}

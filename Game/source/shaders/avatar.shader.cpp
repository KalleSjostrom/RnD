namespace shader_avatar {
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

	flat out vec2 head;
	flat out vec2 foot;
	flat out float radius_sq;

	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;

	void main() {
		gl_Position = vec4(0, 0, 0, 0);

		vec2 a = gl_in[0].gl_Position.xy;
		vec2 b = gl_in[1].gl_Position.xy;

		vec2 zw = gl_in[0].gl_Position.zw;

		float radius = 10;
		radius_sq = radius*radius;

		head = b;
		foot = a;

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
	flat in vec2 head;
	flat in vec2 foot;
	flat in float radius_sq;
	in vec2 pos;

	out vec4 color;

	float lsq(vec2 v) {
		return v.x*v.x + v.y*v.y;
	}

	void main() {
		vec2 foot_to_pos = pos - foot;
		vec2 head_to_pos = pos - head;

		bool inside = lsq(foot_to_pos) < 16.0f || lsq(head_to_pos) < 16.0f;
		vec3 c = vec3(0.8f, 0.1f, 0.1f);
		if (inside) {
			c = vec3(1, 1, 1);
		}

		vec2 line = head - foot;

		float length_along_line = dot(foot_to_pos, normalize(line));
		if (length_along_line < 0) {
			color = vec4(c, lsq(foot_to_pos) < radius_sq);
		} else if ((length_along_line*length_along_line) > lsq(line)) {
			color = vec4(c, lsq(head_to_pos) < radius_sq);
		} else {
			color = vec4(c, 1);
		}
	}
);
}

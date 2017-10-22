namespace fluid {
static const char *vertex = GLSL(
	//@ uniform
	//@ passthrough matrix4x4 view = look_at(V3(0.0f, 0.0f, 20.0f), V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
	//@ passthrough matrix4x4 projection = perspective_fov(100, RES_WIDTH / RES_HEIGHT, 1.0f, 100.0f);
	//@ value (projection * view).m
	uniform mat4 model;
	uniform mat4 view_projection;

	layout(location = 0) in vec2 positions;
	layout(location = 1) in vec2 density_pressure;

	out float density;

	void main () {
		density = density_pressure.x;
		gl_Position = view_projection * model * vec4(positions * 25, 0.0f, 1.0f);
	}
);

static const char *fragment = GLSL(
	in float density;
	out vec4 color;

	float remap(float t) {
		return t*2.0f - 1.0f;
	}

	void main () {
		vec2 test = vec2(remap(gl_PointCoord.s), remap(gl_PointCoord.t));
		float alpha = sqrt(test.x*test.x+test.y*test.y);

		vec3 bright = vec3(0.1176f, 0.6314f, 0.8431f) * 1.3f;
		vec3 dark = vec3(0.1137f, 0.6000f, 0.8039) * 0.8f;

		float t = (density-100) / 60.0f;

		vec3 blue = vec3(0, 0.2, 0.8);
		vec3 red = vec3(1, 0, 0);

		color = vec4(mix(blue, red, clamp(t, 0.0f, 1.0f)), (1-pow(alpha, 8.0f)) * 0.2f);
		// color = vec4(1, 1, 1, (1-pow(alpha, 8.0f)) * 0.2f);
	}
);
}

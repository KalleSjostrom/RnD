namespace shader_passthrough {
static const char *vertex = GLSL(
	out vec2 uv;

	layout(location = 0) in vec3 positions;

	void main(){
		uv.xy = (positions.xy + 1.0f) / 2.0f;
		gl_Position = vec4(positions, 1.0f);
	}
);

static const char *fragment = GLSL(
	in vec2 uv;

	out vec4 color;

	uniform sampler2D render_texture;

	void main(){
		color = texture(render_texture, uv.xy);
	}
);
}

namespace shader_bloom {
static const char *vertex = GLSL(
	out vec2 uv;

	layout(location = 0) in vec3 positions;

	void main(){
		uv.xy = (positions.xy + 1.0f) / 2.0f;
		gl_Position = vec4(positions, 1.0f);
	}
);

static const char *fragment = GLSL(
	in vec2 uv;

	out vec4 color;

	uniform sampler2D render_texture;
	uniform vec2 direction;

	const vec2 resolution = vec2(1024, 768);

	vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
		vec4 color = vec4(0.0);
		vec2 off1 = vec2(1.411764705882353) * direction;
		vec2 off2 = vec2(3.2941176470588234) * direction;
		vec2 off3 = vec2(5.176470588235294) * direction;
		color += texture(image, uv) * 0.1964825501511404;
		color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
		color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
		color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
		color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
		color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
		color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
		return color;
	}

	vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
		vec4 color = vec4(0.0);
		vec2 off1 = vec2(1.3333333333333333) * direction;
		color += texture(image, uv) * 0.29411764705882354;
		color += texture(image, uv + (off1 / resolution)) * 0.35294117647058826;
		color += texture(image, uv - (off1 / resolution)) * 0.35294117647058826;
		return color;
	}

	void main() {
		vec4 test = blur13(render_texture, uv, resolution, direction);
		color = test; // texture(render_texture, uv.xy);
	}
);
}

namespace shader_lightmap {
static const char *vertex = GLSL(
	out vec2 uv;

	layout(location = 0) in vec3 positions;

	void main(){
		uv.xy = (positions.xy + 1.0f) / 2.0f;
		gl_Position = vec4(positions, 1.0f);
	}
);

static const char *fragment = GLSL(
	in vec2 uv;

	layout(location = 0) out vec4 color;
	layout(location = 1) out vec4 info;

	uniform sampler2D shadowmap;

	uniform vec2 light_positions[4];
	uniform float light_radii[4];
	uniform vec3 light_colors[4];

	const float pi = 3.14159265359;
	const vec2 resolution = vec2(1024, 768);

	//sample from the 1D distance map
	float sample_distance(vec2 coord, int index, float r, float[4] light_radii) {
		vec4 temp = texture(shadowmap, coord);
		float colors[4] = float[4](temp.x, temp.y, temp.z, temp.w);
		return step(r, colors[index] * light_radii[index]);
	}

	void main() {
		//rectangular to polar
		color = vec4(0, 0, 0, 0);
		info = vec4(0, 0, 0, 0);
		vec2 norm = uv.xy * 2.0 - 1.0; // To clip-space

		for (int i = 0; i < 4; ++i) {
			vec2 norm_relative = norm - light_positions[i];
			float theta = atan(norm_relative.y, norm_relative.x);
			float r = length(norm_relative);
			if (r >= light_radii[i])
				continue;

			float coord = theta / (2.0*pi);

			//the tex coord to sample our 1D lookup texture
			//always 0.0 on y axis
			vec2 tc = vec2(coord, 0.0);

			//we multiply the blur amount by our distance from center
			//this leads to more blurriness as the shadow "fades away"
			float blur = (1.0/resolution.x) * smoothstep(0.0, 1.0, r);

			//now we use a simple gaussian blur
			float sum = 0.0;

			sum += sample_distance(vec2(tc.x - 4.0*blur, tc.y), i, r, light_radii) * 0.05;
			sum += sample_distance(vec2(tc.x - 3.0*blur, tc.y), i, r, light_radii) * 0.09;
			sum += sample_distance(vec2(tc.x - 2.0*blur, tc.y), i, r, light_radii) * 0.12;
			sum += sample_distance(vec2(tc.x - 1.0*blur, tc.y), i, r, light_radii) * 0.15;

			sum += sample_distance(tc, i, r, light_radii) * 0.16;

			sum += sample_distance(vec2(tc.x + 1.0*blur, tc.y), i, r, light_radii) * 0.15;
			sum += sample_distance(vec2(tc.x + 2.0*blur, tc.y), i, r, light_radii) * 0.12;
			sum += sample_distance(vec2(tc.x + 3.0*blur, tc.y), i, r, light_radii) * 0.09;
			sum += sample_distance(vec2(tc.x + 4.0*blur, tc.y), i, r, light_radii) * 0.05;
			//sum of 1.0 -> in light, 0.0 -> in shadow

			//multiply the summed amount by our distance, which gives us a radial falloff
			//then multiply by vertex (light) color
			float light_a = smoothstep(light_radii[i], 0.0, r);
			color.rgb += light_colors[i].rgb*light_a;
			info += vec4(light_a, sum*light_a, 0, 0);
		}
	}
);
}

namespace shader_combine {
static const char *vertex = GLSL(
	out vec2 uv;

	layout(location = 0) in vec3 positions;

	void main() {
		uv.xy = (positions.xy + 1.0f) / 2.0f;
		gl_Position = vec4(positions, 1.0f);
	}
);

static const char *fragment = GLSL(
	in vec2 uv;

	out vec4 color;

	uniform sampler2D lightmap_color;
	uniform sampler2D lightmap_info;
	uniform sampler2D scene;
	uniform sampler2D bloom;
	uniform sampler2D hatch;

	vec2 rotate_uv(vec2 uv, float angle) {
		float ca = cos(angle);
		float sa = sin(angle);

		float rx = -ca*uv.x + sa*uv.y;
		float ry = -sa*uv.x - ca*uv.y;

		return vec2(rx, ry);
	}

	void main() {
		vec4 light_info = texture(lightmap_info, uv);
		vec4 light_color = texture(lightmap_color, uv);

		vec4 scene_color = texture(scene, uv);
		vec4 bloom_color = texture(bloom, uv);

		vec4 hatch_vert_color = texture(hatch, rotate_uv(uv, -0.0f).xy*2);
		vec4 hatch_hori_color = texture(hatch, rotate_uv(uv, -0.0f).yx*2);

		// bloom_color.a = 0;
		scene_color.rgb *= light_info.r;

		color = scene_color + bloom_color + vec4(vec3(light_color.rgb), light_info.g);

		float a1 = 1 - hatch_vert_color.g;

		float hatch = hatch_vert_color.r + hatch_hori_color.r;
		float a2 = (1-hatch) * (1-scene_color.a) + scene_color.a;

		if (scene_color.a == 1) {
			color.a = 1;
		} else {
			color.a = a1 * light_info.g + a2 * (1-light_info.g);
		}
	}
);
}

namespace shader_shadowmap {
static const char *vertex = GLSL(
	out vec2 uv;

	layout(location = 0) in vec3 positions;

	void main(){
		uv.xy = (positions.xy + 1.0f) / 2.0f;
		gl_Position = vec4(positions, 1.0f);
	}
);

static const char *fragment = GLSL(
	in vec2 uv;

	out vec4 color;

	uniform sampler2D scene;
	uniform vec2 light_positions[4];
	uniform float light_radii[4];

	const float pi = 3.14159265359;
	const float height = 768;

	void main() {
		float distances[4] = float[4](1,1,1,1);
		color = vec4(1,1,1,1);
		for (float y = 0.0; y < height; y += 1.0) {
			float theta = uv.x * 2 * pi;
			float r = y / height;

			for (int i = 0; i < 4; ++i) {
				vec2 coord = vec2(r * cos(theta) + light_positions[i].x, r * sin(theta) + light_positions[i].y) / 2.0 + 0.5;
				r /= light_radii[i];

				vec4 data = texture(scene, coord);
				if (data.a > 0.5f) {
					distances[i] = min(distances[i], r);
				}
			}
		}
		color = vec4(distances[0], distances[1], distances[2], distances[3]);
	}
);
}

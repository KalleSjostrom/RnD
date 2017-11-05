
u32 round_f32_u32(f32 a) {
	return (u32)a;
}
f32 exact_linear_to_sRGB(f32 c) {
	c = saturate(c);

	f32 s = c * 12.92f;
	if (c > 0.0031308f) {
		s = 1.055f * powf(c, 1.0f/2.4f) - 0.055f;
	}

	return s;
}

u32 pack_color(v4 color) {
	color *= 255.0f;
	u32 c = round_f32_u32(color.a) << 24 | round_f32_u32(color.b) << 16 | round_f32_u32(color.g) << 8 | round_f32_u32(color.r);
	return c;
}

u32 pack_color(v3 color) {
	color *= 255.0f;
	u32 c = (u32)0xFF << 24 | round_f32_u32(color.b) << 16 | round_f32_u32(color.g) << 8 | round_f32_u32(color.r);
	return c;
}

v3 cast_ray(Game &game, v3 ray_position, v3 ray_direction) {
	v3 color = {};

	v3 hit_position;
	v3 hit_normal;
	v3 attenuation = V3(1, 1, 1);
	int max_bounce_count = 8;
	float tol = 0.001f;
	for (int bounce_index = 0; bounce_index < max_bounce_count; ++bounce_index) {
		float min_t = FLT_MAX;
		int hit_entity_index = -1;

		for (int i = 0; i < game.entity_count; ++i) {
			Entity &entity = game.entities[i];
			m4 &p = game.components.model.get_pose(entity.model_id);
			v3 position = translation(p);
			switch (entity.type) {
				case EntityType_Plane: {
					v3 n = V3(0, 1, 0);
					float d = position.y;

					float denom = dot(n, ray_direction);
					if (denom < -tol || denom > tol) {
						float t = (d - dot(n, ray_position)) / denom;
						if (t > tol && t < min_t) {
							min_t = t;
							hit_entity_index = i;

							hit_position = ray_position + ray_direction * t;
							hit_normal = n;
						}
					}
				}; break;
				case EntityType_Sphere: {
					float r = p.m[0];
					v3 to = ray_position - position;

					float a = dot(ray_direction, to);
					float b = length_squared(to);
					float c = a*a - b + r*r;
					if (c > tol) {
						float s = sqrtf(c);
						float base = -a;

						float t = (base - s) < (base + s) ? (base - s) : (base + s);
						if (t > tol && t < min_t) {
							min_t = t;
							hit_entity_index = i;

							hit_position = ray_position + ray_direction * t;
							hit_normal = normalize(hit_position - position);
						}
					}
				}; break;
			}
		}

		if (hit_entity_index != -1) {
			Entity &hit_entity = game.entities[hit_entity_index];
			Material &material = game.components.material.instances[hit_entity.material_id];

			color += hadamard(material.emittance_color, attenuation);
			f32 cos_atten = 1; // dot(-ray_direction, hit_normal);
			if (cos_atten < 0) {
				cos_atten = 0;
			}
			attenuation = hadamard(cos_atten * material.reflection_color, attenuation);

			ray_position = hit_position;
			v3 reflection = ray_direction - (2 * dot(ray_direction, hit_normal)) * hit_normal;
			v3 random_reflection = normalize(hit_normal + V3(random_bilateral_f32(game.random), random_bilateral_f32(game.random), random_bilateral_f32(game.random)));
			ray_direction = normalize(lerp(reflection, random_reflection, material.roughness));
		} else {
			// Hit nothing, add the color of the null material??
			v3 null_emittance = V3(0.3f, 0.4f, 0.5f);
			color += hadamard(null_emittance, attenuation);
			break;
		}
	}
	return color;
}


void raycast(Game &game) {
	ImageData &image_data = game.render_pipe.image_data;
	// TODO(kalle):what happens if we use unsigned here?
	float half_width = image_data.width * 0.5f;
	float half_height = image_data.height * 0.5f;
	v3 ray_position = game.camera.position;
	static int RAYS_PER_PIXEL = 1;
	f32 ray_contribution = 1.0f / RAYS_PER_PIXEL;

	f32 one_over_w = 1.0f / image_data.width;
	f32 one_over_h = 1.0f / image_data.height;

	f32 half_pix_w = 0.5f * one_over_w;
	f32 half_pix_h = 0.5f * one_over_h;

	for (int y = 0; y < image_data.height; ++y) {
		for (int x = 0; x < image_data.width; ++x) {
			v3 color = {};

			for (int ray_index = 0; ray_index < RAYS_PER_PIXEL; ++ray_index) {
				v3 ray_goal = V3(x - half_width, y - half_height, 0);
				ray_goal.x += random_bilateral_f32(game.random) * half_pix_w;
				ray_goal.y += random_bilateral_f32(game.random) * half_pix_h;
				v3 ray_direction = normalize(ray_goal - ray_position);
				color += cast_ray(game, ray_position, ray_direction);
			}

			char *pixel = (char*)(image_data.pixels) + (y * image_data.width + x) * image_data.bytes_per_pixel;
			*(u32*)pixel = pack_color(ray_contribution * color); // ABGR
		}
	}
	update_image(game.render_pipe);
}

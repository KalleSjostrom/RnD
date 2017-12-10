namespace cl_shaders {
static const char *source = CL(
	__kernel void copy_buffer(__global float2 *input, __global float2 *output) {
		uint i = get_global_id(0);
		output[i] = input[i];
	}
);

static const char *trace_ray = CL(
	typedef struct {
		float3 position;
		float3 data;
		int type;
	} EntityGeometry;

	typedef struct {
		float3 emittance_color;
		float3 reflection_color;
		float roughness;
	} EntityMaterial;

	typedef struct {
		uint width;
		uint height;
		float half_width;
		float half_height;
		float one_over_w;
		float one_over_h;
		float half_pix_w;
		float half_pix_h;
		int rays_per_pixel;
	} Settings;

	typedef struct {
		float3 position;
		float3 normal;
		int entity_index;
	} Hit;

	// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
	// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
	typedef struct {
		ulong state;
		ulong inc;
	} Random;

	uint random_u32(Random *r);
	float random_f32(Random *r);
	float random_bilateral_f32(Random *r);
	void random_init(Random *r, ulong initstate, ulong initseq);

	uint random_u32(Random *r) {
		// int const a = 16807; //ie 7**5
		// int const m = 2147483647; //ie 2**31-1

		// r->state = (r->state * a) % m;
		// return r->state;

		ulong oldstate = r->state;
		r->state = oldstate * (ulong)6364136223846793005 + (r->inc|1); // Advance internal state
		uint xorshifted = (uint)(((oldstate >> 18u) ^ oldstate) >> 27u); // Calculate output function (XSH RR), uses old state for max ILP
		uint rot = oldstate >> 59u;
		return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
	}

	float random_f32(Random *r) {
		return (float)random_u32(r) / (float)UINT_MAX;
	}

	float random_bilateral_f32(Random *r) {
		float a = (float)random_u32(r) / (float)UINT_MAX;
		return a * 2.0f - 1.0f;
	}

	void random_init(Random *r, ulong initstate, ulong initseq) {
		r->state = 0U;
		r->inc = (initseq << 1u) | 1u;
		random_u32(r);
		r->state += initstate;
		random_u32(r);
	}


	float3 lerp(float3 a, float3 b, float t);
	Hit trace(__constant EntityGeometry *genometries, int entity_count, float3 ray_position, float3 ray_direction);
	float3 cast_ray(__constant EntityGeometry *genometries, __constant EntityMaterial *materials, int entity_count, Random *random, float3 ray_position, float3 ray_direction);
	float safe_divide(float numerator, float divisior, float default_value);
	float max_elem(float a, float b, float c);
	float min_elem(float a, float b, float c);

	float3 lerp(float3 a, float3 b, float t) {
		return (float3)(a.x + (b.x-a.x)*t, a.y + (b.y-a.y)*t, a.z + (b.z-a.z)*t);
	}

	float safe_divide(float numerator, float divisior, float default_value) {
		return divisior != 0.0f ? (numerator / divisior) : default_value;
	}

	float max_elem(float a, float b, float c) {
		return a > b ? (a > c ? a : c) : (b > c ? b : c);
	}

	float min_elem(float a, float b, float c) {
		return a < b ? (a < c ? a : c) : (b < c ? b : c);
	}

	Hit trace(__constant EntityGeometry *genometries, int entity_count, float3 ray_position, float3 ray_direction) {
		float tol = 0.001f;
		float min_t = FLT_MAX;
		float min_t_np = FLT_MAX;

		Hit hit;
		hit.entity_index = -1;
		for (int i = 0; i < entity_count; ++i) {
			EntityGeometry genometry = genometries[i];
			switch (genometry.type) {
				case 0: {
					float3 n = genometry.data;
					float d = -genometry.position.y;

					float denom = dot(n, ray_direction);
					if (denom < -tol || denom > tol) {
						float t = (d - dot(n, ray_position)) / denom;
						if (t > tol && t < min_t) {
							min_t = t;
							hit.entity_index = i;

							hit.position = ray_position + ray_direction * t;
							hit.normal = n;
						}
					}
				}; break;
				case 1:
				case 2: {
					float r = genometry.data.x;
					float3 to = ray_position - genometry.position;

					float a = dot(ray_direction, to);
					float b = dot(to, to);
					float c = a*a - b + r*r;
					if (c > tol) {
						float s = sqrt(c);
						float base = -a;

						float t = (base - s) < (base + s) ? (base - s) : (base + s);
						if (t > tol && t < min_t) {
							min_t = t;
							hit.entity_index = i;

							hit.position = ray_position + ray_direction * t;
							hit.normal = normalize(hit.position - genometry.position);
						}
					}
				}; break; // (around 1.3 for water and 1.5 for glass)
				case 3: { // Disc
					float3 n = genometry.data;
					float d = -genometry.position.y;

					float denom = dot(n, ray_direction);
					if (denom < -tol || denom > tol) {
						float t = (d - dot(n, ray_position)) / denom;
						if (t > tol && t < min_t) {
							float3 hit_position = ray_position + ray_direction * t;

							// float3 dummy_plane_pos = (float3)
							float3 to = hit_position - genometry.position;
							float b = dot(to, to);
							if (b < 100*100) {
								min_t = t;
								hit.entity_index = i;

								hit.position = hit_position;
								hit.normal = n;
							}
						}
					}
				}; break;
				case 4: { // AABB
					float r = genometry.data.x * 2.0f; // Should be sqrt(6) instead of 2, no? Why does this work?
					float3 to = ray_position - genometry.position;

					float a = dot(ray_direction, to);
					float b = dot(to, to);
					float c = a*a - b + r*r;
					if (c > tol) {
						float s = sqrt(c);
						float base = -a;

						float t = (base - s) < (base + s) ? (base - s) : (base + s);
						if (t > tol && t < min_t) {


							float radius = genometry.data.x;

							float x = genometry.position.x - radius;
							float w = radius * 2;

							float y = genometry.position.y - radius;
							float h = radius * 2;

							float z = genometry.position.z - radius;
							float d = radius * 2;

							float3 inv_delta;
							inv_delta.x = safe_divide(1.0f, ray_direction.x, FLT_MAX);
							inv_delta.y = safe_divide(1.0f, ray_direction.y, FLT_MAX);
							inv_delta.z = safe_divide(1.0f, ray_direction.z, FLT_MAX);

							float tx1 = (x - ray_position.x) * inv_delta.x;
							float tx2 = ((x + w) - ray_position.x) * inv_delta.x;

							float ty1 = (y - ray_position.y) * inv_delta.y;
							float ty2 = ((y + h) - ray_position.y) * inv_delta.y;

							float tz1 = (z - ray_position.z) * inv_delta.z;
							float tz2 = ((z + d) - ray_position.z) * inv_delta.z;

							float tmin = max_elem(min(tx1, tx2), min(ty1, ty2), min(tz1, tz2));
							float tmax = min_elem(max(tx1, tx2), max(ty1, ty2), max(tz1, tz2));

							if (tmax > tmin && tmin < min_t_np) {
								min_t = t;
								min_t_np = tmin;
								hit.entity_index = i;

								// Better way to calulate the normal!
								if (tmin == tx1) {
									hit.normal = (float3)(-1, 0, 0);
								} else if (tmin == tx2) {
									hit.normal = (float3)(1, 0, 0);
								} else if (tmin == ty1) {
									hit.normal = (float3)(0, -1, 0);
								} else if (tmin == ty2) {
									hit.normal = (float3)(0, 1, 0);
								} else if (tmin == tz1) {
									hit.normal = (float3)(0, 0, -1);
								} else if (tmin == tz2) {
									hit.normal = (float3)(0, 0, 1);
								}

								hit.position = ray_position + ray_direction * tmin;
							}
						}
					}
				}; break;
			}
		}

		return hit;
	}

	float3 cast_ray(__constant EntityGeometry *genometries, __constant EntityMaterial *materials, int entity_count, Random *random, float3 ray_position, float3 ray_direction) {
		float3 color = (float3)(0);
		float3 attenuation = (float3)(1);

		int max_bounce_count = 8;
		for (int bounce_index = 0; bounce_index < max_bounce_count; ++bounce_index) {
			Hit hit = trace(genometries, entity_count, ray_position, ray_direction);

			if (hit.entity_index != -1) {
				EntityMaterial hit_material = materials[hit.entity_index];
				float cos_term = 1; // dot(-ray_direction, hit_normal);
				if (cos_term < 0)
					cos_term = 0;

				float3 reflection = ray_direction - (2 * dot(ray_direction, hit.normal)) * hit.normal;
				float3 reflection_color = hit_material.emittance_color * attenuation;
				color += reflection_color;

				float3 random_reflection = normalize(hit.normal + (float3)(random_bilateral_f32(random), random_bilateral_f32(random), random_bilateral_f32(random)));
				attenuation *= (cos_term * hit_material.reflection_color);
				ray_direction = normalize(lerp(reflection, random_reflection, hit_material.roughness));
				ray_position = hit.position;
			} else {
				float3 null_emittance = (float3)(0.3f, 0.4f, 0.5f);
				color += null_emittance * attenuation;
				break;
			}
		}
		return color;
	}

	// struct m4 {
	// 	float m[16];
	// };

	struct m4 {
		float4 x;
		float4 y;
		float4 z;
		float4 w;
	};

	// float3 &right_axis(m4 &m) {
	// 	return *(float3*)(m.m + 0);
	// }
	// float3 &forward_axis(m4 &m) {
	// 	return *(float3*)(m.m + 8);
	// }
	// float3 &up_axis(m4 &m) {
	// 	return *(float3*)(m.m + 4);
	// }
	float3 &translation(m4 &m) {
		return m[3].xyz;
	}

	__kernel void cast_rays(__constant EntityGeometry *genometries, __constant EntityMaterial *materials, uint entity_count, __constant Settings *settings, ulong time, ulong frame_counter, m4 camera_pose, __global float3 *buffer, __write_only image2d_t image) {
		uint x = get_global_id(0);
		uint y = get_global_id(1);

		// Setup random
		Random random = {};
		random_init(&random, x*y*time, 54u);

		float3 ray_position = translation(camera_pose);
		float3 color = (float3)(0, 0, 0);
		for (int ray_index = 0; ray_index < settings->rays_per_pixel; ++ray_index) {
			float3 ray_goal = (float3)(x - settings->half_width, y - settings->half_height, 0);
			// ray_goal.x += random_bilateral_f32(&random) * settings->half_pix_w;
			// ray_goal.y += random_bilateral_f32(&random) * settings->half_pix_h;
			float3 ray_direction = normalize(ray_goal - ray_position);
			color += cast_ray(genometries, materials, entity_count, &random, ray_position, ray_direction);
		}
		color *= 1.0f/settings->rays_per_pixel;
		buffer[y * settings->width + x] += color;
		write_imagef(image, (int2)(x,y), (float4)(buffer[y * settings->width + x] * (1.0f / (float)frame_counter), 1.0f));
		// write_imagef(image, (int2)(x,y), (float4)(color, 1.0f));
	}
);
}


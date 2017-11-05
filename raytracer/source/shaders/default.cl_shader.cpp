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

		float3 emittance_color;
		float3 reflection_color;
		float roughness;
		int type;
	} Entity;

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


	// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
	// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
	typedef struct {
		ulong state;
		ulong inc;
	} Random;

	uint random_u32(Random *r);
	ulong random_u64(Random *r);
	float random_f32(Random *r);
	float random_bilateral_f32(Random *r);
	void random_init(Random *r, ulong initstate, ulong initseq);

	uint random_u32(Random *r) {
		ulong oldstate = r->state;
		r->state = oldstate * (ulong)6364136223846793005 + (r->inc|1); // Advance internal state
		uint xorshifted = (uint)(((oldstate >> 18u) ^ oldstate) >> 27u); // Calculate output function (XSH RR), uses old state for max ILP
		uint rot = oldstate >> 59u;
		return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
	}

	ulong random_u64(Random *r) {
		ulong a = (ulong)random_u32(r);
		ulong b = (ulong)random_u32(r);
		return (a << 32) ^ (b);
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
	float3 cast_ray(__constant Entity *entities, int entity_count, Random *random, float3 ray_position, float3 ray_direction);

	float3 lerp(float3 a, float3 b, float t) {
		return (float3)(a.x + (b.x-a.x)*t, a.y + (b.y-a.y)*t, a.z + (b.z-a.z)*t);
	}

	float3 cast_ray(__constant Entity *entities, int entity_count, Random *random, float3 ray_position, float3 ray_direction) {
		float3 color = (float3)(0);
		float3 hit_position = (float3)(0);
		float3 hit_normal = (float3)(0);
		float3 attenuation = (float3)(1);

		int max_bounce_count = 3;
		float tol = 0.001f;
		for (int bounce_index = 0; bounce_index < max_bounce_count; ++bounce_index) {
			float min_t = FLT_MAX;
			int hit_entity_index = -1;

			for (int i = 0; i < entity_count; ++i) {
				Entity entity = entities[i];
				switch (entity.type) {
					case 0: {
						float3 n = entity.data;
						float d = entity.position.y;

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
					case 1: {
						float r = entity.data.x;
						float3 to = ray_position - entity.position;

						float a = dot(ray_direction, to);
						float b = dot(to, to);
						float c = a*a - b + r*r;
						if (c > tol) {
							float s = sqrt(c);
							float base = -a;

							float t = (base - s) < (base + s) ? (base - s) : (base + s);
							if (t > tol && t < min_t) {
								min_t = t;
								hit_entity_index = i;

								hit_position = ray_position + ray_direction * t;
								hit_normal = normalize(hit_position - entity.position);
							}
						}
					}; break;
				}
			}

			if (hit_entity_index != -1) {
				Entity hit_entity = entities[hit_entity_index];

				float cos_term = 1; // dot(-ray_direction, hit_normal);
				if (cos_term < 0)
					cos_term = 0;

				color += hit_entity.emittance_color * attenuation;
				attenuation *= (cos_term * hit_entity.reflection_color);

				float3 reflection = ray_direction - (2 * dot(ray_direction, hit_normal)) * hit_normal;
				float3 random_reflection = normalize(hit_normal + (float3)(random_bilateral_f32(random), random_bilateral_f32(random), random_bilateral_f32(random)));
				ray_direction = normalize(lerp(reflection, random_reflection, hit_entity.roughness));
				ray_position = hit_position + hit_normal * 0.001f;


				// // compute the surface hit_normal and flip it if necessary to face the incoming ray */
				// float3 normal_facing = dot(hit_normal, ray_direction) < 0.0f ? hit_normal : hit_normal * (-1.0f);

				// // compute two random numbers to pick a random point on the hemisphere above the hit_position*/
				// float rand1 = 2.0f * 3.1415f * random_f32(random);
				// float rand2 = random_f32(random);
				// float rand2s = sqrt(rand2);

				// // create a local orthogonal coordinate frame centered at the hit_position */
				// float3 w = normal_facing;
				// float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
				// float3 u = normalize(cross(axis, w));
				// float3 v = cross(w, u);

				// // use the coordinte frame and random numbers to compute the next ray direction */
				// float3 newdir = normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));
			} else {
				float3 null_emittance = (float3)(0.3f, 0.4f, 0.5f);
				color += null_emittance * attenuation;
				break;
			}
		}
		return color;
	}

	__kernel void cast_rays(__constant Entity *entities, uint entity_count, __constant Settings *settings, ulong time, ulong frame_counter, float3 camera_position, __global float3 *buffer, __write_only image2d_t image) {
		uint x = get_global_id(0);
		uint y = get_global_id(1);

		// Setup random
		Random random = {};
		random_init(&random, x*y*time, 54u);

		float3 ray_position = (float3)(camera_position);
		float3 color = (float3)(0, 0, 0);
		for (int ray_index = 0; ray_index < settings->rays_per_pixel; ++ray_index) {
			float3 ray_goal = (float3)(x - settings->half_width, y - settings->half_height, 0);
			ray_goal.x += random_bilateral_f32(&random) * settings->half_pix_w;
			ray_goal.y += random_bilateral_f32(&random) * settings->half_pix_h;
			float3 ray_direction = normalize(ray_goal - ray_position);
			color += cast_ray(entities, entity_count, &random, ray_position, ray_direction);
		}
		color *= 1.0f/settings->rays_per_pixel;
		buffer[y * settings->width + x] += color;
		write_imagef(image, (int2)(x,y), (float4)(buffer[y * settings->width + x] * (1.0f / (float)frame_counter), 1.0f));
	}
);
}


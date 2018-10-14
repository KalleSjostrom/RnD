#include "engine/utils/math/random.h"

#define PARTICLE_COUNT 1024 * 2 // Must be a power of two

#include "engine/utils/math/vectorization.h"

namespace fluid_common {
	static float PI = 3.14159265359f;

	static float MASS = 1.0f;
	static float INV_MASS = 1.0f / MASS;

	static float DT = 1.0f / 240.0f; // timestep

	static float rho_0 = 100.0f;
	static float inv_rho_0 = 1.0f / rho_0;

	static float GRAVITY = -10.0f;

	static float SCP = 1.0f;
	static float H = (sqrtf((4*MASS) / (PI*SCP*rho_0)));
	static float BOUNDS = 5.0f;

	static const u32 MAX_NEIGHBORS = 7;

	static vec VEC_GRAVITY = _mm256_set_ps(GRAVITY * DT, 0.0f, GRAVITY * DT, 0.0f, GRAVITY * DT, 0.0f, GRAVITY * DT, 0.0f);
	static vec VEC_DT = v_set(DT);
	static vec VEC_NEG_BOUNDS = v_set(-BOUNDS);
	static vec VEC_POS_BOUNDS = v_set(BOUNDS);
	static vec VEC_TWO = v_set(2.0f);

	/// NEIGHBORS ///
	struct NeighborList {
		u32 counter;
		u32 neighbors[MAX_NEIGHBORS];
	};
	#define FOR_ALL_NEIGHBORS() \
		list = neighbors_list + i; \
		for (u32 _k = 0; _k < list->counter; ++_k) { \
			u32 j = list->neighbors[_k];
	/// ---- ///

	/// HASHMAP ///
	struct Element {
		u32 index;
		Element *next;
	};
	#define INV_HASH_CELL_SIZE (1.0f / (2.0f*H))
	inline u32 calculate_hash(float x, float y) {
		u32 cx = (u32)((x + BOUNDS) * INV_HASH_CELL_SIZE);
		u32 cy = (u32)((y + BOUNDS) * INV_HASH_CELL_SIZE);
		u32 hash = ((cx * 73856093) ^ (cy * 19349663)) & (PARTICLE_COUNT-1);
		return hash;
	}
	void insert_in_hashmap(Element **hash_map, Element *elements, u32 index, u32 offset, float x, float y) {
		u32 hash = calculate_hash(x, y);
		Element *existing = hash_map[hash];

		Element *element = elements + index*9 + offset;
		element->index = index;
		element->next = 0;

		if (existing) {
			if (existing->index == index)
				return;

			element->next = existing;
		}
		hash_map[hash] = element;
	}
	inline void fill_hashmap(v2 *positions, Element **hash_map, Element *elements, NeighborList *neighbors_list) {
		for (u32 i = 0; i < PARTICLE_COUNT; ++i) {
			float x = positions[i].x;
			float y = positions[i].y;

			insert_in_hashmap(hash_map, elements, i, 0, x-H, y-H);
			insert_in_hashmap(hash_map, elements, i, 1, x-H, y);
			insert_in_hashmap(hash_map, elements, i, 2, x-H, y+H);

			insert_in_hashmap(hash_map, elements, i, 3, x, y-H);
			insert_in_hashmap(hash_map, elements, i, 4, x, y);
			insert_in_hashmap(hash_map, elements, i, 5, x, y+H);

			insert_in_hashmap(hash_map, elements, i, 6, x+H, y-H);
			insert_in_hashmap(hash_map, elements, i, 7, x+H, y);
			insert_in_hashmap(hash_map, elements, i, 8, x+H, y+H);

			neighbors_list[i].counter = 0;
		}
	}
	/// ---- ///

	// TODO(kalle): singed distance field boundary detection

	inline void integrate_wide_boundary_projection(v2 *positions, v2 *velocities) {
		// Eight wide boundary projection (4 vectors at a time).
		for (int i = 0; i < PARTICLE_COUNT; i+=4) {
			vec vel8 = v_load((float*)(velocities + i));
			// vel8 = v_add(vel8, VEC_GRAVITY);

			vec pos8 = v_load((float*)(positions + i));
			pos8 = v_add(pos8, v_mul(vel8, VEC_DT));

			{
				vec lessthan_mask = v_cmp_lt(pos8, VEC_NEG_BOUNDS);
				pos8 = v_max(pos8, VEC_NEG_BOUNDS);
				vec temp = v_mul(v_and(lessthan_mask, vel8), VEC_TWO);
				vel8 = v_sub(vel8, temp);

				vec greaterthan_mask = v_cmp_gt(pos8, VEC_POS_BOUNDS);
				pos8 = v_min(pos8, VEC_POS_BOUNDS);
				temp = v_mul(v_and(greaterthan_mask, vel8), VEC_TWO);
				vel8 = v_sub(vel8, temp);
			}

			v_store((float*)(positions + i), pos8);
			v_store((float*)(velocities + i), vel8);
		}
	}

	inline void integrate_wide_boundary_projection_v2(v2 *positions, v2 *velocities, v2 *accelerations) {
		// Eight wide boundary projection (4 vectors at a time).
		for (int i = 0; i < PARTICLE_COUNT; i+=4) {
			vec acc8 = v_load((float*)(accelerations + i));
			vec vel8 = v_load((float*)(velocities + i));
			vel8 = v_add(vel8, v_mul(acc8, VEC_DT));

			vec pos8 = v_load((float*)(positions + i));
			pos8 = v_add(pos8, v_mul(vel8, VEC_DT));

			{
				vec lessthan_mask = v_cmp_lt(pos8, VEC_NEG_BOUNDS);
				pos8 = v_max(pos8, VEC_NEG_BOUNDS);
				vec temp = v_mul(v_and(lessthan_mask, vel8), VEC_TWO);
				vel8 = v_sub(vel8, temp);

				vec greaterthan_mask = v_cmp_gt(pos8, VEC_POS_BOUNDS);
				pos8 = v_min(pos8, VEC_POS_BOUNDS);
				temp = v_mul(v_and(greaterthan_mask, vel8), VEC_TWO);
				vel8 = v_sub(vel8, temp);
			}

			v_store((float*)(positions + i), pos8);
			v_store((float*)(velocities + i), vel8);
		}
	}
}

#define SPOOK 1

// #if defined(MULLER)
// #include "fluid_muller.cpp"
// #elif defined(SPOOK)
// #include "fluid_spook.cpp"
// #elif defined(SPOOK_PTHREAD)
// #include "fluid_spook_pthread.cpp"
// #elif defined(MPM)
// #include "fluid_mpm.cpp"
// #endif

#include "engine/utils/time.c"

struct Buffer {
	GLuint vbo;
	// cl_mem mem;
};
inline v2 zero(u64 i) {
	return V2_f32(0.0f, 0.0f);
}
inline v2 gen_random_pos(u64 i) {
	Random r;
	random_init(r, rdtsc(), 54u);
	float x = random_f32(r);
	x *= 9;
	float y = random_f32(r);
	y *= 9;
	return V2_f32(x, y);
}

Buffer gen_buffer(/*cl_context context, */v2 (*f)(u64 i)) {
	Buffer buffer = {};
	glGenBuffers(1, &buffer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	v2 array[PARTICLE_COUNT];
	for (u64 i = 0; i < PARTICLE_COUNT; ++i) {
		array[i] = f(i);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(v2)*PARTICLE_COUNT, array, GL_DYNAMIC_DRAW);

	// cl_int errcode_ret;
	// buffer.mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, buffer.vbo, &errcode_ret);
	// CL_CHECK_ERRORCODE(clCreateFromGLBuffer, errcode_ret);

	return buffer;
}

struct Fluid {
	Renderable renderable;

	GLenum buffer_type; // e.g. GL_STATIC_DRAW;
	Buffer positions;
	Buffer velocities;
	Buffer density_pressure;

	i32 vertex_count;
	i32 __padding;
};

struct FluidComponent {
	Fluid fluids[8];
	cid count;
};

void add(FluidComponent &fc, Entity &entity, ArenaAllocator &arena) {
	ASSERT((u32)fc.count < ARRAY_COUNT(fc.fluids), "Component full!");
	entity.fluid_id = fc.count++;
	Fluid &fluid = fc.fluids[entity.fluid_id];
	Renderable &renderable = fluid.renderable;

	renderable.pose = identity();
	renderable.mesh.groups = PUSH_STRUCTS(arena, 1, Group);

	Group &group = renderable.mesh.groups[0];

	group.index_count = PARTICLE_COUNT;
	renderable.datatype = RenderableDataType_Arrays;
	renderable.draw_mode = GL_POINTS;

	glGenVertexArrays(1, &renderable.mesh.vertex_array_object);
	glBindVertexArray(renderable.mesh.vertex_array_object);

	fluid.positions = gen_buffer(gen_random_pos);
	fluid.velocities = gen_buffer(zero);
	fluid.density_pressure = gen_buffer(zero);

	glBindBuffer(GL_ARRAY_BUFFER, fluid.positions.vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, fluid.density_pressure.vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

inline void rotate(FluidComponent &fc, Entity &entity, float angle) {
	Fluid &fluid = fc.fluids[entity.fluid_id];

	float ca;
	float sa;
	sincosf(angle, &sa, &ca);

	m4 rotation = identity();

	rotation.m[INDEX(0, 0)] = ca;
	rotation.m[INDEX(0, 1)] = -sa;
	rotation.m[INDEX(1, 0)] = sa;
	rotation.m[INDEX(1, 1)] = ca;

	fluid.renderable.pose *= rotation;
}

void update(FluidComponent &fc, f32 dt) {
	for (i32 i = 0; i < fc.count; ++i) {
		Fluid &fluid = fc.fluids[i];

		glBindBuffer(GL_ARRAY_BUFFER, fluid.density_pressure.vbo);
		v2 *gpu_density_pressure = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

		glBindBuffer(GL_ARRAY_BUFFER, fluid.velocities.vbo);
		v2 *gpu_velocities = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

		glBindBuffer(GL_ARRAY_BUFFER, fluid.positions.vbo);
		v2 *gpu_positions = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

		v2 density_pressure[PARTICLE_COUNT] = {};
		ALIGNED_(32) v2 velocities[PARTICLE_COUNT];
		ALIGNED_(32) v2 positions[PARTICLE_COUNT];

		// memcpy(density_pressure, gpu_density_pressure, PARTICLE_COUNT* sizeof(v2));
		memcpy(velocities, gpu_velocities, PARTICLE_COUNT* sizeof(v2));
		memcpy(positions, gpu_positions, PARTICLE_COUNT* sizeof(v2));

		float cos_angle = fluid.renderable.pose.m[INDEX(0, 0)];
		float sin_angle = fluid.renderable.pose.m[INDEX(1, 0)];

		v2 gravity = V2_f32(0, -9.82f);
		float gx = cos_angle * gravity.x - sin_angle * gravity.y;
		float gy = sin_angle * gravity.x + cos_angle * gravity.y;
		gravity.x = -gx;
		gravity.y = gy;

		// fluid::simulate(positions, velocities, density_pressure, gravity);
		// memset(density_pressure, 0, sizeof(density_pressure));
		// fluid::simulate(positions, velocities, density_pressure, gravity);
		// memset(density_pressure, 0, sizeof(density_pressure));
		// fluid::simulate(positions, velocities, density_pressure, gravity);
		// memset(density_pressure, 0, sizeof(density_pressure));
		// fluid::simulate(positions, velocities, density_pressure, gravity);

		memcpy(gpu_density_pressure, density_pressure, PARTICLE_COUNT* sizeof(v2));
		memcpy(gpu_velocities, velocities, PARTICLE_COUNT* sizeof(v2));
		memcpy(gpu_positions, positions, PARTICLE_COUNT* sizeof(v2));

		glBindBuffer(GL_ARRAY_BUFFER, fluid.density_pressure.vbo);
		glUnmapBuffer(GL_ARRAY_BUFFER);

		glBindBuffer(GL_ARRAY_BUFFER, fluid.velocities.vbo);
		glUnmapBuffer(GL_ARRAY_BUFFER);

		glBindBuffer(GL_ARRAY_BUFFER, fluid.positions.vbo);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
}

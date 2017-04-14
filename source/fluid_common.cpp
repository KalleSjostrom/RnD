#include "../utils/vectorization.h"

namespace fluid_common {
	static float PI = 3.14159265359f;

	static float MASS = 1.0f;
	static float INV_MASS = 1.0f / MASS;

	static float DT = 1.0f / 240.0f; // timestep

	static float ρ_0 = 100.0f;
	static float inv_ρ_0 = 1.0f / ρ_0;

	static float GRAVITY = -10.0f;

	static float SCP = 1.0f;
	static float H = (sqrt((4*MASS) / (PI*SCP*ρ_0)));
	static float BOUNDS = 20.0f;

	static const u32 MAX_NEIGHBORS = 7;

	vec VEC_GRAVITY = _mm256_set_ps(GRAVITY * DT, 0.0f, GRAVITY * DT, 0.0f, GRAVITY * DT, 0.0f, GRAVITY * DT, 0.0f);
	vec VEC_DT = v_set(DT);
	vec VEC_NEG_BOUNDS = v_set(-BOUNDS);
	vec VEC_POS_BOUNDS = v_set(BOUNDS);
	vec VEC_TWO = v_set(2.0f);

	/// NEIGHBORS ///
	struct NeighborList {
		u32 counter;
		u32 neighbors[MAX_NEIGHBORS];
	};
	#define FOR_ALL_NEIGHBORS() \
		list = neighbors_list + i; \
		for (int k = 0; k < list->counter; ++k) { \
			u32 j = list->neighbors[k];
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
		u32 hash = ((cx * 73856093) ^ (cy * 19349663)) % NR_PARTICLES;
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
		PROFILER_START(hashmap_insert)
		for (int i = 0; i < NR_PARTICLES; ++i) {
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
		PROFILER_STOP(hashmap_insert)
	}
	/// ---- ///

	// TODO(kalle): singed distance field boundary detection

	inline void integrate_wide_boundary_projection(v2 *positions, v2 *velocities) {
		PROFILER_START(integrate)
		// Eight wide boundary projection (4 vectors at a time).
		for (int i = 0; i < NR_PARTICLES; i+=4) {
			vec vel8 = v_load((float*)(velocities + i));
			vel8 = v_add(vel8, VEC_GRAVITY);

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
		PROFILER_STOP(integrate)
	}

	inline void integrate_wide_boundary_projection_v2(v2 *positions, v2 *velocities, v2 *accelerations) {
		PROFILER_START(integrate)
		// Eight wide boundary projection (4 vectors at a time).
		for (int i = 0; i < NR_PARTICLES; i+=4) {
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
		PROFILER_STOP(integrate)
	}
}
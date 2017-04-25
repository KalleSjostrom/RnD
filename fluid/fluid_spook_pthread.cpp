#include "pthread_aux.cpp"

namespace fluid {
	using namespace fluid_common;
	using namespace pthread_aux;

	float k = 1000.0f; // spring constant
	float d = 4.0f; // number of timesteps it takes to stabalize (~3-5) - τ / DT
	float ξ = 1.0f / (1.0f + 4.0f*d); // regularization to relax the system
	float e = 4.0f / (DT*DT*k*(1.0f + 4.0f*d));
	float a = 4.0f / (DT*(1.0f + 4.0f*d));
	float b = (4.0f * d) / (1.0f + 4.0f * d);

	struct JacobianEntry {
		v2 value;
		v2 neighbor_values[MAX_NEIGHBORS];
	};

	char *transient_memory = 0;
	#define TRANSIENT_MEMORY_SIZE ( \
		NR_PARTICLES * sizeof(float) + \
		NR_PARTICLES * 9 * sizeof(Element) + \
		NR_PARTICLES * sizeof(JacobianEntry) + \
		NR_PARTICLES * sizeof(Element *) \
	)

	float *Φ = 0;
	Element *elements = 0;
	JacobianEntry *jacobian = 0;
	Element **hash_map = 0;

	typedef struct {
		v2 *positions;
		v2 *velocities;
		v2 *density_pressure;
		float *λ;
		float *g;
		NeighborList *neighbors_list;
	} SpookMemory;

	PTHREAD_TASK(pthread_narrowphase_task) {
		Job *job = (Job *)userdata;

		SpookMemory *memory = (SpookMemory *)job->userdata;

		v2            *positions        = memory->positions;
		v2            *velocities       = memory->velocities;
		v2            *density_pressure = memory->density_pressure;
		float         *g                = memory->g;
		NeighborList  *neighbors_list   = memory->neighbors_list;

		for (int i = job->start; i < job->stop; ++i) {
			u32 hash = calculate_hash(positions[i].x, positions[i].y);
			Element *element = hash_map[hash];
			ASSERT(element, "Null element in hash map");
			Element *cursor;

			#define POLY6_WHEN_DIST_IS_ZERO (4.0f/(PI * H*H))
			density_pressure[i].x = MASS * POLY6_WHEN_DIST_IS_ZERO;
			NeighborList &list_i = neighbors_list[i];

			cursor = element;
			while (cursor) {
				u32 j = cursor->index;
				cursor = cursor->next;
				if (j != i) {
					v2 r_ij = positions[j] - positions[i];
					float dist_sq = dot(r_ij, r_ij);
					if (dist_sq < H*H) {
						bool can_have_more_neighbors = list_i.counter < MAX_NEIGHBORS;
						if (!can_have_more_neighbors)
							continue;

						float diff   = H*H    - dist_sq; // (h^2 - r^2)
						float diff_2 = diff   * diff;    // (h^2 - r^2)^2
						float diff_3 = diff_2 * diff;    // (h^2 - r^2)^3

						#define POLY6_C        ( 4.0f / (PI * H*H*H*H*H*H*H*H))
						#define NABLA_POLY6H_C (24.0f / (PI * H*H*H*H*H*H*H  ))

						// I already know that dist_sq is less than H*H, so no need to check in the poly6
						float density = (MASS * POLY6_C        * diff_3);
						v2 G_ij =      -(MASS * NABLA_POLY6H_C * diff_2 * inv_ρ_0) * r_ij;

						float G_ij_sq = dot(G_ij, G_ij);
						Φ[i] += G_ij_sq;

						density_pressure[i].x += density;
						JacobianEntry *entry_i = jacobian + i;
						entry_i->value -= G_ij;
						entry_i->neighbor_values[list_i.counter] = G_ij;
						list_i.neighbors[list_i.counter++] = j;
						if (list_i.counter == MAX_NEIGHBORS)
							break;
					}
				}
			}
			if (Φ[i] != 0) {
				v2 d = jacobian[i].value; // We have only looked on neighbors, not ourself.
				Φ[i] = (Φ[i] + dot(d, d)) / MASS + e;
				Φ[i] = 1.0f / Φ[i];
			} else {
				Φ[i] = FLT_MAX;
			}
			g[i] = (density_pressure[i].x * inv_ρ_0) - 1; // The constraint violation.
		}

		return NULL;
	}

	PTHREAD_TASK(pthread_gauss_seidel_itr_task) {
		Job *job = (Job *)userdata;

		SpookMemory *memory = (SpookMemory *)job->userdata;

		v2           *positions      = memory->positions;
		v2           *velocities     = memory->velocities;
		float        *c              = memory->g;
		float        *λ              = memory->λ;
		NeighborList *neighbors_list = memory->neighbors_list;
		NeighborList *list; // Used by FOR_ALL_NEIGHBORS()

		// TODO(kalle): This isn't thread safe! The velocities can be modified for a particle before it's "supposed to".
		// However, it's not a huge problem since all this does is try and reduce the error, not remove it.
		for (int i = job->start; i < job->stop; ++i) {
			if (Φ[i] != FLT_MAX) {
				float r_i = -c[i] + e*λ[i];

				r_i += dot(jacobian[i].value, velocities[i]);
				FOR_ALL_NEIGHBORS()
					r_i += dot(jacobian[i].neighbor_values[k], velocities[j]);
				}

				float dλ_i = -r_i * Φ[i];
				λ[i] += dλ_i;

				// TODO(kalle): Where the hell are the external forces?
				// Insert gravity here instead of in the integration step.
				// TODO(kalle): Don't update velocities here, this is a pointer to the opengl mapped buffer and hence super slow.
				velocities[i] = velocities[i] + (INV_MASS * dλ_i) * jacobian[i].value;
				list = neighbors_list + i;
				for (int k = 0; k < list->counter; ++k) {
					u32 j = list->neighbors[k];
					velocities[j] = velocities[j] + (INV_MASS * dλ_i) * jacobian[i].neighbor_values[k];
				}
			}
		}

		return NULL;
	}

	/* Gauss Seidel with the number of iterations fixed at one.
	When this is done, the init and iter can be merged.
	NOTE(kalle): This version is cheating!
	We are updating v[j] in the same "go" as we update α.
	*/
	PTHREAD_TASK(pthread_gauss_seidel_one_task) {
		Job *job = (Job *)userdata;

		SpookMemory *memory = (SpookMemory *)job->userdata;

		v2            *positions        = memory->positions;
		v2            *velocities       = memory->velocities;
		float         *g                = memory->g;
		NeighborList  *neighbors_list   = memory->neighbors_list;
		NeighborList *list; // Used by FOR_ALL_NEIGHBORS()

		// TODO(kalle): This isn't thread safe! The velocities can be modified for a particle before it's "supposed to".
		// However, it's not a huge problem since all this does is try and reduce the error, not remove it.
		for (int i = job->start; i < job->stop; ++i) {
			if (Φ[i] != FLT_MAX) {

				// Accumulator all the contributions for the Gv
				float Gv = dot(jacobian[i].value, velocities[i]);
				FOR_ALL_NEIGHBORS()
					Gv += dot(jacobian[i].neighbor_values[k], velocities[j]);
				}

				float r_i = Gv + a*g[i] + b*Gv;
				float dλ_i = -r_i * Φ[i];
				float inv_mass_times_dλ_i = INV_MASS * dλ_i;

				velocities[i] = velocities[i] + inv_mass_times_dλ_i * jacobian[i].value;
				list = neighbors_list + i;
				for (int k = 0; k < list->counter; ++k) {
					u32 j = list->neighbors[k];
					velocities[j] = velocities[j] + inv_mass_times_dλ_i * jacobian[i].neighbor_values[k];
				}
			}
		}

		return NULL;
	}

	SpookMemory *spook_memory;

	void simulate(v2 *positions, v2 *velocities, v2 *density_pressure) {
		PROFILER_START(memset)
		if (transient_memory == 0) {
			transient_memory = (char *) malloc(TRANSIENT_MEMORY_SIZE);
			char *memory = transient_memory;

			Φ = (float *)memory;
			memory += NR_PARTICLES * sizeof(float);

			elements = (Element *)memory;
			memory += NR_PARTICLES * 9 * sizeof(Element);

			// The Jacobian matrix (G) - the matrix of all first-order partial derivatives of the constraint functions.
			// row i in this matrix is the gradient of g_i(x)
			jacobian = (JacobianEntry *)memory;
			memory += NR_PARTICLES * sizeof(JacobianEntry);

			// Elements for keeping the linked list in the hash.
			hash_map = (Element **)memory;
			memory += NR_PARTICLES * sizeof(Element *);

			spook_memory = (SpookMemory *)malloc(sizeof(SpookMemory));
			pthread_init((void*) spook_memory);
		}
		memset(transient_memory, 0, TRANSIENT_MEMORY_SIZE);
		PROFILER_STOP(memset);

		NeighborList neighbors_list[NR_PARTICLES];
		fill_hashmap(positions, hash_map, elements, neighbors_list);

		// Magnitude ot the constraint violation.
		float g[NR_PARTICLES];

		spook_memory->positions        = positions;
		spook_memory->velocities       = velocities;
		spook_memory->density_pressure = density_pressure;
		spook_memory->g                = g;
		spook_memory->neighbors_list   = neighbors_list;

		{
			PROFILER_START(narrowphase)
				pthread_run_task(pthread_narrowphase_task);
			PROFILER_STOP(narrowphase)
		}

		{ // gauss_seidel
#if 0
			// Auxiliary holding for the aggregate value of the right hand side of the spook equation
			// Initialized to g for efficiency.
			float *c = g;
			PROFILER_START(gauss_seidel_init)
			for (int i = 0; i < NR_PARTICLES; ++i) {
				if (Φ[i] != FLT_MAX) {
					// Accumulator all the contributions for the Gv
					float Gv = dot(jacobian[i].value, velocities[i]);
					FOR_ALL_NEIGHBORS()
						Gv += dot(jacobian[i].neighbor_values[k], velocities[j]);
					}
					c[i] = -a*c[i] - b*Gv;
				}
			}
			PROFILER_STOP(gauss_seidel_init)

			// Lagrange multiliers - magnitude of the force needed to restore constraint violations.
			float λ[NR_PARTICLES] = {};
			spook_memory->λ = λ;

			PROFILER_START(gauss_seidel_iter)
			#define NR_GAUSS_SEIDEL_ITERATIONS 1
			for (int iter = 0; iter < NR_GAUSS_SEIDEL_ITERATIONS; iter++) {
#if 0
				pthread_run_task(pthread_gauss_seidel_itr_task);
#else
				for (int i = 0; i < NR_PARTICLES; ++i) {
					if (Φ[i] != FLT_MAX) {
						float r_i = -c[i] + e*λ[i];

						r_i += dot(jacobian[i].value, velocities[i]);
						FOR_ALL_NEIGHBORS()
							r_i += dot(jacobian[i].neighbor_values[k], velocities[j]);
						}

						float dλ_i = -r_i * Φ[i];
						λ[i] += dλ_i;

						// TODO(kalle): Where the hell are the external forces?
						// Insert gravity here instead of in the integration step.
						// TODO(kalle): Don't update velocities here, this is a pointer to the opengl mapped buffer and hence super slow.
						velocities[i] = velocities[i] + (INV_MASS * dλ_i) * jacobian[i].value;
						FOR_ALL_NEIGHBORS()
							velocities[j] = velocities[j] + (INV_MASS * dλ_i) * jacobian[i].neighbor_values[k];
						}
					}
				}
#endif
			}
			PROFILER_STOP(gauss_seidel_iter)
#else

 			PROFILER_START(gauss_seidel_iter)
 				pthread_run_task(pthread_gauss_seidel_one_task);
			PROFILER_STOP(gauss_seidel_iter)
#endif
		}

		integrate_wide_boundary_projection(positions, velocities);
	}
}

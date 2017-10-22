namespace fluid {
	using namespace fluid_common;

	// #define FLT_MAX 3.40282347E+38F // "No-entry-marker" for the schur complement matrix to

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
		PARTICLE_COUNT * sizeof(float) + \
		PARTICLE_COUNT * 9 * sizeof(Element) + \
		PARTICLE_COUNT * sizeof(JacobianEntry) + \
		PARTICLE_COUNT * sizeof(Element *) \
	)

	float *Φ = 0;
	Element *elements = 0;
	JacobianEntry *jacobian = 0;
	Element **hash_map = 0;

	void simulate(v2 *positions, v2 *velocities, v2 *density_pressure) {
		if (transient_memory == 0) {
			transient_memory = (char *) malloc(TRANSIENT_MEMORY_SIZE);
			char *memory = transient_memory;

			Φ = (float *)memory;
			memory += PARTICLE_COUNT * sizeof(float);

			elements = (Element *)memory;
			memory += PARTICLE_COUNT * 9 * sizeof(Element);

			// The Jacobian matrix (G) - the matrix of all first-order partial derivatives of the constraint functions.
			// row i in this matrix is the gradient of g_i(x)
			jacobian = (JacobianEntry *)memory;
			memory += PARTICLE_COUNT * sizeof(JacobianEntry);

			// Elements for keeping the linked list in the hash.
			hash_map = (Element **)memory;
			memory += PARTICLE_COUNT * sizeof(Element *);
		}
		memset(transient_memory, 0, TRANSIENT_MEMORY_SIZE);

		NeighborList neighbors_list[PARTICLE_COUNT];
		fill_hashmap(positions, hash_map, elements, neighbors_list);

		// Magnitude ot the constraint violation.
		float g[PARTICLE_COUNT];
		{
			for (u32 i = 0; i < PARTICLE_COUNT; ++i) {
				u32 hash = calculate_hash(positions[i].x, positions[i].y);
				Element *element = hash_map[hash];
				ASSERT(element, "No such element in hashmap");
				Element *cursor;

				#define POLY6_WHEN_DIST_IS_ZERO (4.0f/(PI * H*H))
				density_pressure[i].x += MASS * POLY6_WHEN_DIST_IS_ZERO;
				NeighborList &list_i = neighbors_list[i];

				cursor = element;
				while (cursor) {
					u32 j = cursor->index;
					cursor = cursor->next;
					if (j > i) {
						v2 r_ij = positions[j] - positions[i];
						float dist_sq = dot(r_ij, r_ij);
						if (dist_sq < H*H) {
							NeighborList &list_j = neighbors_list[j];
							bool can_have_more_neighbors = list_i.counter < MAX_NEIGHBORS && list_j.counter < MAX_NEIGHBORS;
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
							Φ[j] += G_ij_sq;

							density_pressure[i].x += density;
							density_pressure[j].x += density;

							// Fill in the G^T*G part of the schur complement matrix at ii.
							// This is a diagonal matrix so we just store them sequentially.
							// The M^-1 and Σ parts are calculated at the end.
							JacobianEntry *entry_i = jacobian + i;
							JacobianEntry *entry_j = jacobian + j;

							entry_i->value -= G_ij;
							entry_j->value += G_ij;

							entry_i->neighbor_values[list_i.counter] = G_ij;
							entry_j->neighbor_values[list_j.counter] = -G_ij;

							list_i.neighbors[list_i.counter++] = j;
							list_j.neighbors[list_j.counter++] = i;

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
		}

		NeighborList *list; // Used by FOR_ALL_NEIGHBORS()
		{ // gauss_seidel
#if 0
			// Auxiliary holding for the aggregate value of the right hand side of the spook equation
			// Initialized to g for efficiency.
			float *c = g;
			for (int i = 0; i < PARTICLE_COUNT; ++i) {
				if (Φ[i] != FLT_MAX) {
					// Accumulator all the contributions for the Gv
					float Gv = dot(jacobian[i].value, velocities[i]);
					FOR_ALL_NEIGHBORS()
						Gv += dot(jacobian[i].neighbor_values[k], velocities[j]);
					}
					c[i] = -a*c[i] - b*Gv;
				}
			}

			// Lagrange multiliers - magnitude of the force needed to restore constraint violations.
			float λ[PARTICLE_COUNT] = {};

			#define NR_GAUSS_SEIDEL_ITERATIONS 8
			for (int iter = 0; iter < NR_GAUSS_SEIDEL_ITERATIONS; iter++) {
				for (int i = 0; i < PARTICLE_COUNT; ++i) {
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
			}
#else
			/* Gauss Seidel with the number of iterations fixed at one.
			When this is done, the init and iter can be merged.
			NOTE(kalle): This version is cheating!
			We are updating the v[j] in the same "go" as we update α.
			*/
			for (int i = 0; i < PARTICLE_COUNT; ++i) {
				if (Φ[i] != FLT_MAX) {

					// Accumulator all the contributions for the Gv
					float Gv = dot(jacobian[i].value, velocities[i]);
					FOR_ALL_NEIGHBORS()
						Gv += dot(jacobian[i].neighbor_values[_k], velocities[j]);
					}

					float r_i = Gv + a*g[i] + b*Gv;
					float dλ_i = -r_i * Φ[i];
					float inv_mass_times_dλ_i = INV_MASS * dλ_i;

					velocities[i] = velocities[i] + inv_mass_times_dλ_i * jacobian[i].value;
					FOR_ALL_NEIGHBORS()
						velocities[j] = velocities[j] + inv_mass_times_dλ_i * jacobian[i].neighbor_values[_k];
					}
				}
			}
#endif
		}

		integrate_wide_boundary_projection(positions, velocities);
	}
}

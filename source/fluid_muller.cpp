namespace fluid {
	using namespace fluid_common;

	static float K_GAS = 8000.0f;
	static float MU = 20.0f;

	static float POLY6_C = 4.0f/(PI * H*H*H*H*H*H*H*H);
	static float SPIKY_C = -30.0f/(PI * H*H*H*H*H);
	static float VISCOSITY_C = 360.0f/(29.0f * PI * H*H*H*H*H);

	void simulate(v2 *positions, v2 *velocities, v2 *density_pressure) {
		{
			PROFILER_START(memset)
			memset(density_pressure, 0, NR_PARTICLES * sizeof(v2));
			PROFILER_STOP(memset)
		}

		// Elements for keeping the linked list in the hash.
		Element *hash_map[NR_PARTICLES] = {};
		NeighborList neighbors_list[NR_PARTICLES];
		Element elements[NR_PARTICLES * 9];
		fill_hashmap(positions, hash_map, elements, neighbors_list);
		{
			PROFILER_START(narrowphase)
			for (int i = 0; i < NR_PARTICLES; ++i) {
				u32 hash = calculate_hash(positions[i].x, positions[i].y);
				Element *element = hash_map[hash];
				ASSERT(element);
				Element *cursor;

				NeighborList &list_i = neighbors_list[i];

				float self_density = MASS * POLY6_C * H*H*H*H*H*H;
				density_pressure[i].x += self_density;

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

							float diff = H*H - dist_sq; // (h^2 - r^2)
							float density = MASS * POLY6_C * (diff*diff*diff);
							density_pressure[i].x += density;
							density_pressure[j].x += density;

							list_i.neighbors[list_i.counter++] = j;
							list_j.neighbors[list_j.counter++] = i;

							if (list_i.counter == MAX_NEIGHBORS)
								break;
						}
					}
				}
				density_pressure[i].y = K_GAS * (density_pressure[i].x - ρ_0);
			}
			PROFILER_STOP(narrowphase)
		}

		v2 accelerations[NR_PARTICLES];
		NeighborList *list; // Used by FOR_ALL_NEIGHBORS()
		{
			PROFILER_START(acceleration)
			for (int i = 0; i < NR_PARTICLES; ++i) {
				v2 a_pressure = V2(0, 0);
				v2 a_viscosity = V2(0, 0);
				v2 a_external = V2(0, GRAVITY);

				FOR_ALL_NEIGHBORS()
					v2 r_ji = positions[j] - positions[i];
					float r = length(r_ji);
					float temp = (H-r);

					float d_i = density_pressure[i].x;
					float p_i = density_pressure[i].y;

					float d_j = density_pressure[j].x;
					float p_j = density_pressure[j].y;

					float pressure_magnitude = MASS * (p_i + p_j) / (2*d_i*d_j);
					v2 p = r_ji * (pressure_magnitude * SPIKY_C * (temp*temp));
					a_pressure += p;

					float mu_magnitude = MASS * MU * VISCOSITY_C * temp;
					v2 mu = mu_magnitude * ((velocities[j] - velocities[i]) / (d_i*d_j));
					a_viscosity += mu;
				}
				accelerations[i] = a_pressure + a_viscosity + a_external;
			}
			PROFILER_STOP(acceleration)
		}

		integrate_wide_boundary_projection_v2(positions, velocities, accelerations);
	}
}


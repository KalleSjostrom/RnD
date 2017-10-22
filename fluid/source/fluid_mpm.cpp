namespace fluid {
	using namespace fluid_common;

	u32 GRID_WIDTH = (BOUNDS * 2);
	u32 GRID_HEIGHT = (BOUNDS * 2);

	inline u32 position_to_grid_index(float x, float y) {
		u32 cx = (u32)(x + BOUNDS);
		u32 cy = (u32)(y + BOUNDS);
		return cx * GRID_HEIGHT + cy;
	}

	char *transient_memory = 0;
	#define TRANSIENT_MEMORY_SIZE ( \
		GRID_WIDTH * GRID_HEIGHT * sizeof(float) + \
		NR_PARTICLES * sizeof(float) \
	)

	#define MASS 1

	float *grid_masses = 0;
	// float *point_masses = 0;

	void simulate(v2 *positions, v2 *velocities, v2 *density_pressure) {
		if (transient_memory == 0) {
			transient_memory = (char *) malloc(TRANSIENT_MEMORY_SIZE);
			char *memory = transient_memory;

			grid_masses = (float *) memory;
			memory += GRID_WIDTH * GRID_HEIGHT * sizeof(float);

			// point_masses = (float *) memory;
			// memory += NR_PARTICLES * sizeof(float);
		}
		memset(transient_memory, 0, TRANSIENT_MEMORY_SIZE);

		// Rasterize particle data to the grid
		for (int i = 0; i < NR_PARTICLES; i++) {
			v2 &p = positions[i];

			float x = p.x + BOUNDS;
			float y = p.y + BOUNDS;

			u32 cx = (u32) x;
			u32 cy = (u32) y;
			u32 grid_index = cx * GRID_HEIGHT + cy;

			float frac_x = x - cx;
			float frac_y = y - cy;

			float N(float x) {
				float ax = abs(x);
				if (ax < 1.0f)
					return ((1.0f/2.0f) * ax*ax*ax) - (ax*ax) + (2.0f/3.0f);
				if (ax < 2.0f)
					return ((-1.0f/6.0f) * ax*ax*ax) + (ax*ax) - (2*ax) + (4.0f/3.0f);
				return 0.0f;
			}

			#define MASS 1
			grid_masses[grid_index] += MASS * (N(frac_x) * N(frac_y)); // NOTE(kalle): dyadic is assumed to be regular multiply.

			// TODO(kalle): Keep an active list of grid points that some particle has been in for efficiency.
			// "We also mark the grid cells that contain at least one particle."

			// Not normalized:
			// grid_velocities[grid_index] += (velocities[i] * (N(frac_x) * N(frac_y)));

			// Velocity also should be transferred to the grid, but weighting with 'w' does not result in momentum conservation.
			// Instead, we use normalized weights for velocity:
			grid_velocities[grid_index] += (velocities[i] * MASS * (N(frac_x) * N(frac_y))) / grid_masses[grid_index];

		}

		// Compute particle volumes and densities. First timestep only.

	}
}

#include "fluid_common.cl"

//@ density_pressure zero
//@ accelerations zero
//@ velocities zero
//@ positions gen_random_pos
__kernel void calc_density_pressure(__global float2 *density_pressure, __global float2 *positions) {
	unsigned int i = get_global_id(0);
	density_pressure[i].x = 0.0;
	density_pressure[i].y = 0.0;
	for (int j = 0; j < NR_PARTICLES; ++j) {
		density_pressure[i].x += MASS * poly6(positions[j] - positions[i]);
	}

	density_pressure[i].y = K_GAS * (density_pressure[i].x - REFERENCE_DENSITY);
}

__kernel void calc_acceleration(__global float2 *density_pressure, __global float2 *accelerations, __global float2 *velocities, __global float2 *positions) {
	unsigned int i = get_global_id(0);

	float2 a_pressure = (float2)(0, 0);
	float2 a_viscosity = (float2)(0, 0);
	float2 a_external = (float2)(0, GRAVITY);
	for (int j = 0; j < NR_PARTICLES; ++j) {
		float2 r_ji = positions[j] - positions[i];

		float d_i = density_pressure[i].x;
		float p_i = density_pressure[i].y;

		float p_j = density_pressure[j].y;
		float d_j = density_pressure[j].x;

		float pressure_magnitude = MASS * (p_i + p_j) / (2*d_i*d_j);
		float2 p = pressure_magnitude * spiky(r_ji);
		a_pressure -= p;

		float2 mu = MASS * MU * (velocities[j] - velocities[i]) / (d_i*d_j) * viscosity(r_ji);
		a_viscosity += mu;
	}
	accelerations[i] = a_pressure + a_viscosity + a_external;

	velocities[i] += accelerations[i] * TIMESTEP;
	positions[i] += velocities[i] * TIMESTEP;
	project_boundary(&velocities[i], &positions[i]);
}


#define REFERENCE_DENSITY 100.0f
#define PI 3.14159265359f
#define GRAVITY -10.0f
#define TIMESTEP 0.016f

#define MASS 1.0f
#define H 1.0f // 0.16f
#define K_GAS 1.0f
#define MU 1.0f
//@ export
#define NR_PARTICLES 1024 * 10

__constant float POLY6_C = 4.0f/(PI * H);
__constant float SPIKY_C = -30.0f/(PI * H*H*H*H*H);
__constant float VISCOSITY_C = 360.0f/(29.0f * PI * H*H*H*H*H);

static float poly6(float2 r_ji) {
	float r = length(r_ji);
	return r <= H ? (POLY6_C * pow((H*H - r*r), 3)) : 0;
}

static float2 spiky(float2 r_ji) {
	float r = length(r_ji);
	float2 zero = {};
	return r <= H ? SPIKY_C * r_ji * pow((H - r), 2.0f) : zero;
}

static float viscosity(float2 r_ji) {
	float r = length(r_ji);
	return r <= H ? VISCOSITY_C * (H - r) : 0;
}

static void project_boundary(__global float2 *velocity, __global float2 *position) {
	velocity->y *= sign(position->y - (-10.0f));
	velocity->x *= sign(position->x - (-10.0f));

	if (position->x > 10.0f) {
		velocity->x = -velocity->x;
	}
	if (position->y > 10.0f) {
		velocity->y = -velocity->y;
	}

	position->y = clamp(position->y, -10.0f, 10.0f);
	position->x = clamp(position->x, -10.0f, 10.0f);
}

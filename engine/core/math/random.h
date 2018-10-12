#pragma once

#pragma warning(push)
#pragma warning(disable:4146)

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
struct Random {
	uint64_t state;
	uint64_t inc;
};

uint32_t random_u32(Random &r) {
	uint64_t oldstate = r.state;
	// Advance internal state
	r.state = oldstate * 6364136223846793005ULL + (r.inc|1);
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint64_t random_u64(Random &r) {
	u64 a = (u64)random_u32(r);
	u64 b = (u64)random_u32(r);
	return (a << 32) ^ (b);
}

float random_f32(Random &r) {
	return (float)random_u32(r) / (float)UINT_MAX;
}

float random_bilateral_f32(Random &r) {
	float a = (float)random_u32(r) / (float)UINT_MAX;
	return a * 2.0f - 1.f;
}

void random_init(Random &r, uint64_t initstate, uint64_t initseq) {
	r.state = 0U;
	r.inc = (initseq << 1u) | 1u;
	random_u32(r);
	r.state += initstate;
	random_u32(r);
}

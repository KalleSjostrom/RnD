#pragma once

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
struct RandomPCG32 {
	uint64_t state;
	uint64_t inc;
};

uint32_t random_pcg32(RandomPCG32 &r) {
	uint64_t oldstate = r.state;
	// Advance internal state
	r.state = oldstate * 6364136223846793005ULL + (r.inc|1);
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

float random_pcg32f(RandomPCG32 &r) {
    return (float)random_pcg32(r) / (float)UINT_MAX;
}

void random_pcg32_init(RandomPCG32 &r, uint64_t initstate, uint64_t initseq) {
	r.state = 0U;
	r.inc = (initseq << 1u) | 1u;
	random_pcg32(r);
	r.state += initstate;
	random_pcg32(r);
}

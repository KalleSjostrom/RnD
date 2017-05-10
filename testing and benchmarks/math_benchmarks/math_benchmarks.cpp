#include <stdio.h>
#include <stdlib.h>
#include "engine/utils/common.h"
#include "engine/utils/profiler.c"
#include "math_benchmarks.h"

enum ProfilerScopes {
	ProfilerScopes__generation,

	ProfilerScopes__vec4_mul_mm128,
	ProfilerScopes__vec4_mul_scalar,

	ProfilerScopes__vec4_mul_mm128_,

	ProfilerScopes__vec4_mul,
	ProfilerScopes__v4_mul,

	ProfilerScopes__count
};

#define SIZE (0x9FFFFFFF)
#define NR_VECTORS (SIZE/4)

void vec4_mul_mm128(f32 *v1, f32 *v2, f32 *v3) {
	__m128 v = _mm_mul_ps(*(__m128*)v1, *(__m128*)v2);
	_mm_store_ps(v3, v);
}

void vec4_mul_scalar(f32 *v1, f32 *v2, f32 *v3) {
	v3[0] = v1[0] * v2[0];
	v3[1] = v1[1] * v2[1];
	v3[2] = v1[2] * v2[2];
	v3[3] = v1[3] * v2[3];
}

void test_vec4_mul_mm128(f32 *memory) {
	for (u32 i = 0; i < SIZE-2*4; i+=4) {
		vec4_mul_mm128(memory + i, memory + i+4, memory + i+8);
	}
}
void test_vec4_mul_scalar(f32 *memory) {
	for (u32 i = 0; i < SIZE-2*4; i+=4) {
		vec4_mul_scalar(memory + i, memory + i+4, memory + i+8);
	}
}

void test_vec4_mul_mm128_(__m128 *memory) {
	for (u32 i = 0; i < NR_VECTORS-2; i++) {
		memory[i+2] = _mm_mul_ps(memory[i], memory[i+1]);
	}
}

void test_vec4_mul(vec4 *memory) {
	for (u32 i = 0; i < NR_VECTORS-2; i++) {
		memory[i+2] = memory[i] * memory[i+1];
	}
}

void test_v4_mul(v4 *memory) {
	for (u32 i = 0; i < NR_VECTORS-2; i++) {
		memory[i+2] = memory[i] * memory[i+1];
	}
}

inline void *alloc(size_t align, size_t size) {
	void *p = 0;
	posix_memalign(&p, align, size);
	return p;
}

int main() {
	f32 *memory = (f32*)alloc(8 * sizeof(f32), SIZE * sizeof(f32));
	PROFILER_START(generation);
	for (u32 i = 0; i < SIZE; ++i) {
		memory[i] = i;
	}
	PROFILER_STOP_HITS(generation, SIZE);
	PROFILER_PRINT(generation);

	TIME_IT_HITS(vec4_mul_mm128, test_vec4_mul_mm128(memory), NR_VECTORS-2);
	TIME_IT_HITS(vec4_mul_mm128_, test_vec4_mul_mm128_((__m128*) memory), NR_VECTORS-2);
	TIME_IT_HITS(vec4_mul_scalar, test_vec4_mul_scalar(memory), NR_VECTORS-2);
	TIME_IT_HITS(vec4_mul, test_vec4_mul((vec4*) memory), NR_VECTORS-2);
	TIME_IT_HITS(v4_mul, test_v4_mul((v4*) memory), NR_VECTORS-2);

	return 0;
}


#include <stdio.h>
#include "immintrin.h"
#include "../../utils/profiler.c"

#define _mm256_set_m128(va, vb) \
	    _mm256_insertf128_ps(_mm256_castps128_ps256(vb), va, 1)

enum ProfilerScopes {
	ProfilerScopes__mm_vector_128,
	ProfilerScopes__mm_vector_256,
	ProfilerScopes__mm_scalar_unrolled,
	ProfilerScopes__mm_scalar_loop,
	ProfilerScopes__random_generation,

	ProfilerScopes__count,
};

void mm_vector_128(float *am, float *bm, float *cm) {
	__m128 col_a0 = _mm_load_ps(&am[0]);
	__m128 col_a1 = _mm_load_ps(&am[4]);
	__m128 col_a2 = _mm_load_ps(&am[8]);
	__m128 col_a3 = _mm_load_ps(&am[12]);

	__m128 col_c0 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c0 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c0);
	col_c0 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c0);
	col_c0 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c0);

	__m128 col_c1 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c1 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c1);
	col_c1 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c1);
	col_c1 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c1);

	__m128 col_c2 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c2 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c2);
	col_c2 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c2);
	col_c2 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c2);

	__m128 col_c3 = _mm_mul_ps(_mm_broadcast_ss(bm++), col_a0);
	col_c3 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a1), col_c3);
	col_c3 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a2), col_c3);
	col_c3 = _mm_add_ps(_mm_mul_ps(_mm_broadcast_ss(bm++), col_a3), col_c3);

	_mm_store_ps(cm + 0, col_c0);
	_mm_store_ps(cm + 4, col_c1);
	_mm_store_ps(cm + 8, col_c2);
	_mm_store_ps(cm + 12, col_c3);
}
#if 1
void mm_vector_256(float *am, float *bm, float *cm) {
	__m256 col0 = _mm256_load_ps(&am[0]);
	__m256 col1 = _mm256_load_ps(&am[8]);

	float zero = 0;
	for (int block = 0; block < 2; ++block) {
		int bi = block * 8;

		__m256 temp = _mm256_set_m128(_mm_broadcast_ss(bm+bi), _mm_broadcast_ss(bm+5+bi));
		__m256 col_result = _mm256_mul_ps(temp, col0);

		temp = _mm256_set_m128(_mm_broadcast_ss(bm+2+bi), _mm_broadcast_ss(bm+7+bi));
		col_result = _mm256_add_ps(temp, col_result);

		temp = _mm256_set_m128(_mm_broadcast_ss(bm+4+bi), _mm_broadcast_ss(bm+1+bi));
		__m256 col_result2 = _mm256_mul_ps(temp, col0);

		temp = _mm256_set_m128(_mm_broadcast_ss(bm+6+bi), _mm_broadcast_ss(bm+4+bi));
		col_result2 = _mm256_add_ps(_mm256_mul_ps(temp, col1), col_result2);

		col_result2 = _mm256_permute2f128_ps(col_result2, col_result2, 1);
		_mm256_store_ps(cm + bi, _mm256_add_ps(col_result, col_result2));
	}
}
#else
void mm_vector_256(float *am, float *bm, float *cm) {
	__m256 col[4] = {
		_mm256_load_ps(&am[0]),
		_mm256_load_ps(&am[4]),
		_mm256_load_ps(&am[8]),
		_mm256_insertf128_ps(_mm256_load_ps(&am[12]), _mm_load_ps(&am[0]), 1)
	};

	float zero = 0;
	for (int block = 0; block < 2; ++block) {
		int bi = block * 8;
		__m256 col_result = _mm256_broadcast_ss(&zero);
		for (int i = 0; i < 4; ++i) {
			__m256 bm05 = _mm256_insertf128_ps(_mm256_broadcast_ss(&bm[i + bi]), _mm_broadcast_ss(&bm[bi + 4 + (i+1)%4]), 1);
			col_result = _mm256_add_ps(_mm256_mul_ps(bm05, col[i]), col_result);
		}
		_mm256_store_ps(cm + bi, col_result);
	}
}
#endif

void mm_scalar_unrolled(float *am, float *bm, float *cm) {
	cm[0] = am[0]*bm[0] + am[4]*bm[1] + am[8]*bm[2] + am[12]*bm[3];
	cm[1] = am[1]*bm[0] + am[5]*bm[1] + am[9]*bm[2] + am[13]*bm[3];
	cm[2] = am[2]*bm[0] + am[6]*bm[1] + am[10]*bm[2] + am[14]*bm[3];
	cm[3] = am[3]*bm[0] + am[7]*bm[1] + am[11]*bm[2] + am[15]*bm[3];

	cm[4] = am[0]*bm[4] + am[4]*bm[5] + am[8]*bm[6] + am[12]*bm[7];
	cm[5] = am[1]*bm[4] + am[5]*bm[5] + am[9]*bm[6] + am[13]*bm[7];
	cm[6] = am[2]*bm[4] + am[6]*bm[5] + am[10]*bm[6] + am[14]*bm[7];
	cm[7] = am[3]*bm[4] + am[7]*bm[5] + am[11]*bm[6] + am[15]*bm[7];

	cm[8] = am[0]*bm[8] + am[4]*bm[9] + am[8]*bm[10] + am[12]*bm[11];
	cm[9] = am[1]*bm[8] + am[5]*bm[9] + am[9]*bm[10] + am[13]*bm[11];
	cm[10] = am[2]*bm[8] + am[6]*bm[9] + am[10]*bm[10] + am[14]*bm[11];
	cm[11] = am[3]*bm[8] + am[7]*bm[9] + am[11]*bm[10] + am[15]*bm[11];

	cm[12] = am[0]*bm[12] + am[4]*bm[13] + am[8]*bm[14] + am[12]*bm[15];
	cm[13] = am[1]*bm[12] + am[5]*bm[13] + am[9]*bm[14] + am[13]*bm[15];
	cm[14] = am[2]*bm[12] + am[6]*bm[13] + am[10]*bm[14] + am[14]*bm[15];
	cm[15] = am[3]*bm[12] + am[7]*bm[13] + am[11]*bm[14] + am[15]*bm[15];
}
#define INDEX(row, col) col*4+row
void mm_scalar_loop(float *am, float *bm, float *cm) {
	int index = 0;
	for (int col = 0; col < 4; ++col) {
		int col_index = col * 4;
		for (int row = 0; row < 4; ++row) {
			// This is the dot product of a row of 'a' and a col of 'b'.
			float row_col_dot =
				am[INDEX(row, 0)] * bm[INDEX(0, col)] +
				am[INDEX(row, 1)] * bm[INDEX(1, col)] +
				am[INDEX(row, 2)] * bm[INDEX(2, col)] +
				am[INDEX(row, 3)] * bm[INDEX(3, col)];

			cm[index++] = row_col_dot;
		}
	}
}

#define NR_MATRICES 100000000
#define SIZE (NR_MATRICES*16)

void test_mm_vector_256(float *memory) {
	for (int i = 0; i < SIZE-2*16; i+=16) {
		mm_vector_256(memory + i, memory + i+16, memory + i+32);
	}
}
void test_mm_vector_128(float *memory) {
	for (int i = 0; i < SIZE-2*16; i+=16) {
		mm_vector_128(memory + i, memory + i+16, memory + i+32);
	}
}
void test_mm_scalar_unrolled(float *memory) {
	for (int i = 0; i < SIZE-2*16; i+=16) {
		mm_scalar_unrolled(memory + i, memory + i+16, memory + i+32);
	}
}
void test_mm_scalar_loop(float *memory) {
	for (int i = 0; i < SIZE-2*16; i+=16) {
		mm_scalar_loop(memory + i, memory + i+16, memory + i+32);
	}
}

void unit_test() {
	float a[16] __attribute__((aligned(16))) = {
		0.0f, 1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f, 7.0f,
		9.0f, 10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f,
	};
	float b[16] __attribute__((aligned(16))) = {
		0.0f, 1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f, 7.0f,
		9.0f, 10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f,
	};
	float c[16] = { 0 };


	mm_vector_256(a, b, c);
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%.1f ", c[row * 4 + col]);
		}
		printf("\n");
	}
	printf("\n");


	mm_vector_128(a, b, c);
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%.1f ", c[row * 4 + col]);
		}
		printf("\n");
	}
	printf("\n");


	mm_scalar_unrolled(a, b, c);
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%.1f ", c[row * 4 + col]);
		}
		printf("\n");
	}
	printf("\n");


	mm_scalar_loop(a, b, c);
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			printf("%.1f ", c[row * 4 + col]);
		}
		printf("\n");
	}
}

int main() {
#if 0
	unit_test();
#else
	float *memory;
	posix_memalign((void*)&memory, 8 * sizeof(float), SIZE * sizeof(float));

	PROFILER_START(random_generation);
	for (int i = 0; i < SIZE; ++i) {
		memory[i] = i;
	}
	PROFILER_STOP_HITS(random_generation, SIZE);
	PROFILER_PRINT(random_generation);

	TIME_IT_HITS(mm_vector_128, test_mm_vector_128(memory), NR_MATRICES-2);
	TIME_IT_HITS(mm_vector_256, test_mm_vector_256(memory), NR_MATRICES-2);
	TIME_IT_HITS(mm_scalar_unrolled, test_mm_scalar_unrolled(memory), NR_MATRICES-2);
	TIME_IT_HITS(mm_scalar_loop, test_mm_scalar_loop(memory), NR_MATRICES-2);
#endif
	return 0;
}


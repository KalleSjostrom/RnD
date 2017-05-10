//<mmintrin.h>  MMX
//<xmmintrin.h> SSE
//<emmintrin.h> SSE2
//<pmmintrin.h> SSE3
//<tmmintrin.h> SSSE3
//<smmintrin.h> SSE4.1
//<nmmintrin.h> SSE4.2
//<ammintrin.h> SSE4A
//<wmmintrin.h> AES
//<immintrin.h> AVX

#if defined(__AVX__)

	#include <immintrin.h>

	#define vec __m256
	#define v_set(a) _mm256_set1_ps(a)
	#define v_broad(a) _mm256_broadcast_ss(a)
	#define v_add(a, b) _mm256_add_ps(a, b)
	#define v_sub(a, b) _mm256_sub_ps(a, b)
	#define v_mul(a, b) _mm256_mul_ps(a, b)
	#define v_div(a, b) _mm256_div_ps(a, b)
	#define v_and(a, b) _mm256_and_ps(a, b)
	#define v_xor(a, b) _mm256_xor_ps(a, b)
	#define v_max(a, b) _mm256_max_ps(a, b)
	#define v_min(a, b) _mm256_min_ps(a, b)
	#define v_cmp_gt(a, b) _mm256_cmp_ps(a, b, _CMP_GT_OQ)
	#define v_cmp_lt(a, b) _mm256_cmp_ps(a, b, _CMP_LT_OQ)
	#define v_movemask(a) _mm256_movemask_ps(a)
	#define v_store(a, b) _mm256_store_ps(a, b)
	#define v_load(a) _mm256_load_ps(a)

	#define VECTOR_WIDTH 8
	#define VECTOR_INDICES { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f }

#elif defined(__SSE__)

	#include <x86intrin.h>

	#define vec __m128
	#define v_set(a) _mm_set1_ps(a)
	#define v_broad(a) _mm_set1_ps(a)
	#define v_add(a, b) _mm_add_ps(a, b)
	#define v_sub(a, b) _mm_sub_ps(a, b)
	#define v_mul(a, b) _mm_mul_ps(a, b)
	#define v_div(a, b) _mm_div_ps(a, b)
	#define v_and(a, b) _mm_and_ps(a, b)
	#define v_xor(a, b) _mm_xor_ps(a, b)
	#define v_max(a, b) _mm_max_ps(a, b)
	#define v_min(a, b) _mm_min_ps(a, b)
	#define v_cmp_gt(a, b) _mm_cmpgt_ps(a, b)
	#define v_cmp_lt(a, b) _mm_cmplt_ps(a, b)
	#define v_movemask(a) _mm_movemask_ps(a)
	#define v_store(a, b) _mm_store_ps(a, b)
	#define v_load(a) _mm_load_ps(a)

	#define VECTOR_WIDTH 4
	#define VECTOR_INDICES { 0.0f, 1.0f, 2.0f, 3.0f }

#endif

// const float flop = -0.0; //1000,0000,0000,0000,0000,0000,0000,0000=-0.0(IEEE)
#define v_neg(a) v_xor(v_set(-0.0f), a);

/*2. right shift
//Suppose "a" is a vtype SIMD data(128bit or 256bit), and the value of which is a7, a6, a5, a4, a3, a2, a1, a0 from high bits to low bits.
(1)AVX version(AVX)
//a7, a6, a5, a4, a3, a2, a1, a0 -> a6, a5, a4, a3, a2, a1, a0, 0
Step 1:
__m256 p0 = _mm256_permute_ps(a, 0x93) //10010011=0x93
//p0 = a6, a5, a4, a7, a2, a1, a0, a3
Step 2:
__m256 p1 = _mm256_permute2f128_ps(p0, p0, 0x8) //00001000=0x8
//p1 = a2, a1, a0, a3, 0, 0, 0, 0
Step 3:
p2 = _mm256_blend_ps(p0, p1, 0x11) //00010001=0x11
//p2 = a6, a5, a4, a3, a2, a1, a0, 0*/

/*(2)SSE version(SSE)
p0 = _mm_shuffle_ps(a, a, 0x93) //10010011=0x93
// p0 = a2, a1, a0, a3
// whenever AVX is enabled, use _mm_permute_ps(latency~1, throughput~1)  instead of _mm_shuffle_ps(latency~1~6, throughput~1)
p0 = _mm_permute_ps(a, 0x93)*/



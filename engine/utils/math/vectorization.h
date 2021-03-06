#include <intrin.h>
#define cpuid(info, x) __cpuidex(info, x, 0)

enum CpuFeatureFlag : unsigned {
	//  Misc.
	CpuFeatureFlag_MMX = 1<<0,
	CpuFeatureFlag_x64 = 1<<1,
	CpuFeatureFlag_ABM = 1<<2,
	CpuFeatureFlag_RDRAND = 1<<3,
	CpuFeatureFlag_BMI1 = 1<<4,
	CpuFeatureFlag_BMI2 = 1<<5,
	CpuFeatureFlag_ADX = 1<<6,
	CpuFeatureFlag_PREFETCHWT1 = 1<<7,

	//  SIMD: 128-bit
	CpuFeatureFlag_SSE = 1<<8,
	CpuFeatureFlag_SSE2 = 1<<9,
	CpuFeatureFlag_SSE3 = 1<<10,
	CpuFeatureFlag_SSSE3 = 1<<11,
	CpuFeatureFlag_SSE41 = 1<<12,
	CpuFeatureFlag_SSE42 = 1<<13,
	CpuFeatureFlag_SSE4a = 1<<14,
	CpuFeatureFlag_AES = 1<<15,
	CpuFeatureFlag_SHA = 1<<16,

	//  SIMD: 256-bit
	CpuFeatureFlag_AVX = 1<<17,
	CpuFeatureFlag_XOP = 1<<18,
	CpuFeatureFlag_FMA3 = 1<<19,
	CpuFeatureFlag_FMA4 = 1<<20,
	CpuFeatureFlag_AVX2 = 1<<21,

	//  SIMD: 512-bit
	CpuFeatureFlag_AVX512F = 1<<22, // AVX512 Foundation
	CpuFeatureFlag_AVX512CD = 1<<23, // AVX512 Conflict Detection
	CpuFeatureFlag_AVX512PF = 1<<24, // AVX512 Prefetch
	CpuFeatureFlag_AVX512ER = 1<<25, // AVX512 Exponential + Reciprocal
	CpuFeatureFlag_AVX512VL = 1<<26, // AVX512 Vector Length Extensions
	CpuFeatureFlag_AVX512BW = 1<<27, // AVX512 Byte + Word
	CpuFeatureFlag_AVX512DQ = 1<<28, // AVX512 Doubleword + Quadword
	CpuFeatureFlag_AVX512IFMA = 1<<29, // AVX512 Integer 52-bit Fused Multiply-Add
	CpuFeatureFlag_AVX512VBMI = 1<<30, // AVX512 Vector Byte Manipulation Instructions
};

unsigned detect_cpu_features() {
	int info[4];
	cpuid(info, 0);
	int nIds = info[0];

	cpuid(info, 0x80000000);
	int nExIds = info[0];

	unsigned flags = 0;

	// Detect Features
	if (nIds >= 0x00000001) {
		cpuid(info, 0x00000001);
		flags |= (((unsigned)info[3] & ((unsigned)1 << 23)) != 0) ? CpuFeatureFlag_MMX : 0;
		flags |= (((unsigned)info[3] & ((unsigned)1 << 25)) != 0) ? CpuFeatureFlag_SSE : 0;
		flags |= (((unsigned)info[3] & ((unsigned)1 << 26)) != 0) ? CpuFeatureFlag_SSE2 : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 <<  0)) != 0) ? CpuFeatureFlag_SSE3 : 0;

		flags |= (((unsigned)info[2] & ((unsigned)1 <<  9)) != 0) ? CpuFeatureFlag_SSSE3 : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 << 19)) != 0) ? CpuFeatureFlag_SSE41 : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 << 20)) != 0) ? CpuFeatureFlag_SSE42 : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 << 25)) != 0) ? CpuFeatureFlag_AES : 0;

		flags |= (((unsigned)info[2] & ((unsigned)1 << 28)) != 0) ? CpuFeatureFlag_AVX : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 << 12)) != 0) ? CpuFeatureFlag_FMA3 : 0;

		flags |= (((unsigned)info[2] & ((unsigned)1 << 30)) != 0) ? CpuFeatureFlag_RDRAND : 0;
	}
	if (nIds >= 0x00000007) {
		cpuid(info, 0x00000007);
		flags |= (((unsigned)info[1] & ((unsigned)1 <<  5)) != 0) ? CpuFeatureFlag_AVX2 : 0;

		flags |= (((unsigned)info[1] & ((unsigned)1 <<  3)) != 0) ? CpuFeatureFlag_BMI1 : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 <<  8)) != 0) ? CpuFeatureFlag_BMI2 : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 19)) != 0) ? CpuFeatureFlag_ADX : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 29)) != 0) ? CpuFeatureFlag_SHA : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 <<  0)) != 0) ? CpuFeatureFlag_PREFETCHWT1 : 0;

		flags |= (((unsigned)info[1] & ((unsigned)1 << 16)) != 0) ? CpuFeatureFlag_AVX512F : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 28)) != 0) ? CpuFeatureFlag_AVX512CD : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 26)) != 0) ? CpuFeatureFlag_AVX512PF : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 27)) != 0) ? CpuFeatureFlag_AVX512ER : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 31)) != 0) ? CpuFeatureFlag_AVX512VL : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 30)) != 0) ? CpuFeatureFlag_AVX512BW : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 17)) != 0) ? CpuFeatureFlag_AVX512DQ : 0;
		flags |= (((unsigned)info[1] & ((unsigned)1 << 21)) != 0) ? CpuFeatureFlag_AVX512IFMA : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 <<  1)) != 0) ? CpuFeatureFlag_AVX512VBMI : 0;
	}
	if ((unsigned)nExIds >= 0x80000001) {
		cpuid(info, 0x80000001);
		flags |= (((unsigned)info[3] & ((unsigned)1 << 29)) != 0) ? CpuFeatureFlag_x64 : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 <<  5)) != 0) ? CpuFeatureFlag_ABM : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 <<  6)) != 0) ? CpuFeatureFlag_SSE4a : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 << 16)) != 0) ? CpuFeatureFlag_FMA4 : 0;
		flags |= (((unsigned)info[2] & ((unsigned)1 << 11)) != 0) ? CpuFeatureFlag_XOP : 0;
	}

	return flags;
}


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
#if 0
struct wf32 {
	__m128 v;
	wf32(__m128 _v) { v = _v; }
	operator __m128() const { return v; }
};
struct wf64 {
	__m128d v;
	wf64(__m128d _v) { v = _v; }
	operator __m128() const { return v; }
};
struct wi32 {
	__m128i v;
	wi32(__m128i _v) { v = _v; }
	operator __m128i() const { return v; }
};
struct wi16 {
	__m128i v;
	wi16(__m128i _v) { v = _v; }
	operator __m128i() const { return v; }
};
struct wi8 {
	__m128i v;
	wi8(__m128i _v) { v = _v; }
	operator __m128i() const { return v; }
};

FORCE_INLINE wf32 operator*(wf32 a, wf32 b) {
	wf32 result = { _mm_mul_ps(a.v, b.v) };
	return result;
}
FORCE_INLINE wf32 operator/(wf32 a, wf32 b) {
	wf32 result = _mm_div_ps(a.v, b.v);
	return result;
}
FORCE_INLINE wf32 operator+(wf32 a, wf32 b) {
	wf32 result = { _mm_add_ps(a.v, b.v) };
	return result;
}
FORCE_INLINE wf32 operator-(wf32 a, wf32 b) {
	wf32 result = { _mm_sub_ps(a.v, b.v) };
	return result;
}
static wf32 wf32_zero = { _mm_set1_ps(0) };
FORCE_INLINE wf32 operator-(wf32 a) {
	wf32 result = { _mm_sub_ps(wf32_zero, a.v) };
	return result;
}

FORCE_INLINE wf64 operator*(wf64 a, wf64 b) {
	wf64 result = { _mm_mul_pd(a.v, b.v) };
	return result;
}
FORCE_INLINE wf64 operator/(wf64 a, wf64 b) {
	wf64 result = { _mm_div_pd(a.v, b.v) };
	return result;
}
FORCE_INLINE wf64 operator+(wf64 a, wf64 b) {
	wf64 result = { _mm_add_pd(a.v, b.v) };
	return result;
}
FORCE_INLINE wf64 operator-(wf64 a, wf64 b) {
	wf64 result = { _mm_sub_pd(a.v, b.v) };
	return result;
}


FORCE_INLINE wi32 operator*(wi32 a, wi32 b) {
	wi32 result = { _mm_mul_epi32(a.v, b.v) };
	return result;
}
// FORCE_INLINE wi32 operator/(wi32 a, wi32 b) {
// 	wi32 result = { _mm_div_epi32(a.v, b.v) };
// 	return result;
// }
FORCE_INLINE wi32 operator+(wi32 a, wi32 b) {
	wi32 result = { _mm_add_epi32(a.v, b.v) };
	return result;
}
FORCE_INLINE wi32 operator-(wi32 a, wi32 b) {
	wi32 result = { _mm_sub_epi32(a.v, b.v) };
	return result;
}

struct wv3 {
	wf32 x;
	wf32 y;
	wf32 z;
};

FORCE_INLINE wv3 wV3(wf32 x, wf32 y, wf32 z) {
	wv3 result = {x, y, z};
	return result;
}

FORCE_INLINE wv3 operator*(wf32 a, wv3 b) {
	return wV3(a * b.x, a * b.y, a * b.z);
}
FORCE_INLINE wv3 operator*(wv3 a, wf32 b) {
	return b * a;
}
FORCE_INLINE wv3 & operator*=(wv3 &a, wf32 b) {
	a = a * b;
	return a;
}

FORCE_INLINE wv3 operator/(wv3 a, wf32 b) {
	return wV3(a.x/b, a.y/b, a.z/b);
}

FORCE_INLINE wv3 & operator/=(wv3 &a, wf32 b) {
	a = a/b;
	return a;
}

FORCE_INLINE wv3 operator+(wv3 a, wv3 b) {
	return wV3(a.x + b.x, a.y + b.y, a.z + b.z);
}
FORCE_INLINE wv3 & operator+=(wv3 &a, wv3 b) {
	a = a + b;
	return a;
}

FORCE_INLINE wv3 operator-(wv3 a) {
	return wV3(-a.x, -a.y, -a.z);
}
FORCE_INLINE wv3 operator-(wv3 a, wv3 b) {
	return wV3(a.x - b.x, a.y - b.y, a.z - b.z);
}
FORCE_INLINE wv3 & operator-=(wv3 &a, wv3 b) {
	a = a - b;
	return a;
}

FORCE_INLINE wv3 hadamard(wv3 a, wv3 b) {
	return wV3(a.x * b.x, a.y * b.y, a.z * b.z);
}
FORCE_INLINE wf32 dot(wv3 a, wv3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
FORCE_INLINE wf32 length_squared(wv3 a) {
	return dot(a, a);
}
// FORCE_INLINE wf32 length(wv3 a) {
// 	return sqrtf(length_squared(a));
// }
// FORCE_INLINE wv3 normalize(wv3 a) {
// 	wf32 l = length(a);
// 	return (1.0f/l) * a;
// }
// FORCE_INLINE wv3 normalize_or_zero(wv3 a) {
// 	wf32 l = length(a);
// 	return l > 0 ? (1.0f/l) * a : wV3(0, 0, 0);
// }
FORCE_INLINE wv3 cross(wv3 a, wv3 b) {
	wf32 x = a.y*b.z - a.z*b.y;
	wf32 y = a.z*b.x - a.x*b.z;
	wf32 z = a.x*b.y - a.y*b.x;
	return wV3(x, y, z);
}

// FORCE_INLINE wv3 lerp(wv3 a, wv3 b, wf32 t) {
// 	return wV3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
// }
#endif

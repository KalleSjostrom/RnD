#define MAKE_NAME(prefix, type) prefix##type

#define VECTOR_NAME_(type) MAKE_NAME(v2_, type)
#define VECTOR_NAME VECTOR_NAME_(ELEMENT_TYPE)

#define CONSTRUCTOR_NAME_(type) MAKE_NAME(V2_, type)
#define CONSTRUCTOR_NAME CONSTRUCTOR_NAME_(ELEMENT_TYPE)

struct VECTOR_NAME {
	ELEMENT_TYPE x, y;
};

FORCE_INLINE VECTOR_NAME CONSTRUCTOR_NAME(ELEMENT_TYPE x, ELEMENT_TYPE y) {
	VECTOR_NAME result = {x, y};
	return result;
}
#if 0
FORCE_INLINE VECTOR_NAME random_v2() {
	ELEMENT_TYPE x = (ELEMENT_TYPE)random()/RAND_MAX;
	x = x*2.0f - 1.0;
	ELEMENT_TYPE y = (ELEMENT_TYPE)random()/RAND_MAX;
	y = y*2.0 - 1.0;
	return CONSTRUCTOR_NAME(x, y);
}
#endif

FORCE_INLINE VECTOR_NAME operator*(ELEMENT_TYPE a, VECTOR_NAME b) {
	return CONSTRUCTOR_NAME(a * b.x, a * b.y);
}
FORCE_INLINE VECTOR_NAME operator*(VECTOR_NAME a, ELEMENT_TYPE b) {
	return b * a;
}
FORCE_INLINE VECTOR_NAME & operator*=(VECTOR_NAME &a, ELEMENT_TYPE b) {
	a = a * b;
	return a;
}

FORCE_INLINE VECTOR_NAME operator/(VECTOR_NAME a, ELEMENT_TYPE b) {
	return CONSTRUCTOR_NAME(a.x/b, a.y/b);
}
FORCE_INLINE VECTOR_NAME & operator/=(VECTOR_NAME &a, ELEMENT_TYPE b) {
	a = a/b;
	return a;
}

FORCE_INLINE VECTOR_NAME operator+(VECTOR_NAME a, VECTOR_NAME b) {
	return CONSTRUCTOR_NAME(a.x + b.x, a.y + b.y);
}
FORCE_INLINE VECTOR_NAME & operator+=(VECTOR_NAME &a, VECTOR_NAME b) {
	a = a + b;
	return a;
}

FORCE_INLINE VECTOR_NAME operator-(VECTOR_NAME a) {
	return CONSTRUCTOR_NAME(-a.x, -a.y);
}
FORCE_INLINE VECTOR_NAME operator-(VECTOR_NAME a, VECTOR_NAME b) {
	return CONSTRUCTOR_NAME(a.x - b.x, a.y - b.y);
}
FORCE_INLINE VECTOR_NAME & operator-=(VECTOR_NAME &a, VECTOR_NAME b) {
	a = a - b;
	return a;
}

FORCE_INLINE VECTOR_NAME hadamard(VECTOR_NAME a, VECTOR_NAME b) {
	return CONSTRUCTOR_NAME(a.x * b.x, a.y * b.y);
}
FORCE_INLINE ELEMENT_TYPE dot(VECTOR_NAME a, VECTOR_NAME b) {
	return a.x * b.x + a.y * b.y;
}
FORCE_INLINE ELEMENT_TYPE length_squared(VECTOR_NAME a) {
	return dot(a, a);
}
FORCE_INLINE ELEMENT_TYPE length(VECTOR_NAME a) {
	return (ELEMENT_TYPE)sqrt((f64)length_squared(a));
}
FORCE_INLINE VECTOR_NAME normalize(VECTOR_NAME a) {
	ELEMENT_TYPE l = length(a);
	return ((ELEMENT_TYPE)1.0/l) * a;
}
FORCE_INLINE VECTOR_NAME normalize_or_zero(VECTOR_NAME a) {
	ELEMENT_TYPE l = length(a);
	return l > 0 ? ((ELEMENT_TYPE)1.0/l) * a : CONSTRUCTOR_NAME(0, 0);
}
FORCE_INLINE ELEMENT_TYPE cross(VECTOR_NAME a, VECTOR_NAME b) {
	return (a.x*b.y) - (a.y*b.x);
}
FORCE_INLINE VECTOR_NAME cross(VECTOR_NAME a) {
	return CONSTRUCTOR_NAME(-a.y, a.x);
}

#undef ELEMENT_TYPE
// #undef VECTOR_NAME
// #undef VECTOR_NAME_

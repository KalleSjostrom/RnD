#pragma once

/// A fast LCG random number generator. The numbers it produces are not completely
/// random but it is faster and more random than vanilla C rand(). It should be good
/// enough for most purposes.
class Random
{
	// This variable should NOT need to be volatile, but there is a bug in VC++. If
	// this variable is not volatile Vector3(random(), random(), random()) sets all
	// components to the same value.
	volatile unsigned _state;

public:
	static const unsigned A = 1664525 , B = 1013904223;

	__forceinline Random() : _state(0) {}
	__forceinline Random(unsigned long s) : _state(unsigned(s)) {}
	__forceinline unsigned long seed() const {return _state;}
	__forceinline void set_seed(unsigned long s) {_state = unsigned(s);}

	__forceinline void next() {_state = A * _state + B;}
	__forceinline unsigned long rand_u32() {next(); return _state;}
	__forceinline double rand_double() {return double(rand_u32()) * (0.5 / 0x80000000);}
	__forceinline float rand_float() {return float(rand_u32()) * (0.5f / 0x80000000);}

	__forceinline double operator()(double max) {return max * rand_double();}
	__forceinline double operator()(double min, double max) {return min + (max - min)*rand_double();}

	__forceinline float operator()() {return rand_float();}
	__forceinline float operator()(float max) {return max * rand_float();}
	__forceinline float operator()(float min, float max) {return min + (max - min)*rand_float();}

	__forceinline int operator()(int max_plus_1)  {return (int)floor(max_plus_1*rand_double());}
	__forceinline int operator()(int min, int max) {return (int)floor(min + (max - min + 1)*rand_double());}

	__forceinline unsigned operator()(unsigned max_plus_1)  {return unsigned(max_plus_1*rand_double());}
	__forceinline unsigned operator()(unsigned min, unsigned max) {return unsigned(min + (max - min + 1)*rand_double());}
};
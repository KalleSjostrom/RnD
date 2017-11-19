#pragma once

#ifdef OS_WINDOWS
	struct Stopwatch {
		LARGE_INTEGER frequency;
		LARGE_INTEGER start_counter;
		float time;
		Stopwatch() {
			QueryPerformanceFrequency(&frequency);
			start();
		}
		void start() {
			QueryPerformanceCounter(&start_counter);
		}
		float stop() {
			LARGE_INTEGER stop_counter;
			QueryPerformanceCounter(&stop_counter);
			time = (float)(stop_counter.QuadPart - start_counter.QuadPart) / frequency.QuadPart;
			return time;
		}
	};

	#define rdtsc __rdtsc
#else
	#include <sys/time.h>
	u64 rdtsc() {
		unsigned int lo,hi;
		__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
		return ((u64)hi << 32) | lo;
	}
	f64 my_wtime() {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (f64) tv.tv_sec + (f64) tv.tv_usec * 1E-6;
	}
#endif

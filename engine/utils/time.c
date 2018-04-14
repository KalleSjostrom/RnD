#pragma once

#ifdef OS_WINDOWS
	typedef struct {
		LARGE_INTEGER frequency;
		LARGE_INTEGER start_counter;
	} Stopwatch;

	void stopwatch_init(Stopwatch *sw) {
		QueryPerformanceFrequency(&sw->frequency);
	}

	void stopwatch_start(Stopwatch *sw) {
		QueryPerformanceCounter(&sw->start_counter);
	}

	double stopwatch_stop(Stopwatch *sw) {
		LARGE_INTEGER stop_counter;
		QueryPerformanceCounter(&stop_counter);
		double time = (double)(stop_counter.QuadPart - sw->start_counter.QuadPart) / sw->frequency.QuadPart;
		return time;
	}

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

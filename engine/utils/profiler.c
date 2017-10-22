#include <stdio.h>
#include "engine/utils/logger.h"

typedef struct {
	u64 cycle_count;
	u64 hit_count;
	f64 elapsed_time;
} PerfCounter;

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

	#define PROFILER_START(id) uint64_t __start_cycle_count##id = __rdtsc(); Stopwatch __start_time##id;
	#define PROFILER_STOP(id) profilers[ProfilerScopes__##id].cycle_count += (__rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count++; profilers[ProfilerScopes__##id].elapsed_time += __start_time##id.stop();
	#define PROFILER_STOP_HITS(id, hits) profilers[ProfilerScopes__##id].cycle_count += (__rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count+=hits; profilers[ProfilerScopes__##id].elapsed_time += __start_time##id.stop();
	#define PROFILER_PRINT(id) LOG_INFO("Profiler", "\t[time: %6f\t\t%s]\n", profilers[ProfilerScopes__##id].elapsed_time, #id);
	#define PROFILER_RESET(id) { PerfCounter &counter = profilers[ProfilerScopes__##id]; counter.cycle_count = 0; counter.hit_count = 0; counter.elapsed_time = 0.0f; }
	#define rdtsc __rdtsc
#else
	#include <stdlib.h>
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

	#define PROFILER_START(id) u64 __start_cycle_count##id = rdtsc(); f64 __start_time##id = my_wtime();
	#define PROFILER_STOP(id) profilers[ProfilerScopes__##id].cycle_count += (rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count++; profilers[ProfilerScopes__##id].elapsed_time += my_wtime() - __start_time##id;
	#define PROFILER_STOP_HITS(id, hits) profilers[ProfilerScopes__##id].cycle_count += (rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count+=hits; profilers[ProfilerScopes__##id].elapsed_time += my_wtime() - __start_time##id;
	#define PROFILER_PRINT(id) LOG_INFO("Profiler", "[%-20s]\ttime: %8f\n", #id, profilers[ProfilerScopes__##id].elapsed_time);
	#define PROFILER_RESET(id) { PerfCounter &id##counter = profilers[ProfilerScopes__##id]; id##counter.cycle_count = 0; id##counter.hit_count = 0; id##counter.elapsed_time = 0.0; }
#endif

#define TIME_IT(id, a) PROFILER_START(id); a; PROFILER_STOP(id); PROFILER_PRINT(id);
#define TIME_IT_HITS(id, a, hits) PROFILER_START(id); a; PROFILER_STOP_HITS(id, hits); PROFILER_PRINT(id);

/*
enum ProfilerScopes {
	ProfilerScopes__sequential_vector,
	ProfilerScopes__sequential_vector_inner,
	ProfilerScopes__sequential_scalar,
	ProfilerScopes__sequential_scalar_inner,
	ProfilerScopes__pthread_scalar,
	ProfilerScopes__pthread_scalar_inner,
	ProfilerScopes__pthread_vector,
	ProfilerScopes__pthread_vector_inner,
	ProfilerScopes__write_ppm,

	ProfilerScopes__count,
};
*/

static PerfCounter profilers[1024] = {};

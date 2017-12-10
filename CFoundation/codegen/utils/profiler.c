#include <stdint.h>

typedef struct {
	uint64_t cycle_count;
	unsigned int hit_count;
	double elapsed_time;
} PerfCounter;

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
// #define PROFILER_PRINT(id) printf("[%-20s]\tcycles: %12llu,\ttime: %6f\n", #id, profilers[ProfilerScopes__##id].cycle_count, profilers[ProfilerScopes__##id].elapsed_time);
#define PROFILER_PRINT(id) printf("time: %6f\t\t%s\n", profilers[ProfilerScopes__##id].elapsed_time, #id);
#define PROFILER_RESET(id) { PerfCounter &counter = profilers[ProfilerScopes__##id]; counter.cycle_count = 0; counter.hit_count = 0; counter.elapsed_time = 0.0f; }

#define TIME_IT(id, a) PROFILER_START(id); a; PROFILER_STOP(id); PROFILER_PRINT(id);
#define TIME_IT_HITS(id, a, hits) PROFILER_START(id); a; PROFILER_STOP_HITS(id, hits); PROFILER_PRINT(id);

/*
enum ProfilerScopes {
	ProfilerScopes__some_scope,
	ProfilerScopes__count,
};
*/

PerfCounter profilers[1024] = {0};
#include <stdio.h>
#include "core/utils/stopwatch.cpp"

typedef struct {
	u64 cycle_count;
	u64 hit_count;
	f64 elapsed_time;
} PerfCounter;

#define PROFILER_START(id) uint64_t __start_cycle_count##id = __rdtsc(); Stopwatch __start_time##id;
#define PROFILER_STOP(id) profilers[ProfilerScopes__##id].cycle_count += (__rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count++; profilers[ProfilerScopes__##id].elapsed_time += __start_time##id.stop();
#define PROFILER_STOP_HITS(id, hits) profilers[ProfilerScopes__##id].cycle_count += (__rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count+=hits; profilers[ProfilerScopes__##id].elapsed_time += __start_time##id.stop();
#define PROFILER_PRINT(id) LOG_INFO("Profiler", "\t[time: %6f\t\t%s]\n", profilers[ProfilerScopes__##id].elapsed_time, #id);
#define PROFILER_RESET(id) { PerfCounter &counter = profilers[ProfilerScopes__##id]; counter.cycle_count = 0; counter.hit_count = 0; counter.elapsed_time = 0.0f; }
#define rdtsc __rdtsc

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

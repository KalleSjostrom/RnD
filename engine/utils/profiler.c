#include <stdio.h>
#include "logger.h"
#include "time.c"

typedef struct {
	u64 cycle_count;
	u64 hit_count;
	f64 elapsed_time;
} PerfCounter;

#ifdef OS_WINDOWS
	#define PROFILER_START(id) uint64_t __start_cycle_count##id = __rdtsc(); Stopwatch __start_time##id;
	#define PROFILER_STOP(id) profilers[ProfilerScopes__##id].cycle_count += (__rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count++; profilers[ProfilerScopes__##id].elapsed_time += __start_time##id.stop();
	#define PROFILER_STOP_HITS(id, hits) profilers[ProfilerScopes__##id].cycle_count += (__rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count+=hits; profilers[ProfilerScopes__##id].elapsed_time += __start_time##id.stop();
	#define PROFILER_PRINT(id) LOG_INFO("Profiler", "\t[time: %6f\t\t%s]\n", profilers[ProfilerScopes__##id].elapsed_time, #id);
	#define PROFILER_RESET(id) { PerfCounter &counter = profilers[ProfilerScopes__##id]; counter.cycle_count = 0; counter.hit_count = 0; counter.elapsed_time = 0.0f; }
	#define rdtsc __rdtsc
#else
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

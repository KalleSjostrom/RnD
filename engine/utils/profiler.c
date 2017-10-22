#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/*
	sysctl -a | grep machdep.cpu

	machdep.cpu.cache.linesize: 64
	machdep.cpu.cache.L2_associativity: 8
	machdep.cpu.cache.size: 256

	L2 Cache (per Core):	256 KB
	L3 Cache:	6 MB
*/

typedef struct {
	u64 cycle_count;
	u64 hit_count;
	double elapsed_time;
} PerfCounter;

u64 rdtsc() {
	unsigned int lo,hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((u64)hi << 32) | lo;
}
double my_wtime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double) tv.tv_usec * 1E-6;
}

#define PROFILER_START(id) u64 __start_cycle_count##id = rdtsc(); double __start_time##id = my_wtime();
#define PROFILER_STOP(id) profilers[ProfilerScopes__##id].cycle_count += (rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count++; profilers[ProfilerScopes__##id].elapsed_time += my_wtime() - __start_time##id;
#define PROFILER_STOP_HITS(id, hits) profilers[ProfilerScopes__##id].cycle_count += (rdtsc() - __start_cycle_count##id); profilers[ProfilerScopes__##id].hit_count+=hits; profilers[ProfilerScopes__##id].elapsed_time += my_wtime() - __start_time##id;
//#define PROFILER_PRINT(id) printf("[%-30s]\tcycles: %12llu,\t\thits: %10u,\t\tcycles/hit: %12llu,\t\ttime: %10f\n", #id, profilers[ProfilerScopes__##id].cycle_count, profilers[ProfilerScopes__##id].hit_count, profilers[ProfilerScopes__##id].hit_count > 0 ? profilers[ProfilerScopes__##id].cycle_count / profilers[ProfilerScopes__##id].hit_count : 0, profilers[ProfilerScopes__##id].elapsed_time);
#define PROFILER_PRINT(id) printf("[%-20s]\ttime: %8f\n", #id, profilers[ProfilerScopes__##id].elapsed_time);
#define PROFILER_RESET(id) { PerfCounter &id##counter = profilers[ProfilerScopes__##id]; id##counter.cycle_count = 0; id##counter.hit_count = 0; id##counter.elapsed_time = 0.0; }

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

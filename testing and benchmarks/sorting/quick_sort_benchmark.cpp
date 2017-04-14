#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../../utils/common.h"
#include "../../utils/profiler.c"

#include "../../utils/quick_sort.cpp"

enum ProfilerScopes {
	ProfilerScopes__alloc,
	ProfilerScopes__memcpy,
	ProfilerScopes__manualcpy,
	ProfilerScopes__manualcpy2,
	ProfilerScopes__generation,
	ProfilerScopes__quick_sort,
	ProfilerScopes__assertion,

	ProfilerScopes__count,
};

inline void *alloc(size_t align, size_t SIZE) {
	void *p = 0;
	posix_memalign(&p, align, SIZE);
	return p;
}

#define SIZE 50000000
void profile_sort_elements() {
	PROFILER_START(alloc);

	SortElement *entries = (SortElement*)alloc(16, SIZE * sizeof(SortElement));
	PROFILER_STOP(alloc);

#if 0
	SortElement *entries2 = (SortElement*)alloc(16, SIZE * sizeof(SortElement));

	char *a = (char*)entries;
	char *b = (char*)entries2;
	for (int i = 0; i < SIZE*sizeof(SortElement); i++) {
		a[i] = *(char*)(b+i);
	}
	PROFILER_START(manualcpy2);
	for (int i = 0; i < SIZE; i++) {
		entries[i] = entries2[i];
	}
	PROFILER_STOP(manualcpy2);

	PROFILER_START(manualcpy);
	for (int i = 0; i < SIZE*sizeof(SortElement); i++) {
		a[i] = *(char*)(b+i);
	}
	PROFILER_STOP(manualcpy);

	PROFILER_START(memcpy);
	memcpy(entries2, entries, SIZE * sizeof(SortElement));
	PROFILER_STOP(memcpy);
#else
	PROFILER_START(generation);
	int seed = rdtsc();
	srandom(seed);
	for (int i = 0; i < SIZE; i++) {
		entries[i].value = random();
	}
	PROFILER_STOP(generation);

	PROFILER_START(quick_sort);
	quick_sort(entries, SIZE);
	PROFILER_STOP(quick_sort);

	/*for (int i = 0; i < SIZE; i++) {
		printf("%u\n", entries[i].value);
	}*/

	PROFILER_START(assertion);
	for (int i = 1; i < SIZE; i++) {
		ASSERT_MSG_VAR(entries[i].value >= entries[i-1].value, "%u, %u, %d %d", entries[i].value, entries[i-1].value, i, seed);
	}
	PROFILER_STOP(assertion);
#endif
	PROFILER_PRINT(alloc);
	PROFILER_PRINT(manualcpy);
	PROFILER_PRINT(manualcpy2);
	PROFILER_PRINT(memcpy);
	PROFILER_PRINT(generation);
	PROFILER_PRINT(quick_sort);
	PROFILER_PRINT(assertion);
}

int main() {
	profile_sort_elements();
}
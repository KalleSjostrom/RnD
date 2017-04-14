
#define FTL_FIBER_STACK_GUARD_PAGES

#include "task_scheduler.cpp"

struct NumberSubset {
	uint64 start;
	uint64 end;

	uint64 total;
};

void AddNumberSubset(TaskScheduler *scheduler, void *arg) {
	(void)scheduler;
	NumberSubset *subset = reinterpret_cast<NumberSubset *>(arg);

	subset->total = 0;

	while (subset->start != subset->end) {
		subset->total += subset->start;
		++subset->start;
	}

	subset->total += subset->end;
}

void TriangleNumberMainTask(TaskScheduler *scheduler, void *arg) {
	MemoryArena arena = init_memory(32*MB);
	(void)arg;

	printf("Hello2\n"); fflush(stdout);

// #if 0
	// Define the constants to test
	const uint64 triangle_num = 4759324;
	const uint64 num_additions_per_task = 10000;
	const uint64 num_tasks = (triangle_num + num_additions_per_task - 1) / num_additions_per_task;

	// Create the tasks
	Task *tasks = PUSH_STRUCTS(arena, num_tasks, Task);
	// We have to declare this on the heap so other threads can access it
	NumberSubset *subsets = PUSH_STRUCTS(arena, num_tasks, NumberSubset);
	uint64 nextNumber = 1;

	printf("num tasks %llu\n", num_tasks);

	for (uint64 i = 0; i < num_tasks; ++i) {
		NumberSubset *subset = &subsets[i];

		subset->start = nextNumber;
		subset->end = nextNumber + num_additions_per_task - 1;
		if (subset->end > triangle_num) {
			subset->end = triangle_num;
		}

		tasks[i].Function = AddNumberSubset;
		tasks[i].ArgData = subset;

		nextNumber = subset->end + 1;
	}
	printf("1\n");

	// Schedule the tasks and wait for them to complete
	int job_handle = scheduler->add_tasks(num_tasks, tasks);
	printf("3\n");
	scheduler->wait_for_job(job_handle, 0);

	printf("5\n");

	// Add the results
	uint64 result = 0;
	for (uint64 i = 0; i < num_tasks; ++i) {
		result += subsets[i].total;
	}

	printf("Hello (%llu) (%llu) (%llu)\n", triangle_num, triangle_num * (triangle_num + 1) / 2, result);

	// Test
	// GTEST_ASSERT_EQ(triangle_num * (triangle_num + 1ull) / 2ull, result);
// #endif
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

#if 0
uint64 input = (uint64)atoi(argv[1]);
uint64 output;

asm("andq $-16, %0"
    : "=r" (output)
    : "r" (input));

printf("%llu\n", output);

uint64 hej = -16u;

printf("%llu\n", input & hej);
#endif

#if 1
	MemoryArena arena = init_memory(32*MB, true);
	TaskScheduler scheduler = {};
	printf("Hello\n");
	scheduler.run(arena, TriangleNumberMainTask);
#endif
}

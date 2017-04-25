#include "../utils/common.h"
#include "../utils/memory_arena.cpp"

#if 0
extern "C" {
	#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
		extern void * __stack_chk_guard = (void *)(0xDEADBEEF); // initialize guard variable
	#pragma clang diagnostic pop

	void __stack_chk_fail(void) { // called by stack checking code if guard variable is corrupted
		ASSERT(false, "___stack_chk_fail");
	}

	void _exit(int status) {
		(void) status;
	}
}
#endif

#define FIBER_STACK_GUARD_PAGES
#include "task_scheduler.cpp"

struct NumberSubset {
	u64 id;

	u64 start;
	u64 end;

	u64 total;
};

void add_subset(TaskScheduler *scheduler, void *arg) {
	(void) scheduler;
	NumberSubset *subset = (NumberSubset *)arg;
	subset->total = 0;

	while (subset->start != subset->end) {
		subset->total += subset->start;
		++subset->start;
	}

	subset->total += subset->end;
}

void triangle_number_main_task(TaskScheduler *scheduler, void *arg) {
	MemoryArena arena = init_memory(32*MB);
	(void)arg;

	// Define the constants to test
	const u64 triangle_num = 4759324;
	const u64 num_additions_per_task = 10000;
	const u64 num_tasks = (triangle_num + num_additions_per_task - 1) / num_additions_per_task;

	// Create the tasks
	Task *tasks = PUSH_STRUCTS(arena, num_tasks, Task);

	// We have to declare this on the heap so other threads can access it
	NumberSubset *subsets = PUSH_STRUCTS(arena, num_tasks, NumberSubset);
	u64 next_number = 1;

	for (u64 i = 0; i < num_tasks; ++i) {
		NumberSubset *subset = &subsets[i];

		subset->id = i;

		subset->start = next_number;
		subset->end = next_number + num_additions_per_task - 1;
		if (subset->end > triangle_num) {
			subset->end = triangle_num;
		}

		tasks[i].Function = add_subset;
		tasks[i].ArgData = subset;

		next_number = subset->end + 1;
	}

	// Schedule the tasks and wait for them to complete
	printf("Add tasks\n");
	int job_handle = scheduler_add_tasks(*scheduler, num_tasks, tasks);

	printf("wait_for_job\n");
	scheduler_wait_for_job(*scheduler, job_handle, 0);

	printf("Done waiting\n");

	// Add the results
	u64 result = 0;
	for (u64 i = 0; i < num_tasks; ++i) {
		result += subsets[i].total;
	}

	printf("Hello (%llu) (%llu) (%llu)\n", triangle_num, triangle_num * (triangle_num + 1) / 2, result);
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	MemoryArena arena = init_memory(32*MB, true);
	TaskScheduler scheduler = {};
	scheduler_start(scheduler, arena, triangle_number_main_task);
}

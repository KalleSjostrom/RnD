



























































				__atomic_fetch_sub(&job.counter, 1, __ATOMIC_SEQ_CST);
				break;
				continue;
				continue;
				continue;
				free_fiber_cursor = 0;
				Job &job = scheduler->jobs[next_task.job_handle];
				next_task.task.Function(scheduler, next_task.task.ArgData);
				return i;
				waiting_fiber_index = i;
			*next_task = tls.bundle_storage[index];
			// And we're back
			// Found a waiting fiber
			// Found a waiting task that is ready to continue
			// Get a new task from the queue, and execute it
			// Switch
			// Test if it's ready
			// TODO(kalle): Double lock, why??!
			bool expected = true;
			continue;
			free_fiber_cursor++;
			if (!__atomic_load_n(&scheduler->waiting_fibers[i], __ATOMIC_ACQUIRE)) {
			if (!__atomic_load_n(&scheduler->waiting_fibers[i], __ATOMIC_RELAXED)) {
			if (__atomic_compare_exchange_n(&scheduler->waiting_fibers[i], &expected, false, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
			if (__atomic_load_n(&job.counter, __ATOMIC_RELAXED) != bundle.target) {
			if (__sync_bool_compare_and_swap(&free_fibers[free_fiber_cursor], true, false)) {
			if (free_fiber_cursor == FIBER_POOL_SIZE)
			if (scheduler->get_next_task(&next_task)) {
			Job &job = scheduler->jobs[bundle.job_handle];
			printf("Error: Failed to create all the worker threads");
			return true;
			return;
			return;
			scheduler->fibers[tls.previous_fiber_index].switch_to_fiber(&scheduler->fibers[tls.current_fiber_index]);
			TaskBundle next_task;
			thread_index_to_steal_from = 0;
			ThreadLocalStorage &tls = scheduler->tls_array[scheduler->get_current_thread_index()];
			tls.current_fiber_index = waiting_fiber_index;
			tls.last_free_bundle = 0;
			tls.last_successful_steal = i;
			tls.previous_fiber_destination = FiberDestination_ToPool;
			tls.previous_fiber_index = tls.current_fiber_index;
			WaitingBundle &bundle = scheduler->waiting_bundles[i];
			}
			}
			}
			}
			}
			}
		*next_task = tls.bundle_storage[index];
		// Check if any of the waiting tasks are ready
		// Clean up from the last fiber to run on this thread
		__atomic_store_n(&free_fibers[tls.previous_fiber_index], true, __ATOMIC_RELEASE);
		__atomic_store_n(&waiting_fibers[tls.previous_fiber_index], true, __ATOMIC_RELEASE);
		break;
		break;
		break;
		bundle.available = false;
		bundle.job_handle = job_handle;
		bundle.task = tasks[i];
		fibers[i].setup(512000, fiber_start, this);
		for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
		for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
		free_fibers[i] = true;
		if (!CreateThread(524288, thread_start, thread_args + i, (size_t)i, &threads[i])) {
		if (queue_steal(&other_tls.task_queue, index)) {
		if (thread_index_to_steal_from == current_thread_index) {
		if (thread_index_to_steal_from >= thread_count) {
		if (tls.bundle_storage[tls.last_free_bundle].available) {
		if (tls.last_free_bundle >= count) {
		if (waiting_fiber_index != FTL_INVALID_INDEX) {
		int waiting_fiber_index = FTL_INVALID_INDEX;
		JoinThread(threads[i]);
		queue_push(&tls.task_queue, tls.last_free_bundle);
		return true;
		return;
		scheduler->cleanup_old_fiber();
		TaskBundle &bundle = tls.bundle_storage[tls.last_free_bundle];
		thread_args[i].scheduler = this;
		thread_args[i].thread_index = i;
		thread_index_to_steal_from++;
		ThreadLocalStorage &other_tls = tls_array[thread_index_to_steal_from];
		tls.last_free_bundle++;
		tls.previous_fiber_destination = FiberDestination_None;
		tls.previous_fiber_destination = FiberDestination_None;
		tls.previous_fiber_index = FTL_INVALID_INDEX;
		tls.previous_fiber_index = FTL_INVALID_INDEX;
		update_last_free_bundle(tls);
		waiting_fibers[i] = false;
		}
		}
		}
		}
		}
		}
		}
		}
		}
		} else {
	//
	//
	//
	//
	//
	//
	//     currentFiber.switch_to_fiber(waitingFiber)
	//     fibers.Push(currentFiber)
	//     fibers.Push(currentFiber)
	//     helperFiber.switch_to_fiber(waitingFiber)
	//    a. Finish the task and return
	//    b. Wait on another counter
	// 1. When we're waiting for a counter, we pull a new fiber from the fiber pool and switch to it.
	// 2. When we found a counter that's ready, we put the current fiber back in the fiber pool, and switch to the waiting fiber.
	// A fiber from the pool will always either be completely new or just come back from switching to a waiting fiber
	// A waiting fiber can do two things:
	// AKA, we stored currentFiber and waitingFiber in TLS, and then switched to the helper fiber. The
	// And we're back
	// And we're back
	// Call the main task procedure
	// Case 1:
	// Case 2:
	// Clean up from the last Fiber to run on this thread
	// Clean up the old fiber
	// Cleanup and shutdown
	// Create the remaining threads
	// Explanation:
	// Fast out
	// Fill in the WaitingBundle data
	// Fill in tls
	// For example, let's say we discover a waiting fiber that's ready. We need to put the currently
	// Get a free fiber
	// Get a free fiber
	// Get a free fiber to switch to
	// helper fiber then did:
	// If we have 1 helper fiber per thread, we can guarantee that currentFiber is free to be executed by any thread
	// In case a, the while loop and if/else will again guarantee the code will call cleanup_old_fiber() before executing any other fiber.
	// In case b, WaitOnCounter will directly call cleanup_old_fiber(). Any further code is just a recursion.
	// In the previous implementation of TaskScheduler, we used helper fibers to do this work for us.
	// In the time between us adding the current fiber to the fiber pool and switching to the waiting fiber, another
	// In this specific implementation, the fiber pool and waiting list are flat arrays signaled by atomics
	// Initialize all the things
	// Initialize thread local storage
	// just do the equivalent of:
	// once it is added back to the fiber pool
	// Ours is empty, try to steal from the others'
	// Proof:
	// QED
	// QED
	// Repurpose it as the main task fiber and switch to it
	// Request that all the threads quit
	// running fiber back into the fiber pool, and then switch to the waiting fiber. However, we can't
	// Set the properties for the current thread
	// So in order to "Push" the fiber to the fiber pool or waiting list, we just set their corresponding atomics to true
	// Start the main task
	// Start the quit sequence
	// Switch
	// Switch to the thread fibers
	// Switch to the thread fibers
	// The code structure guarantees that between any two fiber switches, the code will always end up in WaitForCounter or FibeStart.
	// The while and the if/else in fiber_start will guarantee the code will call cleanup_old_fiber() before executing any other fiber.
	// There are only two places where we switch fibers:
	// Therefore, instead of using a helper fiber and immediately pushing the fiber to the fiber pool or waiting list,
	// This leads to stack corruption and/or other undefined behavior.
	// This solution works well, however, we actually don't need the helper fibers
	// thread could come along and pop the current fiber from the fiber pool and try to run it.
	// Try to pop from our own queue
	// Wait for the worker threads to finish
	// we defer the push until the next fiber gets to one of those two places
	// We should never get here
	// We should never get here
	// When switching between fibers, there's the innate problem of tracking the fibers.
	__atomic_store_n(&scheduler->quit, true, __ATOMIC_RELEASE);
	args->main_task(scheduler, args->data);
	args.data = main_task_arg;
	args.main_task = main_task;
	args.scheduler = this;
	bundle.job_handle = job_handle;
	bundle.target = target;
	case FiberDestination_None:
	case FiberDestination_ToPool:
	case FiberDestination_ToWaiting:
	cleanup_old_fiber();
	EndCurrentThread();
	Fiber &free_fiber = fibers[fiber_index];
	fibers[tls.previous_fiber_index].switch_to_fiber(&fibers[fiber_index]);
	for (int i = 0; i < count; ++i) {
	for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
	for (int i = 0; i < task_count; ++i) {
	for (int i = 0; i < thread_count; ++i) {
	for (int i = 1; i < thread_count; ++i) {
	for (int i = 1; i < thread_count; ++i) {
	for (int j = 0; 10; ++j) {
	free_fiber.reset(main_fiber_start, &args);
	FTL_THREAD_FUNC_END;
	if (__atomic_load_n(&job.counter, __ATOMIC_RELAXED) == target) {
	if (queue_take(&tls.task_queue, index)) {
	int __padding;
	int count = (int) ARRAY_COUNT(tls.bundle_storage);
	int current_thread_index = get_current_thread_index();
	int fiber_index = next_free_fiber_index();
	int fiber_index = next_free_fiber_index();
	int fiber_index = scheduler->next_free_fiber_index();
	int index = args->thread_index;
	int index;
	int job_handle = 0;
	int thread_index;
	int thread_index_to_steal_from = tls.last_successful_steal;
	Job &job = jobs[job_handle];
	Job &job = jobs[job_handle];
	job.counter = 0;
	MainFiberArgs *args = (MainFiberArgs *) main_fiber_args;
	MainFiberArgs args;
	printf("Error: fiber_start should never return");
	printf("Error: main_fiber_start should never return");
	printf("fiber_index %u\n", fiber_index); fflush(stdout);
	printf("next_free_fiber_index\n"); fflush(stdout);
	printf("No free fibers in the pool. Possible deadlock");
	printf("switched\n");
	printf("t1 %u\n", index); fflush(stdout);
	printf("t2 %u\n", index); fflush(stdout);
	printf("Trying to switch\n"); fflush(stdout);
	quit = false;
	return false;
	return job_handle;
	return;
	scheduler->fibers[tls.current_fiber_index].switch_to_fiber(&tls.thread_fiber);
	scheduler->fibers[tls.current_fiber_index].switch_to_fiber(&tls.thread_fiber);
	scheduler->tls_array[index].current_fiber_index = fiber_index;
	scheduler->tls_array[index].thread_fiber.switch_to_fiber(&scheduler->fibers[fiber_index]);
	SetCurrentThreadAffinity(1);
	switch (tls.previous_fiber_destination) {
	TaskFunction main_task;
	TaskScheduler *scheduler = (TaskScheduler *) task_scheduler;
	TaskScheduler *scheduler = args->scheduler;
	TaskScheduler *scheduler = args->scheduler;
	TaskScheduler *scheduler;
	TaskScheduler *scheduler;
	thread_count = thread_pool_size ? thread_pool_size : GetNumHardwareThreads();
	ThreadArgs *args = (ThreadArgs *) thread_args;
	ThreadArgs *thread_args = PUSH_STRUCTS(arena, thread_count, ThreadArgs);
	ThreadLocalStorage &tls = scheduler->tls_array[scheduler->get_current_thread_index()];
	ThreadLocalStorage &tls = scheduler->tls_array[scheduler->get_current_thread_index()];
	ThreadLocalStorage &tls = tls_array[current_thread_index];
	ThreadLocalStorage &tls = tls_array[get_current_thread_index()];
	ThreadLocalStorage &tls = tls_array[get_current_thread_index()];
	ThreadLocalStorage &tls = tls_array[get_current_thread_index()];
	threads = PUSH_STRUCTS(arena, thread_count, ThreadType);
	threads[0] = GetCurrentThread();
	tls.current_fiber_index = fiber_index;
	tls.previous_fiber_destination = FiberDestination_ToWaiting;
	tls.previous_fiber_index = tls.current_fiber_index;
	tls_array = PUSH_STRUCTS(arena, thread_count, ThreadLocalStorage);
	tls_array[0].current_fiber_index = fiber_index;
	tls_array[0].thread_fiber.switch_to_fiber(&free_fiber);
	void *data;
	WaitingBundle &bundle = waiting_bundles[tls.current_fiber_index];
	while (!__atomic_load_n(&scheduler->quit, __ATOMIC_ACQUIRE)) {
	}
	}
	}
	}
	}
	}
	}
	}
	}
	}
	}
#include "../utils/common.h"
#include "../utils/memory_arena.cpp"
#include "task_scheduler.h"
bool TaskScheduler::get_next_task(TaskBundle *next_task) {
int TaskScheduler::add_tasks(int task_count, Task *tasks) {
int TaskScheduler::next_free_fiber_index() {
static FTL_THREAD_FUNC_RETURN_TYPE thread_start(void *thread_args) {
static void fiber_start(void *task_scheduler) {
static void main_fiber_start(void *main_fiber_args) {
struct MainFiberArgs {
struct ThreadArgs {
void TaskScheduler::cleanup_old_fiber() {
void TaskScheduler::run(MemoryArena &arena, TaskFunction main_task, void *main_task_arg, int thread_pool_size) {
void TaskScheduler::update_last_free_bundle(ThreadLocalStorage &tls) {
void TaskScheduler::wait_for_job(int job_handle, int target) {
}
}
}
}
}
}
}
}
}
}
};
};
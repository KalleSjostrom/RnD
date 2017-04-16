#include "../utils/common.h"
#include "../utils/memory_arena.cpp"
#include "task_scheduler.h"

struct ThreadArgs {
	TaskScheduler *scheduler;
	int thread_index;
	int __padding;
};
struct MainFiberArgs {
	TaskFunction main_task;
	void *data;
	TaskScheduler *scheduler;
};
#define MAX_JOBS_PER_THREAD 256

namespace {
	// Gets the 0-based index of the current thread. This is useful for tls[GetCurrentThreadIndex()]
	int _current_thread_index(ThreadType *threads, int thread_count) {
		#if defined(FTL_WIN32_THREADS)
			DWORD threadId = GetCurrentThreadId();
			for (int i = 0; i < thread_count; ++i) {
				if (threads[i].Id == threadId) {
					return i;
				}
			}
		#elif defined(FTL_POSIX_THREADS)
			pthread_t currentThread = pthread_self();
			for (int i = 0; i < thread_count; ++i) {
				if (pthread_equal(currentThread, threads[i])) {
					return i;
				}
			}
		#endif

		return FTL_INVALID_INDEX;
	}

	// Gets the index of the next available fiber in the pool
	static int _next_free_fiber_index(TaskScheduler &scheduler) {
		int cursor = scheduler.free_fiber_cursor;
		for (int j = 0; 10; ++j) {
			for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
				cursor++;
				if (cursor == FIBER_POOL_SIZE)
					cursor = 0;

				if (__sync_bool_compare_and_swap(&scheduler.free_fibers[cursor], true, false)) {
	                // NOTE(kalle): The 'free_fiber_cursor' is a hint and we dont care enought to sync it between threads.
	                scheduler.free_fiber_cursor = cursor;
					return cursor;
				}
			}
		}
		printf("No free fibers in the pool. Possible deadlock");
	}

	// If necessary, moves the old fiber to the fiber pool or the waiting list. The old fiber is the last fiber to run on the thread before the current fiber
	static void _cleanup_old_fiber(TaskScheduler &scheduler) {
		// Clean up from the last Fiber to run on this thread
		//
		// Explanation:
		// When switching between fibers, there's the innate problem of tracking the fibers.
		// For example, let's say we discover a waiting fiber that's ready. We need to put the currently
		// running fiber back into the fiber pool, and then switch to the waiting fiber. However, we can't
		// just do the equivalent of:
		//     fibers.Push(currentFiber)
		//     currentFiber.switch_to_fiber(waitingFiber)
		// In the time between us adding the current fiber to the fiber pool and switching to the waiting fiber, another
		// thread could come along and pop the current fiber from the fiber pool and try to run it.
		// This leads to stack corruption and/or other undefined behavior.
		//
		// In the previous implementation of TaskScheduler, we used helper fibers to do this work for us.
		// AKA, we stored currentFiber and waitingFiber in TLS, and then switched to the helper fiber. The
		// helper fiber then did:
		//     fibers.Push(currentFiber)
		//     helperFiber.switch_to_fiber(waitingFiber)
		// If we have 1 helper fiber per thread, we can guarantee that currentFiber is free to be executed by any thread
		// once it is added back to the fiber pool
		//
		// This solution works well, however, we actually don't need the helper fibers
		// The code structure guarantees that between any two fiber switches, the code will always end up in WaitForCounter or FibeStart.
		// Therefore, instead of using a helper fiber and immediately pushing the fiber to the fiber pool or waiting list,
		// we defer the push until the next fiber gets to one of those two places
		//
		// Proof:
		// There are only two places where we switch fibers:
		// 1. When we're waiting for a counter, we pull a new fiber from the fiber pool and switch to it.
		// 2. When we found a counter that's ready, we put the current fiber back in the fiber pool, and switch to the waiting fiber.
		//
		// Case 1:
		// A fiber from the pool will always either be completely new or just come back from switching to a waiting fiber
		// The while and the if/else in _fiber_start will guarantee the code will call cleanup_old_fiber() before executing any other fiber.
		// QED
		//
		// Case 2:
		// A waiting fiber can do two things:
		//    a. Finish the task and return
		//    b. Wait on another counter
		// In case a, the while loop and if/else will again guarantee the code will call cleanup_old_fiber() before executing any other fiber.
		// In case b, WaitOnCounter will directly call cleanup_old_fiber(). Any further code is just a recursion.
		// QED

		// In this specific implementation, the fiber pool and waiting list are flat arrays signaled by atomics
		// So in order to "Push" the fiber to the fiber pool or waiting list, we just set their corresponding atomics to true
		ThreadLocalStorage &tls = scheduler.tls_array[_current_thread_index(scheduler.threads, scheduler.thread_count)];
		switch (tls.previous_fiber_destination) {
		case FiberDestination_ToPool:
			__atomic_store_n(&scheduler.free_fibers[tls.previous_fiber_index], true, __ATOMIC_RELEASE);
			tls.previous_fiber_destination = FiberDestination_None;
			tls.previous_fiber_index = FTL_INVALID_INDEX;
			break;
		case FiberDestination_ToWaiting:
			__atomic_store_n(&scheduler.waiting_fibers[tls.previous_fiber_index], true, __ATOMIC_RELEASE);
			tls.previous_fiber_destination = FiberDestination_None;
			tls.previous_fiber_index = FTL_INVALID_INDEX;
			break;
		case FiberDestination_None:
			break;
		}
	}

	static FTL_THREAD_FUNC_RETURN_TYPE _thread_start(void *thread_args) {
		ThreadArgs *args = (ThreadArgs *) thread_args;
		TaskScheduler *scheduler = args->scheduler;
		int index = args->thread_index;

		printf("t1 %u\n", index); fflush(stdout);

		// Get a free fiber to switch to
		int fiber_index = _next_free_fiber_index(*scheduler);

		// Initialize thread local storage
		scheduler->tls_array[index].current_fiber_index = fiber_index;
		fiber_switch(scheduler->tls_array[index].thread_fiber, scheduler->fibers[fiber_index]);

		printf("t2 %u\n", index); fflush(stdout);

		// Cleanup and shutdown
		EndCurrentThread();
		FTL_THREAD_FUNC_END;
	}
	static void _main_fiber_start(void *main_fiber_args) {
		MainFiberArgs *args = (MainFiberArgs *) main_fiber_args;
		TaskScheduler *scheduler = args->scheduler;

		// Call the main task procedure
		args->main_task(scheduler, args->data);

		// Request that all the threads quit
		__atomic_store_n(&scheduler->quit, true, __ATOMIC_RELEASE);

		// Switch to the thread fibers
		ThreadLocalStorage &tls = scheduler->tls_array[_current_thread_index(scheduler->threads, scheduler->thread_count)];
		fiber_switch(scheduler->fibers[tls.current_fiber_index], tls.thread_fiber);

		// We should never get here
		ASSERT(false, "Error: _main_fiber_start should never return");
	}

	// Pops the next task off the queue into nextTask. If there are no tasks in the the queue, it will return false.
	static bool _scheduler_get_next_task(TaskScheduler &scheduler, TaskBundle *next_task) {
		int current_thread_index = _current_thread_index(scheduler.threads, scheduler.thread_count);
		ThreadLocalStorage &tls = scheduler.tls_array[current_thread_index];

		// Try to pop from our own queue
		int index;
		if (queue_take(&tls.task_queue, index)) {
			*next_task = tls.bundle_storage[index];
			return true;
		}

		// Ours is empty, try to steal from the others'
		int thread_index_to_steal_from = tls.last_successful_steal;
		for (int i = 0; i < scheduler.thread_count; ++i) {
			if (thread_index_to_steal_from >= scheduler.thread_count) {
				thread_index_to_steal_from = 0;
			}

			if (thread_index_to_steal_from == current_thread_index) {
				continue;
			}

			ThreadLocalStorage &other_tls = scheduler.tls_array[thread_index_to_steal_from];
			if (queue_steal(&other_tls.task_queue, index)) {
				*next_task = other_tls.bundle_storage[index];
				tls.last_successful_steal = i;
				return true;
			}

			thread_index_to_steal_from++;
		}

		return false;
	}

	static void _fiber_start(void *task_scheduler) {
		TaskScheduler *scheduler = (TaskScheduler *) task_scheduler;

		while (!__atomic_load_n(&scheduler->quit, __ATOMIC_ACQUIRE)) {
			// Clean up from the last fiber to run on this thread
			_cleanup_old_fiber(*scheduler);

			// Check if any of the waiting tasks are ready
			int waiting_fiber_index = FTL_INVALID_INDEX;

			for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
				// TODO(kalle): Double lock, why??!
				if (!__atomic_load_n(&scheduler->waiting_fibers[i], __ATOMIC_RELAXED)) {
					continue;
				}

				if (!__atomic_load_n(&scheduler->waiting_fibers[i], __ATOMIC_ACQUIRE)) {
					continue;
				}

				// Found a waiting fiber
				// Test if it's ready
				WaitingBundle &bundle = scheduler->waiting_bundles[i];
				Job &job = scheduler->jobs[bundle.job_handle];
				if (__atomic_load_n(&job.counter, __ATOMIC_RELAXED) != bundle.target) {
					continue;
				}

				bool expected = true;
				if (__atomic_compare_exchange_n(&scheduler->waiting_fibers[i], &expected, false, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
					waiting_fiber_index = i;
					break;
				}
			}

			if (waiting_fiber_index != FTL_INVALID_INDEX) {
				// Found a waiting task that is ready to continue
				ThreadLocalStorage &tls = scheduler->tls_array[_current_thread_index(scheduler->threads, scheduler->thread_count)];

				tls.previous_fiber_index = tls.current_fiber_index;
				tls.current_fiber_index = waiting_fiber_index;
				tls.previous_fiber_destination = FiberDestination_ToPool;

				// Switch
				fiber_switch(scheduler->fibers[tls.previous_fiber_index], scheduler->fibers[tls.current_fiber_index]);

				// And we're back
			} else {
				// Get a new task from the queue, and execute it
				TaskBundle next_task;
				if (_scheduler_get_next_task(*scheduler, &next_task)) {
					next_task.task.Function(scheduler, next_task.task.ArgData);
					Job &job = scheduler->jobs[next_task.job_handle];
					__atomic_fetch_sub(&job.counter, 1, __ATOMIC_SEQ_CST);
				}
			}
		}

		// Start the quit sequence
		// Switch to the thread fibers
		ThreadLocalStorage &tls = scheduler->tls_array[_current_thread_index(scheduler->threads, scheduler->thread_count)];
		fiber_switch(scheduler->fibers[tls.current_fiber_index], tls.thread_fiber);

		// We should never get here
		printf("Error: _fiber_start should never return");
	}

	static void _update_last_free_bundle(ThreadLocalStorage &tls) {
		int count = (int) ARRAY_COUNT(tls.bundle_storage);
		for (int i = 0; i < count; ++i) {
			b32 available = !tls.bundle_storage[tls.last_free_bundle].occupied;
			if (available)
				return;

			tls.last_free_bundle++;
			if (tls.last_free_bundle >= count) {
				tls.last_free_bundle = 0;
			}
		}
	    ASSERT(false, "Bundle storage empty!");
	}

	static int _generate_job_handle(TaskScheduler &scheduler) {
		int thread_index = _current_thread_index(scheduler.threads, scheduler.thread_count);
		ThreadLocalStorage &tls = scheduler.tls_array[thread_index];

		// TODO(kalle):  Need to check if job slots are free to use!!
		int job_handle = thread_index * MAX_JOBS_PER_THREAD + tls.job_handle_counter++;

		if (tls.job_handle_counter == MAX_JOBS_PER_THREAD) {
			tls.job_handle_counter = 0;
		}

		return job_handle;
	}
}

/**
 * Initializes the TaskScheduler and then starts executing 'main_task'
 *
 * NOTE: Run will "block" until 'main_task' returns. However, it doesn't block in the traditional sense; 'main_task' is created as a Fiber.
 * Therefore, the current thread will save it's current state, and then switch execution to the the 'main_task' fiber. When 'main_task'
 * finishes, the thread will switch back to the saved state, and Run() will return.
 */
void scheduler_start(TaskScheduler &scheduler, MemoryArena &arena, TaskFunction main_task, void *main_task_arg = 0, int thread_pool_size = 0) {
	for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
		fiber_setup(scheduler.fibers[i], 512000, _fiber_start, &scheduler);
		scheduler.free_fibers[i] = true;
		scheduler.waiting_fibers[i] = false;
	}

	int thread_count = thread_pool_size ? thread_pool_size : GetNumHardwareThreads();
	scheduler.thread_count = thread_count;

	// Initialize all the things
	scheduler.quit = false;
	scheduler.threads = PUSH_STRUCTS(arena, scheduler.thread_count, ThreadType);
	ThreadArgs *thread_args = PUSH_STRUCTS(arena, thread_count, ThreadArgs);
	scheduler.tls_array = PUSH_STRUCTS(arena, thread_count, ThreadLocalStorage, 16, true);
	scheduler.jobs = PUSH_STRUCTS(arena, thread_count * MAX_JOBS_PER_THREAD, Job);

	for (int i = 0; i < thread_count; ++i) {
		ThreadLocalStorage &tls = scheduler.tls_array[i];
		tls.task_queue.array = PUSH_STRUCTS(arena, 256, int);
		tls.current_fiber_index = FTL_INVALID_INDEX;
		tls.previous_fiber_index = FTL_INVALID_INDEX;
	}

	// Set the properties for the current thread
	SetCurrentThreadAffinity(1);
	scheduler.threads[0] = GetCurrentThread();

	// Create the remaining threads
	for (int i = 1; i < thread_count; ++i) {
		thread_args[i].scheduler = &scheduler;
		thread_args[i].thread_index = i;

		if (!CreateThread(524288, _thread_start, thread_args + i, (size_t)i, &scheduler.threads[i])) {
			printf("Error: Failed to create all the worker threads");
			return;
		}
	}

	// Start the main task
	printf("next_free_fiber_index\n"); fflush(stdout);

	// Get a free fiber
	int fiber_index = _next_free_fiber_index(scheduler);
	printf("fiber_index %u\n", fiber_index); fflush(stdout);
	Fiber &free_fiber = scheduler.fibers[fiber_index];

	// Repurpose it as the main task fiber and switch to it
	MainFiberArgs args;
	args.scheduler = &scheduler;
	args.main_task = main_task;
	args.data = main_task_arg;

	fiber_reset(free_fiber, _main_fiber_start, &args);
	scheduler.tls_array[0].current_fiber_index = fiber_index;
	printf("Trying to switch\n"); fflush(stdout);
	fiber_switch(scheduler.tls_array[0].thread_fiber, free_fiber);
	printf("switched\n");

	// And we're back
	// Wait for the worker threads to finish
	for (int i = 1; i < thread_count; ++i) {
		JoinThread(scheduler.threads[i]);
	}

	return;
}

// Adds a group of tasks to the internal queue. Returns the index to the job containgin an atomic counter corresponding to the task group as a whole. Initially it will equal num_tasks. When each task completes, it will be decremented.
int scheduler_add_tasks(TaskScheduler &scheduler, int task_count, Task *tasks) {
	ThreadLocalStorage &tls = scheduler.tls_array[_current_thread_index(scheduler.threads, scheduler.thread_count)];
	int job_handle = _generate_job_handle(scheduler);
	Job &job = scheduler.jobs[job_handle];
	job.counter = task_count;
	for (int i = 0; i < task_count; ++i) {
		_update_last_free_bundle(tls);

		TaskBundle &bundle = tls.bundle_storage[tls.last_free_bundle];
		bundle.task = tasks[i];
		bundle.job_handle = job_handle;
		bundle.occupied = true;
		queue_push(&tls.task_queue, tls.last_free_bundle);
	}

	return job_handle;
}

// Yields execution to another task until job reaches target
void scheduler_wait_for_job(TaskScheduler &scheduler, int job_handle, int target) {
	Job &job = scheduler.jobs[job_handle];
	// Fast out
	if (__atomic_load_n(&job.counter, __ATOMIC_RELAXED) == target) {
		return;
	}

	ThreadLocalStorage &tls = scheduler.tls_array[_current_thread_index(scheduler.threads, scheduler.thread_count)];

	// Fill in the WaitingBundle data
	WaitingBundle &bundle = scheduler.waiting_bundles[tls.current_fiber_index];
	bundle.job_handle = job_handle;
	bundle.target = target;

	// Get a free fiber
	int fiber_index = _next_free_fiber_index(scheduler);

	// Clean up the old fiber
	_cleanup_old_fiber(scheduler);

	// Fill in tls
	tls.previous_fiber_index = tls.current_fiber_index;
	tls.current_fiber_index = fiber_index;
	tls.previous_fiber_destination = FiberDestination_ToWaiting;

	// Switch
	fiber_switch(scheduler.fibers[tls.previous_fiber_index], scheduler.fibers[fiber_index]);

	// And we're back
}

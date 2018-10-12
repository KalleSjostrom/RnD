#include "core/common.h"
#include "core/utils/assert.h"
#include "core/memory/arena_allocator.h"
#include "core/utils/jobs.h"
#include "core/utils/fiber_jobs.h"

#include <atomic>
#define ai32 std::atomic_int
#define abool std::atomic_bool

#define ATOMIC_RELAXED std::memory_order_relaxed
#define ATOMIC_CONSUME std::memory_order_consume
#define ATOMIC_ACQUIRE std::memory_order_acquire
#define ATOMIC_RELEASE std::memory_order_release
#define ATOMIC_ACQ_REL std::memory_order_acq_rel
#define ATOMIC_SEQ_CST std::memory_order_seq_cst

#define atomic_load_n(ptr, order) std::atomic_load_explicit(ptr, order)
#define atomic_store_n(ptr, value, order) std::atomic_store_explicit(ptr, value, order)
#define atomic_fetch_sub(ptr, value, order) std::atomic_fetch_sub_explicit(ptr, value, order)
#define atomic_thread_fence(order) std::atomic_thread_fence(order)
#define atomic_compare_exchange_weak_n(ptr, expected, desired, success_memorder, failure_memorder) std::atomic_compare_exchange_weak_explicit(ptr, expected, desired, success_memorder, failure_memorder)
#define atomic_compare_exchange_strong_n(ptr, expected, desired, success_memorder, failure_memorder) std::atomic_compare_exchange_strong_explicit(ptr, expected, desired, success_memorder, failure_memorder)

#include <process.h>

// Get the number of hardware threads. This should take Hyperthreading, etc. into account
__forceinline int get_num_hardware_threads() {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

/// Lock free queue
struct FJQueue {
	ai32 *array;
	ai32 bottom;
	ai32 top;
};

#define MAX_ARRAY_SIZE 1024
#define ARRAY_MASK (MAX_ARRAY_SIZE-1)

// These are crazy expensive operations! ~2000 cycles
// See if we can't optimize this!
bool _queue_take(FJQueue *q, int &value) {
	int b = atomic_load_n(&q->bottom, ATOMIC_RELAXED) - 1;
	// int *a = atomic_load_n(&q->array, ATOMIC_RELAXED); // Would need to load the array if using some realloc scheme.
	atomic_store_n(&q->bottom, b, ATOMIC_RELAXED);

	atomic_thread_fence(ATOMIC_SEQ_CST);

	int t = atomic_load_n(&q->top, ATOMIC_RELAXED);
	bool result = 1;
	if (t <= b) {
		// Non-empty queue.
		value = atomic_load_n(&q->array[b & ARRAY_MASK], ATOMIC_RELAXED);
		if (t == b) {
			/* Single last element in queue. */
			if (!atomic_compare_exchange_strong_n(&q->top, &t, t + 1, ATOMIC_SEQ_CST, ATOMIC_RELAXED)) {
				/* Failed race. */
				result = false;
			}
			atomic_store_n(&q->bottom, b + 1, ATOMIC_RELAXED);
		}
	} else { /* Empty queue. */
		result = false;
		atomic_store_n(&q->bottom, b + 1, ATOMIC_RELAXED);
	}
	return result;
}

void _queue_push(FJQueue *q, int x) {
	int b = atomic_load_n(&q->bottom, ATOMIC_RELAXED);
	int t = atomic_load_n(&q->top, ATOMIC_ACQUIRE);
	// int *a = atomic_load_n(&q->array, ATOMIC_RELAXED); // Would need to load the array if using some realloc scheme.
	if (b - t > MAX_ARRAY_SIZE - 1) { /* Full queue. */
		ASSERT(false, "FJQueue full!");
	}
	atomic_store_n(&q->array[b & ARRAY_MASK], x, ATOMIC_RELAXED);
	atomic_thread_fence(ATOMIC_RELEASE);
	atomic_store_n(&q->bottom, b + 1, ATOMIC_RELAXED);
}

bool _queue_steal(FJQueue *q, int &value) {
	int t = atomic_load_n(&q->top, ATOMIC_ACQUIRE);
	atomic_thread_fence(ATOMIC_SEQ_CST);
	int b = atomic_load_n(&q->bottom, ATOMIC_ACQUIRE);
	if (t < b) {
		// Non-empty queue.
		// int *a = atomic_load_n(&q->array, __ATOMIC_CONSUME);
		value = atomic_load_n(&q->array[t & ARRAY_MASK], ATOMIC_RELAXED);
		if (!atomic_compare_exchange_strong_n(&q->top, &t, t + 1, ATOMIC_SEQ_CST, ATOMIC_RELAXED)) {
			return false; // Failed race.
		}
		return true;
	}
	return false;
}

#define INVALID_INDEX 0x7fffffff
#define FIBER_POOL_SIZE 128 // Naughty dog uses 160, 128 with 64 kb stack and 32 with 512 kb stack
#define FIBER_STACK_SIZE 512000
#define MAX_JOBS_PER_THREAD 256
#define SLEEP_TIME_MS_WHEN_IDLE 10

// Holds a counter that is being waited on. Specifically, until counter == target_value
struct WaitingBundle {
	int job_handle;
	int target;
};

enum FiberDestination {
	FiberDestination_None = 0,

	FiberDestination_ToPool,
	FiberDestination_ToWaiting
};

// Holds a task that is ready to to be executed by the worker threads counter is the counter for the task(group). It will be decremented when the task completes
struct TaskBundle {
	Task task;
	int job_handle;
	bool occupied;
};

struct ThreadLocalStorage {
	TaskBundle bundle_storage[4096];
	// Boost fibers require that fibers created from threads finish on the same thread where they started.
	// To accommodate this, we have save the initial fibers created in each thread, and immediately switch out of them into the general fiber pool. Once the 'main_task' has finished, we signal all the threads to start quitting. When the receive the signal, they switch back to the thread_fiber, allowing it to safely clean up.
	void *thread_fiber;
	// The queue of waiting tasks
	FJQueue task_queue;
	// The index of the current fiber in fibers
	int current_fiber_index;
	// The index of the previously executed fiber in fibers
	int previous_fiber_index;
	// Where OldFiber should be stored when we call CleanUpPoolAndWaiting()
	FiberDestination previous_fiber_destination;
	// The last queue that we tried to _queue_steal from. This is an offset index from the current thread index
	int last_steal_attempt;
	int last_free_bundle;
	int job_handle_counter;
};

struct FiberJobManager {
	// TLS is not fiber safe on all platforms, so we fake thread local stoarge for each thread.
	ThreadLocalStorage *tls_array;
	ai32 quit;
	int free_fiber_cursor;
	int thread_count;

	// The backing storage for the fiber pool
	void *fibers[FIBER_POOL_SIZE];
	// An array of atomics, which signify if a fiber is available to be used. The indices of free_fibers correspond 1 to 1 with fibers. So, if free_fibers[i] == true, then fibers[i] can be used. Each atomic acts as a lock to ensure that threads do not try to use the same fiber at the same time
	abool free_fibers[FIBER_POOL_SIZE];
	// An array of atomics, which signify if a fiber is waiting for a counter. The indices of waiting_fibers correspond 1 to 1 with fibers. So, if waiting_fibers[i] == true, then fibers[i] is waiting for a counter
	abool waiting_fibers[FIBER_POOL_SIZE];
	// An array of WaitingBundles, which correspond 1 to 1 with waiting_fibers. If waiting_fibers[i] == true, waiting_bundles[i] will contain the data for the waiting fiber in fibers[i].
	WaitingBundle waiting_bundles[FIBER_POOL_SIZE];

	ai32 *jobs;
};

struct ThreadArgs {
	FiberJobManager *fjm;
	int thread_index;
};

static __declspec(thread) int tls_thread_index;

namespace {
	// Gets the index of the next available fiber in the pool
	static int _next_free_fiber_index(FiberJobManager *fjm) {
		int cursor = fjm->free_fiber_cursor;
		for (int j = 0; 10; ++j) {
			for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
				cursor++;
				if (cursor == FIBER_POOL_SIZE)
					cursor = 0;

				bool expected = true;
				if (atomic_compare_exchange_weak_n(&fjm->free_fibers[cursor], &expected, false, ATOMIC_RELEASE, ATOMIC_RELAXED)) {
					// NOTE(kalle): The 'free_fiber_cursor' is a hint and we dont care enought to sync it between threads.
					fjm->free_fiber_cursor = cursor;
					return cursor;
				}
			}
		}
		printf("No free fibers in the pool. Possible deadlock");
	}

	// If necessary, moves the old fiber to the fiber pool or the waiting list. The old fiber is the last fiber to run on the thread before the current fiber
	static void _cleanup_old_fiber(ThreadLocalStorage &tls, abool *free_fibers, abool *waiting_fibers) {
		// When switching between fibers, there's the innate problem of tracking the fibers. For example, let's say we discover a waiting fiber that's ready. We need to put the currently running fiber back into the fiber pool, and then switch to the waiting fiber. However, we can't just do the equivalent of:
		//	 fibers.Push(currentFiber)
		//	 currentFiber.switch_to_fiber(waitingFiber) In the time between us adding the current fiber to the fiber pool and switching to the waiting fiber, another thread could come along and pop the current fiber from the fiber pool and try to run it. This leads to stack corruption and/or other undefined behavior.
		//
		// In this specific implementation, the fiber pool and waiting list are flat arrays signaled by atomics. So in order to "Push" the fiber to the fiber pool or waiting list, we just set their corresponding atomics to true
		switch (tls.previous_fiber_destination) {
		case FiberDestination_ToPool:
			atomic_store_n(free_fibers + tls.previous_fiber_index, true, ATOMIC_RELEASE);
			tls.previous_fiber_destination = FiberDestination_None;
			tls.previous_fiber_index = INVALID_INDEX;
			break;
		case FiberDestination_ToWaiting:
			atomic_store_n(waiting_fibers + tls.previous_fiber_index, true, ATOMIC_RELEASE);
			tls.previous_fiber_destination = FiberDestination_None;
			tls.previous_fiber_index = INVALID_INDEX;
			break;
		case FiberDestination_None:
			break;
		}
	}

	static DWORD _thread_start(void *thread_args) {
		ThreadArgs *args = (ThreadArgs *) thread_args;
		FiberJobManager *fjm = args->fjm;
		int index = args->thread_index;
		tls_thread_index = index;

		ThreadLocalStorage &tls = fjm->tls_array[tls_thread_index];
		tls.thread_fiber = ConvertThreadToFiberEx(0, FIBER_FLAG_FLOAT_SWITCH);
		fjm->fibers[tls_thread_index] = tls.thread_fiber;
		tls.current_fiber_index = tls_thread_index;
		SwitchToFiber(tls.thread_fiber);

		ConvertFiberToThread();
		return 0;
	}

	// Pops the next task off the queue into nextTask. If there are no tasks in the queue, it will return false.
	static bool _scheduler_get_next_task(ThreadLocalStorage *tls_array, int thread_count, TaskBundle *next_task) {
		ThreadLocalStorage &tls = tls_array[tls_thread_index];

		// Try to pop from our own queue
		int index;
		if (_queue_take(&tls.task_queue, index)) {
			*next_task = tls.bundle_storage[index];
			return true;
		}

		// Ours is empty, try to _queue_steal from the others
		int thread_index_to_steal_from = tls.last_steal_attempt;
		for (int i = 0; i < thread_count; ++i) {
			if (thread_index_to_steal_from >= thread_count) {
				thread_index_to_steal_from = 0;
			}

			if (thread_index_to_steal_from == tls_thread_index) {
				thread_index_to_steal_from++;
				continue;
			}

			ThreadLocalStorage &other_tls = tls_array[thread_index_to_steal_from];
			if (_queue_steal(&other_tls.task_queue, index)) {
				*next_task = other_tls.bundle_storage[index];
				tls.last_steal_attempt = thread_index_to_steal_from;
				return true;
			}

			thread_index_to_steal_from++;
			tls.last_steal_attempt = thread_index_to_steal_from;
			break;
		}

		return false;
	}

	static void _fiber_start(void *arg) {
		FiberJobManager *fjm = (FiberJobManager *) arg;
		ThreadLocalStorage &tls = fjm->tls_array[tls_thread_index];

		while (!atomic_load_n(&fjm->quit, ATOMIC_ACQUIRE)) {
			// Clean up from the last fiber to run on this thread
			_cleanup_old_fiber(tls, fjm->free_fibers, fjm->waiting_fibers);

			// Check if any of the waiting tasks are ready
			int waiting_fiber_index = INVALID_INDEX;

			// u64 time = __rdtsc();
			for (int i = 0; i < FIBER_POOL_SIZE; ++i) {
				// TODO(kalle): Double lock, why??!
				if (!atomic_load_n(&fjm->waiting_fibers[i], ATOMIC_RELAXED)) {
					continue;
				}

				if (!atomic_load_n(&fjm->waiting_fibers[i], ATOMIC_ACQUIRE)) {
					continue;
				}

				// Found a waiting fiber. Test if it's ready
				WaitingBundle &bundle = fjm->waiting_bundles[i];
				ai32 *job = fjm->jobs + bundle.job_handle;
				if (atomic_load_n(job, ATOMIC_RELAXED) != bundle.target) {
					continue;
				}

				bool expected = true;
				if (atomic_compare_exchange_strong_n(&fjm->waiting_fibers[i], &expected, false, ATOMIC_RELEASE, ATOMIC_RELAXED)) {
					waiting_fiber_index = i;
					break;
				}
			}
			// printf("%zu\n", __rdtsc() - time);

			if (waiting_fiber_index != INVALID_INDEX) {
				// Found a waiting task that is ready to continue
				tls.previous_fiber_index = tls.current_fiber_index;
				tls.current_fiber_index = waiting_fiber_index;
				tls.previous_fiber_destination = FiberDestination_ToPool;

				SwitchToFiber(fjm->fibers[tls.current_fiber_index]);
			} else {
				// Get a new task from the queue, and execute it
				TaskBundle next_task;
				if (_scheduler_get_next_task(fjm->tls_array, fjm->thread_count, &next_task)) {
					next_task.task.function(next_task.task.argument);
					ai32 *job = fjm->jobs + next_task.job_handle;
					atomic_fetch_sub(job, 1, ATOMIC_SEQ_CST);
				} else {
					Sleep(SLEEP_TIME_MS_WHEN_IDLE); // I have no waiting fibers ad no new tasks, sleep a while
				}
			}
		}

		// Start the quit sequence
		// Switch to the thread fibers
		SwitchToFiber(tls.thread_fiber);

		// We should never get here
		printf("Error: _fiber_start should never return");
	}

	static void _update_last_free_bundle(ThreadLocalStorage &tls) {
		int count = (int) ARRAY_COUNT(tls.bundle_storage);
		for (int i = 0; i < count; ++i) {
			bool available = !tls.bundle_storage[tls.last_free_bundle].occupied;
			if (available)
				return;

			tls.last_free_bundle++;
			if (tls.last_free_bundle >= count) {
				tls.last_free_bundle = 0;
			}
		}
		ASSERT(false, "Bundle storage empty!");
	}

	static int _generate_job_handle(ThreadLocalStorage &tls) {
		// TODO(kalle):  Need to check if job slots are free to use!!
		int job_handle = tls_thread_index * MAX_JOBS_PER_THREAD + tls.job_handle_counter++;

		if (tls.job_handle_counter == MAX_JOBS_PER_THREAD) {
			tls.job_handle_counter = 0;
		}

		return job_handle;
	}
}

// Initializes the FiberJobManager and then starts executing 'main_task'
// NOTE: scheduler_start will "block" until 'main_task' returns. However, it doesn't block in the traditional sense; 'main_task' is created as a Fiber.
// Therefore, the current thread will save it's current state, and then switch execution to the the 'main_task' fiber. When 'main_task'
// finishes, the thread will switch back to the saved state, and scheduler_start() will return.
FiberJobsResult run_fibertask(MainTaskFunction main_task, void *user_data, int thread_pool_size) {
	ArenaAllocator arena = {}; // will get cleaned up in it's destructor
	FiberJobManager *fjm = (FiberJobManager *)allocate(&arena, sizeof(FiberJobManager), true, 4);

	int thread_count = thread_pool_size ? thread_pool_size : get_num_hardware_threads();
	fjm->thread_count = thread_count;

	// Initialize all the things
	fjm->quit = false;
	HANDLE *threads = (HANDLE*)allocate(&arena, thread_count * sizeof(HANDLE));
	ThreadArgs *thread_args = (ThreadArgs*)allocate(&arena, thread_count * sizeof(ThreadArgs));
	fjm->tls_array = (ThreadLocalStorage*)allocate(&arena, thread_count * sizeof(ThreadLocalStorage), true, 16);
	fjm->jobs = (ai32*)allocate(&arena, thread_count * MAX_JOBS_PER_THREAD * sizeof(ai32), true);

	for (int i = 0; i < thread_count; ++i) {
		ThreadLocalStorage &tls = fjm->tls_array[i];
		tls.task_queue.array = (ai32*)allocate(&arena, 256 * sizeof(ai32));
		tls.current_fiber_index = INVALID_INDEX;
		tls.previous_fiber_index = INVALID_INDEX;
	}

	tls_thread_index = 0;

	ThreadLocalStorage &tls = fjm->tls_array[tls_thread_index];
	threads[tls_thread_index] = GetCurrentThread();

	void *thread_fiber = ConvertThreadToFiberEx(0, FIBER_FLAG_FLOAT_SWITCH);
	if (!thread_fiber) {
		return FiberJobsResult_Failed;
	}
	fjm->fibers[tls_thread_index] = thread_fiber;
	tls.thread_fiber = thread_fiber;
	tls.current_fiber_index = tls_thread_index;

	for (int i = 0; i < thread_count; ++i) {
		fjm->free_fibers[i] = false;
		fjm->waiting_fibers[i] = false;
	}

	for (int i = thread_count; i < FIBER_POOL_SIZE; ++i) {
		fjm->fibers[i] = CreateFiber(0, _fiber_start, fjm);
		fjm->free_fibers[i] = true;
		fjm->waiting_fibers[i] = false;
	}

	// Create the remaining threads
	for (int i = 1; i < thread_count; ++i) {
		thread_args[i].fjm = fjm;
		thread_args[i].thread_index = i;

		// TODO(kalle): lock the thread to a core
		// How do we determine the size of the stacks? Variable sized stacks?
		threads[i] = CreateThread(0, 0, _thread_start, thread_args + i, 0, 0);
		if (!threads[i]) {
			return FiberJobsResult_Failed;
		}
	}

	main_task(user_data); // Call the main task procedure
	atomic_store_n(&fjm->quit, 1, ATOMIC_RELEASE); // Request that all the threads quit

	// Switch to the thread fibers
	SwitchToFiber(thread_fiber);

	// And we're back
	// Wait for the worker threads to finish
	for (int i = 1; i < thread_count; ++i) {
		WaitForSingleObject(threads[i], INFINITE);
	}

	ConvertFiberToThread();

	return FiberJobsResult_Completed;
}

// Adds a group of tasks to the internal queue. Returns the index to the job containgin an atomic counter corresponding to the task group as a whole. Initially it will equal num_tasks. When each task completes, it will be decremented.
FiberJobHandle add_tasks(FiberJobManager *fjm, int task_count, Task *tasks) {
	ThreadLocalStorage &tls = fjm->tls_array[tls_thread_index];
	FiberJobHandle job_handle = _generate_job_handle(tls);
	fjm->jobs[job_handle] = task_count;

	for (int i = 0; i < task_count; ++i) {
		_update_last_free_bundle(tls);

		TaskBundle &bundle = tls.bundle_storage[tls.last_free_bundle];
		bundle.task = tasks[i];
		bundle.job_handle = job_handle;
		bundle.occupied = true;
		_queue_push(&tls.task_queue, tls.last_free_bundle);
	}

	return job_handle;
}

// Yields execution to another task until job reaches target
void wait_for_job(FiberJobManager *fjm, int job_handle) {
	int target = 0; // Why would we ever want to wait until target is something else?

	ai32 *job = fjm->jobs + job_handle;
	if (atomic_load_n(job, ATOMIC_RELAXED) == target) {
		return;
	}

	ThreadLocalStorage &tls = fjm->tls_array[tls_thread_index];

	// Fill in the WaitingBundle data
	WaitingBundle &bundle = fjm->waiting_bundles[tls.current_fiber_index];
	bundle.job_handle = job_handle;
	bundle.target = target;

	// Get a free fiber
	int fiber_index = _next_free_fiber_index(fjm);

	// Clean up the old fiber
	_cleanup_old_fiber(tls, fjm->free_fibers, fjm->waiting_fibers);

	// Fill in tls
	tls.previous_fiber_index = tls.current_fiber_index;
	tls.current_fiber_index = fiber_index;
	tls.previous_fiber_destination = FiberDestination_ToWaiting;

	// Switch
	SwitchToFiber(fjm->fibers[fiber_index]);
}

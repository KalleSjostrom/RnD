#include "engine/utils/platform.h"
#include "engine/utils/threading/threading.cpp"

/// Includes for running with guard pages
#if defined(FIBER_STACK_GUARD_PAGES)
	#if defined(OS_LINUX) || defined(OS_MAC) || defined(iOS)
		#include <sys/mman.h>
	#elif defined(OS_WINDOWS)
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#endif
#else
	inline void protect_memory(void *, size_t) { }
	inline void unprotect_memory(void *, size_t) { }
#endif


/// Wrap os memory handling
#if defined(OS_WINDOWS)
	#if defined(FIBER_STACK_GUARD_PAGES)
		inline void protect_memory(void *memory, size_t bytes) {
			DWORD ignored;
			BOOL result = VirtualProtect(memory, bytes, PAGE_NOACCESS, &ignored);
			ASSERT(result, "Error in protect_memory");
		}

		inline void unprotect_memory(void *memory, size_t bytes) {
			DWORD ignored;
			BOOL result = VirtualProtect(memory, bytes, PAGE_READWRITE, &ignored);
			ASSERT(result, "Error in unprotect_memory");
		}
	#endif

	inline size_t get_page_size() {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwPageSize;
	}
	inline void *aligned_allocation(size_t size, size_t alignment) {
		return _aligned_malloc(size, alignment);
	}
	inline void aligned_free(void *block) {
		_aligned_free(block);
	}
#elif defined(OS_LINUX) || defined(OS_MAC) || defined(iOS)
	#if defined(FIBER_STACK_GUARD_PAGES)
		inline void protect_memory(void *memory, size_t bytes) {
			i32 result = mprotect(memory, bytes, PROT_NONE);
			ASSERT(!result, "Error in protect_memory");
		}

		inline void unprotect_memory(void *memory, size_t bytes) {
			i32 result = mprotect(memory, bytes, PROT_READ | PROT_WRITE);
			ASSERT(!result, "Error in unprotect_memory");
		}
	#endif

	inline size_t get_page_size() {
		i32 page_size = getpagesize();
		return (size_t)page_size;
	}
	inline void *aligned_allocation(size_t size, size_t alignment) {
		void *returnPtr;
		posix_memalign(&returnPtr, alignment, size);
		return returnPtr;
	}
	inline void aligned_free(void *block) {
		free(block);
	}
#endif

inline size_t round_up(size_t number, size_t multiple) {
	if (multiple == 0) {
		return number;
	}

	size_t remainder = number % multiple;
	if (remainder == 0)
		return number;

	return number + multiple - remainder;
}


/// Fibers
typedef void *FiberContext;

extern "C" void jump_fcontext(FiberContext *from, FiberContext to, void *arg);
// stack_pointer is the pointer to the _top_ of the stack (ie &stack_buffer[size]).
extern "C" FiberContext make_fcontext(void * stack_pointer, size_t size, void(*func)(void *));

typedef void (*FiberRoutine)(void *arg);

struct Fiber {
	void *stack;
	size_t system_page_size;
	size_t stack_size;
	FiberContext context;
	void *arg;
};

// Allocates a stack and sets it up to start executing 'routine' when first switched to
void setup_fiber(Fiber &fiber, size_t wanted_stack_size, FiberRoutine routine, void *arguments) {
	fiber.arg = arguments;
	#if defined(FIBER_STACK_GUARD_PAGES)
		fiber.system_page_size = get_page_size();
	#else
		fiber.system_page_size = 0;
	#endif

	fiber.stack_size = round_up(wanted_stack_size, fiber.system_page_size);
	// We add a guard page both the top and the bottom of the stack
	fiber.stack = aligned_allocation(fiber.system_page_size + fiber.stack_size + fiber.system_page_size, fiber.system_page_size);

	// Setup the assembly stack with make_x86_64_sysv_macho_gas.S
	// The stack grows "downwards" from high memory address to low, so set the start at the highest address.
	char *stack_start = ((char *)fiber.stack) + fiber.system_page_size + fiber.stack_size;
	fiber.context = make_fcontext(stack_start, fiber.stack_size, routine);

	#if defined(FIBER_STACK_GUARD_PAGES)
		protect_memory((char *)(fiber.stack), fiber.system_page_size);
		protect_memory((char *)(fiber.stack) + fiber.system_page_size + fiber.stack_size, fiber.system_page_size);
	#endif
}

// Saves the current stack context and then switches to the given fiber. Execution will resume here once another fiber switches to this fiber
void fiber_switch(Fiber &source, Fiber &dest) {
	jump_fcontext(&source.context, dest.context, dest.arg);
}

// Re-initializes the stack with a new routine and arg
void reset_fiber(Fiber &fiber, FiberRoutine routine, void *arguments) {
	fiber.context = make_fcontext(((char *)fiber.stack) + fiber.system_page_size + fiber.stack_size, fiber.stack_size, routine);
	fiber.arg = arguments;
}

void destroy_fiber(Fiber &fiber) {
	if (fiber.stack != NULL) {
		if (fiber.system_page_size != 0) {
			unprotect_memory((char *)(fiber.stack), fiber.system_page_size);
			unprotect_memory((char *)(fiber.stack) + fiber.system_page_size + fiber.stack_size, fiber.system_page_size);
		}

		aligned_free(fiber.stack);
	}
}


/// Lock free queue
namespace queue {
	struct Queue {
		i32 *array;
		i32 bottom;
		i32 top;
	};

	#define MAX_ARRAY_SIZE 1024
	#define ARRAY_MASK (MAX_ARRAY_SIZE-1)

	b32 take(Queue *q, i32 &value) {
		i32 b = __atomic_load_n(&q->bottom, __ATOMIC_RELAXED) - 1;
		i32 *a = __atomic_load_n(&q->array, __ATOMIC_RELAXED);
		__atomic_store_n(&q->bottom, b, __ATOMIC_RELAXED);

		__atomic_thread_fence(__ATOMIC_SEQ_CST);

		i32 t = __atomic_load_n(&q->top, __ATOMIC_RELAXED);
		b32 result = 1;
		if (t <= b) {
			// Non-empty queue.
			value = __atomic_load_n(&a[b & ARRAY_MASK], __ATOMIC_RELAXED);
			if (t == b) {
				/* Single last element in queue. */
				if (!__atomic_compare_exchange_n(&q->top, &t, t + 1, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
					/* Failed race. */
					result = false;
				}
				__atomic_store_n(&q->bottom, b + 1, __ATOMIC_RELAXED);
			}
		} else { /* Empty queue. */
			result = false;
			__atomic_store_n(&q->bottom, b + 1, __ATOMIC_RELAXED);
		}
		return result;
	}

	void push(Queue *q, i32 x) {
		i32 b = __atomic_load_n(&q->bottom, __ATOMIC_RELAXED);
		i32 t = __atomic_load_n(&q->top, __ATOMIC_ACQUIRE);
		i32 *a = __atomic_load_n(&q->array, __ATOMIC_RELAXED);
		if (b - t > MAX_ARRAY_SIZE - 1) { /* Full queue. */
			ASSERT(false, "Queue full!");
		}
		__atomic_store_n(&a[b & ARRAY_MASK], x, __ATOMIC_RELAXED);
		__atomic_thread_fence(__ATOMIC_RELEASE);
		__atomic_store_n(&q->bottom, b + 1, __ATOMIC_RELAXED);
	}

	b32 steal(Queue *q, i32 &value) {
		i32 t = __atomic_load_n(&q->top, __ATOMIC_ACQUIRE);
		__atomic_thread_fence(__ATOMIC_SEQ_CST);
		i32 b = __atomic_load_n(&q->bottom, __ATOMIC_ACQUIRE);
		if (t < b) {
			// Non-empty queue.
			i32 *a = __atomic_load_n(&q->array, __ATOMIC_CONSUME);
			value = __atomic_load_n(&a[t & ARRAY_MASK], __ATOMIC_RELAXED);
			if (!__atomic_compare_exchange_n(&q->top, &t, t + 1, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
				return false; // Failed race.
			}
	        return true;
		}
		return false;
	}
}


/// Task scheduler
struct TaskScheduler;
typedef void(*TaskFunction)(TaskScheduler *taskScheduler, void *arg);
struct Task {
	TaskFunction function;
	void *argument;
};

#define INVALID_INDEX 0x7fffffff
#define FIBER_POOL_SIZE 256 // Naughty dog uses 160, 128 with 64 kb stack and 32 with 512 kb stack
#define THREAD_STACK_SIZE 524288
#define FIBER_STACK_SIZE 512000
#define MAX_JOBS_PER_THREAD 256

// Holds a counter that is being waited on. Specifically, until counter == target_value
struct WaitingBundle {
	i32 job_handle;
	i32 target;
};

enum FiberDestination {
	FiberDestination_None = 0,

	FiberDestination_ToPool,
	FiberDestination_ToWaiting
};

// Holds a task that is ready to to be executed by the worker threads counter is the counter for the task(group). It will be decremented when the task completes
struct TaskBundle {
	Task task;
	i32 job_handle;
	b32 occupied;
};

struct ThreadLocalStorage {
	TaskBundle bundle_storage[4096];
	// Boost fibers require that fibers created from threads finish on the same thread where they started.
	// To accommodate this, we have save the initial fibers created in each thread, and immediately switch out of them into the general fiber pool. Once the 'main_task' has finished, we signal all the threads to start quitting. When the receive the signal, they switch back to the thread_fiber, allowing it to safely clean up.
	Fiber thread_fiber;
	// The queue of waiting tasks
	queue::Queue task_queue;
	// The index of the current fiber in fibers
	i32 current_fiber_index;
	// The index of the previously executed fiber in fibers
	i32 previous_fiber_index;
	// Where OldFiber should be stored when we call CleanUpPoolAndWaiting()
	FiberDestination previous_fiber_destination;
	// The last queue that we successfully stole from. This is an offset index from the current thread index
	i32 last_successful_steal;
	i32 last_free_bundle;
	i32 job_handle_counter;
};

struct TaskScheduler {
	// TLS is not fiber safe on all platforms, so we fake thread local stoarge for each thread.
	ThreadLocalStorage *tls_array;
	b32 quit;
	i32 free_fiber_cursor;
	i32 thread_count;
	i32 __padding;

	// The backing storage for the fiber pool
	Fiber fibers[FIBER_POOL_SIZE];
	// An array of atomics, which signify if a fiber is available to be used. The indices of free_fibers correspond 1 to 1 with fibers. So, if free_fibers[i] == true, then fibers[i] can be used. Each atomic acts as a lock to ensure that threads do not try to use the same fiber at the same time
	bool free_fibers[FIBER_POOL_SIZE];
	// An array of atomics, which signify if a fiber is waiting for a counter. The indices of waiting_fibers correspond 1 to 1 with fibers. So, if waiting_fibers[i] == true, then fibers[i] is waiting for a counter
	bool waiting_fibers[FIBER_POOL_SIZE];
	// An array of WaitingBundles, which correspond 1 to 1 with waiting_fibers. If waiting_fibers[i] == true, waiting_bundles[i] will contain the data for the waiting fiber in fibers[i].
	WaitingBundle waiting_bundles[FIBER_POOL_SIZE];

	i32 *jobs;
};

struct ThreadArgs {
	TaskScheduler *scheduler;
	i32 thread_index;
	i32 __padding;
};
struct MainFiberArgs {
	TaskFunction main_task;
	void *data;
	TaskScheduler *scheduler;
};

#if defined(OS_WINDOWS)
	static __thread i32 tls_thread_index;
	static i32 get_thread_index() {
		return tls_thread_index;
	}
	static void set_thread_index(i32 index) {
		tls_thread_index = index;
	}
#else
	static __thread i32 tls_thread_index;

	// To avoid the problem of tls optimization under clang, we lookup the current thread index...
	static i32 tls_hax_thread_count;
	static ThreadType *tls_hax_threads;

	static i32 get_thread_index() {
		pthread_t current_thread = pthread_self();
		for (i32 i = 0; i < tls_hax_thread_count; ++i) {
			if (pthread_equal(current_thread, tls_hax_threads[i])) {
				ASSERT(i == tls_thread_index, "TLS optimization?");
				return i;
			}
		}

		// printf("%d\n", tls_hax_thread_count);
		// printf("%lu\n", (uintptr_t) current_thread);
		// for (i32 i = 0; i < tls_hax_thread_count; ++i) {
		// 	printf("-- %lu\n", (uintptr_t) tls_hax_threads[i]);
		// }

		ASSERT(false, "Could not find an entry in threads for the current running thread!");
		return -1;
	}
	static void set_thread_index(i32 index) {
		tls_thread_index = index;
	}
#endif

namespace {
	// Gets the index of the next available fiber in the pool
	static i32 _next_free_fiber_index(TaskScheduler &scheduler) {
		i32 cursor = scheduler.free_fiber_cursor;
		for (i32 j = 0; 10; ++j) {
			for (i32 i = 0; i < FIBER_POOL_SIZE; ++i) {
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
	static void _cleanup_old_fiber(ThreadLocalStorage &tls, bool *free_fibers, bool *waiting_fibers) {
		// When switching between fibers, there's the innate problem of tracking the fibers. For example, let's say we discover a waiting fiber that's ready. We need to put the currently running fiber back into the fiber pool, and then switch to the waiting fiber. However, we can't just do the equivalent of:
		//     fibers.Push(currentFiber)
		//     currentFiber.switch_to_fiber(waitingFiber) In the time between us adding the current fiber to the fiber pool and switching to the waiting fiber, another thread could come along and pop the current fiber from the fiber pool and try to run it. This leads to stack corruption and/or other undefined behavior.
		//
		// In this specific implementation, the fiber pool and waiting list are flat arrays signaled by atomics. So in order to "Push" the fiber to the fiber pool or waiting list, we just set their corresponding atomics to true
		switch (tls.previous_fiber_destination) {
		case FiberDestination_ToPool:
			__atomic_store_n(free_fibers + tls.previous_fiber_index, true, __ATOMIC_RELEASE);
			tls.previous_fiber_destination = FiberDestination_None;
			tls.previous_fiber_index = INVALID_INDEX;
			break;
		case FiberDestination_ToWaiting:
			__atomic_store_n(waiting_fibers + tls.previous_fiber_index, true, __ATOMIC_RELEASE);
			tls.previous_fiber_destination = FiberDestination_None;
			tls.previous_fiber_index = INVALID_INDEX;
			break;
		case FiberDestination_None:
			break;
		}
	}

	static THREAD_FUNC_RETURN_TYPE _thread_start(void *thread_args) {
		ThreadArgs *args = (ThreadArgs *) thread_args;
		TaskScheduler *scheduler = args->scheduler;
		i32 index = args->thread_index;
		set_thread_index(index);

		pthread_t current_thread = pthread_self();
		while (!pthread_equal(current_thread, tls_hax_threads[index])) {};

		// Get a free fiber to switch to
		i32 fiber_index = _next_free_fiber_index(*scheduler);

		// Initialize thread local storage
		scheduler->tls_array[index].current_fiber_index = fiber_index;
		fiber_switch(scheduler->tls_array[index].thread_fiber, scheduler->fibers[fiber_index]);

		// Cleanup and shutdown
		exit_thread();

		return 0;
	}
	static void _main_fiber_start(void *main_fiber_args) {
		MainFiberArgs *args = (MainFiberArgs *) main_fiber_args;
		TaskScheduler *scheduler = args->scheduler;

		// Call the main task procedure
		args->main_task(scheduler, args->data);

		// Request that all the threads quit
		__atomic_store_n(&scheduler->quit, true, __ATOMIC_RELEASE);

		// Switch to the thread fibers
		ThreadLocalStorage &tls = scheduler->tls_array[get_thread_index()];
		fiber_switch(scheduler->fibers[tls.current_fiber_index], tls.thread_fiber);

		// We should never get here
		ASSERT(false, "Error: _main_fiber_start should never return");
	}

	// Pops the next task off the queue into nextTask. If there are no tasks in the the queue, it will return false.
	static bool _scheduler_get_next_task(ThreadLocalStorage *tls_array, i32 thread_count, TaskBundle *next_task) {
		ThreadLocalStorage &tls = tls_array[get_thread_index()];

		// Try to pop from our own queue
		i32 index;
		if (queue::take(&tls.task_queue, index)) {
			*next_task = tls.bundle_storage[index];
			return true;
		}

		// Ours is empty, try to steal from the others'
		i32 thread_index_to_steal_from = tls.last_successful_steal;
		for (i32 i = 0; i < thread_count; ++i) {
			if (thread_index_to_steal_from >= thread_count) {
				thread_index_to_steal_from = 0;
			}

			if (thread_index_to_steal_from == get_thread_index()) {
				continue;
			}

			ThreadLocalStorage &other_tls = tls_array[thread_index_to_steal_from];
			if (queue::steal(&other_tls.task_queue, index)) {
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

		ThreadLocalStorage &tls = scheduler->tls_array[get_thread_index()];

		while (!__atomic_load_n(&scheduler->quit, __ATOMIC_ACQUIRE)) {
			// Clean up from the last fiber to run on this thread
			_cleanup_old_fiber(tls, scheduler->free_fibers, scheduler->waiting_fibers);

			// Check if any of the waiting tasks are ready
			i32 waiting_fiber_index = INVALID_INDEX;

			for (i32 i = 0; i < FIBER_POOL_SIZE; ++i) {
				// TODO(kalle): Double lock, why??!
				if (!__atomic_load_n(&scheduler->waiting_fibers[i], __ATOMIC_RELAXED)) {
					continue;
				}

				if (!__atomic_load_n(&scheduler->waiting_fibers[i], __ATOMIC_ACQUIRE)) {
					continue;
				}

				// Found a waiting fiber. Test if it's ready
				WaitingBundle &bundle = scheduler->waiting_bundles[i];
				i32 *job = scheduler->jobs + bundle.job_handle;
				if (__atomic_load_n(job, __ATOMIC_RELAXED) != bundle.target) {
					continue;
				}

				bool expected = true;
				if (__atomic_compare_exchange_n(&scheduler->waiting_fibers[i], &expected, false, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
					waiting_fiber_index = i;
					break;
				}
			}

			if (waiting_fiber_index != INVALID_INDEX) {
				// Found a waiting task that is ready to continue
				tls.previous_fiber_index = tls.current_fiber_index;
				tls.current_fiber_index = waiting_fiber_index;
				tls.previous_fiber_destination = FiberDestination_ToPool;

				fiber_switch(scheduler->fibers[tls.previous_fiber_index], scheduler->fibers[tls.current_fiber_index]);
			} else {
				// Get a new task from the queue, and execute it
				TaskBundle next_task;
				if (_scheduler_get_next_task(scheduler->tls_array, scheduler->thread_count, &next_task)) {
					next_task.task.function(scheduler, next_task.task.argument);
					i32 *job = scheduler->jobs + next_task.job_handle;
					__atomic_fetch_sub(job, 1, __ATOMIC_SEQ_CST);
				}
			}
		}

		// Start the quit sequence
		// Switch to the thread fibers
		fiber_switch(scheduler->fibers[tls.current_fiber_index], tls.thread_fiber);

		// We should never get here
		printf("Error: _fiber_start should never return");
	}

	static void _update_last_free_bundle(ThreadLocalStorage &tls) {
		i32 count = (i32) ARRAY_COUNT(tls.bundle_storage);
		for (i32 i = 0; i < count; ++i) {
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

	static i32 _generate_job_handle(ThreadLocalStorage &tls) {
		// TODO(kalle):  Need to check if job slots are free to use!!
		i32 job_handle = get_thread_index() * MAX_JOBS_PER_THREAD + tls.job_handle_counter++;

		if (tls.job_handle_counter == MAX_JOBS_PER_THREAD) {
			tls.job_handle_counter = 0;
		}

		return job_handle;
	}
}

// Initializes the TaskScheduler and then starts executing 'main_task'
// NOTE: scheduler_start will "block" until 'main_task' returns. However, it doesn't block in the traditional sense; 'main_task' is created as a Fiber.
// Therefore, the current thread will save it's current state, and then switch execution to the the 'main_task' fiber. When 'main_task'
// finishes, the thread will switch back to the saved state, and scheduler_start() will return.
void scheduler_start(TaskScheduler &scheduler, MemoryArena &arena, TaskFunction main_task, void *main_task_arg = 0, i32 thread_pool_size = 0) {
	for (i32 i = 0; i < FIBER_POOL_SIZE; ++i) {
		setup_fiber(scheduler.fibers[i], FIBER_STACK_SIZE, _fiber_start, &scheduler);
		scheduler.free_fibers[i] = true;
		scheduler.waiting_fibers[i] = false;
	}

	i32 thread_count = thread_pool_size ? thread_pool_size : get_num_hardware_threads();
	scheduler.thread_count = thread_count;

	// Initialize all the things
	scheduler.quit = false;
	ThreadType *threads = PUSH_STRUCTS(arena, thread_count, ThreadType);
	ThreadArgs *thread_args = PUSH_STRUCTS(arena, thread_count, ThreadArgs);
	scheduler.tls_array = PUSH_STRUCTS(arena, thread_count, ThreadLocalStorage, 16, true);
	scheduler.jobs = PUSH_STRUCTS(arena, thread_count * MAX_JOBS_PER_THREAD, i32, true);

	for (i32 i = 0; i < thread_count; ++i) {
		ThreadLocalStorage &tls = scheduler.tls_array[i];
		tls.task_queue.array = PUSH_STRUCTS(arena, 256, i32);
		tls.current_fiber_index = INVALID_INDEX;
		tls.previous_fiber_index = INVALID_INDEX;
	}

	set_thread_index(0);
	threads[0] = get_current_thread();

#ifndef OS_WINDOWS
	tls_hax_threads = threads;
	tls_hax_thread_count = thread_count;
#endif

	// Create the remaining threads
	for (i32 i = 1; i < thread_count; ++i) {
		thread_args[i].scheduler = &scheduler;
		thread_args[i].thread_index = i;

		// TODO(kalle): lock the thread to a core
		// How do we determine the size of the stacks? Variable sized stacks?
		if (!create_thread(_thread_start, thread_args + i, &threads[i], THREAD_STACK_SIZE)) {
			printf("Error: Failed to create all the worker threads");
			return;
		}
	}

	// Get a free fiber
	i32 fiber_index = _next_free_fiber_index(scheduler);
	Fiber &free_fiber = scheduler.fibers[fiber_index];

	// Repurpose it as the main task fiber and switch to it
	MainFiberArgs args;
	args.scheduler = &scheduler;
	args.main_task = main_task;
	args.data = main_task_arg;

	reset_fiber(free_fiber, _main_fiber_start, &args);
	scheduler.tls_array[0].current_fiber_index = fiber_index;
	fiber_switch(scheduler.tls_array[0].thread_fiber, free_fiber);

	// And we're back
	// Wait for the worker threads to finish
	for (i32 i = 1; i < thread_count; ++i) {
		join_thread(threads[i]);
	}

	return;
}

// Adds a group of tasks to the internal queue. Returns the index to the job containgin an atomic counter corresponding to the task group as a whole. Initially it will equal num_tasks. When each task completes, it will be decremented.
i32 scheduler_add_tasks(TaskScheduler &scheduler, i32 task_count, Task *tasks) {
	ThreadLocalStorage &tls = scheduler.tls_array[get_thread_index()];
	i32 job_handle = _generate_job_handle(tls);
	scheduler.jobs[job_handle] = task_count;

	for (i32 i = 0; i < task_count; ++i) {
		_update_last_free_bundle(tls);

		TaskBundle &bundle = tls.bundle_storage[tls.last_free_bundle];
		bundle.task = tasks[i];
		bundle.job_handle = job_handle;
		bundle.occupied = true;
		queue::push(&tls.task_queue, tls.last_free_bundle);
	}

	return job_handle;
}

// Yields execution to another task until job reaches target
void scheduler_wait_for_job(TaskScheduler &scheduler, i32 job_handle, i32 target) {
	i32 *job = scheduler.jobs + job_handle;
	if (__atomic_load_n(job, __ATOMIC_RELAXED) == target) {
		return;
	}

	ThreadLocalStorage &tls = scheduler.tls_array[get_thread_index()];

	// Fill in the WaitingBundle data
	WaitingBundle &bundle = scheduler.waiting_bundles[tls.current_fiber_index];
	bundle.job_handle = job_handle;
	bundle.target = target;

	// Get a free fiber
	i32 fiber_index = _next_free_fiber_index(scheduler);

	// Clean up the old fiber
	_cleanup_old_fiber(tls, scheduler.free_fibers, scheduler.waiting_fibers);

	// Fill in tls
	tls.previous_fiber_index = tls.current_fiber_index;
	tls.current_fiber_index = fiber_index;
	tls.previous_fiber_destination = FiberDestination_ToWaiting;

	// Switch
	fiber_switch(scheduler.fibers[tls.previous_fiber_index], scheduler.fibers[fiber_index]);
}

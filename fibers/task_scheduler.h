#include "typedefs.h"
#include "config.h"
#include "thread_abstraction.h"
#include "fiber.h"

struct TaskScheduler;
typedef void(*TaskFunction)(TaskScheduler *taskScheduler, void *arg);
struct Task {
	TaskFunction Function;
	void *ArgData;
};

#define FTL_INVALID_INDEX 0x7fffffff

struct Job {
	int counter;
};

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
	b32 occupied;
};

#include "wait_free_queue.h"

struct ThreadLocalStorage {
	ThreadLocalStorage()
		: thread_fiber(),
		  task_queue(),
		  current_fiber_index(FTL_INVALID_INDEX),
		  previous_fiber_index(FTL_INVALID_INDEX),
		  previous_fiber_destination(FiberDestination_None),
		  last_successful_steal(1) {
	}

	/**
	* Boost fibers require that fibers created from threads finish on the same thread where they started
	*
	* To accommodate this, we have save the initial fibers created in each thread, and immediately switch
	* out of them into the general fiber pool. Once the 'main_task' has finished, we signal all the threads to
	* start quitting. When the receive the signal, they switch back to the thread_fiber, allowing it to
	* safely clean up.
	*/
	Fiber thread_fiber;

	TaskBundle bundle_storage[4096];

	// The queue of waiting tasks
	WaitFreeQueue task_queue;
	// The index of the current fiber in fibers
	int current_fiber_index;
	// The index of the previously executed fiber in fibers
	int previous_fiber_index;
	// Where OldFiber should be stored when we call CleanUpPoolAndWaiting()
	FiberDestination previous_fiber_destination;
	// The last queue that we successfully stole from. This is an offset index from the current thread index
	int last_successful_steal;
	int last_free_bundle;
	int job_handle_counter;
};

#define FIBER_POOL_SIZE 256

struct TaskScheduler {
	ThreadType *threads;

	/**
	 * c++ Thread Local Storage is, by definition, static/global. This poses some problems, such as multiple TaskScheduler
	 * instances. In addition, with Boost::Context, we have no way of telling the compiler to disable TLS optimizations, so we
	 * have to fake TLS anyhow.
	 *
	 * During initialization of the TaskScheduler, we create one ThreadLocalStorage instance per thread. Threads index into
	 * their storage using tls[GetCurrentThreadIndex()]
	 */
	ThreadLocalStorage *tls_array;

	// size of both threads and tls_array
	int thread_count;
	b32 quit;

	// The backing storage for the fiber pool
	Fiber fibers[FIBER_POOL_SIZE];
	// An array of atomics, which signify if a fiber is available to be used. The indices of free_fibers correspond 1 to 1 with fibers. So, if free_fibers[i] == true, then fibers[i] can be used. Each atomic acts as a lock to ensure that threads do not try to use the same fiber at the same time
	bool free_fibers[FIBER_POOL_SIZE];
	// An array of atomics, which signify if a fiber is waiting for a counter. The indices of waiting_fibers correspond 1 to 1 with fibers. So, if waiting_fibers[i] == true, then fibers[i] is waiting for a counter
	bool waiting_fibers[FIBER_POOL_SIZE];
	// An array of WaitingBundles, which correspond 1 to 1 with waiting_fibers. If waiting_fibers[i] == true, waiting_bundles[i] will contain the data for the waiting fiber in fibers[i].
	WaitingBundle waiting_bundles[FIBER_POOL_SIZE];

	Job *jobs;

	int free_fiber_cursor;
	int __padding;

	void update_last_free_bundle(ThreadLocalStorage &tls);

	/**
	 * Initializes the TaskScheduler and then starts executing 'main_task'
	 *
	 * NOTE: Run will "block" until 'main_task' returns. However, it doesn't block in the traditional sense; 'main_task' is created as a Fiber.
	 * Therefore, the current thread will save it's current state, and then switch execution to the the 'main_task' fiber. When 'main_task'
	 * finishes, the thread will switch back to the saved state, and Run() will return.
	 */
	void run(MemoryArena &arena, TaskFunction main_task, void *main_task_arg = nullptr, int thread_pool_size = 0);

	int generate_job_handle();

	// Adds a group of tasks to the internal queue. Returns the index to the job containgin an atomic counter corresponding to the task group as a whole. Initially it will equal num_tasks. When each task completes, it will be decremented.
	int add_tasks(int num_tasks, Task *tasks);

	// Yields execution to another task until job reaches target
	void wait_for_job(int job_handle, int target);

	// Gets the 0-based index of the current thread. This is useful for tls[GetCurrentThreadIndex()]
	int get_current_thread_index() {
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

	// Pops the next task off the queue into nextTask. If there are no tasks in the the queue, it will return false.
	bool get_next_task(TaskBundle *next_task);

	// Gets the index of the next available fiber in the pool
	int next_free_fiber_index();

	// If necessary, moves the old fiber to the fiber pool or the waiting list. The old fiber is the last fiber to run on the thread before the current fiber
	void cleanup_old_fiber();
};

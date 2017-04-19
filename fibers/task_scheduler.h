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

	TaskBundle bundle_storage[4096];

	/**
	* Boost fibers require that fibers created from threads finish on the same thread where they started
	*
	* To accommodate this, we have save the initial fibers created in each thread, and immediately switch
	* out of them into the general fiber pool. Once the 'main_task' has finished, we signal all the threads to
	* start quitting. When the receive the signal, they switch back to the thread_fiber, allowing it to
	* safely clean up.
	*/
	Fiber thread_fiber;

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
};

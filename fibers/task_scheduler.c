#pragma warning(disable : 4255)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4189) // local variable is initialized but not referenced
#pragma warning(disable : 4820) // 'x' bytes padding added after data member 'name'

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <process.h>

#define OS_WINDOWS
#include "threading.c"

#define MAX_WORKER_THREADS 32
#define MAX_FIBER_COUNT 128 // Naughty dog uses 160, 128 with 64 kb stack and 32 with 512 kb stack
#define FIBER_STACK_SIZE 64000
#define TASK_BITS 9

#define TASK_MAX (1 << TASK_BITS)
#define JOB_GENERATION_MASK (0xffffffff << TASK_BITS)
#define JOB_INDEX_MASK (~JOB_GENERATION_MASK)

#define INVALID_INDEX 0x7fffffff
// #define MAX_JOBS_PER_THREAD 256
#define SLEEP_TIME_MS_WHEN_IDLE 10
#define INVALID_JOB 0

// A fiber can retrieve the fiber data by calling the GetFiberData macro. A fiber can retrieve the fiber address at any time by calling the GetCurrentFiber macro.
// A fiber can use fiber local storage (FLS) to create a unique copy of a variable for each fiber. If no fiber switching occurs, FLS acts exactly the same as thread local storage. The FLS functions (FlsAlloc, FlsFree, FlsGetValue, and FlsSetValue) manipulate the FLS associated with the current thread. If the thread is executing a fiber and the fiber is switched, the FLS is also switched.

static __declspec(thread) int tls_thread_index;
static __declspec(thread) int tls_scheduler_thread_index;
static __declspec(thread) int tls_hint__free_cursor;
static __declspec(thread) int tls_hint__waiting_cursor;


#define QUEUE_MAX_ARRAY_SIZE 1024
#define QUEUE_ARRAY_MASK (QUEUE_MAX_ARRAY_SIZE-1)

typedef struct {
	long *array;
	long read;
	long write; // Only accessed by the scheduler thread
} Queue;

// Only the scheduler thread can call this
bool queue_push(Queue *q, int value) {
	assert(tls_thread_index == tls_scheduler_thread_index && "Only the scheduler thread can push jobs to the workers!");

	int write = q->write;
	write = (write + 1) & QUEUE_ARRAY_MASK;
	if (write == q->read) { /* Full queue. */
		return false;
	}
	q->write = write;
	q->array[write] = value;
	return true;
}

// Any thread can call this
bool queue_pop(Queue *q, int *value) {
	if (q->read == q->write)
		return false;

	int read = q->read;        // Where do I think we are reading from?
	int val  = q->array[read]; // What do I think the value is?
	int next = (read + 1) & QUEUE_ARRAY_MASK;
	if (!InterlockedCompareExchange(&q->read, read, next) == read) // TODO(kalle): Where can I use the NoFence variant?
		return false;

	*value = val; // My guesses were correct! Set the value!
	return true;
}


typedef void(*TaskFunction)(void *arg);
typedef struct {
	TaskFunction function;
	void *argument;
} Task;

static int thread_count;

typedef enum {
	FiberState_Free = 0,
	FiberState_Waiting = 1,
	FiberState_Busy = 2,
} FiberState;

typedef struct {
	Task task;
	int job_index;
} TaskInfo;

typedef struct {
	long job_index;
	long generation;
} JobHandle;

typedef struct {
	void *fibers[MAX_FIBER_COUNT];
	long fiber_states[MAX_FIBER_COUNT];

	Queue queue;

	TaskInfo tasks[TASK_MAX];
	long jobs[TASK_MAX];
	long generation[TASK_MAX];

	int last_free_task_info;
	int last_free_job_index;

	bool quit;
} TaskScheduler;

unsigned new_job_storage(TaskScheduler *scheduler) {
	bool available = false;
	for (int i = 0; i < TASK_MAX && !available; ++i) {
		long task_count = scheduler->jobs[scheduler->last_free_job_index];
		available = task_count == 0; // This job is done, we have no more tasks
		if (!available)
			scheduler->last_free_job_index = (scheduler->last_free_job_index + 1) & (TASK_MAX - 1);
	}
	assert(available && "No available job storage!");

	// We have a job storage, increase the generation before using it
	scheduler->generation[scheduler->last_free_job_index]++;
	return scheduler->last_free_job_index;
}

TaskInfo *get_task_info(TaskScheduler *scheduler) {
	bool available = false;
	for (int i = 0; i < TASK_MAX && !available; ++i) {
		// Check if we can use this task info slot in the scheduler
		TaskInfo *ti = scheduler->tasks + scheduler->last_free_task_info;
		available = ti->job_index == INVALID_JOB;
		if (!available)
			scheduler->last_free_task_info = (scheduler->last_free_task_info + 1) & (TASK_MAX - 1);
	}
	assert(available && "No available task storage!");

	return scheduler->tasks + scheduler->last_free_task_info;
}


// Need to have a list of index -> state mapping

// There is no such thing as a fiber interrupting another fiber!

// Low, Medium and High prio queues?
// Three separate queues which you check for work?

// scheduler is shared memory accessed by all threads
int _get_free_fiber_index(TaskScheduler *scheduler) {
	int cursor = tls_hint__free_cursor;
	while (1) {
		for (int i = 0; i < MAX_FIBER_COUNT; ++i) {
			FiberState state = scheduler->fiber_states[cursor];
			if (state == FiberState_Free) { // Check if we think that it is free before trying to atomically set it.
				// if (free_fibers[cursor] == 1) { free_fibers[cursor] = 0; }
				// The parameters for this function must be aligned on a 32-bit boundary; otherwise, the function will behave unpredictably on multiprocessor x86 systems and any non-x86 systems. See _aligned_malloc.
				if (InterlockedCompareExchangeNoFence(scheduler->fiber_states + cursor, FiberState_Free, FiberState_Busy) == FiberState_Free) {
					// I was right, and now I've set this as _not_ free!
					tls_hint__free_cursor = cursor;
					return cursor;
				}
			}

			cursor++;
			if (cursor == MAX_FIBER_COUNT)
				cursor = 0;
		}

		assert(0 && "Found no free fibers!"); // We should have free fibers available unless we have a huge amount of cores to churn away at the tasks.
		Sleep(SLEEP_TIME_MS_WHEN_IDLE); // No free fibers, waiting a while and try again
	}
	return 0;
}

typedef struct {
	TaskScheduler *scheduler;
	int thread_index;
	int padding;
} ThreadContext;


void __stdcall _fiber_start(void *params) {
	TaskScheduler *scheduler = (TaskScheduler *) params;
	while (!scheduler->quit) {
		// Check if any of the waiting tasks are ready
		int waiting_fiber_index = INVALID_INDEX;

		// u64 time = __rdtsc();
		int cursor = tls_hint__waiting_cursor;
		for (int i = 0; i < MAX_FIBER_COUNT; ++i) {
			FiberState state = scheduler->fiber_states[cursor];
			if (state == FiberState_Waiting) { // Do I think that this fiber is waiting?
				void *fiber = scheduler->fibers[cursor];

				// if (waiting_fiber->task_counter == waiting_fiber->target_count) { // TODO(kalle): Should target_count always be 0, when would you want to wait for something else?
					// If I locally think that this fiber is waiting and the task_counter is at target, try to grab it!

					if (InterlockedCompareExchangeNoFence(scheduler->fiber_states + cursor, FiberState_Waiting, FiberState_Busy) == FiberState_Waiting) {
						waiting_fiber_index = cursor;
						break;
					}
				// }
			}
		}
		if (waiting_fiber_index != INVALID_INDEX) {
			// Found a waiting fiber that is ready to continue
			SwitchToFiber(scheduler->fibers + waiting_fiber_index);
		} else {
			// Get a new task from the queue and execute it
			int index;
			if (queue_pop(&scheduler->queue, &index)) {
				TaskInfo *next_task = scheduler->tasks + index;
				next_task->task.function(next_task->task.argument);
				long *job = scheduler->jobs + next_task->job_index;

				InterlockedDecrementNoFence(job);

				if (*job == 0) { // TODO(kalle): More than one can see this as 0, which does matter!!!

				}
			} else {
				Sleep(SLEEP_TIME_MS_WHEN_IDLE); // I have no waiting fibers and no new tasks, sleep a while
			}
		}
	}
}

unsigned __stdcall _thread_start(void *params) {
	ThreadContext *context = (ThreadContext *)params;

	TaskScheduler *scheduler = context->scheduler;
	tls_thread_index = context->thread_index;

	// These are hints where this thread should start looking for free / waiting fibers.
	// This is to avoid rechecking from the start of the fiber pool each time we want to get a free fiber, for example.
	tls_hint__free_cursor = tls_thread_index * (MAX_FIBER_COUNT / thread_count);
	tls_hint__waiting_cursor = tls_hint__free_cursor;

	// We need to convert us to a fiber in order to switch to a worker fiber
	ConvertThreadToFiberEx(0, FIBER_FLAG_FLOAT_SWITCH);

	int fiber_index = _get_free_fiber_index(scheduler); // Get a free fiber to switch to
	SwitchToFiber(scheduler->fibers + fiber_index); // Switch to it

	assert(false && "We should never return here, since when the fibers return the calling thread exits!");
	exit_thread();
	return 0;
}

// A job consist of one or more tasks
// Maybe do a linked list instead?

// Adds a group of tasks to the internal queue.
// Returns a counter that will initially be equal to task_count.
// When a task is completed, the counter is decremented.
// When the returned counter is 0, the tasks are done.
JobHandle scheduler_add_job(TaskScheduler *scheduler, int task_count, Task *tasks) {
	assert(tls_thread_index == tls_scheduler_thread_index && "Only the scheduler thread can add jobs!");

	// counter of the job
	unsigned job_index = new_job_storage(scheduler);
	assert(scheduler->jobs[job_index] == 0);

	// Go through and add each task to the job
	for (int i = 0; i < task_count; ++i) {
		TaskInfo *task_info = get_task_info(scheduler);
		assert(task_info->job_index == INVALID_JOB);

		task_info->task = tasks[i];
		task_info->job_index = job_index;
		queue_push(&scheduler->queue, scheduler->last_free_task_info);
	}

	scheduler->jobs[job_index] = task_count;

	JobHandle job_handle;
	job_handle.job_index = job_index;
	job_handle.generation = scheduler->generation[job_index];
	return job_handle;
}

typedef void(*MainTaskFunction)(TaskScheduler *scheduler, void *args);
void scheduler_start(MainTaskFunction main_task, void *args) {
	// TODO(kalle): Require this memory to be allocated outside? Require alignment in that case (because of the InterlockedCompareExchange).
	TaskScheduler *scheduler = _aligned_malloc(sizeof(TaskScheduler), 4);
	memset(scheduler, 0, sizeof(TaskScheduler));

	for (int i = 0; i < MAX_FIBER_COUNT; ++i) {
		scheduler->fibers[i] = CreateFiberEx(0, FIBER_STACK_SIZE, FIBER_FLAG_FLOAT_SWITCH, _fiber_start, scheduler);
	}

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	thread_count = sysinfo.dwNumberOfProcessors;
	thread_count = thread_count < MAX_WORKER_THREADS ? thread_count : MAX_WORKER_THREADS;

	ThreadContext thread_contexts[MAX_WORKER_THREADS];
	ThreadType threads[MAX_WORKER_THREADS];
	for (int i = 1; i < thread_count; ++i) {
		ThreadContext *tc = thread_contexts + i;
		tc->scheduler = scheduler;
		tc->thread_index = i;

		int thread_affinity = 1 << i;
		unsigned stack_size = 0;
		bool success = create_thread(_thread_start, tc, threads + i, stack_size, thread_affinity);
		assert(success && "Error creating thread!");
	}

	main_task(scheduler, args);

	// And we're back. Wait for the worker threads to finish.
	for (int i = 1; i < thread_count; ++i) {
		join_thread(threads[i]);
	}

	// Do we need to cleanup?
	for (int i = 0; i < MAX_FIBER_COUNT; ++i) {
		DeleteFiber(scheduler->fibers[i]);
	}
}

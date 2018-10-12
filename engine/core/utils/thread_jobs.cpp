#include <stdafx.h>

#include "core/common.h"
#include "core/utils/assert.h"
#include "core/memory/allocator.h"
#include "core/utils/jobs.h"
#include "core/utils/thread_jobs.h"

#define MAX_WORKER_THREADS 32
#define TASK_BITS 9

#define TASK_MAX (1 << TASK_BITS)
#define SLEEP_TIME_MS_WHEN_IDLE 10
#define INVALID_JOB_INDEX 0

#define QUEUE_MAX_ARRAY_SIZE 1024
#define QUEUE_ARRAY_MASK (QUEUE_MAX_ARRAY_SIZE-1)

static __declspec(thread) DWORD tls_manager_thread_id;

// Get the number of hardware workers. This should take Hyperthreading, etc. into account
__forceinline int get_num_hardware_threads() {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

struct TJQueue {
	long array[QUEUE_MAX_ARRAY_SIZE];
	long read;
	long write; // Only accessed by the tjm thread
};

// Only the tjm thread can call this, it needs to wait if the queue is full.
bool _queue_push(TJQueue *q, int value) {
	ASSERT(tls_manager_thread_id == GetCurrentThreadId(), "Only the tjm thread can push jobs to the workers!");

	int write = q->write;
	write = (write + 1) & QUEUE_ARRAY_MASK;
	if (write == q->read) { // Full queue
		return false;
	}
	q->array[q->write] = value;
	q->write = write;
	return true;
}

// Any thread can call this
bool _queue_pop(TJQueue *q, int *value) {
	if (q->read == q->write)
		return false;

	int read = q->read;        // Where do I think we are reading from?
	int val  = q->array[read]; // What do I think the value is?
	int next = (read + 1) & QUEUE_ARRAY_MASK;
	if (!InterlockedCompareExchange(&q->read, next, read) == read) // TODO(kalle): Where can I use the NoFence variant?
		return false;

	*value = val; // My guesses were correct! Set the value!
	return true;
}

struct TaskInfo {
	Task task;
	int job_index;
};
struct ThreadJobManager {
	HANDLE *workers;
	int worker_count;

	Allocator *allocator;

	TJQueue queue;

	TaskInfo tasks[TASK_MAX];
	long jobs[TASK_MAX];
	long generation[TASK_MAX]; // Generation makes sure that we don't overwrite a job that is done with new work. If we would, the client would never know that the job was done.

	int last_free_task_info;
	int last_free_job_index;

	bool quit;
};

unsigned _get_new_job_storage(ThreadJobManager *tjm) {
	bool available = false;
	for (int i = 0; i < TASK_MAX && !available; ++i) {
		long task_count = tjm->jobs[tjm->last_free_job_index];
		available = task_count == 0; // This job is done, we have no more tasks
		if (!available) {
			tjm->last_free_job_index++;
			if (tjm->last_free_job_index == TASK_MAX)
				tjm->last_free_job_index = 1;
		}
	}
	ASSERT(available, "No available job storage!");

	// We have a job storage, increase the generation before using it
	tjm->generation[tjm->last_free_job_index]++;
	return tjm->last_free_job_index;
}

TaskInfo *_get_task_info(ThreadJobManager *tjm) {
	bool available = false;
	for (int i = 0; i < TASK_MAX && !available; ++i) {
		// Check if we can use this task info slot in the tjm
		TaskInfo *ti = tjm->tasks + tjm->last_free_task_info;
		available = ti->job_index == INVALID_JOB_INDEX;
		if (!available)
			tjm->last_free_task_info = (tjm->last_free_task_info + 1) & (TASK_MAX - 1);
	}
	ASSERT(available, "No available task storage!");

	return tjm->tasks + tjm->last_free_task_info;
}

bool _try_do_work(ThreadJobManager *tjm) {
	// Get a new task from the queue and execute it
	int index;
	bool found_work = _queue_pop(&tjm->queue, &index);
	if (found_work) {
		TaskInfo *next_task = tjm->tasks + index;
		next_task->task.function(next_task->task.argument);

		// Decrement the job counter
		long *job = tjm->jobs + next_task->job_index;
		InterlockedDecrementNoFence(job);
		next_task->job_index = INVALID_JOB_INDEX;
	}
	return found_work;
}

DWORD _worker_start(void *params) {
	ThreadJobManager *tjm = (ThreadJobManager *) params;
	while (!tjm->quit) {
		if (!_try_do_work(tjm)) {
			Sleep(SLEEP_TIME_MS_WHEN_IDLE); // I have no new tasks, sleep a while
		}
	}
	return 0;
}

ThreadJobManager *job_manager(Allocator *a, int thread_pool_size) {
	ThreadJobManager *tjm = (ThreadJobManager *)allocate(a, sizeof(ThreadJobManager), true, 4);
	tjm->allocator = a;

	int worker_count = thread_pool_size ? thread_pool_size : get_num_hardware_threads();

	tls_manager_thread_id = GetCurrentThreadId();
	tjm->last_free_job_index = 1;
	tjm->workers = (HANDLE *)allocate(a, worker_count * sizeof(HANDLE), true, 4);
	tjm->worker_count = worker_count;
	for (int i = 0; i < worker_count; ++i) {
		HANDLE handle = CreateThread(0, 0, _worker_start, tjm, CREATE_SUSPENDED, 0);
		if (!handle) {
			printf("Error: Failed to create all the worker threads");
			return 0;
		}

		SetThreadAffinityMask(handle, (uint64_t)1 << i);
		ResumeThread(handle);
		tjm->workers[i] = handle;
	}

	return tjm;
}

// A job consist of one or more tasks
// Maybe do a linked list instead?

// Adds a group of tasks to the internal queue.
// Returns a counter that will initially be equal to task_count.
// When a task is completed, the counter is decremented.
// When the returned counter is 0, the tasks are done.

///// THIS IS NOT EXTERNALLY THREAD SAFE. If you need to add tasks from two workers, you need handle their synchronization
ThreadJobHandle add_tasks(ThreadJobManager *tjm, int task_count, Task *tasks) {
	ASSERT(tls_manager_thread_id == GetCurrentThreadId(), "Only the thread that created the job manager can add jobs!");

	// counter of the job
	unsigned job_index = _get_new_job_storage(tjm);
	ASSERT(job_index != INVALID_JOB_INDEX, "Job index invalid!");
	ASSERT(tjm->jobs[job_index] == 0, "Job storage is already taken!");
	tjm->jobs[job_index] = task_count;

	// Go through and add each task to the job
	for (int i = 0; i < task_count; ++i) {
		TaskInfo *task_info = _get_task_info(tjm);
		ASSERT(task_info->job_index == INVALID_JOB_INDEX, "Task info is already taken!");

		task_info->task = tasks[i];
		task_info->job_index = job_index;
		bool success = _queue_push(&tjm->queue, tjm->last_free_task_info);
		if (!success) {
			// What to do? I need to wait here until there is room in the queue
			ASSERT(false, "Add a wait in here?");
		}
	}

	ThreadJobHandle job_handle;
	job_handle.job_index = job_index;
	job_handle.generation = tjm->generation[job_index];
	return job_handle;
}

bool is_done(ThreadJobManager *tjm, ThreadJobHandle handle) {
	if (handle.job_index != INVALID_JOB_INDEX) {
		long generation = tjm->generation[handle.job_index];
		if (generation != handle.generation) // If we aren't on the same generation, the workers are finished and have moved on
			return true;

		long job = tjm->jobs[handle.job_index];
		if (job == 0)
			return true;
	}
	return false;
}

void wait_for_job(ThreadJobManager *tjm, ThreadJobHandle job, bool work_while_waiting, int sleep_time) {
	while (!is_done(tjm, job)) {
		if (work_while_waiting && _try_do_work(tjm)) {
		} else {
			Sleep(sleep_time);
		}
	}
}

void destroy_job_manager(ThreadJobManager *tjm) {
	tjm->quit = true;

	for (int i = 0; i < tjm->worker_count; ++i) {
		WaitForSingleObject(tjm->workers[i], INFINITE);
	}

	deallocate(tjm->allocator, tjm);
}

#pragma once

/** \addtogroup ThreadJobs
 * Thread Job Manager
 *
 * A job manager that uses threads to execute tasks.
 * This can be started at anytime and used later to execute tasks.
 *  @{
 */
struct ThreadJobManager;

/// A JobHandle is linked to a collection of tasks. This handle can be used to check the status of these tasks.
struct ThreadJobHandle {
	long job_index;
	long generation;
};

/// Start the fiber job manager, with the task given. When all issued tasks are done, this function returns.
ThreadJobManager *job_manager(Allocator *allocator, int thread_pool_size = 0);
/// Adds a number of tasks to be executed by the threads.
ThreadJobHandle add_tasks(ThreadJobManager *tjm, int task_count, Task *tasks);
/// Checks if a job is done.
bool is_done(ThreadJobManager *tjm, ThreadJobHandle handle);
/// Waits until the job is done. If work_while_waiting is true, it will help out with the task list. If not, it will sleep 'sleep_time' until checking again.
void wait_for_job(ThreadJobManager *tjm, ThreadJobHandle job, bool work_while_waiting = true, int sleep_time = 10);
/// Destroys the job manager and deallocates it's memory.
void destroy_job_manager(ThreadJobManager *tjm);

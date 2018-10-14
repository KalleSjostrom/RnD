#pragma once

/** \addtogroup FiberJobs
 * Fiber Job Manager
 *
 * A job manager that uses fibers to execute small tasks.
 * This will only execute the main task when started, which is then supposed to add more tasks.
 * When the main task returns, the fiber manager quits.
 *  @{
 */
struct FiberJobManager;

typedef void(*MainTaskFunction)(FiberJobManager *job_manager, void *user_data);

enum FiberJobsResult {
	FiberJobsResult_Failed,
	FiberJobsResult_Completed,
};

/// A JobHandle is linked to a collection of tasks. This handle can be used to check the status of these tasks.
typedef int FiberJobHandle;

/// Start the fiber job manager, with the task given. When all issued tasks are done, this function returns.
FiberJobsResult run_fibertask(MainTaskFunction main_task, void *user_data = 0, int thread_pool_size = 0);
/// Adds a number of tasks to be executed by the fibers.
FiberJobHandle add_tasks(FiberJobManager *fjm, int task_count, Task *tasks);
/// Checks if a job is done.
bool is_done(FiberJobManager *fjm, FiberJobHandle job);
/// Waits until the job is done, meanwhile it will help out with the task list.
void wait_for_job(FiberJobManager *fjm, FiberJobHandle job);

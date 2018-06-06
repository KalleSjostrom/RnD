#include "task_scheduler.c"

void run(TaskScheduler *scheduler, void *user_data) {

}

int main(int argc, char const *argv[])
{
	scheduler_start(run, 0);
	return 0;
}

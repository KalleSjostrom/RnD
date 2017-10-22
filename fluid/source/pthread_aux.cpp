#define PTHREAD_TASK(name) void* name(void *userdata)
typedef PTHREAD_TASK(pthread_task_t);

#define NUMBER_OF_THREADS 8
#define LAST_THREAD_INDEX (NUMBER_OF_THREADS - 1)
#include <pthread.h>

namespace pthread_aux {
	typedef struct {
		void *userdata;
		u32 start;
		u32 stop;
	} Job;

	static pthread_t *threads = 0;
	static pthread_attr_t attr;
	static Job *jobs;

	void pthread_init(void *userdata) {
		threads = (pthread_t*) calloc(NUMBER_OF_THREADS, sizeof(pthread_t));
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		jobs = (Job *)malloc(NUMBER_OF_THREADS * sizeof(Job));
		u32 chunks = NR_PARTICLES / NUMBER_OF_THREADS;

		for (u32 i = 0; i < NUMBER_OF_THREADS; i++) {
			jobs[i].userdata = userdata;
			jobs[i].start = i * chunks;
			jobs[i].stop = (i+1) * chunks;
		}
	}

	void pthread_run_task(pthread_task_t *task) {
		for (int i = 0; i < LAST_THREAD_INDEX; i++) {
			if (pthread_create(&threads[i], &attr, task, (void *) (jobs + i))) {
				fprintf(stderr, "Failed to create a thread\n");
			}
		}
		task((void *) (jobs + LAST_THREAD_INDEX));

		for (int i = 0; i < LAST_THREAD_INDEX; i++) {
			if (pthread_join(threads[i], NULL)) {
				fprintf(stderr, "Failed to join a thread\n");
			}
		}
	}
}

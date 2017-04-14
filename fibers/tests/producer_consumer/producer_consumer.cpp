#include "fiber_tasking_lib/task_scheduler.h"

const uint kNumProducerTasks = 100u;
const uint kNumConsumerTasks = 10000u;

void Consumer(FiberTaskingLib::TaskScheduler *taskScheduler, void *arg) {
	std::atomic_uint *globalCounter = reinterpret_cast<std::atomic_uint *>(arg);

	globalCounter->fetch_add(1);
}

void Producer(FiberTaskingLib::TaskScheduler *taskScheduler, void *arg) {
	FiberTaskingLib::Task *tasks = new FiberTaskingLib::Task[kNumConsumerTasks];
	for (uint i = 0; i < kNumConsumerTasks; ++i) {
		tasks[i] = { Consumer, arg };
	}

	std::shared_ptr<std::atomic_uint> counter = taskScheduler->AddTasks(kNumConsumerTasks, tasks);
	delete[] tasks;

	taskScheduler->WaitForCounter(counter, 0);
}


void ProducerConsumerMainTask(FiberTaskingLib::TaskScheduler *taskScheduler, void *arg) {
	std::atomic_uint globalCounter(0u);

	FiberTaskingLib::Task tasks[kNumProducerTasks];
	for (uint i = 0; i < kNumProducerTasks; ++i) {
		tasks[i] = { Producer, &globalCounter };
	}

	std::shared_ptr<std::atomic_uint> counter = taskScheduler->AddTasks(kNumProducerTasks, tasks);
	taskScheduler->WaitForCounter(counter, 0);


	// Test to see that all tasks finished
	GTEST_ASSERT_EQ(kNumProducerTasks * kNumConsumerTasks, globalCounter.load());

	// Cleanup
}


/**
 * Tests that all scheduled tasks finish properly
 */
TEST(FunctionalTests, ProducerConsumer) {
	FiberTaskingLib::TaskScheduler taskScheduler;
	taskScheduler.Run(400, ProducerConsumerMainTask);
}

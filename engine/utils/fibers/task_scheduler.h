#pragma once

/// Task scheduler
typedef void(*TaskFunction)(void *arg);
struct Task {
	TaskFunction function;
	void *argument;
};

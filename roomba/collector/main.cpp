#include <time.h>
#include <mach/mach_time.h>

#include "utils/common.h"
#include "utils/memory_arena.cpp"

#define USE_INTRINSICS 0
#include "utils/math.h"

enum Flags {
	Flags_Bumped
};

struct Snapshot {
	double time;
	v2 position;
	u64 flags;
};

struct State {
	double last_snapshot;

	Snapshot snapshot;

	char *output_stream;
	u64 output_cursor;
};

static void take_snapshot(Snapshot &snapshot) {
	snapshot.flags = 0;
	snapshot.position = V2(0, 0);
}

static void store(State &state) {
	Snapshot &snapshot = state.snapshot;
	snapshot.flags = false;
	snapshot.position = V2(0, 0);

	memcpy(state.output_stream + state.output_cursor, &snapshot, sizeof(Snapshot));

	state.output_cursor += sizeof(Snapshot);

	ASSERT(state.output_cursor <= 32*MB, "Out of memory");
}

static void update(State &state, double dt) {
	Snapshot &snapshot = state.snapshot;

	snapshot.time += dt;

	if (snapshot.flags & Flags_Bumped) {
		state.last_snapshot += 0.25;
		store(state);
	} else if (snapshot.time >= state.last_snapshot) {
		state.last_snapshot += 0.25;
		store(state);
	}
}

static void write_state(State &state) {
	(void) state;
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	MemoryArena arena = init_memory(32*MB, true);

	// generate map of the appartment

	// remember where obstacles are

	// if you can push stuff, it's dynamic
	// else i don't know

	// Store position every x second _and_ at bump

	// Trösklar?

	bool running = true;

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);

	double time_resolution = (double) timebase_info.numer / (timebase_info.denom * 1.0e9);
	uint64_t current_time;
	uint64_t previous_time = mach_absolute_time();

	State state = {};

	state.output_stream = allocate_memory(arena, 32*MB);;

	while (running) {
		current_time = mach_absolute_time();
		double dt = (current_time - previous_time) * time_resolution;

		take_snapshot(state.snapshot);

		update(state, dt);

		write_state(state);
	}
}

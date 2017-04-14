#include "common.h"
#include "profiler.c"
#include <math.h>

enum ProfilerScopes {
	ProfilerScopes__add,
	ProfilerScopes__remove,
	ProfilerScopes__simulation,

	ProfilerScopes__count,
};

/*
Based on the "Hacker's Delight" version:
unsigned clp2(unsigned x) {
   x = x - 1;
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   x = x | (x >>16);
   return x + 1;
}*/
#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

#define SIZE 4
// NOTE(kalle): Must be a power of two!
#define BROADPHASE_SIZE next_power_of_2(SIZE)
#define INVALID_BROADPHASE_HASH 0xFFFFFFFF

#include "broadphase.h"
Vector2 v2(float x, float y) { Vector2 v = {x, y}; return v; }

float random01() {
	return ((float) rand() / (RAND_MAX));
}
float r() {
	return random01() * 2 - 1;
}

void print_broadphase(Broadphase &broadphase) {
	printf("------\n");
	for (int i = 0; i < broadphase.count; ++i) {
		BroadphaseEntry &entry = broadphase.entries[i];
		printf("entry [p={%.2f, %.2f}, %u, %u]\n", entry.position.x, entry.position.y, entry.hash, entry.next);
	}
	printf("------\n");
}

inline void test_add(Broadphase &broadphase, unsigned *handles, unsigned &num_handles, unsigned i) {
	printf("Try add\n");

	float x = r() * 1024.0f;
	float y = r() * 1024.0f;
	unsigned handle = broadphase_add(broadphase, v2(x, y));
	handles[num_handles++] = handle;

	printf("Add %u\n", handle);
	print_broadphase(broadphase);
}

inline void test_move(Broadphase &broadphase, unsigned *handles, unsigned &num_handles) {
	unsigned index = random() % num_handles;

	printf("Try move %u\n", handles[index]);

	float x = r() * 1024.0f;
	float y = r() * 1024.0f;
	handles[index] = broadphase_move(broadphase, handles[index], v2(x, y));

	printf("Move %u\n", handles[index]);
	print_broadphase(broadphase);
}

inline void test_remove(Broadphase &broadphase, unsigned *handles, unsigned &num_handles, unsigned &i) {
	unsigned index = random() % num_handles;
	printf("Try remove %u\n", handles[index]);

	broadphase_remove(broadphase, handles[index]);

	printf("Remove %u\n", handles[index]);
	print_broadphase(broadphase);
	i--;
	handles[index] = handles[--num_handles];
}


int main(int argc, char const *argv[]) {
	BroadphaseEntry entries[BROADPHASE_SIZE] = {};
	Broadphase broadphase;
	broadphase_init(broadphase, entries, BROADPHASE_SIZE, INVALID_BROADPHASE_HASH);

	unsigned num_itr = 512;

	unsigned num_handles = 0;
	unsigned *handles = (unsigned*)malloc(SIZE*sizeof(unsigned));

	int *choices = (int*)malloc(num_itr*sizeof(int));
	for (unsigned i = 1; i < num_itr; ++i) {
		choices[i] = random()%3;
	}

	unsigned *randoms = (unsigned*)malloc(num_itr*sizeof(unsigned));
	for (unsigned i = 1; i < num_itr; ++i) {
		randoms[i] = random();
	}

	PROFILER_START(simulation);
	for (unsigned i = 1; i < num_itr; ++i) {
		printf("%u\n", num_handles);
		if (num_handles == 0) {
			test_add(broadphase, handles, num_handles, i);
		} else if (num_handles == SIZE-1) {
			test_remove(broadphase, handles, num_handles, i);
		} else {
			if (choices[i] == 0) {
				test_add(broadphase, handles, num_handles, i);
			} else if (choices[i] == 1) {
				test_remove(broadphase, handles, num_handles, i);
			} else {
				test_move(broadphase, handles, num_handles);
			}
		}
	}

	for (unsigned i = 0; i < num_handles; ++i) {
		test_remove(broadphase, handles, num_handles, i);
	}
	// PROFILER_STOP(simulation);

	// /*printf("Adds %u\n", _add_itr);
	// printf("Add lookups %u\n", _add_lookups_itr);
	// printf("\tAdd ratio %.3f\n", (float)_add_lookups_itr / (float)_add_itr);
	// printf("Removes %u\n", _remove_itr);
	// printf("Remove lookups %u\n", _remove_lookups_itr);
	// printf("\tRemove ratio %.3f\n", (float)_remove_lookups_itr / (float)_remove_itr);
	// printf("Remove moves %u\n", _move_into_place_itr);
	// printf("\tRemove moves ratio %.3f\n", _move_into_place_itr / (float)_remove_itr);
	// printf("\n");*/

	// PROFILER_PRINT(add);
	// PROFILER_PRINT(remove);
	// PROFILER_PRINT(simulation);

	return 0;
}








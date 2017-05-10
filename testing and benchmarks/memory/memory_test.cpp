#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "engine/utils/common.h"

// #define MSPACES 1
// #define ONLY_MSPACES 1
#define DEBUG 1
// #define HAVE_MMAP 0
// #define NO_MALLINFO 1

#include "engine/utils/memory/dlmalloc.c"
#include "engine/utils/profiler.c"

enum ProfilerScopes {
	ProfilerScopes__count
};

int main() {
	void *memory = malloc(1024*1024);
	memset(memory, 0, 1024*1024);

	mspace myspace = create_mspace_with_base(memory, 512*1024);

	void *blocks[] = {
		mspace_malloc(myspace, 64),
		mspace_malloc(myspace, 64),
		mspace_malloc(myspace, 64),
		mspace_malloc(myspace, 64),
		mspace_malloc(myspace, 64),
		mspace_malloc(myspace, 64),

		mspace_malloc(myspace, 256),
		mspace_malloc(myspace, 256),
		mspace_malloc(myspace, 256),
		mspace_malloc(myspace, 256),
		mspace_malloc(myspace, 256),
		mspace_malloc(myspace, 256),
	};

	intptr_t address[ARRAY_COUNT(blocks)];
	for (u32 i = 0; i < ARRAY_COUNT(blocks); ++i) {
		address[i] = (intptr_t)blocks[i];
	}

	intptr_t offsets[ARRAY_COUNT(blocks)];
	for (u32 i = 0; i < ARRAY_COUNT(blocks); ++i) {
		offsets[i] = (intptr_t)blocks[i] - (intptr_t)memory;
	}

	malloc_chunk *chunks[ARRAY_COUNT(blocks)];
	for (u32 i = 0; i < ARRAY_COUNT(blocks); ++i) {
		chunks[i] = mem2chunk(blocks[i]);
	}

	size_t sizes[ARRAY_COUNT(blocks)];
	for (u32 i = 0; i < ARRAY_COUNT(blocks); ++i) {
		sizes[i] = chunksize(chunks[i]);
	}

	mspace_free(myspace, blocks[0]);
	mspace_free(myspace, blocks[1]);
	mspace_free(myspace, blocks[2]);
	mspace_free(myspace, blocks[3]);
	mspace_free(myspace, blocks[4]);
	mspace_free(myspace, blocks[5]);

	mspace_free(myspace, blocks[6]);
	mspace_free(myspace, blocks[7]);
	mspace_free(myspace, blocks[8]);
	mspace_free(myspace, blocks[9]);
	mspace_free(myspace, blocks[10]);
	mspace_free(myspace, blocks[11]);
}

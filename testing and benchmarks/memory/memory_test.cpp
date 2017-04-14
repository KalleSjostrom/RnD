#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../../utils/common.h"

#define MSPACES 1
#define ONLY_MSPACES 1
#define DEBUG 1
#define HAVE_MMAP 0
#define NO_MALLINFO 1

#include "../../utils/dlmalloc.c"
#include "../../utils/profiler.c"

enum ProfilerScopes {
	ProfilerScopes__count,
};

int main() {
	void *memory = malloc(1024*1024);
	memset(memory, 0, 1024*1024);

	mspace myspace = create_mspace_with_base(memory, 512*1024);

	unsigned test = TOP_FOOT_SIZE;

	mstate ms = (mstate)myspace;

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

	intptr_t address[ARRAY_SIZE(blocks)];
	for (int i = 0; i < ARRAY_SIZE(blocks); ++i) {
		address[i] = (intptr_t)blocks[i];
	}

	intptr_t offsets[ARRAY_SIZE(blocks)];
	for (int i = 0; i < ARRAY_SIZE(blocks); ++i) {
		offsets[i] = (intptr_t)blocks[i] - (intptr_t)memory;
	}

	malloc_chunk *chunks[ARRAY_SIZE(blocks)];
	for (int i = 0; i < ARRAY_SIZE(blocks); ++i) {
		chunks[i] = mem2chunk(blocks[i]);
	}

	size_t sizes[ARRAY_SIZE(blocks)];
	for (int i = 0; i < ARRAY_SIZE(blocks); ++i) {
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

	printf("Hello\n");
}
#pragma once

#define KB 1024
#define MB 1024*KB
#define GB 1024*MB

#include "arena_allocator.h"
#include "mspace_allocator.h"

enum AllocatorType {
	AllocatorType_Arena,
	AllocatorType_MSpace,
};

struct Allocator {
	AllocatorType type;
	union {
		ArenaAllocator *arena;
		MSpaceAllocator *mspace;
	};
};

Allocator allocator_arena(ArenaAllocator *arena, size_t page_count = 0);
Allocator allocator_mspace(size_t capacity = 0);

/// Allocates size bytes
void *allocate(Allocator *allocator, size_t size, bool clear_to_zero = false, unsigned alignment = 4);
/// Resets the arena by just setting the offset to 0. Note that the memory is not cleared!
void reset(Allocator *allocator);
// void init_allocator(Allocator *a, size_t capacity = 0);
void *realloc(Allocator *a, void *p, size_t new_size);
void deallocate(Allocator *a, void *p);
void destroy(Allocator *a);

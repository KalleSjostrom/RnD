#include "allocator.h"

Allocator allocator_arena(ArenaAllocator *arena) {
	Allocator allocator = {};
	allocator.type = AllocatorType_Arena;
	allocator.arena = arena;
	return allocator;
}
Allocator allocator_mspace(size_t capacity) {
	Allocator allocator = {};
	allocator.type = AllocatorType_MSpace;
	allocator.mspace = mspace_allocator(capacity);
	return allocator;
}

void *allocate(Allocator *allocator, size_t size, bool clear_to_zero , unsigned alignment) {
	void *memory = 0;
	switch (allocator->type) {
		case AllocatorType_Arena: {
			memory = allocate(allocator->arena, size, clear_to_zero, alignment);
		} break;
		case AllocatorType_MSpace: {
			memory = allocate(allocator->mspace, size, clear_to_zero, alignment);
		} break;
	}
	return memory;
}
void reset(Allocator *allocator) {
	switch (allocator->type) {
		case AllocatorType_Arena: {
			reset(allocator->arena);
		} break;
		case AllocatorType_MSpace: {
			// reset(allocator->mspace);
			ASSERT(false, "Cannot reset with mspace allocator!");
		} break;
	}
}

void *realloc(Allocator *allocator, void *p, size_t new_size) {
	void *memory = 0;
	switch (allocator->type) {
		case AllocatorType_Arena: {
			ASSERT(false, "Cannot realloc with arena allocator!");
			// memory = allocate(allocator->arena, new_size);
		} break;
		case AllocatorType_MSpace: {
			memory = realloc(allocator->mspace, p, new_size);
		} break;
	}
	return memory;
}

void deallocate(Allocator *allocator, void *p) {
	switch (allocator->type) {
		case AllocatorType_Arena: {
			ASSERT(false, "Cannot deallocate with arena allocator!");
		} break;
		case AllocatorType_MSpace: {
			deallocate(allocator->mspace, p);
		} break;
	}
}

void destroy(Allocator *allocator) {
	switch (allocator->type) {
		case AllocatorType_Arena: {
			free(allocator->arena);
		} break;
		case AllocatorType_MSpace: {
			destroy(allocator->mspace);
		} break;
	}
}

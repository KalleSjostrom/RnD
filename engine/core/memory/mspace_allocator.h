#pragma once

typedef void* MSpaceAllocator;

MSpaceAllocator *mspace_allocator(size_t capacity = 0);
void init_allocator(MSpaceAllocator *a, size_t capacity = 0);
void *allocate(MSpaceAllocator *a, size_t size, bool clear_to_zero = false, unsigned alignment = 4);
void *realloc(MSpaceAllocator *a, void *p, size_t new_size);
void *top_memory(MSpaceAllocator *a);
void deallocate(MSpaceAllocator *a, void *p);
void destroy(MSpaceAllocator *a);

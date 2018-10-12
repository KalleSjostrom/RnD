#pragma once

struct MSpaceAllocator *mspace_allocator(size_t capacity = 0);
void init_allocator(struct MSpaceAllocator *a, size_t capacity = 0);
void *allocate(struct MSpaceAllocator *a, size_t size, bool clear_to_zero = false, unsigned alignment = 4);
void *realloc(struct MSpaceAllocator *a, void *p, size_t new_size);
void deallocate(struct MSpaceAllocator *a, void *p);
void destroy(struct MSpaceAllocator *a);

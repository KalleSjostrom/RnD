#include "arena_allocator.h"

__forceinline size_t _get_alignment_offset(void *memory, size_t alignment) {
	size_t result = (size_t) memory;
	size_t alignment_mask = alignment - 1;
	if (result & alignment_mask) {
		return alignment - (result & alignment_mask);
	}
	return 0;
}
__forceinline size_t _get_pagesize() {
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwPageSize;
}

size_t reserved_size = 1 * GB;

void init_arena_allocator(ArenaAllocator *arena, size_t initial_page_count) {
	size_t pagesize = _get_pagesize();

	arena->size = pagesize * initial_page_count;
	arena->offset = 0;
	arena->memory = VirtualAlloc(0, reserved_size, MEM_RESERVE, PAGE_READWRITE);
	arena->memory = VirtualAlloc(arena->memory, arena->size, MEM_COMMIT, PAGE_READWRITE);
}

void *allocate(ArenaAllocator *arena, size_t size, bool clear_to_zero, unsigned alignment) {
	if (!arena->memory) {
		init_arena_allocator(arena);
	}

	char *mem = (char*)arena->memory;
	size_t alignment_offset = alignment <= 1 ? 0 : _get_alignment_offset(mem + arena->offset, alignment);
	size_t offset = arena->offset + alignment_offset;

	if ((offset + size) > arena->size) {
		size_t pagesize = _get_pagesize();

		size_t diff = (offset + size) - arena->size;
		size_t pages_required = (diff / pagesize) + 1;

		VirtualAlloc(mem + arena->size, pagesize * pages_required, MEM_COMMIT, PAGE_READWRITE);
		arena->size += pagesize * pages_required;
	}
	char *result = mem + offset;
	arena->offset = offset + size;

	if (clear_to_zero) {
		memset(result, 0, size);
	}
	return result;
}
void reset(ArenaAllocator *arena) {
	arena->offset = 0;
}
void free(ArenaAllocator *arena) {
	if (arena->memory) {
		VirtualFree(arena->memory, 0, MEM_RELEASE);
		arena->memory = 0;
		arena->offset = 0;
		arena->size = 0;
	}
}
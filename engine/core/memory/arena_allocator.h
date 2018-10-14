#pragma once

/** \addtogroup ArenaAllocator
 * The memory arena is a simple push-style allocator.
 * It's one large chunk of virtual memory that we just bumps an offset into which makes it really fast.
 * This is the perfect choice for scratch-space or long lived memory without need for deallocation of it's parts.
 *
 * It will reserve a huge chunk of memory, and keep commiting as much as needed (in full pages).
 *
 * The 'allocate' function will initialize the arena if not explicitly called.
 *  @{
 */
/// Initializes the arena by reserving memory and commiting the initial_page_count.
void init_arena_allocator(struct ArenaAllocator *arena, size_t initial_page_count = 1);
/// Allocates size bytes
void *allocate(struct ArenaAllocator *arena, size_t size, bool clear_to_zero = false, unsigned alignment = 4);
/// Resets the arena by just setting the offset to 0. Note that the memory is not cleared!
void reset(struct ArenaAllocator *arena);
/// Frees all memory back to the OS. This is called when the memory arena is destroyed (goes out of scope).
void free(struct ArenaAllocator *arena);

/// Used to keep track of the reserved memory chunk. When destroyed, it frees it's memory (if any allocation have taken place.)
struct ArenaAllocator {
	~ArenaAllocator() {
		free(this);
	}
	void *memory;
	size_t size;
	size_t offset;
};

/// Used to reset the memory arena state after this object goes out of scope.
struct TempAllocator {
	TempAllocator(ArenaAllocator *a) : arena(a), offset(arena->offset) { }
	~TempAllocator() {
		arena->offset = offset;
	}

	ArenaAllocator *arena;
	size_t offset;
};

#define PUSH(arena, count, type, ...) ((type*)allocate(arena, count * sizeof(type), __VA_ARGS__))

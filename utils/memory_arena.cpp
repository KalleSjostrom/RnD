//@ reloadable_struct
struct MemoryArena {
	size_t offset;
	char *memory; //@ size offset
	size_t _DEBUG_maxsize;
};

struct MemoryBlockHandle {
	size_t offset;
};

MemoryArena init_memory(size_t size, b32 clear_to_zero = false) {
	MemoryArena arena = {};

	arena.memory = (char*)malloc(size);
	arena.offset = 0;
	arena._DEBUG_maxsize = size;

	if (clear_to_zero) {
		memset(arena.memory, 0, size);
	}

	return arena;
}
MemoryArena init_from_existing(char *memory, size_t size) {
	MemoryArena arena = {};

	arena.memory = memory;
	arena.offset = 0;
	arena._DEBUG_maxsize = size;

	return arena;
}
inline char *allocate_memory(MemoryArena &arena, size_t size) {
	char *memory = arena.memory + arena.offset;
	arena.offset += size;
	ASSERT(arena.offset <= arena._DEBUG_maxsize, "Memory out of bounds");
	return memory;
}
inline void clear_memory(MemoryArena &arena) {
	arena.offset = 0;
}
inline void free_memory(MemoryArena &arena) {
	free(arena.memory);
	arena.memory = 0;
	arena.offset = 0;
	arena._DEBUG_maxsize = 0;
}

inline MemoryBlockHandle begin_block(MemoryArena &arena) {
	MemoryBlockHandle block_handle = { arena.offset };
	return block_handle;
}
inline void end_block(MemoryArena &arena, MemoryBlockHandle handle) {
	arena.offset = handle.offset;
}

inline size_t get_alignment_offset(MemoryArena &arena, unsigned alignment) {
	size_t result = (size_t) arena.memory + arena.offset;
	size_t alignment_mask = alignment - 1;
	if (result & alignment_mask) {
		return alignment - (result & alignment_mask);
	}

	return 0;
}

inline size_t get_effective_size_for(MemoryArena &arena, size_t size, unsigned alignment) {
	size_t offset = get_alignment_offset(arena, alignment);
	size += offset;
	return size;
}

inline void *_push_size(MemoryArena &arena, size_t size, unsigned alignment = 4, bool clear_to_zero = false) {
	size = get_effective_size_for(arena, size, alignment);

	ASSERT((arena.offset + size) <= arena._DEBUG_maxsize, "Memory out of bounds");

	size_t alignment_offset = get_alignment_offset(arena, alignment);
	void *result = arena.memory + arena.offset + alignment_offset;
	arena.offset += size;

	if (clear_to_zero) {
		memset(result, 0, size);
	}

	return result;
}

#define PUSH_STRUCT(arena, type, ...) (type *)_push_size(arena, sizeof(type), ## __VA_ARGS__)
#define PUSH_STRUCTS(arena, count, type, ...) (type *)_push_size(arena, (size_t)count * sizeof(type),  ## __VA_ARGS__)
#define PUSH_SIZE(arena, size, ...) _push_size(arena, size, ## __VA_ARGS__)

struct MemoryArena {
	size_t offset;
	char *memory;
	size_t _DEBUG_maxsize;
};

struct MemoryBlockHandle {
	size_t offset;
};

MemoryArena init_memory(size_t size) {
	MemoryArena arena = {0};

	arena.memory = (char*)malloc(size);
	arena.offset = 0;
	arena._DEBUG_maxsize = size;

	return arena;
}
MemoryArena init_from_existing(char *memory, size_t size) {
	MemoryArena arena = {0};

	arena.memory = memory;
	arena.offset = 0;
	arena._DEBUG_maxsize = size;

	return arena;
}
inline char *allocate_memory(MemoryArena &arena, size_t size) {
	char *memory = arena.memory + arena.offset;
	arena.offset += size;
	ASSERT(arena.offset <= arena._DEBUG_maxsize, "Out of memory in arena! (wanted size=%d, max size=%d)", size, arena._DEBUG_maxsize);
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
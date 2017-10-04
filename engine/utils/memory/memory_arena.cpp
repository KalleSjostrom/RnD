#pragma once

struct MemoryBlock {
	size_t offset;
	size_t blocksize;
	char *memory;

	MemoryBlock *previous_block;
};

struct MemoryArena {
	MemoryBlock *block;
	size_t minimum_blocksize;
};

struct MemoryBlockHandle {
	MemoryBlock *block;
	size_t offset;
};

/// Wrap os memory handling
#if defined(OS_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	inline void protect_memory(void *memory, size_t bytes) {
		DWORD ignored;
		BOOL result = VirtualProtect(memory, bytes, PAGE_NOACCESS, &ignored);
		ASSERT(result, "Error in protect_memory");
	}
	inline void unprotect_memory(void *memory, size_t bytes) {
		DWORD ignored;
		BOOL result = VirtualProtect(memory, bytes, PAGE_READWRITE, &ignored);
		ASSERT(result, "Error in unprotect_memory");
	}
	inline size_t get_pagesize() {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwPageSize;
	}
	inline void *virtual_allocation(size_t blocksize) {
		void *chunk = VirtualAlloc(0, blocksize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		return chunk;
	}
	inline void *aligned_allocation(size_t size, size_t alignment) {
		return _aligned_malloc(size, alignment);
	}
	inline void aligned_free(void *block) {
		_aligned_free(block);
	}
#elif defined(OS_LINUX) || defined(OS_MAC) || defined(iOS)
	#include <sys/mman.h>
	#include <unistd.h>

	inline void protect_memory(void *memory, size_t bytes) {
		i32 result = mprotect(memory, bytes, PROT_NONE);
		ASSERT(!result, "Error in protect_memory");
	}
	inline void unprotect_memory(void *memory, size_t bytes) {
		i32 result = mprotect(memory, bytes, PROT_READ | PROT_WRITE);
		ASSERT(!result, "Error in unprotect_memory");
	}
	inline size_t get_pagesize() {
		i32 page_size = getpagesize();
		return (size_t)page_size;
	}
	inline void *virtual_allocation(size_t blocksize) {
		void *chunk = mmap(0, blocksize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		return chunk;
	}
	inline void *aligned_allocation(size_t size, size_t alignment) {
		void *returnPtr;
		posix_memalign(&returnPtr, alignment, size);
		return returnPtr;
	}
	inline void aligned_free(void *block) {
		free(block);
	}
#endif

// #include <unistd.h>

inline MemoryBlock *_allocate_block(size_t blocksize) {
	void *chunk = virtual_allocation(blocksize);

	MemoryBlock *block = (MemoryBlock*)chunk;

	block->memory = (char*)chunk + sizeof(MemoryBlock);
	block->offset = 0;
	block->previous_block = 0;
	return block;
}

inline void free_all_blocks(MemoryBlock *block) {
	if (block) {
		free_all_blocks(block->previous_block);
		block->previous_block = 0;

		char *start = block->memory - sizeof(MemoryBlock);
		free(start);
	}
}
inline void free_memory(MemoryArena &arena) {
	free_all_blocks(arena.block);
	arena.block = 0;
}

inline MemoryBlockHandle begin_block(MemoryArena &arena) {
	MemoryBlockHandle block_handle = { arena.block, arena.block->offset };
	return block_handle;
}
inline void end_block(MemoryArena &arena, MemoryBlockHandle handle) {
	while (arena.block != handle.block) {
		MemoryBlock *block = arena.block;
		arena.block = block->previous_block;

		free(block->memory - sizeof(MemoryBlock));
	}
	arena.block->offset = handle.offset;
}

inline size_t get_alignment_offset(MemoryBlock *block, unsigned alignment) {
	size_t result = (size_t) block->memory + block->offset;
	size_t alignment_mask = alignment - 1;
	if (result & alignment_mask) {
		return alignment - (result & alignment_mask);
	}

	return 0;
}

inline size_t get_effective_size_for(MemoryBlock *block, size_t size, unsigned alignment) {
	size_t offset = get_alignment_offset(block, alignment);
	size += offset;
	return size;
}

inline void *_push_size(MemoryArena &arena, size_t size, bool clear_to_zero = false, unsigned alignment = 4) {
	size_t pagesize = get_pagesize();

	size_t alignment_offset = 0;
	size_t offset = 0;

	if (arena.block) {
		alignment_offset = get_alignment_offset(arena.block, alignment);
		offset = arena.block->offset + alignment_offset;
	} else {
		arena.minimum_blocksize = pagesize * 256;
	}

	size_t max_offset = arena.minimum_blocksize - sizeof(MemoryBlock);
	if ((offset + size) > max_offset || arena.block == 0) {
		size_t blocksize = arena.minimum_blocksize;
		if (size > max_offset) {
			size_t num_pages = size / pagesize + 1;
            blocksize = num_pages * pagesize;
		}

		MemoryBlock *new_block = _allocate_block(blocksize);
		new_block->previous_block = arena.block;
		arena.block = new_block;

		offset = get_alignment_offset(arena.block, alignment);
	}

	arena.block->offset = offset + size;
	char *result = arena.block->memory + offset;

	if (clear_to_zero) {
		memset(result, 0, size);
	}
#if DEVELOPMENT
	else {
		uint32_t *buf = (uint32_t*)result;
		for (size_t i = 0; i < size/4; i++) {
			buf[i] = 0xDEADBEEF;
		}
	}
#endif

	return result;
}

inline void setup_arena(MemoryArena &arena, size_t size, bool clear_to_zero = false, unsigned alignment = 4) {
	ASSERT(arena.block == 0, "Arena already setup!");
	_push_size(arena, size, clear_to_zero, alignment);
	arena.block->offset = 0;
}

inline void reset_arena(MemoryArena &arena, size_t size, bool clear_to_zero = false, unsigned alignment = 4) {
	arena.block = 0;
	setup_arena(arena, size, clear_to_zero, alignment);
}

#define PUSH_STRUCT(arena, type, ...) (type *)_push_size(arena, sizeof(type), ## __VA_ARGS__)
#define PUSH_STRUCTS(arena, count, type, ...) (type *)_push_size(arena, (size_t)count * sizeof(type),  ## __VA_ARGS__)
#define PUSH_STRING(arena, count, ...) (char *)_push_size(arena, (size_t)count * sizeof(char),  ## __VA_ARGS__)
#define PUSH_SIZE(arena, size, ...) _push_size(arena, size, ## __VA_ARGS__)

struct TempAllocator {
	TempAllocator(MemoryArena *a) : arena(a) {
		handle = begin_block(*arena);
	}
	~TempAllocator() { end_block(*arena, handle); }

	MemoryBlockHandle handle;
	MemoryArena *arena;
};

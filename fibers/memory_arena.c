#pragma once

typedef struct MemoryBlock MemoryBlock;

struct MemoryBlock {
	size_t offset;
	size_t blocksize;
	char *memory;

	MemoryBlock *previous_block;
};

typedef struct {
	MemoryBlock *block;
	size_t minimum_blocksize;
} MemoryArena ;

typedef struct {
	MemoryBlock *block;
	size_t offset;
} MemoryBlockHandle;

typedef struct {
	int alignment;
	int clear_to_zero;
} MemoryPushParams;

inline MemoryPushParams default_push_params() {
	MemoryPushParams mpp;
	mpp.alignment = 4;
	mpp.clear_to_zero = false;
	return mpp;
}

inline MemoryPushParams push_params(int alignment, int clear_to_zero) {
	MemoryPushParams mpp;
	mpp.alignment = alignment;
	mpp.clear_to_zero = clear_to_zero;
	return mpp;
}

/// Wrap os memory handling
#if defined(OS_WINDOWS)
	inline void protect_memory(void *memory, size_t bytes) {
		DWORD ignored;
		BOOL result = VirtualProtect(memory, bytes, PAGE_NOACCESS, &ignored);
		assert(result && "Error in protect_memory");
	}
	inline void unprotect_memory(void *memory, size_t bytes) {
		DWORD ignored;
		BOOL result = VirtualProtect(memory, bytes, PAGE_READWRITE, &ignored);
		assert(result && "Error in unprotect_memory");
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
	inline void virtual_free(void *block) {
		VirtualFree(block, 0, MEM_RELEASE);
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
		assert(!result &&, "Error in protect_memory");
	}
	inline void unprotect_memory(void *memory, size_t bytes) {
		i32 result = mprotect(memory, bytes, PROT_READ | PROT_WRITE);
		assert(!result &&, "Error in unprotect_memory");
	}
	inline size_t get_pagesize() {
		i32 page_size = getpagesize();
		return (size_t)page_size;
	}
	inline void *virtual_allocation(size_t blocksize) {
		void *chunk = mmap(0, blocksize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		return chunk;
	}
	inline void virtual_free(void *block) {
		munmap(block, 5000);
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
		virtual_free(start);
	}
}
inline void free_memory(MemoryArena *arena) {
	free_all_blocks(arena->block);
	arena->block = 0;
}

inline void reset_transient_memory(MemoryArena *arena) {
	while (arena->block && arena->block->previous_block) {
        MemoryBlock *previous_block = arena->block->previous_block;

		virtual_free(arena->block);

		arena->block = previous_block;
	}
	if (arena->block) {
		arena->block->offset = 0;
	}
}

inline MemoryBlockHandle begin_block(MemoryArena *arena) {
	MemoryBlockHandle block_handle;
	block_handle.block = arena->block;
	block_handle.offset = arena->block->offset;
	return block_handle;
}
inline void end_block(MemoryArena *arena, MemoryBlockHandle handle) {
	while (arena->block != handle.block) {
		MemoryBlock *block = arena->block;
		arena->block = block->previous_block;

		virtual_free(block->memory - sizeof(MemoryBlock));
	}
	arena->block->offset = handle.offset;
}

inline size_t get_alignment_offset(MemoryBlock *block, size_t alignment) {
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

void *_push_size(MemoryArena *arena, size_t size, MemoryPushParams params) {
	size_t pagesize = get_pagesize();

	size_t alignment_offset = 0;
	size_t offset = 0;

	if (arena->block) {
		alignment_offset = get_alignment_offset(arena->block, params.alignment);
		offset = arena->block->offset + alignment_offset;
	} else {
		arena->minimum_blocksize = pagesize * 256;
	}

	size_t max_offset = arena->minimum_blocksize - sizeof(MemoryBlock);
	if ((offset + size) > max_offset || arena->block == 0) {
		size_t blocksize = arena->minimum_blocksize;
		if (size > max_offset) {
			size_t num_pages = size / pagesize + 1;
            blocksize = num_pages * pagesize;
		}

		MemoryBlock *new_block = _allocate_block(blocksize);
		new_block->previous_block = arena->block;
		arena->block = new_block;

		offset = get_alignment_offset(arena->block, params.alignment);
	}

	arena->block->offset = offset + size;
	char *result = arena->block->memory + offset;

	if (params.clear_to_zero) {
		memset(result, 0, size);
	}
// #if DEVELOPMENT
// 	else {
// 		uint32_t *buf = (uint32_t*)result;
// 		for (size_t i = 0; i < size/4; i++) {
// 			buf[i] = 0xDEADBEEF;
// 		}
// 	}
// #endif

	return result;
}

inline void setup_arena(MemoryArena *arena, size_t size, MemoryPushParams params) {
	assert(arena->block == 0 && "Arena already setup!");
	_push_size(arena, size, params);
	arena->block->offset = 0;
}

inline void reset_arena(MemoryArena *arena, size_t size, MemoryPushParams params) {
	arena->block = 0;
	setup_arena(arena, size, params);
}

#define PUSH_STRUCT(arena, type, params) (type *)_push_size(arena, sizeof(type), params)
#define PUSH_STRUCTS(arena, count, type, params) (type *)_push_size(arena, (size_t)count * sizeof(type), params)
#define PUSH_STRING(arena, count, params) (char *)_push_size(arena, (size_t)count * sizeof(char), params)
#define PUSH_SIZE(arena, size, params) _push_size(arena, size, params)

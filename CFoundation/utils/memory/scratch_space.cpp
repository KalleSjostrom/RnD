// Where simple "allocation". Used for temporary memory.
struct ScratchSpace {
	size_t offset;
	char *memory;

	size_t _DEBUG_maxsize;
};

namespace {
	static inline int ss_is_aligned(void *p, size_t align)
	{
		return !( (uintptr_t)p & (align-1) );
	}

	static inline unsigned ss_bytes_to_aligned_address(const void *p, size_t align)
	{
		uintptr_t pi = (uintptr_t)p;
		return (unsigned)(((pi + align - 1)/align)*align - pi);
	}
}

namespace scratch_space {
	inline ScratchSpace init(void *memory, size_t size) {
		ScratchSpace scratch_space = {0};

		scratch_space.memory = (char *)memory;
		scratch_space.offset = 0;
		scratch_space._DEBUG_maxsize = size;

		return scratch_space;
	}
	inline char *allocate(ScratchSpace &scratch_space, size_t size, size_t align = sizeof(void*)) {
		char *memory = scratch_space.memory + scratch_space.offset;
		if (!ss_is_aligned(memory, align)) {
			unsigned num_bytes_to_alignment = ss_bytes_to_aligned_address(memory, align);
			memory += num_bytes_to_alignment;
			scratch_space.offset += num_bytes_to_alignment;
		}

		scratch_space.offset += size;
		ASSERT(scratch_space.offset <= scratch_space._DEBUG_maxsize, "No more memory in scratch space!");
		return memory;
	}
	inline void clear(ScratchSpace &scratch_space) {
		scratch_space.offset = 0;
	}
}
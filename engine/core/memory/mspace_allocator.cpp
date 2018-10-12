#define ONLY_MSPACES 1
#define USE_DL_PREFIX 1

#include "mspace_allocator.h"

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#pragma warning(disable:4574) // 'DEBUG' is defined to be '0': did you mean to use '#if DEBUG'?
#define ONLY_MSPACES 1
#define MSPACES 1
#define PROCEED_ON_ERROR 1
#define USE_DL_PREFIX 1
#include "../third_party/dlmalloc.c"
#pragma warning(pop)

struct MSpaceAllocator {
	mspace _mspace;
};

void *allocate(MSpaceAllocator *a, size_t size, bool clear_to_zero, unsigned alignment) {
	void *result = mspace_memalign(a->_mspace, alignment, size);
	if (clear_to_zero) {
		memset(result, 0, size);
	}
	return result;
}
void *realloc(MSpaceAllocator *a, void *p, size_t new_size) {
	return mspace_realloc(a->_mspace, p, new_size);
}
void deallocate(MSpaceAllocator *a, void *p) {
	mspace_free(a->_mspace, p);
}
void destroy(MSpaceAllocator *a) {
	destroy_mspace(a->_mspace);
}

MSpaceAllocator *mspace_allocator(size_t capacity) {
	MSpaceAllocator a = {};
	a._mspace = create_mspace(capacity, 0);

	MSpaceAllocator *storage = (MSpaceAllocator*)allocate(&a, sizeof(MSpaceAllocator));
	*storage = a;
	return storage;
}

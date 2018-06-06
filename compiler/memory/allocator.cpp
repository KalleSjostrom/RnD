#pragma once

struct Allocator {
	void *mspace;
};

void init_allocator(Allocator &a, void *p, size_t size) {
	a.mspace = create_mspace_with_base(p, size);
}

void *allocate(Allocator &a, size_t size) {
	return mspace_malloc(a.mspace, size);
}

void deallocate(Allocator &a, void * p) {
	mspace_free(a.mspace, p);
}

size_t size_of_top_chunk(Allocator &a) {
	return ((mstate)a.mspace)->topsize;
}

#pragma once

class AhAllocator
{
public:
	AhAllocator(void *p, size_t size) : _base(p), _size(size) {
		_mspace = create_mspace_with_base(_base, size);
	}
	~AhAllocator() {}

	void *allocate(size_t size, unsigned align) { (void)align; return mspace_malloc(_mspace, size); }
	void deallocate(void * p) { mspace_free(_mspace, p); }
	size_t size_of_top_chunk() { return ((mstate)_mspace)->topsize; }

	void *mspace() { return _mspace; }

private:
	void   *_base;
	size_t _size;
	void   *_mspace;
};

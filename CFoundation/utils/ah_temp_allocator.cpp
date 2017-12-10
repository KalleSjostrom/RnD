class AhTempAllocator
{
public:
	AhTempAllocator() {
		_offset = globals::scratch_space.offset;
	}
	~AhTempAllocator() {
		globals::scratch_space.offset = _offset;
	}

	void *allocate(size_t size, unsigned align = 4) {
		return scratch_space::allocate(globals::scratch_space, size, align);
	}
	size_t deallocate(void *p) { (void)p; return 0; }

private:
	size_t _offset;
};
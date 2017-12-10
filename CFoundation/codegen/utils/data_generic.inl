struct Array {
	Data *entries;
	unsigned count;
	unsigned debug_max_count; // Used for range checks
	bool changed;
	bool has_changed() { return changed || count == 0; }

	inline char operator[](int index) { return entries[index]; }
	Data *add_new() {
		ARRAY_CHECK_BOUNDS_COUNT(entries, debug_max_count);
		return entries + count++;
	}
};
__forceinline size_t get_size(unsigned count) {
	return count*sizeof(Data);
}
__forceinline Array make_array(MemoryArena &arena, unsigned count) {
	Data *entries = (Data*)allocate_memory(arena, get_size(count));
	Array array = { entries, 0, count, false };
	return array;
}

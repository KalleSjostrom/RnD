
struct ARRAY_NAME {
	ARRAY_TYPE entries[ARRAY_MAX_SIZE];
	unsigned count;

	inline ARRAY_TYPE &operator[](int index) { return entries[index]; }
	inline ARRAY_TYPE &new_entry() {
		ASSERT(count < ARRAY_MAX_SIZE, "Array index out of bounds!")
		return entries[count++];
	}
	inline void push_back(ARRAY_TYPE &data) {
		ASSERT(count < ARRAY_MAX_SIZE, "Array index out of bounds!")
		entries[count++] = data;
	}
};

#undef ARRAY_NAME
#undef ARRAY_TYPE
#undef ARRAY_MAX_SIZE

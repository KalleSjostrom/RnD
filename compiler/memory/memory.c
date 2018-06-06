#pragma once

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

#pragma once

#include <cassert>
#include <cstdlib>
#include <algorithm>

#if defined(FTL_VALGRIND)
	#include <valgrind/valgrind.h>
#endif

#if defined(FTL_FIBER_STACK_GUARD_PAGES)
	#if defined(FTL_OS_LINUX) || defined(FTL_OS_MAC) || defined(FTL_iOS)
		#include <sys/mman.h>
		#include <unistd.h>
	#elif defined(FTL_OS_WINDOWS)
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
	#endif
#endif

#if defined(FTL_FIBER_STACK_GUARD_PAGES)
	#if defined(FTL_OS_LINUX) || defined(FTL_OS_MAC) || defined(FTL_iOS)
		inline void memory_protect(void *memory, size_t bytes) {
			int result = mprotect(memory, bytes, PROT_NONE);
			assert(!result);
		}

		inline void memory_unprotect(void *memory, size_t bytes) {
			int result = mprotect(memory, bytes, PROT_READ | PROT_WRITE);
			assert(!result);
		}

		inline size_t get_page_size() {
			int page_size = getpagesize();
			return (size_t)page_size;
		}

		inline void *aligned_allocation(size_t size, size_t alignment) {
			void *returnPtr;
			posix_memalign(&returnPtr, alignment, size);

			return returnPtr;
		}

		inline void aligned_free(void *block) {
			free(block);
		}
	#elif defined(FTL_OS_WINDOWS)
		inline void memory_protect(void *memory, size_t bytes) {
			DWORD ignored;

			BOOL result = VirtualProtect(memory, bytes, PAGE_NOACCESS, &ignored);
			assert(result);
		}

		inline void memory_unprotect(void *memory, size_t bytes) {
			DWORD ignored;

			BOOL result = VirtualProtect(memory, bytes, PAGE_READWRITE, &ignored);
			assert(result);
		}

		inline size_t get_page_size() {
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return sysInfo.dwPageSize;
		}

		inline void *aligned_allocation(size_t size, size_t alignment) {
			return _aligned_malloc(size, alignment);
		}

		inline void aligned_free(void *block) {
			_aligned_free(block);
		}
	#else
		#error "Need a way to protect memory for this platform".
	#endif
#else
	inline void memory_protect(void *memory, size_t bytes) {
		(void)memory;
		(void)bytes;
	}

	inline void memory_unprotect(void *memory, size_t bytes) {
		(void)memory;
		(void)bytes;
	}

	inline size_t get_page_size() {
		return 0;
	}

	inline void *aligned_allocation(size_t size, size_t alignment) {
		return malloc(size);
	}

	inline void aligned_free(void *block) {
		free(block);
	}
#endif

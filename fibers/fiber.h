#pragma once

#include <cassert>
#include <cstdlib>
#include <algorithm>

#if defined(FTL_FIBER_STACK_GUARD_PAGES)
	#if defined(FTL_OS_LINUX) || defined(FTL_OS_MAC) || defined(FTL_iOS)
		#include <sys/mman.h>
		#include <unistd.h>
	#elif defined(FTL_OS_WINDOWS)
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
	#endif
#endif

#include "memory.h"

inline size_t round_up(size_t numToRound, size_t multiple) {
	if (multiple == 0) {
		return numToRound;
	}

	size_t remainder = numToRound % multiple;
	if (remainder == 0)
		return numToRound;

	return numToRound + multiple - remainder;
}

typedef void *fcontext_t;

extern "C" void jump_fcontext(fcontext_t *from, fcontext_t to, void *arg);
extern "C" fcontext_t make_fcontext(void * stack_pointer, size_t size, void(*func)(void *));
// stack_pointer is the pointer to the _top_ of the stack (ie &stack_buffer[size]).

typedef void (*FiberRoutine)(void *arg);

struct Fiber {
	// Allocates a stack and sets it up to start executing 'routine' when first switched to
	void setup(size_t wanted_stack_size, FiberRoutine routine, void *arguments) {
		arg = arguments;
		#if defined(FTL_FIBER_STACK_GUARD_PAGES)
			system_page_size = get_page_size();
		#else
			system_page_size = 0;
		#endif

		stack_size = round_up(wanted_stack_size, system_page_size);
		// We add a guard page both the top and the bottom of the stack
		stack = aligned_allocation(system_page_size + stack_size + system_page_size, system_page_size);

		// Setup the assembly stack with make_x86_64_sysv_macho_gas.S
		// The stack grows "downwards" from high memory address to low, so set the start at the highest address.
		char *stack_start = ((char *)stack) + system_page_size + stack_size;
		context = make_fcontext(stack_start, stack_size, routine);

		#if defined(FTL_FIBER_STACK_GUARD_PAGES)
			memory_protect((char *)(stack), system_page_size);
			memory_protect((char *)(stack) + system_page_size + stack_size, system_page_size);
		#endif
	}

	void destroy() {
		if (stack != NULL) {
			if (system_page_size != 0) {
				memory_unprotect((char *)(stack), system_page_size);
				memory_unprotect((char *)(stack) + system_page_size + stack_size, system_page_size);
			}
			// FTL_VALGRIND_DEREGISTER();

			aligned_free(stack);
		}
	}

	void *stack;
	size_t system_page_size;
	size_t stack_size;
	fcontext_t context;
	void *arg;

	// Saves the current stack context and then switches to the given fiber. Execution will resume here once another fiber switches to this fiber
	void switch_to_fiber(Fiber *fiber) {
		jump_fcontext(&context, fiber->context, fiber->arg);
	}

	// Re-initializes the stack with a new routine and arg
	void reset(FiberRoutine routine, void *arguments) {
		context = make_fcontext(((char *)stack) + system_page_size + stack_size, stack_size, routine);
		arg = arguments;
	}

#if 0
private:
	/**
	* Helper function for the move operators
	* Swaps all the member variables
	*
	* @param first     The first fiber
	* @param second    The second fiber
	*/
	void swap(Fiber &first, Fiber &second) {
		using std::swap;

		swap(first.stack, second.stack);
		swap(first.system_page_size, second.system_page_size);
		swap(first.stack_size, second.stack_size);
		swap(first.context, second.context);
		swap(first.arg, second.arg);
	}
#endif
};

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
	void *stack;
	size_t system_page_size;
	size_t stack_size;
	fcontext_t context;
	void *arg;
};

// Allocates a stack and sets it up to start executing 'routine' when first switched to
void fiber_setup(Fiber &fiber, size_t wanted_stack_size, FiberRoutine routine, void *arguments) {
	fiber.arg = arguments;
	#if defined(FTL_FIBER_STACK_GUARD_PAGES)
		fiber.system_page_size = get_page_size();
	#else
		fiber.system_page_size = 0;
	#endif

	fiber.stack_size = round_up(wanted_stack_size, fiber.system_page_size);
	// We add a guard page both the top and the bottom of the stack
	fiber.stack = aligned_allocation(fiber.system_page_size + fiber.stack_size + fiber.system_page_size, fiber.system_page_size);

	// Setup the assembly stack with make_x86_64_sysv_macho_gas.S
	// The stack grows "downwards" from high memory address to low, so set the start at the highest address.
	char *stack_start = ((char *)fiber.stack) + fiber.system_page_size + fiber.stack_size;
	fiber.context = make_fcontext(stack_start, fiber.stack_size, routine);

	#if defined(FTL_FIBER_STACK_GUARD_PAGES)
		memory_protect((char *)(fiber.stack), fiber.system_page_size);
		memory_protect((char *)(fiber.stack) + fiber.system_page_size + fiber.stack_size, fiber.system_page_size);
	#endif
}

// Saves the current stack context and then switches to the given fiber. Execution will resume here once another fiber switches to this fiber
void fiber_switch(Fiber &source, Fiber &dest) {
	jump_fcontext(&source.context, dest.context, dest.arg);
}

// Re-initializes the stack with a new routine and arg
void fiber_reset(Fiber &fiber, FiberRoutine routine, void *arguments) {
	fiber.context = make_fcontext(((char *)fiber.stack) + fiber.system_page_size + fiber.stack_size, fiber.stack_size, routine);
	fiber.arg = arguments;
}

void fiber_destroy(Fiber &fiber) {
	if (fiber.stack != NULL) {
		if (fiber.system_page_size != 0) {
			memory_unprotect((char *)(fiber.stack), fiber.system_page_size);
			memory_unprotect((char *)(fiber.stack) + fiber.system_page_size + fiber.stack_size, fiber.system_page_size);
		}
		// FTL_VALGRIND_DEREGISTER();

		aligned_free(fiber.stack);
	}
}

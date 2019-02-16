#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

// void print_callstack(unsigned skip_frames = 0);
void report_script_assert_failure(int skip_frames, int line, const char *file, const char *assert, const char *format, ...);
LONG WINAPI exception_filter(EXCEPTION_POINTERS *ep);
void error_init();
void error_deinit();

#define STATIC_ASSERT(condition, msg) typedef i32 my_static_assert[(condition) ? 1 : -1]
#define ASSERT(condition, format, ...) do { \
	if (!(condition)) {\
		report_script_assert_failure(0, __LINE__, __FILE__, #condition, format, __VA_ARGS__); \
		if (IsDebuggerPresent()) {DebugBreak();} else { *(volatile int*)0 = 5; } \
	} \
} while(0)

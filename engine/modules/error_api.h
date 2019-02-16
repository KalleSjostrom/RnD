#pragma once

struct ErrorApi {
	void (*report_assert_failure)(int skip_frames, int line, const char *file, const char *assert, const char *format, ...);
};

#if defined(PLUGIN)
	static void (*_global_report_assert_failure)(int skip_frames, int line, const char *file, const char *assert, const char *format, ...);
	void global_set_error(ErrorApi &error) {
		_global_report_assert_failure = error.report_assert_failure;
	}
	#define STATIC_ASSERT(condition, msg) typedef i32 my_static_assert[(condition) ? 1 : -1]
	#define ASSERT(condition, format, ...) do { \
		if (!(condition)) {\
			_global_report_assert_failure(0, __LINE__, __FILE__, #condition, format, __VA_ARGS__); \
			if (IsDebuggerPresent()) {DebugBreak();} else { *(volatile int*)0 = 5; } \
		} \
	} while(0)
#endif

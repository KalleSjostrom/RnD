#pragma once

extern void report_assert_failure(int line, const char *file, const char *assert, const char *msg);

static char _assert_string[512];
#define ASSERT(arg, format, ...) do { \
	if (!(arg)) { \
		_snprintf_s(_assert_string, sizeof(_assert_string), _TRUNCATE, format"\n", ##__VA_ARGS__); \
		report_assert_failure(__LINE__, __FILE__, (#arg), _assert_string); \
	} \
} while(0)

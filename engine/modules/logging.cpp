#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "common.h"

#include <sys/timeb.h>
#include <time.h>

#include "core/memory/allocator.h"
#include "core/memory/arena_allocator.h"

struct TicketMutex {
	i64 volatile ticket;
	i64 volatile serving;
};
__forceinline void acquire_ticket(TicketMutex &mutex) {
	i64 ticket = InterlockedExchangeAdd64(&mutex.ticket, 1);
	while (ticket != mutex.serving);
}
__forceinline void release_ticket(TicketMutex &mutex) {
	InterlockedExchangeAdd64(&mutex.serving, 1);
}

/////////// Service memory ///////////
struct Logger {
	Allocator allocator;
	TicketMutex mutex;
	HANDLE output_file;
};
// TODO(kalle): Good way to store this!
static Logger _log_inst = {};

__declspec(thread) char _log_buffer[2048];
__declspec(thread) char _log_header[512];
__declspec(thread) ArenaAllocator _log_arena;

ArenaAllocator _log_scratch_space;

enum LogLevel {
	LogLevel_Info,
	LogLevel_Warning,
	LogLevel_Error,
};

/////////// Logging API Implementation ///////////
void _log(LogLevel level, const char *system, const char *format, va_list args) {
	// TODO(kalle): Use the logging level!
	(void) level;

	int output_length = vsnprintf(_log_buffer, ARRAY_COUNT(_log_buffer), format, args);

	char *output = _log_buffer;
	if (output_length > ARRAY_COUNT(_log_buffer)) { // We didn't fit :(
		char *buf = (char*)allocate(&_log_arena, output_length);
		vsnprintf(buf, output_length, format, args);

		output = buf;
	}

	__timeb64 time_stamp;
	_ftime64_s(&time_stamp);
	tm time_format;
	_localtime64_s(&time_format, &time_stamp.time);

	int header_length = snprintf(_log_header, ARRAY_COUNT(_log_header), "%02u:%02u:%02u.%03u [%s] ", time_format.tm_hour, time_format.tm_min, time_format.tm_sec, time_stamp.millitm, system);

	acquire_ticket(_log_inst.mutex);
		size_t total_length = header_length + output_length + 2;
		char *at = (char*)allocate(&_log_scratch_space, total_length, false, 1);

		unsigned cursor = 0;
		for (int i = 0; i < header_length; ++i) { at[cursor++] = _log_header[i]; }
		for (int i = 0; i < output_length; ++i) { at[cursor++] = output[i]; }
		at[cursor++] = '\r';
		at[cursor++] = '\n';
	release_ticket(_log_inst.mutex);

	if (output_length > ARRAY_COUNT(_log_buffer)) { // Did we use our _log_arena?
		reset(&_log_arena);
	}
}

void log_info(const char *system, const char *format, ...) {
	va_list args;
	va_start(args, format);
	_log(LogLevel_Info, system, format, args);
	va_end(args);
}
void log_warning(const char *system, const char *format, ...) {
	va_list args;
	va_start(args, format);
	_log(LogLevel_Warning, system, format, args);
	va_end(args);
}
void log_error(const char *system, const char *format, ...) {
	va_list args;
	va_start(args, format);
	_log(LogLevel_Error, system, format, args);
	va_end(args);
}

void log_init() {
	_log_inst.allocator = allocator_mspace();
	_log_inst.mutex = {};
	_log_inst.output_file = CreateFile("../output/logs/log.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
}

void log_deinit() {
	CloseHandle(_log_inst.output_file);
	destroy(&_log_inst.allocator);
}

void log_update() {
	if (_log_scratch_space.offset > 0) {
		acquire_ticket(_log_inst.mutex);

		size_t size = _log_scratch_space.offset;

		bool output_file = true;
		bool output_debugger = true;
		bool output_console = true;

		if (output_debugger || output_console) {
			// Make sure we have room for the null terminator, for the functions that need them.
			char *at = (char*)allocate(&_log_scratch_space, 1); // null terminator
			*at = '\0';
		}

		if (output_file) {
			WriteFile(_log_inst.output_file, _log_scratch_space.memory, (DWORD)size, 0, 0);
		}
		if (output_debugger) {
			OutputDebugStringA((char*)_log_scratch_space.memory);
		}
		if (output_console) {
			puts((char*)_log_scratch_space.memory);
		}

		_log_scratch_space.offset = 0;
		release_ticket(_log_inst.mutex);
	}
}

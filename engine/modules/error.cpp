#include "common.h"
#include "error.h"

#include <psapi.h>
#include <stdio.h>

#include "core/memory/allocator.h"
#include "core/utils/dynamic_string.h"
#include "logging.h"

const char *exception_name(DWORD type) {
	switch(type) {
		case EXCEPTION_ACCESS_VIOLATION: return "Access violation";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "Datatype misalignment";
		case EXCEPTION_BREAKPOINT: return "Breakpoint";
		case EXCEPTION_SINGLE_STEP: return "Single step";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array bounds exceeded";
		case EXCEPTION_FLT_DENORMAL_OPERAND: return "Float denormal operand";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Float divide by zero";
		case EXCEPTION_FLT_INEXACT_RESULT: return "Float inexact result";
		case EXCEPTION_FLT_INVALID_OPERATION: return "Float invalid operation";
		case EXCEPTION_FLT_OVERFLOW: return "Float overflow";
		case EXCEPTION_FLT_STACK_CHECK: return "Float stack overflow";
		case EXCEPTION_FLT_UNDERFLOW: return "Float underflow";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer divide by zero";
		case EXCEPTION_INT_OVERFLOW: return "Integer overflow";
		case EXCEPTION_PRIV_INSTRUCTION: return "Priviliged instruction";
		case EXCEPTION_IN_PAGE_ERROR: return "In page error";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Noncontinuable exception";
		case EXCEPTION_STACK_OVERFLOW: return "Stack overflow";
		case EXCEPTION_INVALID_DISPOSITION: return "Invalid disposition";
		case EXCEPTION_GUARD_PAGE: return "Guard page";
		case EXCEPTION_INVALID_HANDLE: return "Invalid handle";
		case 0xc00002b5: return "Multiple floating point exceptions";
		// default: return "Unhandled exception";
	}

	static char buffer[64] = {};
	_snprintf_s(buffer, 64, _TRUNCATE, "Unhandled exception: %lu", type);
	return buffer;
}

struct SymbolInfoPackage : public SYMBOL_INFO_PACKAGE {
	SymbolInfoPackage() {
		si.SizeOfStruct = sizeof(SYMBOL_INFO);
		si.MaxNameLen   = sizeof(name);
	}
};

CRITICAL_SECTION __cs;

void _fill_stacktrace(CONTEXT &context, unsigned skip_frames = 0) {
#ifndef _M_X64
	static_assert(false, "The crash_handler.cpp is only supported for x64.");
#endif
	DWORD machine_type = IMAGE_FILE_MACHINE_AMD64;
	STACKFRAME64 stack_frame;

	// Setup the stackframe.
	stack_frame.AddrPC.Offset    = context.Rip;
	stack_frame.AddrPC.Mode      = AddrModeFlat;
	stack_frame.AddrFrame.Offset = context.Rbp;
	stack_frame.AddrFrame.Mode   = AddrModeFlat;
	stack_frame.AddrStack.Offset = context.Rsp;
	stack_frame.AddrStack.Mode   = AddrModeFlat;

	ArenaAllocator arena = {};
	Allocator a = allocator_arena(&arena);
	DynamicString buffer = dynamic_string(&a, 1024);


	// FULKOD(kalle): StalkWalk64 will sometimes skip the first frame, i.e. the actual place where the crash occured.
	// context.Rip contains the correct address for the instruction pointer and context.Rbp/context.Rsp contains correct values as well.
	// Still, after the first call to StalkWalk64, the context.Rip (and stack_frame.AddrPC.Offset) are set to the second call stack frame.
	// To circumvent this, we verify that Rip hasn't been changed after the first call. If it has, then ignore StalkWalk64 and print the first frame anyway.
	//
	// This can be noticed in Visual Studio as well, where it will jump and highligth the second entry in the callstack and not the top one.
	//
	// I don't know why this is happening...
	bool ugly_workaround_for_stackwalk_missing_the_first_frame = true;
	CONTEXT context_copy = context;

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	while (StackWalk64(machine_type, process, thread, &stack_frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
		if (ugly_workaround_for_stackwalk_missing_the_first_frame) {
			if (stack_frame.AddrPC.Offset != context_copy.Rip) {
				context = context_copy;
				stack_frame.AddrPC.Offset    = context.Rip;
				stack_frame.AddrPC.Mode      = AddrModeFlat;
				stack_frame.AddrFrame.Offset = context.Rbp;
				stack_frame.AddrFrame.Mode   = AddrModeFlat;
				stack_frame.AddrStack.Offset = context.Rsp;
				stack_frame.AddrStack.Mode   = AddrModeFlat;
			}
			ugly_workaround_for_stackwalk_missing_the_first_frame = false;
		}

		if (skip_frames) {
			skip_frames--;
			continue;
		}

		IMAGEHLP_STACK_FRAME stack_frame_helper = { 0 };
		stack_frame_helper.InstructionOffset = stack_frame.AddrPC.Offset;
		SymSetContext(process, &stack_frame_helper, NULL);

		{ // lookup_file_line
			IMAGEHLP_LINE64 line;
			line.SizeOfStruct = sizeof(line);
			DWORD displacement = 0;

			// Since the instruction pointer is incremented directly ofter the call instruction, it will point to the next instruction after the call.
			// To get the correct line number, we need to back one byte te force the symbol lookup to fetch the line number of the call instruction.
			DWORD64 previous_instruction_offset = stack_frame.AddrPC.Offset - 1;
			if (SymGetLineFromAddr64(process, previous_instruction_offset, &displacement, &line)) {
				// produce line number
				if (line.LineNumber > 99999)
					line.LineNumber = 0;

				printf(buffer, "%s(%d): ", line.FileName, line.LineNumber);
			}
		}

		{ // auto lookup_symbol = [](const void * addr, StringStream &os)
			SymbolInfoPackage symbol_info_package;
			SYMBOL_INFO *symbol_info = &symbol_info_package.si;
			DWORD64 displacement = 0;
			if (SymFromAddr(process, stack_frame.AddrPC.Offset, &displacement, symbol_info)) {
				// try to undecorate
				char undecorated[512];
				if (UnDecorateSymbolName(symbol_info->Name, undecorated, 512, UNDNAME_COMPLETE)) {
					buffer += undecorated;
				} else {
					buffer += symbol_info->Name;
				}
				// if (strcmp("WinMain", symbol_info->Name) == 0)
				//	break;
			}
			buffer += "\n";
		};
	}

	log_error("CrashHandler", "Callstack:\n%.*s\n\n", STR(buffer));
	destroy(&a);
}

void _setup_stack_walk(CONTEXT &context, unsigned skip_frames = 0) {
	auto enumerate_modules_callback = [](PCSTR module_name, DWORD64 module_base, ULONG module_size, PVOID user_context) {
		DWORD64 image_base = SymLoadModuleEx(GetCurrentProcess(), 0, module_name, 0, module_base, module_size, 0, 0);
		if (image_base == 0)
			return 1;

		// touch symbols to force a load of deferred symbols
		IMAGEHLP_MODULEW64 info;
		memset(&info, 0, sizeof(info));
		info.SizeOfStruct = sizeof(info);
		BOOL success = SymGetModuleInfoW64(GetCurrentProcess(), module_base, &info);
		(void)success;
		return 1;
	};
	EnumerateLoadedModules64(GetCurrentProcess(), enumerate_modules_callback, nullptr);

	_fill_stacktrace(context, skip_frames);
}

void write_crash_dump(const char *path, EXCEPTION_POINTERS *ep) {
	MINIDUMP_EXCEPTION_INFORMATION mei;
	mei.ThreadId = GetCurrentThreadId();
	mei.ExceptionPointers = ep;
	mei.ClientPointers = FALSE;

	// TODO(kalle): Where do we place the dumps? Do we need to copy all dll/pdbs too? Can we bundle them into the dump?
	HANDLE handle = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);

	bool full_dump = true;
	MINIDUMP_TYPE type = full_dump ? MiniDumpWithFullMemory : MiniDumpNormal;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), handle, type, ep ? &mei : 0, 0, 0);

	CloseHandle(handle);
}

void print_callstack(int line, const char *file, const char *assert, const char *msg, unsigned skip_frames) {
	log_error("CrashHandler", "Assertion failed at: %s:%d\n\n%s\n%s", file, line, assert, msg);

	CONTEXT context;
	RtlCaptureContext(&context);

	{
		EXCEPTION_RECORD er = {};

		er.ExceptionCode = EXCEPTION_BREAKPOINT;
		// er.ExceptionAddress = ; // TODO(kalle): Use stackwalk once, to get the callee, (or pass it into the existing stackwalk)

		EXCEPTION_POINTERS ep;
		ep.ContextRecord = &context;
		ep.ExceptionRecord = &er;

		static char buffer[512] = {};
		String filename = get_filename(string(file), true);
		_snprintf_s(buffer, 512, _TRUNCATE, "../output/asserts/%.*s_%d.dmp", STR(filename), line);

		write_crash_dump(buffer, &ep); // NOTE(kalle): This needs to go before the other stack walk, otherwise the dump will not contain the actual crash info.
	}

	_setup_stack_walk(context, skip_frames + 1); // Skip this frame too

	log_update();
}

__declspec(thread) char _assert_message_buffer[2048];
void report_script_assert_failure(int skip_frames, int line, const char *file, const char *assert, const char *format, ...) {
	EnterCriticalSection(&__cs);

	va_list args;
	va_start(args, format);
	int output_length = vsnprintf(_assert_message_buffer, ARRAY_COUNT(_assert_message_buffer), format, args);
	(void)output_length;
	va_end(args);

	log_error("CrashHandler", "Assertion failed at: %s:%d\n\n%s\n%s", file, line, assert, _assert_message_buffer);

	CONTEXT context;
	RtlCaptureContext(&context);

	{
		EXCEPTION_RECORD er = {};

		er.ExceptionCode = EXCEPTION_BREAKPOINT;
		// er.ExceptionAddress = ; // TODO(kalle): Use stackwalk once, to get the callee, (or pass it into the existing stackwalk)

		EXCEPTION_POINTERS ep;
		ep.ContextRecord = &context;
		ep.ExceptionRecord = &er;

		static char buffer[512] = {};
		String filename = get_filename(string(file), true);
		_snprintf_s(buffer, 512, _TRUNCATE, "../output/asserts/%.*s_%d.dmp", STR(filename), line);

		write_crash_dump(buffer, &ep); // NOTE(kalle): This needs to go before the other stack walk, otherwise the dump will not contain the actual crash info.
	}

	_setup_stack_walk(context, skip_frames + 1); // Skip this frame too

	LeaveCriticalSection(&__cs);

	log_update();
}

LONG WINAPI exception_filter(EXCEPTION_POINTERS *ep) {
	DWORD code = ep->ExceptionRecord->ExceptionCode;
	if (code == 0xE06D7363) { // We assume (for the time being) that c++ exceptions are asserts...
		return EXCEPTION_EXECUTE_HANDLER;
	}

	write_crash_dump("../output/crashes/crash.dmp", ep); // NOTE(kalle): This needs to go before the other stack walk, otherwise the dump will not contain the actual crash info.

	const char *exception_string = exception_name(ep->ExceptionRecord->ExceptionCode);
	log_error("CrashHandler", "Exception: %s\n", exception_string);

	CONTEXT &context = *ep->ContextRecord;
	EnterCriticalSection(&__cs);
	_setup_stack_walk(context);
	LeaveCriticalSection(&__cs);
	log_update();
	return EXCEPTION_EXECUTE_HANDLER;
}

void error_init() {
	InitializeCriticalSectionAndSpinCount(&__cs, 64 /*spin_count*/);

	LPTOP_LEVEL_EXCEPTION_FILTER pfilter = SetUnhandledExceptionFilter(&exception_filter);
	(void)pfilter;
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_FAIL_CRITICAL_ERRORS);
	SymInitialize(GetCurrentProcess(), 0, TRUE);
}

void error_deinit() {
	SetUnhandledExceptionFilter(0);
	SymSetOptions(0);
	SymCleanup(GetCurrentProcess());
	DeleteCriticalSection(&__cs);
}

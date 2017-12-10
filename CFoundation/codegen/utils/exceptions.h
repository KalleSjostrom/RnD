#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#include <stdio.h>

#ifndef _snprintf
#define snprintf _snprintf
#endif

bool __initialized = false;
CRITICAL_SECTION __cs;

const char *exception_name(DWORD type)
{
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
	snprintf(buffer, 64, "Unhandled exception: %d", type);
	return buffer;
}

const char *address_str(DWORD64 address) {
	static char buffer[1024] = {};
	buffer[0] = 0;
	snprintf(buffer, 1024, "0x%08x ", (unsigned int)address);
	return buffer;
}

const char *address_symbol_str(DWORD64 address) {
	static char buffer[1024] = {};
	buffer[0] = 0;

	char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)symbol_buffer;
	symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol_info->MaxNameLen = MAX_SYM_NAME;

	DWORD64 displacement = 0;
	if (SymFromAddr(GetCurrentProcess(), address, &displacement, symbol_info)) {
		if (!UnDecorateSymbolName(symbol_info->Name, buffer, 512, UNDNAME_COMPLETE))
			snprintf(buffer, 1024, symbol_info->Name);
	} else {
		return address_str(address);
	}
	return buffer;
}

const char *address_line_str(DWORD64 address) {
	static char buffer[1024] = {};
	buffer[0] = 0;

	DWORD displacement = 0;
	IMAGEHLP_LINE64 line = {};
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	if (SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &line)) {
		if (line.LineNumber > 99999)
			line.LineNumber = 0;
		snprintf(buffer, 1024, "%s(%d)", line.FileName, line.LineNumber);
	} else {
		IMAGEHLP_MODULE64 module_info = {};
		module_info.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
		if (SymGetModuleInfo64(GetCurrentProcess(), address, &module_info)) {
			snprintf(buffer, 1024, module_info.ImageName);
		} else {
			return address_symbol_str(address);
		}
	}
	return buffer;
}


// Structured Exception Handler
LONG WINAPI exception_filter(EXCEPTION_POINTERS *info) {
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;

	EnterCriticalSection(&__cs);

	const char *exception_type = exception_name(info->ExceptionRecord->ExceptionCode);
	DWORD64 address = (DWORD64)info->ExceptionRecord->ExceptionAddress;

	if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
		printf("%s : %s : Attempting to access address %p from %p\n",
		       address_line_str(address),
		       exception_type,
		       (void *)info->ExceptionRecord->ExceptionInformation[1],
		       info->ExceptionRecord->ExceptionAddress);
	} else {
		printf("%s : %s\n", address_line_str(address), exception_type);
	}

	STACKFRAME64 stack_frame;
	memset(&stack_frame, 0, sizeof(STACKFRAME64));
	stack_frame.AddrPC.Offset    = info->ContextRecord->Rip;
	stack_frame.AddrPC.Mode      = AddrModeFlat;
	stack_frame.AddrFrame.Offset = info->ContextRecord->Rbp;
	stack_frame.AddrFrame.Mode   = AddrModeFlat;
	stack_frame.AddrStack.Offset = info->ContextRecord->Rsp;
	stack_frame.AddrStack.Mode   = AddrModeFlat;

	for (int i = 0; i < 64; ++i) {
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(),
		    &stack_frame, info->ContextRecord, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0))
			break;
		if (stack_frame.AddrPC.Offset == 0)
			break;

		DWORD64 address = stack_frame.AddrPC.Offset;
		printf("\t[%s] %s : %s\n", address_str(address), address_line_str(address), address_symbol_str(address));
	}

	return EXCEPTION_EXECUTE_HANDLER; // EXCEPTION_CONTINUE_EXECUTION, EXCEPTION_CONTINUE_SEARCH
}

void exceptions_setup() {
	if (__initialized)
		return;

	static const DWORD spin_count = 64;
	(void)InitializeCriticalSectionAndSpinCount(&__cs, spin_count);

	LPTOP_LEVEL_EXCEPTION_FILTER pfilter = SetUnhandledExceptionFilter(&exception_filter);
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_FAIL_CRITICAL_ERRORS);
	SymInitialize(GetCurrentProcess(), 0, TRUE);

	__initialized = true;
}

void exceptions_shutdown() {
	if (!__initialized)
		return;
	SetUnhandledExceptionFilter(0);
	SymSetOptions(0);
	SymCleanup(GetCurrentProcess());
	DeleteCriticalSection(&__cs);
}



#endif // EXCEPTIONS_H

#if defined(WINDOWSPC) && defined(USE_CALLSTACK)

namespace {
	#include "arrowhead_cvconst.h"

	struct UserContext {
		STACKFRAME64 *stack_frame;
		StringStream *string_stream;
	};

	struct SymbolInfoPackage : public SYMBOL_INFO_PACKAGEW {
		SymbolInfoPackage() {
			si.SizeOfStruct = sizeof(SYMBOL_INFOW);
			si.MaxNameLen   = sizeof(name);
		}
	};

	BOOL CALLBACK enum_symbol_callback(SYMBOL_INFO *symbol_info, ULONG symbol_size, PVOID user_data) {
		if (symbol_info) {
			UserContext context = *(UserContext*)user_data;
			STACKFRAME64 &stack_frame = *context.stack_frame;
			StringStream &string_stream = *context.string_stream;

			enum SymTagEnum tag = (enum SymTagEnum)0;
			BOOL result = SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, symbol_info->TypeIndex, TI_GET_SYMTAG, &tag);
			DWORD ErrCode = GetLastError();

			TypeInfo type_info = {};
			TypeInfo child_type_info = {};
			if (tag == SymTagBaseType) {
				BasicType bt = (BasicType)0;
				SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, symbol_info->TypeIndex, TI_GET_BASETYPE, &bt);
				ULONG64 length = 0;
				SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, symbol_info->TypeIndex, TI_GET_LENGTH, &length);
				type_info = get_type_info_basic(bt, length);
			} else if (tag == SymTagPointerType) {
				type_info = make_type("pointer", AhType_Pointer);

				DWORD type_id;
				SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, symbol_info->TypeIndex, TI_GET_TYPEID, &type_id);

				BOOL result = SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, type_id, TI_GET_SYMTAG, &tag);
				if (tag == SymTagBaseType) {
					BasicType bt = (BasicType)0;
					SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, type_id, TI_GET_BASETYPE, &bt);
					ULONG64 length = 0;
					SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, type_id, TI_GET_LENGTH, &length);
					child_type_info = get_type_info_basic(bt, length);
				}
			} else if (tag == SymTagUDT) {
				WCHAR* name;
				SymGetTypeInfo(GetCurrentProcess(), symbol_info->ModBase, symbol_info->TypeIndex, TI_GET_SYMNAME, &name);
				if (wcscmp(name, L"IdString32") == 0) {
					type_info = make_type("IdString32", AhType_ID32);
				} else if (wcscmp(name, L"IdString64") == 0) {
					type_info = make_type("IdString64", AhType_ID64);
				} else if (wcscmp(name, L"capi::Vector3") == 0) {
					type_info = make_type("Vector3", AhType_Vector3);
				} else if (wcscmp(name, L"capi::Vector4") == 0) {
					type_info = make_type("Vector4", AhType_Vector4);
				}
			}

			string_stream.printf("\t");

			if (symbol_info->Flags & SYMFLAG_PARAMETER) { // Is it parameter or a local variable?
				DWORD64 memory = get_symbol_address(stack_frame, *symbol_info);
				string_stream.printf("%-32s = %-20s ", symbol_info->Name, value_str(memory, type_info, child_type_info));
			} else if (symbol_info->Flags & SYMFLAG_LOCAL) {
				DWORD64 memory = get_symbol_address(stack_frame, *symbol_info);
				string_stream.printf("%-32s = %-20s ", symbol_info->Name, value_str(memory, type_info, child_type_info));
			} else {
				string_stream.printf("%-32s = ?", symbol_info->Name);
			}

			{ // Print register info
				// string_stream.printf("%s", register_tostring(symbol_info->Register)); // Register
				// long offset = (long)symbol_info->Address;

				// if (offset >= 0)
				// 	string_stream.printf("+%x  ", offset);
				// else
				// 	string_stream.printf("-%x  ", (UINT_MAX- offset + 1));
				// string_stream.printf("Size: %u  ", symbol_info->Size);
			}

			string_stream.printf("[%s]", tag_tostring(tag));
			string_stream.printf("\n");
		}
		return TRUE;
	}
}

void stingray::callstack::print_stack_with_locals(int frames_to_skip, StringStream &string_stream) {
	if (!__symbols_initialized) {
		BOOL success = SymInitialize(GetCurrentProcess(), NULL, TRUE);
		if (!success)
			return;

		__symbols_initialized = true;
	}

	DWORD machine_type;
	CONTEXT context;
	STACKFRAME64 stack_frame;

	RtlCaptureContext(&context);

#if defined(_M_X64)
	machine_type                 = IMAGE_FILE_MACHINE_AMD64;
	stack_frame.AddrPC.Offset    = context.Rip;
	stack_frame.AddrPC.Mode      = AddrModeFlat;
	stack_frame.AddrFrame.Offset = context.Rsp;
	stack_frame.AddrFrame.Mode   = AddrModeFlat;
	stack_frame.AddrStack.Offset = context.Rsp;
	stack_frame.AddrStack.Mode   = AddrModeFlat;
#elif defined(_M_IA64)
	machine_type                  = IMAGE_FILE_MACHINE_IA64;
	stack_frame.AddrPC.Offset     = context.StIIP;
	stack_frame.AddrPC.Mode       = AddrModeFlat;
	stack_frame.AddrFrame.Offset  = context.IntSp;
	stack_frame.AddrFrame.Mode    = AddrModeFlat;
	stack_frame.AddrBStore.Offset = context.RsBSP;
	stack_frame.AddrBStore.Mode   = AddrModeFlat;
	stack_frame.AddrStack.Offset  = context.IntSp;
	stack_frame.AddrStack.Mode    = AddrModeFlat;
#endif

	HANDLE process = GetCurrentProcess();
	int frame_counter = 0;
	while (StackWalk64(machine_type, process, GetCurrentThread(), &stack_frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
		frames_to_skip--;
		if (frames_to_skip < 0) {
			IMAGEHLP_STACK_FRAME stack_frame_helper = { 0 };
			stack_frame_helper.InstructionOffset = stack_frame.AddrPC.Offset;
			SymSetContext(process, &stack_frame_helper, NULL);

			{ // lookup_file_line
				IMAGEHLP_LINEW64 line;
				line.SizeOfStruct = sizeof(line);
				DWORD displacement = 0;
				if (SymGetLineFromAddrW64(process, stack_frame.AddrPC.Offset, &displacement, &line)) {
					// strip beginning if filename comes from engine
					static const wchar_t *engine_str = L"\\engine\\";
					const wchar_t *engine_name = wcsstr(line.FileName, engine_str);
					if (engine_name == 0)
						string_stream << line.FileName;
					else
						string_stream << engine_name + wcslen(engine_str);

					// produce line number
					if (line.LineNumber > 99999)
						line.LineNumber = 0;
					string_stream.printf("(%d): ", line.LineNumber);
				}
			}

			{ // auto lookup_symbol = [](const void * addr, StringStream &os)
				SymbolInfoPackage symbol_info_package;
				SYMBOL_INFOW *symbol_info = &symbol_info_package.si;
				DWORD64 displacement = 0;
				if (SymFromAddrW(process, stack_frame.AddrPC.Offset, &displacement, symbol_info)) {
					// try to undecorate
					wchar_t undecorated[512];
					if (UnDecorateSymbolNameW(symbol_info->Name, undecorated, 512, UNDNAME_COMPLETE))
						string_stream << undecorated;
					else
						string_stream << symbol_info->Name;
				}
				string_stream << "\n";
			};

			{ // print locals
				UserContext user_context = {};
				user_context.stack_frame = &stack_frame;
				user_context.string_stream = &string_stream;
				SymEnumSymbols(process, 0, NULL, enum_symbol_callback, &user_context);
			}
		}
	}
}

#endif

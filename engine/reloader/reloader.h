/*
Things that could improve:
- The mspace block that gets filled in needs to be a fixed size. Could we dynamically allocate instead of setting the malloc head?
- Allocation of member_infos is wasteful, and potentially cache unfriendly.
- Fixed pointer memory.
- Fixed type memory.
- Default key for types and type-names is 0xffffffff instead of 0, so manual initialization is required.
- Based on dlmalloc _only_.
- estimate_unpadded_request not implemented. What does it entail?
- Make the main algorithm independent of the windbg backend.
- Make a second symbol backend. Maybe using the win

Things it can't handle:
- Namespaced structs. Name collision will occur in the type hash lookup.

Things to verify and test:
- Arrays: growing and shrinking static arrays.
- Two pointers, one pointing to a struct and the other to the first member. What would happen if you add something at the top of the struct?
- Add a new struct (type).

Performance:
- Run 1, unmodified, debug
	Took 0.00355118 seconds to expand types.
	Took 0.0021884 seconds to expand types.
	Took 0.0863985 seconds to collect pointers.
	Took 0.000169573 seconds to move data.
	Took 0.0866793 seconds to patch memory.

- Run 2, unmodified, O2
	Took 0.00221101 seconds to expand types.
	Took 0.00210161 seconds to expand types.
	Took 0.0026694 seconds to collect pointers.
	Took 0.000215521 seconds to move data.
	Took 0.00303808 seconds to patch memory.
*/
#define MAX_POINTERS (1<<17)
#define MAX_RECORDED_TYPES (1024)

// DEBUG
#define RELOAD_VERBOSE_DEBUGGING
#define RELOAD_PROFILING
#define INDENTATION "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"

#include <dbghelp.h>
#include <psapi.h>
enum SymTagEnum {
	SymTagNull,
	SymTagExe,
	SymTagCompiland,
	SymTagCompilandDetails,
	SymTagCompilandEnv,
	SymTagFunction,
	SymTagBlock,
	SymTagData,
	SymTagAnnotation,
	SymTagLabel,
	SymTagPublicSymbol, // 10
	SymTagUDT,
	SymTagEnum,
	SymTagFunctionType,
	SymTagPointerType,
	SymTagArrayType, // 15
	SymTagBaseType,
	SymTagTypedef,
	SymTagBaseClass,
	SymTagFriend,
	SymTagFunctionArgType, // 20
	SymTagFuncDebugStart,
	SymTagFuncDebugEnd,
	SymTagUsingNamespace,
	SymTagVTableShape,
	SymTagVTable,
	SymTagCustom,
	SymTagThunk,
	SymTagCustomType,
	SymTagManagedType,
	SymTagDimension,
	SymTagMax
};
struct SymbolInfoPackage : public SYMBOL_INFO_PACKAGEW {
	SymbolInfoPackage() {
		si.SizeOfStruct = sizeof(SYMBOL_INFOW);
		si.MaxNameLen = sizeof(name);
	}
};

#include "utils/quick_sort.cpp"
#include "symbol_info.cpp"

struct ReloadHeader {
	void *old_mspace;
	void *new_mspace;
	size_t old_memory_size;
};
struct Pointer {
	intptr_t addr_old;
	intptr_t addr_new;
	intptr_t target_addr_old;
	intptr_t target_addr_new;
	u32 type;
};
inline Pointer make_entry_pointer(intptr_t addr_old, SYMBOL_INFOW *symbol) {
	Pointer p = {};
	p.addr_old = addr_old;
	p.addr_new = 0;
	p.target_addr_old = *(intptr_t*)(addr_old);
	p.target_addr_new = 0;
	p.type = symbol->TypeIndex;
	return p;
}

struct PointerArray {
	Pointer *entries;
	u32 count;
};

struct PointerContext {
	void *memory;

	PointerArray pointer_array;
	int current_pointer_index;

	SortElement *sorted_pointer_indices;
	int sorted_pointer_count;
};
struct SymbolImage {
	PointerContext pointer_context;
	SymbolContext symbol_context;
};
PointerContext setup_pointer_context() {
	char *memory = (char*)malloc(sizeof(Pointer) * (MAX_POINTERS) + sizeof(SortElement) * (MAX_POINTERS));

	PointerContext pointer_context = {};

	pointer_context.memory = memory;

	// Set up the address lookup.
	pointer_context.pointer_array.entries = (Pointer *)memory;
	memory += sizeof(Pointer) * MAX_POINTERS;

	pointer_context.sorted_pointer_indices = (SortElement*)memory;
	memory += sizeof(SortElement) * MAX_POINTERS;

	return pointer_context;
}
SymbolContext setup_symbol_context(HANDLE process, u64 mod_base) {
	SymbolContext symbol_context = {};

	symbol_context.process = process;
	symbol_context.mod_base = mod_base;

	// TODO(kalle): Investigate if it is fine to use 0 as invalid keys and types.
	TypeInfo ti_stamp = {};
	ti_stamp.key = ~0llu;

	NameEntry ne_stamp = {};
	ne_stamp.type = ~0u;

	symbol_context.recorded_types = (TypeInfo*)malloc(sizeof(TypeInfo) * MAX_RECORDED_TYPES);
	for (int i = 0; i < MAX_RECORDED_TYPES; ++i) {
		symbol_context.recorded_types[i] = ti_stamp;
	}

	symbol_context.type_to_name = (NameEntry*)malloc(sizeof(NameEntry) * MAX_RECORDED_TYPES);
	for (int i = 0; i < MAX_RECORDED_TYPES; ++i) {
		symbol_context.type_to_name[i] = ne_stamp;
	}

	return symbol_context;
}

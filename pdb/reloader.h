#include <dbghelp.h>
#include <psapi.h>

#include "pdb.cpp"

#include "core/utils/quick_sort.h"
#include "core/containers/array.h"

#include "symbol_lookup.cpp"

struct ReloadHeader {
	void *old_base_memory;
	void *new_base_memory;
	void *old_mspace;
	void *new_mspace;
	size_t old_memory_size;
};
struct Pointer {
	intptr_t addr_old;
	intptr_t addr_new;
	intptr_t target_addr_old;
	intptr_t target_addr_new;
	CV_typ_t type_index;
};
inline Pointer make_entry_pointer(intptr_t addr_old, CV_typ_t type_index) {
	Pointer p = {};
	p.addr_old = addr_old;
	p.addr_new = 0;
	p.target_addr_old = *(intptr_t*)(addr_old);
	p.target_addr_new = 0;
	p.type = type_index;
	return p;
}

struct PointerContext {
	void *memory;

	Pointer *pointer_array;
	SortElement *sorted_pointer_indices;
};
struct SymbolImage {
	PointerContext pointer_context;
	SymbolContext symbol_context;
};
PointerContext setup_pointer_context() {
	PointerContext pointer_context = {};

	// Set up the address lookup.
	array_init(pointer_context.pointer_array, (1 << 16));
	array_init(pointer_context.sorted_pointer_indices, (1 << 16))

	return pointer_context;
}
SymbolContext setup_symbol_context(const char *pdbfile) {
	SymbolContext symbol_context = {};

	PdbFile pdb = parse_pdb(pdbfile);

	symbol_context.types = pdb.types;
	symbol_context.type_names = pdb.type_names;
	symbol_context.children = pdb.children;

	return symbol_context;
}

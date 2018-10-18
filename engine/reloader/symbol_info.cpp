enum TypeInfoMask : unsigned {
	TypeInfoMask_ContainsPointers = 1<<0,
	TypeInfoMask_IsModified = 1<<1,
	TypeInfoMask_IsVisited = 1<<2,
};

// Mapping between pdb type index and the hashed name
// NOTE(kalle): Should I make the name a unsigned instead? (It doubles the size of NameEntry, since padding..)
struct NameEntry {
	uint64_t name_id;
	unsigned type;
};

// Smallest piece of information about a type; the type index and the tag describing what it is.
struct TrimmedType {
	unsigned key;
	unsigned type;
	unsigned tag;
};

struct MemberInfo {
	uint64_t key;
	TrimmedType type;
	unsigned offset;

#ifdef RELOAD_VERBOSE_DEBUGGING
	WCHAR *name;
	WCHAR *type_name;
#endif
};

struct FullType {
	uint64_t key;
	unsigned size;
	unsigned mask;
	unsigned member_count;
	MemberInfo *members;

#ifdef RELOAD_VERBOSE_DEBUGGING
	WCHAR *name;
#endif
};

MemberInfo *try_find_member_info(MemberInfo *members, unsigned count, uint64_t key, unsigned start_index = 0) {
	unsigned index = start_index;
	for (unsigned i = 0; i < count; ++i) {
		if (index >= count) {
			index = 0;
		}

		if (members[index].key == key) {
			return members + index;
		}

		index++;
	}
	return 0;
}

struct SymbolContext {
	HANDLE process;
	ULONG64 mod_base;

	TrimmedType *recorded_trimmed_types;
	FullType *recorded_full_types;
	NameEntry *type_to_name;
};

NameEntry *get_name_hash(SymbolContext &context, unsigned type) {
	unsigned hash_mask = MAX_RECORDED_TYPES - 1;
	unsigned hash = type & hash_mask;
	for (unsigned offset = 0; offset < MAX_RECORDED_TYPES; offset++) {
		unsigned index = (hash + offset) & hash_mask;

		NameEntry &entry = context.type_to_name[index];
		if (entry.type == type || entry.type == ~0u) {
			return &entry;
		}
	}

	ASSERT(false, "Name hashmap full!");
	return 0;
}

FullType *get_recorded_type_info(SymbolContext &context, uint64_t key) {
	unsigned hash_mask = MAX_RECORDED_TYPES - 1;
	unsigned hash = (unsigned)key & hash_mask;
	for (unsigned offset = 0; offset < MAX_RECORDED_TYPES; offset++) {
		unsigned index = (hash + offset) & hash_mask;

		FullType &entry = context.recorded_full_types[index];
		if (entry.key == key || entry.key == ~0llu) {
			return &entry;
		}
	}

	ASSERT(false, "Recorded_type hashmap full!");
	return 0;
}

static unsigned get_tag_count = 0;
unsigned get_tag(SymbolContext &context, unsigned type) {
	get_tag_count++;

	unsigned tag = ~0u;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_SYMTAG, &tag);
#ifdef RELOAD_VERBOSE_DEBUGGING
	if (!success) { log_warning("Reloader", "get_tag failed! (last_error=%ld)", GetLastError()); }
#endif
	return tag;
}

static unsigned get_type_count = 0;
unsigned get_type(SymbolContext &context, unsigned type) {
	get_type_count++;
	unsigned _type = ~0u;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_TYPE, &_type);
#ifdef RELOAD_VERBOSE_DEBUGGING
	if (!success) { log_warning("Reloader", "get_type failed! (last_error=%ld)", GetLastError()); }
#endif

	if (!success) {
		unsigned num_children;
		success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_CHILDRENCOUNT, &num_children);
		if (success)
			_type = type;
	}

	return _type;
}

TrimmedType get_trimmed_type(SymbolContext &context, unsigned key) {
	unsigned hash_mask = MAX_RECORDED_TRIMMED_TYPES - 1;
	unsigned hash = (unsigned) key & hash_mask;
	for (unsigned offset = 0; offset < MAX_RECORDED_TRIMMED_TYPES; offset++) {
		unsigned index = (hash + offset) & hash_mask;

		TrimmedType &entry = context.recorded_trimmed_types[index];
		if (entry.key == key) {
			return entry;
		} else if (entry.key == ~0u) {
			entry.key = key;
			entry.type = get_type(context, key);
			entry.tag = get_tag(context, entry.type);
			return entry;
		}
	}

	ASSERT(false, "Recorded trimmed type hashmap full!");
	TrimmedType tt = {};
	return tt;
}

static unsigned get_size_count = 0;
unsigned get_size(SymbolContext &context, unsigned type) {
	get_size_count++;
	unsigned size = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_LENGTH, &size);
	#ifndef RELOAD_VERBOSE_DEBUGGING
	if (!success) { log_warning("Reloader", "get_size failed! (last_error=%ld)", GetLastError()); }
	#endif
	return size;
}

static unsigned make_type_info_count = 0;
FullType *make_type_info(Allocator &allocator, SymbolContext &context, unsigned type, unsigned tag, TI_FINDCHILDREN_PARAMS **children) {
	make_type_info_count++;
	ASSERT(tag != SymTagFunctionType, "Functions not allowed!");

	unsigned size = get_size(context, type);

	BOOL success;
	if (tag == SymTagUDT) {
		DWORD num_children = 0;
		success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_CHILDRENCOUNT, &num_children);
#ifdef RELOAD_VERBOSE_DEBUGGING
		if (!success) { log_warning("Reloader", "get_num_children failed! (last_error=%ld)", GetLastError()); }
#endif

		// we are responsible for allocating enough space to hold num_children values
		*children = (TI_FINDCHILDREN_PARAMS*) allocate(&allocator, sizeof(TI_FINDCHILDREN_PARAMS) + num_children * sizeof(ULONG));
		(*children)->Count = num_children;
		(*children)->Start = 0;
		success = SymGetTypeInfo(context.process, context.mod_base, type, TI_FINDCHILDREN, *children);
#ifdef RELOAD_VERBOSE_DEBUGGING
		if (!success) { log_warning("Reloader", "get_children failed! (last_error=%ld)", GetLastError()); }
	#endif
	}

	WCHAR *symbol_name;
	success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_SYMNAME, &symbol_name);
#ifdef RELOAD_VERBOSE_DEBUGGING
	if (!success) { log_warning("Reloader", "get_symbol_name failed! (last_error=%ld)", GetLastError()); }
#endif

	uint64_t name_id = murmur_hash_64(symbol_name, int(wcslen(symbol_name)*2), 0);

	FullType *type_info = get_recorded_type_info(context, name_id);

	type_info->key = name_id;
	type_info->size = size;

#ifndef RELOAD_VERBOSE_DEBUGGING
	LocalFree(symbol_name);
#else
	type_info->name = symbol_name;
#endif

	return type_info;
}

static unsigned get_fill_member_info_count = 0;
bool fill_member_info(SymbolContext &context, MemberInfo &member_info, unsigned member, TrimmedType type) {
	unsigned offset = ~0u;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_OFFSET, &offset);
	if (!success) {
#ifdef RELOAD_VERBOSE_DEBUGGING
		WCHAR *symbol_name = 0;
		success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_SYMNAME, &symbol_name);
		log_warning("Reloader", "get_offset failed, assume static member and ignore! (name=%S, last_error=%ld)", symbol_name, GetLastError());
#endif
		return false;
	}

	get_fill_member_info_count++;

	WCHAR *symbol_name = 0;
	success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_SYMNAME, &symbol_name);
#ifdef RELOAD_VERBOSE_DEBUGGING
	if (!success) { log_warning("Reloader", "get_symbol_name failed! (last_error=%ld)", GetLastError()); }
#endif

	uint64_t name_id = murmur_hash_64(symbol_name, int(wcslen(symbol_name)*2), 0);

	member_info.key = name_id;
	member_info.offset = offset;

	member_info.type = type;

#ifndef RELOAD_VERBOSE_DEBUGGING
	LocalFree(symbol_name);
#else
	member_info.name = symbol_name;
#endif

	return true;
}
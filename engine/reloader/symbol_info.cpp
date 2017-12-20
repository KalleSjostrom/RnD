enum TypeInfoMask : u32 {
	TypeInfoMask_ContainsPointers = 1<<0,
	TypeInfoMask_IsModified = 1<<1,
	TypeInfoMask_IsVisited = 1<<2,
};

// Mapping between pdb type index and the hashed name
// NOTE(kalle): Should I make the name a u32 instead? (It doubles the size of NameEntry, since padding..)
struct NameEntry {
	u64 name_id;
	u32 type;
};

// Smallest piece of information about a type; the type index and the tag describing what it is.
struct TrimmedType {
	u32 type;
	u32 tag;
};

struct MemberInfo {
	u64 key;
	TrimmedType type;
	u32 offset;

#ifdef RELOAD_VERBOSE_DEBUGGING
	WCHAR *name;
	WCHAR *type_name;
#endif
};

struct TypeInfo {
	u64 key;
	u32 size;
	u32 count;
	u32 mask;
	u32 member_count;
	MemberInfo *members;

#ifdef RELOAD_VERBOSE_DEBUGGING
	WCHAR *name;
#endif
};

MemberInfo *try_find_member_info(MemberInfo *members, u32 count, u64 key, u32 start_index = 0) {
	u32 index = start_index;
	for (u32 i = 0; i < count; ++i) {
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

	TypeInfo *recorded_types;
	NameEntry *type_to_name;

	MemoryArena arena;
};

NameEntry *get_name_hash(SymbolContext &context, u32 type) {
	u32 hash_mask = MAX_RECORDED_TYPES - 1;
	u32 hash = type & hash_mask;
	for (u32 offset = 0; offset < MAX_RECORDED_TYPES; offset++) {
		u32 index = (hash + offset) & hash_mask;

		NameEntry &entry = context.type_to_name[index];
		if (entry.type == type || entry.type == ~0u) {
			return &entry;
		}
	}

	return 0;
}

TypeInfo *get_recorded_type_info(SymbolContext &context, u64 key) {
	u32 hash_mask = MAX_RECORDED_TYPES - 1;
	u32 hash = (u32)key & hash_mask;
	for (u32 offset = 0; offset < MAX_RECORDED_TYPES; offset++) {
		u32 index = (hash + offset) & hash_mask;

		TypeInfo &entry = context.recorded_types[index];
		if (entry.key == key || entry.key == ~0llu) {
			return &entry;
		}
	}

	return 0;
}

u32 get_tag(SymbolContext &context, u32 type) {
	u32 tag;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_SYMTAG, &tag);
	if (!success) { LOG_WARNING("Reloader", "get_tag failed! (last_error=%ld)\n", GetLastError()); }
	return tag;
}

u32 get_type(SymbolContext &context, u32 type) {
	u32 _type;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_TYPE, &_type);
	if (!success) { LOG_WARNING("Reloader", "get_tag failed! (last_error=%ld)\n", GetLastError()); }
	return _type;
}

TrimmedType get_trimmed_type(SymbolContext &context, u32 type) {
	TrimmedType tt = {};

	do {
		tt.type = get_type(context, type);
		tt.tag = get_tag(context, tt.type);
	} while (tt.tag == SymTagTypedef);

	return tt;
}
u32 get_count(SymbolContext &context, u32 type) {
	u32 count = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_COUNT, &count);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)\n", GetLastError()); }
	return count;
}
u32 get_size(SymbolContext &context, u32 type) {
	u32 size = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_LENGTH, &size);
	if (!success) { LOG_WARNING("Reloader", "get_size failed! (last_error=%ld)\n", GetLastError()); }
	return size;
}

TypeInfo *make_type_info(SymbolContext &context, u32 type, u32 tag, TI_FINDCHILDREN_PARAMS **children) {
	ASSERT(tag != SymTagFunctionType, "Functions not allowed!")

	u32 size = get_size(context, type);
	u32 count = tag == SymTagArrayType ? get_count(context, type) : 0;

	BOOL success;
	if (tag == SymTagUDT) {
		DWORD num_children = 0;
		success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_CHILDRENCOUNT, &num_children);
		if (!success) { LOG_WARNING("Reloader", "get_num_children failed! (last_error=%ld)\n", GetLastError()); }

		// we are responsible for allocating enough space to hold num_children values
		*children = (TI_FINDCHILDREN_PARAMS*) new char[sizeof(TI_FINDCHILDREN_PARAMS) + num_children * sizeof(ULONG)];
		(*children)->Count = num_children;
		(*children)->Start = 0;
		success = SymGetTypeInfo(context.process, context.mod_base, type, TI_FINDCHILDREN, *children);
		if (!success) { LOG_WARNING("Reloader", "get_children failed! (last_error=%ld)\n", GetLastError()); }
	}

	WCHAR *symbol_name;
	success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_SYMNAME, &symbol_name);
	if (!success) { LOG_WARNING("Reloader", "get_symbol_name failed! (last_error=%ld)\n", GetLastError()); }

	u64 name_id = murmur_hash_64(symbol_name, int(wcslen(symbol_name)*2), 0);

	TypeInfo *type_info = get_recorded_type_info(context, name_id);
	
	type_info->key = name_id;
	type_info->size = size;
	type_info->count = count;

#ifndef RELOAD_VERBOSE_DEBUGGING
	LocalFree(symbol_name);
#else
	type_info->name = symbol_name;
#endif

	return type_info;
}

void fill_member_info(SymbolContext &context, MemberInfo &member_info, u32 member, TrimmedType type) {
	DWORD offset;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_OFFSET, &offset);
	if (!success) { LOG_WARNING("Reloader", "get_offset failed! (last_error=%ld)\n", GetLastError()); }

	WCHAR *symbol_name;
	success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_SYMNAME, &symbol_name);
	if (!success) { LOG_WARNING("Reloader", "get_symbol_name failed! (last_error=%ld)\n", GetLastError()); }

	u64 name_id = murmur_hash_64(symbol_name, int(wcslen(symbol_name)*2), 0);

	member_info.key = name_id;
	member_info.offset = offset;

	member_info.type = type;

#ifndef RELOAD_VERBOSE_DEBUGGING
	LocalFree(symbol_name);
#else
	member_info.name = symbol_name;
#endif
}
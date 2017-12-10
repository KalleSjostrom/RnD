#ifdef USE_TYPE_CACHE
#define MAX_RECORDED_TYPES (1024)

enum TypeMask : unsigned {
	TypeMask_Children = 1<<0,
	TypeMask_Type = 1<<1,
	TypeMask_Tag = 1<<2,
	TypeMask_Offset = 1<<3,
	TypeMask_Size = 1<<4,
	TypeMask_Count = 1<<5,
};

struct TypeInfo {
	u32 key;
	u32 mask;

	TI_FINDCHILDREN_PARAMS *children;

	u32 type;
	u32 tag;
	u32 offset;
	u32 size;
	u32 count;
};
#endif // USE_TYPE_CACHE

struct SymbolContext {
	HANDLE process;
	ULONG64 mod_base;

#ifdef USE_TYPE_CACHE
	TypeInfo *recorded_types;
#endif // USE_TYPE_CACHE
};

#ifdef USE_TYPE_CACHE
TypeInfo *get_recorded_type_info(SymbolContext &context, unsigned key) {
	unsigned hash_mask = MAX_RECORDED_TYPES - 1;
	unsigned hash = key & hash_mask;
	for (unsigned offset = 0; offset < MAX_RECORDED_TYPES; offset++) {
		unsigned index = (hash + offset) & hash_mask;

		TypeInfo &entry = context.recorded_types[index];
		if (entry.key == key || entry.mask == 0) {
			entry.key = key;
			return &entry;
		}
	}

	return 0;
}
#endif // USE_TYPE_CACHE

#if VERBOSE_DEBUGGING
	WCHAR *get_symbol_name(SymbolContext &context, u32 type) {
		WCHAR *symbol_name;
		BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_SYMNAME, &symbol_name);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }
		return symbol_name;
	}
#endif

// 'member' is the specific entry in a struct, that has a name/offset and so on.
// This gets the _type_ of that.
u32 get_type(SymbolContext &context, u32 member) {
#ifdef USE_TYPE_CACHE
	TypeInfo *type_info = get_recorded_type_info(context, member);
	if (type_info->mask & TypeMask_Type) {
		return type_info->type;
	}
#endif // USE_TYPE_CACHE

	DWORD type = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_TYPE, &type);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }

#ifdef USE_TYPE_CACHE
	type_info->mask |= TypeMask_Type;
	type_info->type = type;
#endif // USE_TYPE_CACHE

	return type;
}
u32 get_tag(SymbolContext &context, u32 type) {
#ifdef USE_TYPE_CACHE
	TypeInfo *type_info = get_recorded_type_info(context, type);
	if (type_info->mask & TypeMask_Tag) {
		return type_info->tag;
	}
#endif // USE_TYPE_CACHE

	DWORD tag = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_SYMTAG, &tag);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }

#ifdef USE_TYPE_CACHE
	type_info->mask |= TypeMask_Tag;
	type_info->tag = tag;
#endif // USE_TYPE_CACHE

	return tag;
}
u32 get_offset(SymbolContext &context, u32 type) {
#ifdef USE_TYPE_CACHE
	TypeInfo *type_info = get_recorded_type_info(context, type);
	if (type_info->mask & TypeMask_Offset) {
		return type_info->offset;
	}
#endif // USE_TYPE_CACHE

	DWORD offset = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_OFFSET, &offset);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }

#ifdef USE_TYPE_CACHE
	type_info->mask |= TypeMask_Offset;
	type_info->offset = offset;
#endif // USE_TYPE_CACHE

	return offset;
}
u64 get_size(SymbolContext &context, u32 type) {
#ifdef USE_TYPE_CACHE
	TypeInfo *type_info = get_recorded_type_info(context, type);
	if (type_info->mask & TypeMask_Size) {
		return type_info->size;
	}
#endif // USE_TYPE_CACHE

	DWORD length = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_LENGTH, &length);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }

#ifdef USE_TYPE_CACHE
	type_info->mask |= TypeMask_Size;
	type_info->size = length;
#endif // USE_TYPE_CACHE

	return length;
}
u64 get_count(SymbolContext &context, u32 type) {
#ifdef USE_TYPE_CACHE
	TypeInfo *type_info = get_recorded_type_info(context, type);
	if (type_info->mask & TypeMask_Count) {
		return type_info->count;
	}
#endif // USE_TYPE_CACHE

	DWORD count = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_COUNT, &count);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }

#ifdef USE_TYPE_CACHE
	type_info->mask |= TypeMask_Count;
	type_info->count = count;
#endif // USE_TYPE_CACHE

	return count;
}

/// Children
TI_FINDCHILDREN_PARAMS *get_children(SymbolContext &context, u32 type) {
#ifdef USE_TYPE_CACHE
	TypeInfo *type_info = get_recorded_type_info(context, type);
	if (type_info->mask & TypeMask_Children) {
		return type_info->children;
	}
#endif // USE_TYPE_CACHE

	DWORD num_children = 0;
	BOOL success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_CHILDRENCOUNT, &num_children);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }
	if (num_children == 0)
		return 0;

	// we are responsible for allocating enough space to hold num_children values
	TI_FINDCHILDREN_PARAMS *children = (TI_FINDCHILDREN_PARAMS*) new char[sizeof(TI_FINDCHILDREN_PARAMS) + num_children * sizeof(ULONG)];
	children->Count = num_children;
	children->Start = 0;
	success = SymGetTypeInfo(context.process, context.mod_base, type, TI_FINDCHILDREN, children);
	if (!success) { LOG_WARNING("Reloader", "get_count failed! (last_error=%ld)", GetLastError()); }

#ifdef USE_TYPE_CACHE
	type_info->mask |= TypeMask_Children;
	type_info->children = children;
#endif // USE_TYPE_CACHE

	return children;
}

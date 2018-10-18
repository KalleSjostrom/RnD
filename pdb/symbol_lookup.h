struct Children {
	Type *types;
	size_t count;
};
struct TypeName {
	uint64_t id;
	size_t length;
	const char *buffer;
};
struct SymbolContext {
	Type *types;
	TypeName *names;
	Children *children;
};

struct TypeTag {
	CV_typ_t type_index;
	unsigned tag;
};

TypeName *get_type_name(SymbolContext &context, unsigned type_index) {
	return context.names + type_index;
}

Children *get_children(SymbolContext &context, unsigned type_index) {
	return context.children + type_index;
}

unsigned get_child_type(SymbolContext &context, Type *child) {
	lfEasy *base = (lfEasy*)child->data;

	switch (base->leaf) {
		case LF_MEMBER_ST:
		case LF_MEMBER: {
			return ((lfMember*)base)->index;
		} break;
		case LF_STMEMBER_ST:
		case LF_STMEMBER: {} break;
		case LF_ONEMETHOD_ST:
		case LF_ONEMETHOD: {} break;
	}

	return 0;
}

unsigned get_tag(SymbolContext &context, unsigned type_index) {
	if (CV_IS_PRIMITIVE(type_index)) {
		return SymTagBaseType;
	}

	Type type = context.types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		// case LF_FRIENDFCN_ST:
		// case LF_METHOD_ST:
		// case LF_ONEMETHOD_ST:
		// case LF_FRIENDFCN:
		// case LF_METHOD:
		// case LF_ONEMETHOD:
		// case LF_PROCEDURE:
		// case LF_MFUNCTION: { return SymTagFunctionType; } break;
		case LF_POINTER:   { return SymTagPointerType; } break;
		case LF_ARRAY_ST:
		case LF_ARRAY:     { return SymTagArrayType;   } break;
		case LF_ENUM:      { return SymTagEnum;        } break;
		case LF_UNION:
		case LF_CLASS:
		case LF_INTERFACE:
		case LF_STRUCTURE: { return SymTagUDT;         } break;
		// default: {
		// 	PDB_ASSERT(false);
		// }
	}

	return SymTagNull;
}
unsigned get_size(SymbolContext &context, unsigned type_index) {
	unsigned size = 0;

	Type type = context.types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		case LF_ENUMERATE: { extract_info(((lfEnumerate*) base)->value,  &size); } break;
		case LF_ARRAY:     { extract_info(((lfArray*)     base)->data,   &size); } break;
		case LF_CLASS:
		case LF_INTERFACE:
		case LF_STRUCTURE: { extract_info(((lfStructure*) base)->data,   &size); } break;
		case LF_UNION:     { extract_info(((lfUnion*)     base)->data,   &size); } break;
		case LF_MEMBER_ST:
		case LF_MEMBER:    { extract_info(((lfMember*)    base)->offset, &size); } break;
		case LF_BCLASS:    { extract_size(((lfBClass*)    base)->offset, &size); } break;
		default: {
			PDB_ASSERT(false);
		}
	}

	return size;
}


unsigned get_type(SymbolContext &context, unsigned type_index) {
	CV_typ_t result = 0;

	Type type = context.types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		case LF_ARRAY:     { result = ((lfArray*)base)->elemtype; } break;
		case LF_NESTTYPE_ST:
		case LF_NESTTYPE: { result = ((lfNestType*)base)->index; } break;
		case LF_NESTTYPEEX_ST:
		case LF_NESTTYPEEX: { result = ((lfNestTypeEx*)base)->index; } break;
		case LF_MEMBER_ST:
		case LF_MEMBER:    { result = ((lfMember*)base)->index; } break;
		case LF_STMEMBER_ST:
		case LF_STMEMBER:    { result = ((lfSTMember*)base)->index; } break;
		case LF_FRIENDCLS:    { result = ((lfFriendCls*)base)->index; } break;
		case LF_ONEMETHOD_ST:
		case LF_ONEMETHOD: { result = ((lfOneMethod*)base)->index; } break;
		default: {
			PDB_ASSERT(false);
		}
	}

#if 0 // WHY?! :o
	if (!success) {
		unsigned num_children;
		success = SymGetTypeInfo(context.process, context.mod_base, type, TI_GET_CHILDRENCOUNT, &num_children);
		if (success)
			_type = type;
	}
#endif
	return (unsigned)result;
}

// TrimmedType get_trimmed_type(SymbolContext &context, unsigned key) {
// 	unsigned hash_mask = MAX_RECORDED_TRIMMED_TYPES - 1;
// 	unsigned hash = (unsigned) key & hash_mask;
// 	for (unsigned offset = 0; offset < MAX_RECORDED_TRIMMED_TYPES; offset++) {
// 		unsigned index = (hash + offset) & hash_mask;

// 		TrimmedType &entry = context.recorded_trimmed_types[index];
// 		if (entry.key == key) {
// 			return entry;
// 		} else if (entry.key == ~0u) {
// 			entry.key = key;
// 			entry.type = get_type(context, key);
// 			entry.tag = get_tag(context, entry.type);
// 			return entry;
// 		}
// 	}

// 	ASSERT(false, "Recorded trimmed type hashmap full!");
// 	TrimmedType tt = {};
// 	return tt;
// }

TypeTag get_type_tag(SymbolContext &context, Type &type) {
	TypeTag tt = {};
	tt.type_index = get_child_type(context, &type);
	tt.tag = get_tag(context, tt.type_index);
	return tt;
}
TypeTag get_type_tag(SymbolContext &context, CV_typ_t type_index) {
	TypeTag tt = {};
	tt.type_index = get_type(context, type_index);
	tt.tag = get_tag(context, tt.type_index);
	return tt;
}

// FullType *make_type_info(Allocator &allocator, SymbolContext &context, unsigned type_index, unsigned tag, TI_FINDCHILDREN_PARAMS **children) {
// 	PDB_ASSERT(tag != SymTagNull);

// 	Type type = context.types[type_index];
// 	lfEasy *base = (lfEasy*)type.data;

// 	unsigned size = get_size(context, type_index);
// 	if (tag == SymTagUDT) {
// 		CV_typ_t field_list;
// 		switch (base->leaf) {
// 			case LF_UNION: { field_list = ((lfUnion*)base)->field; } break;
// 			case LF_CLASS:
// 			case LF_INTERFACE:
// 			case LF_STRUCTURE: { field_list = ((lfStructure*)base)->field; } break;
// 		}

// 		Children children = context.children[type_index];
// 	}

// 	TypeName name = context.names[type_index];

// 	FullType *type_info = get_recorded_type_info(context, name.id);

// 	type_info->key = name_id;
// 	type_info->size = size;
// 	type_info->count = count;
// 	type_info->name = symbol_name;

// 	return type_info;
// }

// static unsigned get_fill_member_info_count = 0;
// bool fill_member_info(SymbolContext &context, MemberInfo &member_info, unsigned member, TrimmedType type) {
// 	unsigned offset = ~0u;
// 	BOOL success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_OFFSET, &offset);
// 	if (!success) {
// #ifdef RELOAD_VERBOSE_DEBUGGING
// 		WCHAR *symbol_name = 0;
// 		success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_SYMNAME, &symbol_name);
// 		log_warning("Reloader", "get_offset failed, assume static member and ignore! (name=%S, last_error=%ld)", symbol_name, GetLastError());
// #endif
// 		return false;
// 	}

// 	get_fill_member_info_count++;

// 	WCHAR *symbol_name = 0;
// 	success = SymGetTypeInfo(context.process, context.mod_base, member, TI_GET_SYMNAME, &symbol_name);
// #ifdef RELOAD_VERBOSE_DEBUGGING
// 	if (!success) { log_warning("Reloader", "get_symbol_name failed! (last_error=%ld)", GetLastError()); }
// #endif

// 	uint64_t name_id = murmur_hash_64(symbol_name, int(wcslen(symbol_name)*2), 0);

// 	member_info.key = name_id;
// 	member_info.offset = offset;

// 	member_info.type = type;

// #ifndef RELOAD_VERBOSE_DEBUGGING
// 	LocalFree(symbol_name);
// #else
// 	member_info.name = symbol_name;
// #endif

// 	return true;
// }
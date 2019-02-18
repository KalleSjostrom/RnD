enum TypeRecordFlags : uint32_t {
	TypeRecordFlags_NonExistent = 1<<0,
	TypeRecordFlags_ContainsPointers = 1<<1,
	TypeRecordFlags_IsModified = 1<<2,

	TypeRecordFlags_CheckedForPointers = 1<<3,
	TypeRecordFlags_Patched = 1<<4,

	TypeRecordFlags_IsNew = 1<<5,
};

Type *try_find_member(MemberList *member_list, TypeName *name, uint32_t best_guess = 0) {
	int member_list_count = array_count(member_list->types);
	if ((int)best_guess >= member_list_count)
		best_guess = 0;

	int index = best_guess;
	for (int i = 0; i < member_list_count; ++i, ++index) {
		if (index >= member_list_count) {
			index = 0;
		}

		if (member_list->names[index].id == name->id) {
			return member_list->types + index;
		}
	}
	return 0;
}

// NOTE(kalle): This hashmap will be pre-filled when parsing the pdb and no entries will move around during the 'lookup' phase.
// This means it's safe to return the pointer to an entrys value.
static TypeRecord null_type_record = {TypeRecordFlags_NonExistent, 0};
TypeRecord *get_type_record(TypeContext *context, uint64_t key) {
	HashEntry *entry = hash_lookup(context->record_hashmap, key);
	if (entry == 0) {
		return &null_type_record;
	}

	TypeRecord *record = (TypeRecord*)&entry->value;
	if (entry->key != key) {
		record->flags |= TypeRecordFlags_NonExistent;
	}

	return record;
}

TypeName *get_type_name(TypeContext *context, CV_typ_t type_index) {
	ASSERT(!CV_IS_PRIMITIVE(type_index), "Can't get type name from primitive type!");
	type_index -= CV_FIRST_NONPRIM;

	return context->names + type_index;
}
MemberList *get_member_list(TypeContext *context, TypeRecord *type_record) {
	CV_typ_t type_index = type_record->type_index;

	ASSERT(!CV_IS_PRIMITIVE(type_index), "Can't get member list from primitive type!");
	type_index -= CV_FIRST_NONPRIM;

	uint32_t list_index = 0;

	Type type = context->types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		case LF_ENUM:  { list_index = ((lfEnum*)base)->field; } break;
		case LF_UNION: { list_index = ((lfUnion*)base)->field; } break;
		case LF_CLASS:
		case LF_INTERFACE:
		case LF_STRUCTURE: { list_index = ((lfClass*)base)->field; } break;
		default: {
			ASSERT(false, "Cannot get member list! (%s)", context->names[type_index].name);
		}
	}

	ASSERT(list_index, "Could not find a proper list index! (%s)", context->names[type_index].name);

	list_index -= CV_FIRST_NONPRIM;
	return context->member_lists + list_index;
}
uint32_t get_child_type(Type *child) {
	if (child == 0)
		return 0;

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
uint32_t get_tag(TypeContext *context, CV_typ_t type_index) {
	if (CV_IS_PRIMITIVE(type_index)) { return SymTagBaseType; }
	type_index -= CV_FIRST_NONPRIM;

	Type type = context->types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		case LF_FRIENDFCN_ST:
		case LF_METHOD_ST:
		case LF_ONEMETHOD_ST:
		case LF_FRIENDFCN:
		case LF_METHOD:
		case LF_ONEMETHOD:
		case LF_PROCEDURE:
		case LF_MFUNCTION: { return SymTagFunctionType; } break;
		case LF_POINTER:   { return SymTagPointerType; } break;
		case LF_ARRAY_ST:
		case LF_ARRAY:	 { return SymTagArrayType;   } break;
		case LF_ENUM:	  { return SymTagEnum;		} break;
		case LF_UNION:
		case LF_CLASS:
		case LF_INTERFACE:
		case LF_STRUCTURE: { return SymTagUDT;		 } break;
		// default: {
		// 	ASSERT(false);
		// }
	}

	return SymTagNull;
}

int get_primitive_size(CV_typ_t type);
uint32_t get_size(TypeContext *context, CV_typ_t type_index) {
	if (CV_IS_PRIMITIVE(type_index)) {
		return get_primitive_size(type_index);
	}

	type_index -= CV_FIRST_NONPRIM;

	uint32_t size = 0;

	Type type = context->types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		// case LF_ENUMERATE: { extract_info(((lfEnumerate*) base)->value,  &size); } break;
		case LF_ENUM:      { size = get_size(context, ((lfEnum*)base)->utype); } break;
		case LF_ARRAY:     { extract_info(((lfArray*)     base)->data,   &size); } break;
		case LF_CLASS:
		case LF_INTERFACE:
		case LF_STRUCTURE: { extract_info(((lfStructure*) base)->data,   &size); } break;
		case LF_UNION:	   { extract_info(((lfUnion*)     base)->data,   &size); } break;
		case LF_MEMBER_ST:
		case LF_MEMBER:	   { extract_info(((lfMember*)    base)->offset, &size); } break;
		case LF_BCLASS:	   { extract_size(((lfBClass*)    base)->offset, &size); } break;
		case LF_POINTER:   { size = sizeof(void*); } break;
		default: {
			ASSERT(false, "Not Implemented!");
		}
	}

	return size;
}

uint32_t get_offset(Type *child) {
	if (child == 0)
		return 0;

	uint32_t offset = 0;

	lfEasy *base = (lfEasy*)child->data;
	switch (base->leaf) {
		case LF_MEMBER_ST:
		case LF_MEMBER: { extract_info(((lfMember*) base)->offset, &offset); } break;
		default: {
			ASSERT(false, "Not Implemented!");
		}
	}

	return offset;
}

CV_typ_t get_type(TypeContext *context, CV_typ_t type_index) {
	ASSERT(!CV_IS_PRIMITIVE(type_index), "Can't get member list from primitive type!");
	type_index -= CV_FIRST_NONPRIM;

	CV_typ_t result = 0;

	Type type = context->types[type_index];
	lfEasy *base = (lfEasy*)type.data;
	switch (base->leaf) {
		case LF_ARRAY:	 { result = ((lfArray*)base)->elemtype; } break;
		case LF_POINTER:	 { result = ((lfPointer*)base)->utype; } break;
		case LF_NESTTYPE_ST:
		case LF_NESTTYPE: { result = ((lfNestType*)base)->index; } break;
		case LF_NESTTYPEEX_ST:
		case LF_NESTTYPEEX: { result = ((lfNestTypeEx*)base)->index; } break;
		case LF_MEMBER_ST:
		case LF_MEMBER:	{ result = ((lfMember*)base)->index; } break;
		case LF_STMEMBER_ST:
		case LF_STMEMBER:	{ result = ((lfSTMember*)base)->index; } break;
		case LF_FRIENDCLS:	{ result = ((lfFriendCls*)base)->index; } break;
		case LF_ONEMETHOD_ST:
		case LF_ONEMETHOD: { result = ((lfOneMethod*)base)->index; } break;
		default: {
			ASSERT(false, "Not Implemented!");
		}
	}

#if 0 // WHY?! :o
	if (!success) {
		uint32_t num_children;
		success = SymGetTypeInfo(context->process, context->mod_base, type, TI_GET_CHILDRENCOUNT, &num_children);
		if (success)
			_type = type;
	}
#endif
	return (CV_typ_t)result;
}

TypeTag get_child_type_tag(TypeContext *context, Type *child) {
	TypeTag tt = {};
	tt.type_index = get_child_type(child);
	tt.tag = get_tag(context, tt.type_index);
	return tt;
}
TypeTag get_type_tag(TypeContext *context, CV_typ_t type_index) {
	TypeTag tt = {};
	tt.type_index = get_type(context, type_index);
	tt.tag = get_tag(context, tt.type_index);
	return tt;
}

CV_typ_t get_base_pointer_type(CV_typ_t type) {
	switch (type) {
		// Void
		case T_PVOID:                             // near pointer to void
		case T_PFVOID:                            // far pointer to void
		case T_PHVOID:                            // huge pointer to void
		case T_32PVOID:                           // 32 bit pointer to void
		case T_32PFVOID:                          // 16:32 pointer to void
		case T_64PVOID: { return T_VOID; } break; // 64 bit pointer to void

		// Character types
		case T_PCHAR:                             // 16 bit pointer to 8 bit signed
		case T_PFCHAR:                            // 16:16 far pointer to 8 bit signed
		case T_PHCHAR:                            // 16:16 huge pointer to 8 bit signed
		case T_32PCHAR:                           // 32 bit pointer to 8 bit signed
		case T_32PFCHAR:                          // 16:32 pointer to 8 bit signed
		case T_64PCHAR: { return T_CHAR; } break; // 64 bit pointer to 8 bit signed

		case T_PUCHAR:                              // 16 bit pointer to 8 bit unsigned
		case T_PFUCHAR:                             // 16:16 far pointer to 8 bit unsigned
		case T_PHUCHAR:                             // 16:16 huge pointer to 8 bit unsigned
		case T_32PUCHAR:                            // 32 bit pointer to 8 bit unsigned
		case T_32PFUCHAR:                           // 16:32 pointer to 8 bit unsigned
		case T_64PUCHAR: { return T_UCHAR; } break; // 64 bit pointer to 8 bit unsigned

		// really a character types
		case T_PRCHAR:                              // 16 bit pointer to a real char
		case T_PFRCHAR:                             // 16:16 far pointer to a real char
		case T_PHRCHAR:                             // 16:16 huge pointer to a real char
		case T_32PRCHAR:                            // 32 bit pointer to a real char
		case T_32PFRCHAR:                           // 16:32 pointer to a real char
		case T_64PRCHAR: { return T_RCHAR; } break; // 64 bit pointer to a real char

		// really a wide character types
		case T_PWCHAR:                              // 16 bit pointer to a wide char
		case T_PFWCHAR:                             // 16:16 far pointer to a wide char
		case T_PHWCHAR:                             // 16:16 huge pointer to a wide char
		case T_32PWCHAR:                            // 32 bit pointer to a wide char
		case T_32PFWCHAR:                           // 16:32 pointer to a wide char
		case T_64PWCHAR: { return T_WCHAR; } break; // 64 bit pointer to a wide char

		// really a 16-bit unicode char
		case T_CHAR16:                                // 16-bit unicode char
		case T_PCHAR16:                               // 16 bit pointer to a 16-bit unicode char
		case T_PFCHAR16:                              // 16:16 far pointer to a 16-bit unicode char
		case T_PHCHAR16:                              // 16:16 huge pointer to a 16-bit unicode char
		case T_32PCHAR16:                             // 32 bit pointer to a 16-bit unicode char
		case T_32PFCHAR16:                            // 16:32 pointer to a 16-bit unicode char
		case T_64PCHAR16: { return T_CHAR16; } break; // 64 bit pointer to a 16-bit unicode char

		// really a 32-bit unicode char
		case T_PCHAR32:                               // 16 bit pointer to a 32-bit unicode char
		case T_PFCHAR32:                              // 16:16 far pointer to a 32-bit unicode char
		case T_PHCHAR32:                              // 16:16 huge pointer to a 32-bit unicode char
		case T_32PCHAR32:                             // 32 bit pointer to a 32-bit unicode char
		case T_32PFCHAR32:                            // 16:32 pointer to a 32-bit unicode char
		case T_64PCHAR32: { return T_CHAR32; } break; // 64 bit pointer to a 32-bit unicode char

		// 8 bit int types
		case T_PINT1:                             // 16 bit pointer to 8 bit signed int
		case T_PFINT1:                            // 16:16 far pointer to 8 bit signed int
		case T_PHINT1:                            // 16:16 huge pointer to 8 bit signed int
		case T_32PINT1:                           // 32 bit pointer to 8 bit signed int
		case T_32PFINT1:                          // 16:32 pointer to 8 bit signed int
		case T_64PINT1: { return T_INT1; } break; // 64 bit pointer to 8 bit signed int

		case T_PUINT1:                              // 16 bit pointer to 8 bit unsigned int
		case T_PFUINT1:                             // 16:16 far pointer to 8 bit unsigned int
		case T_PHUINT1:                             // 16:16 huge pointer to 8 bit unsigned int
		case T_32PUINT1:                            // 32 bit pointer to 8 bit unsigned int
		case T_32PFUINT1:                           // 16:32 pointer to 8 bit unsigned int
		case T_64PUINT1: { return T_UINT1; } break; // 64 bit pointer to 8 bit unsigned int


		// 16 bit short types
		case T_PSHORT:                              // 16 bit pointer to 16 bit signed
		case T_PFSHORT:                             // 16:16 far pointer to 16 bit signed
		case T_PHSHORT:                             // 16:16 huge pointer to 16 bit signed
		case T_32PSHORT:                            // 32 bit pointer to 16 bit signed
		case T_32PFSHORT:                           // 16:32 pointer to 16 bit signed
		case T_64PSHORT: { return T_SHORT; } break; // 64 bit pointer to 16 bit signed

		case T_PUSHORT:                               // 16 bit pointer to 16 bit unsigned
		case T_PFUSHORT:                              // 16:16 far pointer to 16 bit unsigned
		case T_PHUSHORT:                              // 16:16 huge pointer to 16 bit unsigned
		case T_32PUSHORT:                             // 32 bit pointer to 16 bit unsigned
		case T_32PFUSHORT:                            // 16:32 pointer to 16 bit unsigned
		case T_64PUSHORT: { return T_USHORT; } break; // 64 bit pointer to 16 bit unsigned

		// 16 bit int types
		case T_PINT2:                             // 16 bit pointer to 16 bit signed int
		case T_PFINT2:                            // 16:16 far pointer to 16 bit signed int
		case T_PHINT2:                            // 16:16 huge pointer to 16 bit signed int
		case T_32PINT2:                           // 32 bit pointer to 16 bit signed int
		case T_32PFINT2:                          // 16:32 pointer to 16 bit signed int
		case T_64PINT2: { return T_INT2; } break; // 64 bit pointer to 16 bit signed int

		case T_PUINT2:                              // 16 bit pointer to 16 bit unsigned int
		case T_PFUINT2:                             // 16:16 far pointer to 16 bit unsigned int
		case T_PHUINT2:                             // 16:16 huge pointer to 16 bit unsigned int
		case T_32PUINT2:                            // 32 bit pointer to 16 bit unsigned int
		case T_32PFUINT2:                           // 16:32 pointer to 16 bit unsigned int
		case T_64PUINT2: { return T_UINT2; } break; // 64 bit pointer to 16 bit unsigned int

		// 32 bit long types
		case T_PLONG:                             // 16 bit pointer to 32 bit signed
		case T_PFLONG:                            // 16:16 far pointer to 32 bit signed
		case T_PHLONG:                            // 16:16 huge pointer to 32 bit signed
		case T_32PLONG:                           // 32 bit pointer to 32 bit signed
		case T_32PFLONG:                          // 16:32 pointer to 32 bit signed
		case T_64PLONG: { return T_LONG; } break; // 64 bit pointer to 32 bit signed

		case T_PULONG:                              // 16 bit pointer to 32 bit unsigned
		case T_PFULONG:                             // 16:16 far pointer to 32 bit unsigned
		case T_PHULONG:                             // 16:16 huge pointer to 32 bit unsigned
		case T_32PULONG:                            // 32 bit pointer to 32 bit unsigned
		case T_32PFULONG:                           // 16:32 pointer to 32 bit unsigned
		case T_64PULONG: { return T_ULONG; } break; // 64 bit pointer to 32 bit unsigned

		// 32 bit int types
		case T_PINT4:                             // 16 bit pointer to 32 bit signed int
		case T_PFINT4:                            // 16:16 far pointer to 32 bit signed int
		case T_PHINT4:                            // 16:16 huge pointer to 32 bit signed int
		case T_32PINT4:                           // 32 bit pointer to 32 bit signed int
		case T_32PFINT4:                          // 16:32 pointer to 32 bit signed int
		case T_64PINT4: { return T_INT4; } break; // 64 bit pointer to 32 bit signed int

		case T_PUINT4:                              // 16 bit pointer to 32 bit unsigned int
		case T_PFUINT4:                             // 16:16 far pointer to 32 bit unsigned int
		case T_PHUINT4:                             // 16:16 huge pointer to 32 bit unsigned int
		case T_32PUINT4:                            // 32 bit pointer to 32 bit unsigned int
		case T_32PFUINT4:                           // 16:32 pointer to 32 bit unsigned int
		case T_64PUINT4: { return T_UINT4; } break; // 64 bit pointer to 32 bit unsigned int

		// 64 bit quad types
		case T_PQUAD:                             // 16 bit pointer to 64 bit signed
		case T_PFQUAD:                            // 16:16 far pointer to 64 bit signed
		case T_PHQUAD:                            // 16:16 huge pointer to 64 bit signed
		case T_32PQUAD:                           // 32 bit pointer to 64 bit signed
		case T_32PFQUAD:                          // 16:32 pointer to 64 bit signed
		case T_64PQUAD: { return T_QUAD; } break; // 64 bit pointer to 64 bit signed

		case T_PUQUAD:                              // 16 bit pointer to 64 bit unsigned
		case T_PFUQUAD:                             // 16:16 far pointer to 64 bit unsigned
		case T_PHUQUAD:                             // 16:16 huge pointer to 64 bit unsigned
		case T_32PUQUAD:                            // 32 bit pointer to 64 bit unsigned
		case T_32PFUQUAD:                           // 16:32 pointer to 64 bit unsigned
		case T_64PUQUAD: { return T_UQUAD; } break; // 64 bit pointer to 64 bit unsigned

		// 64 bit int types
		case T_PINT8:                             // 16 bit pointer to 64 bit signed int
		case T_PFINT8:                            // 16:16 far pointer to 64 bit signed int
		case T_PHINT8:                            // 16:16 huge pointer to 64 bit signed int
		case T_32PINT8:                           // 32 bit pointer to 64 bit signed int
		case T_32PFINT8:                          // 16:32 pointer to 64 bit signed int
		case T_64PINT8: { return T_INT8; } break; // 64 bit pointer to 64 bit signed int

		case T_PUINT8:                              // 16 bit pointer to 64 bit unsigned int
		case T_PFUINT8:                             // 16:16 far pointer to 64 bit unsigned int
		case T_PHUINT8:                             // 16:16 huge pointer to 64 bit unsigned int
		case T_32PUINT8:                            // 32 bit pointer to 64 bit unsigned int
		case T_32PFUINT8:                           // 16:32 pointer to 64 bit unsigned int
		case T_64PUINT8: { return T_UINT8; } break; // 64 bit pointer to 64 bit unsigned int

		// 128 bit octet types
		case T_POCT:                              // 16 bit pointer to 128 bit signed
		case T_PFOCT:                             // 16:16 far pointer to 128 bit signed
		case T_PHOCT:                             // 16:16 huge pointer to 128 bit signed
		case T_32POCT:                            // 32 bit pointer to 128 bit signed
		case T_32PFOCT:                           // 16:32 pointer to 128 bit signed
		case T_64POCT:  { return T_OCT;  } break; // 64 bit pointer to 128 bit signed

		case T_PUOCT:                             // 16 bit pointer to 128 bit unsigned
		case T_PFUOCT:                            // 16:16 far pointer to 128 bit unsigned
		case T_PHUOCT:                            // 16:16 huge pointer to 128 bit unsigned
		case T_32PUOCT:                           // 32 bit pointer to 128 bit unsigned
		case T_32PFUOCT:                          // 16:32 pointer to 128 bit unsigned
		case T_64PUOCT: { return T_UOCT; } break; // 64 bit pointer to 128 bit unsigned

		// 128 bit int types
		case T_PINT16:                              // 16 bit pointer to 128 bit signed int
		case T_PFINT16:                             // 16:16 far pointer to 128 bit signed int
		case T_PHINT16:                             // 16:16 huge pointer to 128 bit signed int
		case T_32PINT16:                            // 32 bit pointer to 128 bit signed int
		case T_32PFINT16:                           // 16:32 pointer to 128 bit signed int
		case T_64PINT16: { return T_INT16; } break; // 64 bit pointer to 128 bit signed int

		case T_PUINT16:                               // 16 bit pointer to 128 bit unsigned int
		case T_PFUINT16:                              // 16:16 far pointer to 128 bit unsigned int
		case T_PHUINT16:                              // 16:16 huge pointer to 128 bit unsigned int
		case T_32PUINT16:                             // 32 bit pointer to 128 bit unsigned int
		case T_32PFUINT16:                            // 16:32 pointer to 128 bit unsigned int
		case T_64PUINT16: { return T_UINT16; } break; // 64 bit pointer to 128 bit unsigned int

		// 16 bit real types
		case T_PREAL16:                               // 16 bit pointer to 16 bit real
		case T_PFREAL16:                              // 16:16 far pointer to 16 bit real
		case T_PHREAL16:                              // 16:16 huge pointer to 16 bit real
		case T_32PREAL16:                             // 32 bit pointer to 16 bit real
		case T_32PFREAL16:                            // 16:32 pointer to 16 bit real
		case T_64PREAL16: { return T_REAL16; } break; // 64 bit pointer to 16 bit real

		// 32 bit real types
		case T_PREAL32:                               // 16 bit pointer to 32 bit real
		case T_PFREAL32:                              // 16:16 far pointer to 32 bit real
		case T_PHREAL32:                              // 16:16 huge pointer to 32 bit real
		case T_32PREAL32:                             // 32 bit pointer to 32 bit real
		case T_32PFREAL32:                            // 16:32 pointer to 32 bit real
		case T_64PREAL32: { return T_REAL32; } break; // 64 bit pointer to 32 bit real

		// 32 bit partial-precision real types
		case T_PREAL32PP:                                 // 16 bit pointer to 32 bit PP real
		case T_PFREAL32PP:                                // 16:16 far pointer to 32 bit PP real
		case T_PHREAL32PP:                                // 16:16 huge pointer to 32 bit PP real
		case T_32PREAL32PP:                               // 32 bit pointer to 32 bit PP real
		case T_32PFREAL32PP:                              // 16:32 pointer to 32 bit PP real
		case T_64PREAL32PP: { return T_REAL32PP; } break; // 64 bit pointer to 32 bit PP real

		// 48 bit real types
		case T_PREAL48:                               // 16 bit pointer to 48 bit real
		case T_PFREAL48:                              // 16:16 far pointer to 48 bit real
		case T_PHREAL48:                              // 16:16 huge pointer to 48 bit real
		case T_32PREAL48:                             // 32 bit pointer to 48 bit real
		case T_32PFREAL48:                            // 16:32 pointer to 48 bit real
		case T_64PREAL48: { return T_REAL48; } break; // 64 bit pointer to 48 bit real

		// 64 bit real types
		case T_PREAL64:                               // 16 bit pointer to 64 bit real
		case T_PFREAL64:                              // 16:16 far pointer to 64 bit real
		case T_PHREAL64:                              // 16:16 huge pointer to 64 bit real
		case T_32PREAL64:                             // 32 bit pointer to 64 bit real
		case T_32PFREAL64:                            // 16:32 pointer to 64 bit real
		case T_64PREAL64: { return T_REAL64; } break; // 64 bit pointer to 64 bit real

		// 80 bit real types
		case T_PREAL80:                               // 16 bit pointer to 80 bit real
		case T_PFREAL80:                              // 16:16 far pointer to 80 bit real
		case T_PHREAL80:                              // 16:16 huge pointer to 80 bit real
		case T_32PREAL80:                             // 32 bit pointer to 80 bit real
		case T_32PFREAL80:                            // 16:32 pointer to 80 bit real
		case T_64PREAL80: { return T_REAL80; } break; // 64 bit pointer to 80 bit real

		// 128 bit real types
		case T_PREAL128:                                // 16 bit pointer to 128 bit real
		case T_PFREAL128:                               // 16:16 far pointer to 128 bit real
		case T_PHREAL128:                               // 16:16 huge pointer to 128 bit real
		case T_32PREAL128:                              // 32 bit pointer to 128 bit real
		case T_32PFREAL128:                             // 16:32 pointer to 128 bit real
		case T_64PREAL128: { return T_REAL128; } break; // 64 bit pointer to 128 bit real

		// 32 bit complex types
		case T_PCPLX32:                               // 16 bit pointer to 32 bit complex
		case T_PFCPLX32:                              // 16:16 far pointer to 32 bit complex
		case T_PHCPLX32:                              // 16:16 huge pointer to 32 bit complex
		case T_32PCPLX32:                             // 32 bit pointer to 32 bit complex
		case T_32PFCPLX32:                            // 16:32 pointer to 32 bit complex
		case T_64PCPLX32: { return T_CPLX32; } break; // 64 bit pointer to 32 bit complex

		// 64 bit complex types
		case T_PCPLX64:                               // 16 bit pointer to 64 bit complex
		case T_PFCPLX64:                              // 16:16 far pointer to 64 bit complex
		case T_PHCPLX64:                              // 16:16 huge pointer to 64 bit complex
		case T_32PCPLX64:                             // 32 bit pointer to 64 bit complex
		case T_32PFCPLX64:                            // 16:32 pointer to 64 bit complex
		case T_64PCPLX64: { return T_CPLX64; } break; // 64 bit pointer to 64 bit complex

		// 80 bit complex types
		case T_PCPLX80:                               // 16 bit pointer to 80 bit complex
		case T_PFCPLX80:                              // 16:16 far pointer to 80 bit complex
		case T_PHCPLX80:                              // 16:16 huge pointer to 80 bit complex
		case T_32PCPLX80:                             // 32 bit pointer to 80 bit complex
		case T_32PFCPLX80:                            // 16:32 pointer to 80 bit complex
		case T_64PCPLX80: { return T_CPLX80; } break; // 64 bit pointer to 80 bit complex

		// 128 bit complex types
		case T_PCPLX128:                                // 16 bit pointer to 128 bit complex
		case T_PFCPLX128:                               // 16:16 far pointer to 128 bit complex
		case T_PHCPLX128:                               // 16:16 huge pointer to 128 bit real
		case T_32PCPLX128:                              // 32 bit pointer to 128 bit complex
		case T_32PFCPLX128:                             // 16:32 pointer to 128 bit complex
		case T_64PCPLX128: { return T_CPLX128; } break; // 64 bit pointer to 128 bit complex

		// boolean types
		case T_PBOOL08:                               // 16 bit pointer to  8 bit boolean
		case T_PFBOOL08:                              // 16:16 far pointer to  8 bit boolean
		case T_PHBOOL08:                              // 16:16 huge pointer to  8 bit boolean
		case T_32PBOOL08:                             // 32 bit pointer to 8 bit boolean
		case T_32PFBOOL08:                            // 16:32 pointer to 8 bit boolean
		case T_64PBOOL08: { return T_BOOL08; } break; // 64 bit pointer to 8 bit boolean

		case T_PBOOL16:                               // 16 bit pointer to 16 bit boolean
		case T_PFBOOL16:                              // 16:16 far pointer to 16 bit boolean
		case T_PHBOOL16:                              // 16:16 huge pointer to 16 bit boolean
		case T_32PBOOL16:                             // 32 bit pointer to 18 bit boolean
		case T_32PFBOOL16:                            // 16:32 pointer to 16 bit boolean
		case T_64PBOOL16: { return T_BOOL16; } break; // 64 bit pointer to 18 bit boolean

		case T_PBOOL32:                               // 16 bit pointer to 32 bit boolean
		case T_PFBOOL32:                              // 16:16 far pointer to 32 bit boolean
		case T_PHBOOL32:                              // 16:16 huge pointer to 32 bit boolean
		case T_32PBOOL32:                             // 32 bit pointer to 32 bit boolean
		case T_32PFBOOL32:                            // 16:32 pointer to 32 bit boolean
		case T_64PBOOL32: { return T_BOOL32; } break; // 64 bit pointer to 32 bit boolean

		case T_PBOOL64:                               // 16 bit pointer to 64 bit boolean
		case T_PFBOOL64:                              // 16:16 far pointer to 64 bit boolean
		case T_PHBOOL64:                              // 16:16 huge pointer to 64 bit boolean
		case T_32PBOOL64:                             // 32 bit pointer to 64 bit boolean
		case T_32PFBOOL64:                            // 16:32 pointer to 64 bit boolean
		case T_64PBOOL64: { return T_BOOL64; } break; // 64 bit pointer to 64 bit boolean

		default: {
			ASSERT(false, "Unknown size!");
		}
	}
	return 0;
}


int get_primitive_size(CV_typ_t type) {
	ASSERT(CV_IS_PRIMITIVE(type), "blah");

	switch (type) {
		// case T_PVOID: { return near / 4; } break;   // near pointer to void
		// case T_PFVOID: { return far / 4; } break;   // far pointer to void
		// case T_PHVOID: { return huge / 4; } break;   // huge pointer to void
		case T_32PVOID:  { return 4; } break; // 32 bit pointer to void
		case T_32PFVOID: { return 2; } break; // 16:32 pointer to void
		case T_64PVOID:  { return 8; } break; // 64 bit pointer to void
		case T_BOOL32FF: { return 4; } break; // 32-bit BOOL where true is 0xffffffff

		// Character types
		case T_CHAR:      { return 1; } break; // 8 bit signed
		case T_PCHAR:     { return 2; } break; // 16 bit pointer to 8 bit signed
		case T_PFCHAR:    { return 2; } break; // 16:16 far pointer to 8 bit signed
		case T_PHCHAR:    { return 2; } break; // 16:16 huge pointer to 8 bit signed
		case T_32PCHAR:   { return 4; } break; // 32 bit pointer to 8 bit signed
		case T_32PFCHAR:  { return 2; } break; // 16:32 pointer to 8 bit signed
		case T_64PCHAR:   { return 8; } break; // 64 bit pointer to 8 bit signed

		case T_UCHAR:     { return 1; } break; // 8 bit unsigned
		case T_PUCHAR:    { return 2; } break; // 16 bit pointer to 8 bit unsigned
		case T_PFUCHAR:   { return 2; } break; // 16:16 far pointer to 8 bit unsigned
		case T_PHUCHAR:   { return 2; } break; // 16:16 huge pointer to 8 bit unsigned
		case T_32PUCHAR:  { return 4; } break; // 32 bit pointer to 8 bit unsigned
		case T_32PFUCHAR: { return 2; } break; // 16:32 pointer to 8 bit unsigned
		case T_64PUCHAR:  { return 8; } break; // 64 bit pointer to 8 bit unsigned

		// really a character types
		// case T_RCHAR:     { return 1; } break; // really a char
		case T_PRCHAR:    { return 2; } break; // 16 bit pointer to a real char
		case T_PFRCHAR:   { return 2; } break; // 16:16 far pointer to a real char
		case T_PHRCHAR:   { return 2; } break; // 16:16 huge pointer to a real char
		case T_32PRCHAR:  { return 4; } break; // 32 bit pointer to a real char
		case T_32PFRCHAR: { return 2; } break; // 16:32 pointer to a real char
		case T_64PRCHAR:  { return 8; } break; // 64 bit pointer to a real char

		// really a wide character types
		case T_WCHAR:     { return 2; } break; // wide char
		case T_PWCHAR:    { return 2; } break; // 16 bit pointer to a wide char
		case T_PFWCHAR:   { return 2; } break; // 16:16 far pointer to a wide char
		case T_PHWCHAR:   { return 2; } break; // 16:16 huge pointer to a wide char
		case T_32PWCHAR:  { return 4; } break; // 32 bit pointer to a wide char
		case T_32PFWCHAR: { return 2; } break; // 16:32 pointer to a wide char
		case T_64PWCHAR:  { return 8; } break; // 64 bit pointer to a wide char

		// really a 16-bit unicode char
		case T_CHAR16:     { return 2; } break; // 16-bit unicode char
		case T_PCHAR16:    { return 2; } break; // 16 bit pointer to a 16-bit unicode char
		case T_PFCHAR16:   { return 2; } break; // 16:16 far pointer to a 16-bit unicode char
		case T_PHCHAR16:   { return 2; } break; // 16:16 huge pointer to a 16-bit unicode char
		case T_32PCHAR16:  { return 4; } break; // 32 bit pointer to a 16-bit unicode char
		case T_32PFCHAR16: { return 2; } break; // 16:32 pointer to a 16-bit unicode char
		case T_64PCHAR16:  { return 8; } break; // 64 bit pointer to a 16-bit unicode char

		// really a 32-bit unicode char
		case T_CHAR32:     { return 4; } break; // 32-bit unicode char
		case T_PCHAR32:    { return 2; } break; // 16 bit pointer to a 32-bit unicode char
		case T_PFCHAR32:   { return 2; } break; // 16:16 far pointer to a 32-bit unicode char
		case T_PHCHAR32:   { return 2; } break; // 16:16 huge pointer to a 32-bit unicode char
		case T_32PCHAR32:  { return 4; } break; // 32 bit pointer to a 32-bit unicode char
		case T_32PFCHAR32: { return 2; } break; // 16:32 pointer to a 32-bit unicode char
		case T_64PCHAR32:  { return 8; } break; // 64 bit pointer to a 32-bit unicode char

		// 8 bit int types
		case T_INT1:      { return 1; } break; // 8 bit signed int
		case T_PINT1:     { return 2; } break; // 16 bit pointer to 8 bit signed int
		case T_PFINT1:    { return 2; } break; // 16:16 far pointer to 8 bit signed int
		case T_PHINT1:    { return 2; } break; // 16:16 huge pointer to 8 bit signed int
		case T_32PINT1:   { return 4; } break; // 32 bit pointer to 8 bit signed int
		case T_32PFINT1:  { return 2; } break; // 16:32 pointer to 8 bit signed int
		case T_64PINT1:   { return 8; } break; // 64 bit pointer to 8 bit signed int

		case T_UINT1:     { return 1; } break; // 8 bit unsigned int
		case T_PUINT1:    { return 2; } break; // 16 bit pointer to 8 bit unsigned int
		case T_PFUINT1:   { return 2; } break; // 16:16 far pointer to 8 bit unsigned int
		case T_PHUINT1:   { return 2; } break; // 16:16 huge pointer to 8 bit unsigned int
		case T_32PUINT1:  { return 4; } break; // 32 bit pointer to 8 bit unsigned int
		case T_32PFUINT1: { return 2; } break; // 16:32 pointer to 8 bit unsigned int
		case T_64PUINT1:  { return 8; } break; // 64 bit pointer to 8 bit unsigned int

		// 16 bit short types
		case T_SHORT:      { return 2; } break; // 16 bit signed
		case T_PSHORT:     { return 2; } break; // 16 bit pointer to 16 bit signed
		case T_PFSHORT:    { return 2; } break; // 16:16 far pointer to 16 bit signed
		case T_PHSHORT:    { return 2; } break; // 16:16 huge pointer to 16 bit signed
		case T_32PSHORT:   { return 4; } break; // 32 bit pointer to 16 bit signed
		case T_32PFSHORT:  { return 2; } break; // 16:32 pointer to 16 bit signed
		case T_64PSHORT:   { return 8; } break; // 64 bit pointer to 16 bit signed

		case T_USHORT:     { return 2; } break; // 16 bit unsigned
		case T_PUSHORT:    { return 2; } break; // 16 bit pointer to 16 bit unsigned
		case T_PFUSHORT:   { return 2; } break; // 16:16 far pointer to 16 bit unsigned
		case T_PHUSHORT:   { return 2; } break; // 16:16 huge pointer to 16 bit unsigned
		case T_32PUSHORT:  { return 4; } break; // 32 bit pointer to 16 bit unsigned
		case T_32PFUSHORT: { return 2; } break; // 16:32 pointer to 16 bit unsigned
		case T_64PUSHORT:  { return 8; } break; // 64 bit pointer to 16 bit unsigned

		// 16 bit int types
		case T_INT2:      { return 2; } break; // 16 bit signed int
		case T_PINT2:     { return 2; } break; // 16 bit pointer to 16 bit signed int
		case T_PFINT2:    { return 2; } break; // 16:16 far pointer to 16 bit signed int
		case T_PHINT2:    { return 2; } break; // 16:16 huge pointer to 16 bit signed int
		case T_32PINT2:   { return 4; } break; // 32 bit pointer to 16 bit signed int
		case T_32PFINT2:  { return 2; } break; // 16:32 pointer to 16 bit signed int
		case T_64PINT2:   { return 8; } break; // 64 bit pointer to 16 bit signed int

		case T_UINT2:     { return 2; } break; // 16 bit unsigned int
		case T_PUINT2:    { return 2; } break; // 16 bit pointer to 16 bit unsigned int
		case T_PFUINT2:   { return 2; } break; // 16:16 far pointer to 16 bit unsigned int
		case T_PHUINT2:   { return 2; } break; // 16:16 huge pointer to 16 bit unsigned int
		case T_32PUINT2:  { return 4; } break; // 32 bit pointer to 16 bit unsigned int
		case T_32PFUINT2: { return 2; } break; // 16:32 pointer to 16 bit unsigned int
		case T_64PUINT2:  { return 8; } break; // 64 bit pointer to 16 bit unsigned int


		// 32 bit long types
		case T_LONG:      { return 4; } break; // 32 bit signed
		case T_ULONG:     { return 4; } break; // 32 bit unsigned
		case T_PLONG:     { return 2; } break; // 16 bit pointer to 32 bit signed
		case T_PULONG:    { return 2; } break; // 16 bit pointer to 32 bit unsigned
		case T_PFLONG:    { return 2; } break; // 16:16 far pointer to 32 bit signed
		case T_PFULONG:   { return 2; } break; // 16:16 far pointer to 32 bit unsigned
		case T_PHLONG:    { return 2; } break; // 16:16 huge pointer to 32 bit signed
		case T_PHULONG:   { return 2; } break; // 16:16 huge pointer to 32 bit unsigned

		case T_32PLONG:   { return 4; } break; // 32 bit pointer to 32 bit signed
		case T_32PULONG:     { return  4; } break; // 32 bit pointer to 32 bit unsigned
		case T_32PFLONG:     { return  2; } break; // 16:32 pointer to 32 bit signed
		case T_32PFULONG:    { return  2; } break; // 16:32 pointer to 32 bit unsigned
		case T_64PLONG:      { return  8; } break; // 64 bit pointer to 32 bit signed
		case T_64PULONG:     { return  8; } break; // 64 bit pointer to 32 bit unsigned


		// 32 bit int types
		case T_INT4:         { return  4; } break; // 32 bit signed int
		case T_PINT4:        { return  2; } break; // 16 bit pointer to 32 bit signed int
		case T_PFINT4:       { return  2; } break; // 16:16 far pointer to 32 bit signed int
		case T_PHINT4:       { return  2; } break; // 16:16 huge pointer to 32 bit signed int
		case T_32PINT4:      { return  4; } break; // 32 bit pointer to 32 bit signed int
		case T_32PFINT4:     { return  2; } break; // 16:32 pointer to 32 bit signed int
		case T_64PINT4:      { return  8; } break; // 64 bit pointer to 32 bit signed int

		case T_UINT4:        { return  4; } break; // 32 bit unsigned int
		case T_PUINT4:       { return  2; } break; // 16 bit pointer to 32 bit unsigned int
		case T_PFUINT4:      { return  2; } break; // 16:16 far pointer to 32 bit unsigned int
		case T_PHUINT4:      { return  2; } break; // 16:16 huge pointer to 32 bit unsigned int
		case T_32PUINT4:     { return  4; } break; // 32 bit pointer to 32 bit unsigned int
		case T_32PFUINT4:    { return  2; } break; // 16:32 pointer to 32 bit unsigned int
		case T_64PUINT4:     { return  8; } break; // 64 bit pointer to 32 bit unsigned int


		// 64 bit quad types
		case T_QUAD:         { return  8; } break; // 64 bit signed
		case T_PQUAD:        { return  2; } break; // 16 bit pointer to 64 bit signed
		case T_PFQUAD:       { return  2; } break; // 16:16 far pointer to 64 bit signed
		case T_PHQUAD:       { return  2; } break; // 16:16 huge pointer to 64 bit signed
		case T_32PQUAD:      { return  4; } break; // 32 bit pointer to 64 bit signed
		case T_32PFQUAD:     { return  2; } break; // 16:32 pointer to 64 bit signed
		case T_64PQUAD:      { return  8; } break; // 64 bit pointer to 64 bit signed

		case T_UQUAD:        { return  8; } break; // 64 bit unsigned
		case T_PUQUAD:       { return  2; } break; // 16 bit pointer to 64 bit unsigned
		case T_PFUQUAD:      { return  2; } break; // 16:16 far pointer to 64 bit unsigned
		case T_PHUQUAD:      { return  2; } break; // 16:16 huge pointer to 64 bit unsigned
		case T_32PUQUAD:     { return  4; } break; // 32 bit pointer to 64 bit unsigned
		case T_32PFUQUAD:    { return  2; } break; // 16:32 pointer to 64 bit unsigned
		case T_64PUQUAD:     { return  8; } break; // 64 bit pointer to 64 bit unsigned


		// 64 bit int types
		case T_INT8:         { return  8; } break; // 64 bit signed int
		case T_PINT8:        { return  2; } break; // 16 bit pointer to 64 bit signed int
		case T_PFINT8:       { return  2; } break; // 16:16 far pointer to 64 bit signed int
		case T_PHINT8:       { return  2; } break; // 16:16 huge pointer to 64 bit signed int
		case T_32PINT8:      { return  4; } break; // 32 bit pointer to 64 bit signed int
		case T_32PFINT8:     { return  2; } break; // 16:32 pointer to 64 bit signed int
		case T_64PINT8:      { return  8; } break; // 64 bit pointer to 64 bit signed int

		case T_UINT8:        { return  8; } break; // 64 bit unsigned int
		case T_PUINT8:       { return  2; } break; // 16 bit pointer to 64 bit unsigned int
		case T_PFUINT8:      { return  2; } break; // 16:16 far pointer to 64 bit unsigned int
		case T_PHUINT8:      { return  2; } break; // 16:16 huge pointer to 64 bit unsigned int
		case T_32PUINT8:     { return  4; } break; // 32 bit pointer to 64 bit unsigned int
		case T_32PFUINT8:    { return  2; } break; // 16:32 pointer to 64 bit unsigned int
		case T_64PUINT8:     { return  8; } break; // 64 bit pointer to 64 bit unsigned int


		// 128 bit octet types
		case T_OCT:          { return 16; } break; // 128 bit signed
		case T_POCT:         { return  2; } break; // 16 bit pointer to 128 bit signed
		case T_PFOCT:        { return  2; } break; // 16:16 far pointer to 128 bit signed
		case T_PHOCT:        { return  2; } break; // 16:16 huge pointer to 128 bit signed
		case T_32POCT:       { return  4; } break; // 32 bit pointer to 128 bit signed
		case T_32PFOCT:      { return  2; } break; // 16:32 pointer to 128 bit signed
		case T_64POCT:       { return  8; } break; // 64 bit pointer to 128 bit signed

		case T_UOCT:         { return 16; } break; // 128 bit unsigned
		case T_PUOCT:        { return  2; } break; // 16 bit pointer to 128 bit unsigned
		case T_PFUOCT:       { return  2; } break; // 16:16 far pointer to 128 bit unsigned
		case T_PHUOCT:       { return  2; } break; // 16:16 huge pointer to 128 bit unsigned
		case T_32PUOCT:      { return  4; } break; // 32 bit pointer to 128 bit unsigned
		case T_32PFUOCT:     { return  2; } break; // 16:32 pointer to 128 bit unsigned
		case T_64PUOCT:      { return  8; } break; // 64 bit pointer to 128 bit unsigned


		// 128 bit int types
		case T_INT16:        { return 16; } break; // 128 bit signed int
		case T_PINT16:       { return  2; } break; // 16 bit pointer to 128 bit signed int
		case T_PFINT16:      { return  2; } break; // 16:16 far pointer to 128 bit signed int
		case T_PHINT16:      { return  2; } break; // 16:16 huge pointer to 128 bit signed int
		case T_32PINT16:     { return  4; } break; // 32 bit pointer to 128 bit signed int
		case T_32PFINT16:    { return  2; } break; // 16:32 pointer to 128 bit signed int
		case T_64PINT16:     { return  8; } break; // 64 bit pointer to 128 bit signed int

		case T_UINT16:       { return 16; } break; // 128 bit unsigned int
		case T_PUINT16:      { return  2; } break; // 16 bit pointer to 128 bit unsigned int
		case T_PFUINT16:     { return  2; } break; // 16:16 far pointer to 128 bit unsigned int
		case T_PHUINT16:     { return  2; } break; // 16:16 huge pointer to 128 bit unsigned int
		case T_32PUINT16:    { return  4; } break; // 32 bit pointer to 128 bit unsigned int
		case T_32PFUINT16:   { return  2; } break; // 16:32 pointer to 128 bit unsigned int
		case T_64PUINT16:    { return  8; } break; // 64 bit pointer to 128 bit unsigned int


		// 16 bit real types
		case T_REAL16:       { return  2; } break; // 16 bit real
		case T_PREAL16:      { return  2; } break; // 16 bit pointer to 16 bit real
		case T_PFREAL16:     { return  2; } break; // 16:16 far pointer to 16 bit real
		case T_PHREAL16:     { return  2; } break; // 16:16 huge pointer to 16 bit real
		case T_32PREAL16:    { return  4; } break; // 32 bit pointer to 16 bit real
		case T_32PFREAL16:   { return  2; } break; // 16:32 pointer to 16 bit real
		case T_64PREAL16:    { return  8; } break; // 64 bit pointer to 16 bit real


		// 32 bit real types
		case T_REAL32:       { return  4; } break; // 32 bit real
		case T_PREAL32:      { return  2; } break; // 16 bit pointer to 32 bit real
		case T_PFREAL32:     { return  2; } break; // 16:16 far pointer to 32 bit real
		case T_PHREAL32:     { return  2; } break; // 16:16 huge pointer to 32 bit real
		case T_32PREAL32:    { return  4; } break; // 32 bit pointer to 32 bit real
		case T_32PFREAL32:   { return  2; } break; // 16:32 pointer to 32 bit real
		case T_64PREAL32:    { return  8; } break; // 64 bit pointer to 32 bit real


		// 32 bit partial-precision real types
		case T_REAL32PP:     { return  4; } break; // 32 bit PP real
		case T_PREAL32PP:    { return  2; } break; // 16 bit pointer to 32 bit PP real
		case T_PFREAL32PP:   { return  2; } break; // 16:16 far pointer to 32 bit PP real
		case T_PHREAL32PP:   { return  2; } break; // 16:16 huge pointer to 32 bit PP real
		case T_32PREAL32PP:  { return  4; } break; // 32 bit pointer to 32 bit PP real
		case T_32PFREAL32PP: { return  2; } break; // 16:32 pointer to 32 bit PP real
		case T_64PREAL32PP:  { return  8; } break; // 64 bit pointer to 32 bit PP real


		// 48 bit real types
		case T_REAL48:       { return  6; } break; // 48 bit real
		case T_PREAL48:      { return  2; } break; // 16 bit pointer to 48 bit real
		case T_PFREAL48:     { return  2; } break; // 16:16 far pointer to 48 bit real
		case T_PHREAL48:     { return  2; } break; // 16:16 huge pointer to 48 bit real
		case T_32PREAL48:    { return  4; } break; // 32 bit pointer to 48 bit real
		case T_32PFREAL48:   { return  2; } break; // 16:32 pointer to 48 bit real
		case T_64PREAL48:    { return  8; } break; // 64 bit pointer to 48 bit real


		// 64 bit real types
		case T_REAL64:       { return  8; } break; // 64 bit real
		case T_PREAL64:      { return  2; } break; // 16 bit pointer to 64 bit real
		case T_PFREAL64:     { return  2; } break; // 16:16 far pointer to 64 bit real
		case T_PHREAL64:     { return  2; } break; // 16:16 huge pointer to 64 bit real
		case T_32PREAL64:    { return  4; } break; // 32 bit pointer to 64 bit real
		case T_32PFREAL64:   { return  2; } break; // 16:32 pointer to 64 bit real
		case T_64PREAL64:    { return  8; } break; // 64 bit pointer to 64 bit real


		// 80 bit real types
		case T_REAL80:       { return 10; } break; // 80 bit real
		case T_PREAL80:      { return  2; } break; // 16 bit pointer to 80 bit real
		case T_PFREAL80:     { return  2; } break; // 16:16 far pointer to 80 bit real
		case T_PHREAL80:     { return  2; } break; // 16:16 huge pointer to 80 bit real
		case T_32PREAL80:    { return  4; } break; // 32 bit pointer to 80 bit real
		case T_32PFREAL80:   { return  2; } break; // 16:32 pointer to 80 bit real
		case T_64PREAL80:    { return  8; } break; // 64 bit pointer to 80 bit real


		// 128 bit real types
		case T_REAL128:      { return 16; } break; // 128 bit real
		case T_PREAL128:     { return  2; } break; // 16 bit pointer to 128 bit real
		case T_PFREAL128:    { return  2; } break; // 16:16 far pointer to 128 bit real
		case T_PHREAL128:    { return  2; } break; // 16:16 huge pointer to 128 bit real
		case T_32PREAL128:   { return  4; } break; // 32 bit pointer to 128 bit real
		case T_32PFREAL128:  { return  2; } break; // 16:32 pointer to 128 bit real
		case T_64PREAL128:   { return  8; } break; // 64 bit pointer to 128 bit real


		// 32 bit complex types
		case T_CPLX32:       { return  4; } break; // 32 bit complex
		case T_PCPLX32:      { return  2; } break; // 16 bit pointer to 32 bit complex
		case T_PFCPLX32:     { return  2; } break; // 16:16 far pointer to 32 bit complex
		case T_PHCPLX32:     { return  2; } break; // 16:16 huge pointer to 32 bit complex
		case T_32PCPLX32:    { return  4; } break; // 32 bit pointer to 32 bit complex
		case T_32PFCPLX32:   { return  2; } break; // 16:32 pointer to 32 bit complex
		case T_64PCPLX32:    { return  8; } break; // 64 bit pointer to 32 bit complex


		// 64 bit complex types
		case T_CPLX64:       { return  8; } break; // 64 bit complex
		case T_PCPLX64:      { return  2; } break; // 16 bit pointer to 64 bit complex
		case T_PFCPLX64:     { return  2; } break; // 16:16 far pointer to 64 bit complex
		case T_PHCPLX64:     { return  2; } break; // 16:16 huge pointer to 64 bit complex
		case T_32PCPLX64:    { return  4; } break; // 32 bit pointer to 64 bit complex
		case T_32PFCPLX64:   { return  2; } break; // 16:32 pointer to 64 bit complex
		case T_64PCPLX64:    { return  8; } break; // 64 bit pointer to 64 bit complex


		// 80 bit complex types
		case T_CPLX80:       { return 10; } break; // 80 bit complex
		case T_PCPLX80:      { return  2; } break; // 16 bit pointer to 80 bit complex
		case T_PFCPLX80:     { return  2; } break; // 16:16 far pointer to 80 bit complex
		case T_PHCPLX80:     { return  2; } break; // 16:16 huge pointer to 80 bit complex
		case T_32PCPLX80:    { return  4; } break; // 32 bit pointer to 80 bit complex
		case T_32PFCPLX80:   { return  2; } break; // 16:32 pointer to 80 bit complex
		case T_64PCPLX80:    { return  8; } break; // 64 bit pointer to 80 bit complex


		// 128 bit complex types
		case T_CPLX128:      { return 16; } break; // 128 bit complex
		case T_PCPLX128:     { return  2; } break; // 16 bit pointer to 128 bit complex
		case T_PFCPLX128:    { return  2; } break; // 16:16 far pointer to 128 bit complex
		case T_PHCPLX128:    { return  2; } break; // 16:16 huge pointer to 128 bit real
		case T_32PCPLX128:   { return  4; } break; // 32 bit pointer to 128 bit complex
		case T_32PFCPLX128:  { return  2; } break; // 16:32 pointer to 128 bit complex
		case T_64PCPLX128:   { return  8; } break; // 64 bit pointer to 128 bit complex


		// boolean types
		case T_BOOL08:       { return  1; } break; // 8 bit boolean
		case T_PBOOL08:      { return  2; } break; // 16 bit pointer to  8 bit boolean
		case T_PFBOOL08:     { return  2; } break; // 16:16 far pointer to  8 bit boolean
		case T_PHBOOL08:     { return  2; } break; // 16:16 huge pointer to  8 bit boolean
		case T_32PBOOL08:    { return  4; } break; // 32 bit pointer to 8 bit boolean
		case T_32PFBOOL08:   { return  2; } break; // 16:32 pointer to 8 bit boolean
		case T_64PBOOL08:    { return  8; } break; // 64 bit pointer to 8 bit boolean

		case T_BOOL16:       { return  2; } break; // 16 bit boolean
		case T_PBOOL16:      { return  2; } break; // 16 bit pointer to 16 bit boolean
		case T_PFBOOL16:     { return  2; } break; // 16:16 far pointer to 16 bit boolean
		case T_PHBOOL16:     { return  2; } break; // 16:16 huge pointer to 16 bit boolean
		case T_32PBOOL16:    { return  4; } break; // 32 bit pointer to 18 bit boolean
		case T_32PFBOOL16:   { return  2; } break; // 16:32 pointer to 16 bit boolean
		case T_64PBOOL16:    { return  8; } break; // 64 bit pointer to 18 bit boolean

		case T_BOOL32:       { return  4; } break; // 32 bit boolean
		case T_PBOOL32:      { return  2; } break; // 16 bit pointer to 32 bit boolean
		case T_PFBOOL32:     { return  2; } break; // 16:16 far pointer to 32 bit boolean
		case T_PHBOOL32:     { return  2; } break; // 16:16 huge pointer to 32 bit boolean
		case T_32PBOOL32:    { return  4; } break; // 32 bit pointer to 32 bit boolean
		case T_32PFBOOL32:   { return  2; } break; // 16:32 pointer to 32 bit boolean
		case T_64PBOOL32:    { return  8; } break; // 64 bit pointer to 32 bit boolean

		case T_BOOL64:       { return  8; } break; // 64 bit boolean
		case T_PBOOL64:      { return  2; } break; // 16 bit pointer to 64 bit boolean
		case T_PFBOOL64:     { return  2; } break; // 16:16 far pointer to 64 bit boolean
		case T_PHBOOL64:     { return  2; } break; // 16:16 huge pointer to 64 bit boolean
		case T_32PBOOL64:    { return  4; } break; // 32 bit pointer to 64 bit boolean
		case T_32PFBOOL64:   { return  2; } break; // 16:32 pointer to 64 bit boolean
		case T_64PBOOL64:    { return  8; } break; // 64 bit pointer to 64 bit boolean


		// ???
		// case T_NCVPTR: { return CV / 4; } break;   // CV Internal type for created near pointers
		// case T_FCVPTR: { return CV / 4; } break;   // CV Internal type for created far pointers
		// case T_HCVPTR: { return CV / 4; } break;   // CV Internal type for created huge pointers
		// case T_32NCVPTR: { return CV / 4; } break;   // CV Internal type for created near 32-bit pointers
		// case T_32FCVPTR: { return CV / 4; } break;   // CV Internal type for created far 32-bit pointers
		// case T_64NCVPTR: { return CV / 4; } break;   // CV Internal type for created near 64-bit pointers
		default: {
			ASSERT(false, "Unknown size!");
		}
	}
	return 0;
}

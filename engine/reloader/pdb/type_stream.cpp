// From pdbdump.cpp
size_t CbExtractNumeric(uint8_t *pb, uint32_t *pul) {
	uint16_t leaf = *(uint16_t *) pb;

	if (leaf < LF_NUMERIC) {
		*pul = leaf;

		return sizeof(leaf);
	}

	switch (leaf) {
		case LF_CHAR:
			*pul = *(char *) pb;
			return sizeof(leaf) + sizeof(char);

		case LF_SHORT:
			*pul = *(short *) pb;
			return sizeof(leaf) + sizeof(short);

		case LF_USHORT:
			*pul = *(uint16_t *) pb;
			return sizeof(leaf) + sizeof(uint16_t);

		case LF_LONG:
			*pul = *(long *) pb;
			return sizeof(leaf) + sizeof(long);

		case LF_ULONG:
			*pul = *(uint32_t *) pb;
			return sizeof(leaf) + sizeof(uint32_t);

		case LF_QUADWORD:
			return sizeof(leaf) + sizeof(__int64);

		case LF_UQUADWORD:
			return sizeof(leaf) + sizeof(unsigned __int64);
	}

	return 0;
}

char *extract_info(uint8_t *at, uint32_t *size) {
	size_t cbOffset = CbExtractNumeric(at, size);
	return (char *) at + cbOffset;
}
char *extract_info(Stream &stream, uint32_t *size) {
	uint8_t *at = data(stream);

	size_t cbOffset = CbExtractNumeric(at, size);

	char *name = (char *)at + cbOffset;
	stream.cursor += cbOffset + strlen(name) + 1;

	check_padding(stream);

	return name;
}

char *extract_name(Stream &stream, uint8_t *at) {
	char *name = (char *)at;
	stream.cursor += strlen(name);
	check_padding(stream);

	return name;
}

void extract_size(uint8_t *at, uint32_t *size) {
	size_t cbOffset = CbExtractNumeric(at, size);
}
void extract_size(Stream &stream, uint32_t *offset) {
	uint8_t *at = data(stream);
	size_t cbOffset = CbExtractNumeric(at, offset);
	stream.cursor += cbOffset;
	check_padding(stream);
}

void parse_fieldlist(MemberList &member_list, Stream &stream) {
	uint16_t leaf_type = peek_u16(stream);
	// log_info("PDB", "FieldList type %#06hx", leaf_type);
	switch (leaf_type) {
		case LF_BCLASS: {
			lfBClass *field = (lfBClass*) read(stream, sizeof(lfBClass));
			uint32_t size;
			extract_size(stream, &size);

			// log_info("PDB", "\t[bclass][%u][%lu]", size, field->index); // type index of base class
		} break;
		case LF_VBCLASS:
		case LF_IVBCLASS: {
			lfVBClass *field = (lfVBClass*) read(stream, sizeof(lfVBClass));
			uint32_t size;
			extract_size(stream, &size);

			// log_info("PDB", "\t[vbclass][%u][%lu,%lu]", size,
				// field->index, // type index of direct virtual base class
				// field->vbptr); // type index of virtual base pointer
		} break;
		case LF_INDEX: {
			lfIndex *field = (lfIndex*) read(stream, sizeof(lfIndex));

			// log_info("PDB", "\t[index][%lu]", field->index); // type index of referenced leaf
		} break;
		case LF_VFUNCTAB: {
			lfVFuncTab *field = (lfVFuncTab*) read(stream, sizeof(lfVFuncTab));

			// log_info("PDB", "\t[vfunctab][%lu]", field->type); // type index of pointer
		} break;
		case LF_FRIENDCLS: {
			lfFriendCls *field = (lfFriendCls*) read(stream, sizeof(lfFriendCls));

			// log_info("PDB", "\t[friend class][%lu]", field->index); // index to type record of friend class
		} break;
		case LF_VFUNCOFF: {
			lfVFuncOff *field = (lfVFuncOff*) read(stream, sizeof(lfVFuncOff));

			// log_info("PDB", "\t[virtual function offset][%lu]", field->type); // type index of pointer
		} break;
		case LF_TYPESERVER: {
			lfTypeServer *field = (lfTypeServer*) read(stream, sizeof(lfTypeServer));
			ASSERT(false, "Not implemented!");
			// field->name // unsigned char   name[CV_ZEROLEN];	 // length prefixed name of PDB
		} break;
		case LF_ENUMERATE: {
			lfEnumerate *field = (lfEnumerate*) read(stream, sizeof(lfEnumerate));
			uint32_t size;
			char *name = extract_info(stream, &size);

			// log_info("PDB", "\t[enumerate][%s(%u)]", name, size);
		} break;
		case LF_ARRAY: {
			lfArray *field = (lfArray*) read(stream, sizeof(lfArray));
			uint32_t size;
			char *name = extract_info(stream, &size);

			// log_info("PDB", "\t[array][%s(%u)][%lu,%lu]", name, size,
				// field->elemtype, // type index of element type
				// field->idxtype); // type index of indexing type
		} break;
		case LF_CLASS:
		case LF_STRUCTURE: {
			lfStructure *field = (lfStructure*) read(stream, sizeof(lfStructure));
			uint32_t size;
			char *name = extract_info(stream, &size);

			// log_info("PDB", "\t[structure][%s(%u)][%lu,%lu,%lu]", name, size,
				// field->field, // type index of LF_FIELD descriptor list
				// field->derived, // type index of derived from list if not zero
				// field->vshape); // type index of vshape table for this class
		} break;
		case LF_UNION: {
			lfUnion *field = (lfUnion*) read(stream, sizeof(lfUnion));
			uint32_t size;
			char *name = extract_info(stream, &size);

			// log_info("PDB", "\t[union][%s(%u)][%lu,%lu,%lu]", name, size, field->field); // type index of LF_FIELD descriptor list
		} break;
		case LF_ENUM: {
			lfEnum *field = (lfEnum*) read(stream, sizeof(lfEnum));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[enum][%s][%lu,%lu]", name,
				// field->utype, // underlying type of the enum
				// field->field); // type index of LF_FIELD descriptor list
		} break;
		case LF_DIMARRAY: {
			lfDimArray *field = (lfDimArray*) read(stream, sizeof(lfDimArray));
			char *name = extract_name(stream, field->name);

			// log_info("PDB", "\t[dim-array][%s][%lu,%lu]", name,
				// field->utype, // underlying type of the array
				// field->diminfo); // dimension information
		} break;
		case LF_PRECOMP: {
			lfPreComp *field = (lfPreComp*) read(stream, sizeof(lfPreComp));
			ASSERT(false, "Not implemented!");
		} break;
		case LF_ALIAS: {
			lfAlias *field = (lfAlias*) read(stream, sizeof(lfAlias));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[alias][%s][%lu]", name, field->utype); // underlying type
		} break;
		case LF_DEFARG: {
			lfDefArg *field = (lfDefArg*) read(stream, sizeof(lfDefArg));
			ASSERT(false, "Not implemented!");
		} break;
		case LF_FRIENDFCN_ST:
		case LF_FRIENDFCN: {
			lfFriendFcn *field = (lfFriendFcn*) read(stream, sizeof(lfFriendFcn));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[friend function][%s]", name, field->index); // index to type record of friend function
		} break;
		case LF_MEMBER_ST:
		case LF_MEMBER: {
			lfMember *field = (lfMember*) read(stream, sizeof(lfMember));
			uint32_t size;
			char *name = extract_info(stream, &size);

			Type type = {};
			type.data = field;
			array_push(member_list.types, type);

			TypeName type_name = make_type_name(name);
			array_push(member_list.names, type_name);

			array_push(member_list.flags, 0);

			// log_info("PDB", "\t[member][%s(%u)][%lu]", name, size, field->index); // index of type record for field
		} break;
		case LF_STMEMBER_ST:
		case LF_STMEMBER: {
			lfSTMember *field = (lfSTMember*) read(stream, sizeof(lfSTMember));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[st-member][%s][%lu]", name, field->index); // index of type record for field
		} break;
		case LF_METHOD_ST:
		case LF_METHOD: {
			lfMethod *field = (lfMethod*) read(stream, sizeof(lfMethod));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[method][%s][%lu]", name, field->mList); // index to LF_METHODLIST record
		} break;
		case LF_NESTTYPE_ST:
		case LF_NESTTYPE: {
			lfNestType *field = (lfNestType*) read(stream, sizeof(lfNestType));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[nest-type][%s][%lu]", name, field->index); // index of nested type definition
		} break;
		case LF_ONEMETHOD_ST:
		case LF_ONEMETHOD: {
			lfOneMethod *field = (lfOneMethod*) read(stream, sizeof(lfOneMethod));

			uint32_t size = 0;
			if (field->attr.mprop == CV_MTintro || field->attr.mprop == CV_MTpureintro) {
				size = read_u32(stream);
			}
			char *name = (char*)(stream.data + stream.cursor);
			size_t name_length = strlen(name) + 1;
			stream.cursor += name_length;

			check_padding(stream);

			// log_info("PDB", "\t[one-method][%s(%u)][%lu]", name, size, field->index); // index to type record for procedure
		} break;
		case LF_NESTTYPEEX_ST:
		case LF_NESTTYPEEX: {
			lfNestTypeEx *field = (lfNestTypeEx*) read(stream, sizeof(lfNestTypeEx));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[nest-type-ex][%s][%lu]", name, field->index); // index of nested type definition
		} break;
		case LF_MEMBERMODIFY_ST:
		case LF_MEMBERMODIFY: {
			lfMemberModify *field = (lfMemberModify*) read(stream, sizeof(lfMemberModify));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[member-modify][%s][%lu]", name, field->index); // index of base class type definition
		} break;
		case LF_MANAGED_ST:
		case LF_MANAGED: {
			lfManaged *field = (lfManaged*) read(stream, sizeof(lfManaged));
			char *name = extract_name(stream, field->Name);

			// log_info("PDB", "\t[managed][%s]", name);
		} break;
		case LF_TYPESERVER2: {
			lfTypeServer2 *field = (lfTypeServer2*) read(stream, sizeof(lfTypeServer2));
			ASSERT(false, "Not implemented!");
			// unsigned char   name[CV_ZEROLEN];	 // length prefixed name of PDB
		} break;
		default: {
			ASSERT(false, "Not implemented!");
		}
	}
}

// Padding scheme:
// 	(0xF0 | [number of bytes until next structure]).
// 	Less formally, the upper four bits are all set, and the lower four give the number of bytes to skip until the next structure. This results in patterns that look like "?? F3 F2 F1 [next structure]".

// Values
// 	If the value of the first word is less than LF_NUMERIC (0x8000), the value data is just the value of that word. The name begins at data[2] and is a C string.
//  Otherwise, the word refers to the type of the value data, one of LF_CHAR, LF_SHORT, LF_USHORT, LF_LONG, or LF_ULONG. Then comes the actual value, and then the name as a C string. The length of the value data is determined by the value type--one byte for LF_CHAR, 2 for LF_SHORT, and so on.

// Forward references
// - A forward reference is an empty structure in the types stream and "fwdref" bit set in its attributes (bit 7 of the word at offset 0x06 of an LF_STRUCTURE).
TypeContext parse_types(Allocator &allocator, Stream &stream) {
	TypeContext tc = {};

	uint32_t version = read_u32(stream); // Version, 4 bytes.
	uint32_t header_size = read_u32(stream); // Header size, 4 bytes.
	uint32_t minimum = read_u32(stream); // Minimum index for type records, 4 bytes.
	uint32_t maximum = read_u32(stream); // Maximum (last + 1) index for type records, 4 bytes.
	uint32_t stream_size = read_u32(stream); // Size of following data, 4 bytes, to the end of the stream.

	// Hash information:
	uint32_t stream_number = read_u32(stream) & 0x0000FFFF; // main hash stream, 2 bytes with 2 bytes padding.
	uint32_t hash_key = read_u32(stream); // size of hash key, 4 bytes.
	uint32_t buckets = read_u32(stream); // number of buckets, 4 bytes.

	// Each composed of an offset and length, each 4 bytes.
	uint32_t hash_value_offset = read_u32(stream);
	uint32_t hash_value_length = read_u32(stream);

	uint32_t ti_off_offset = read_u32(stream);
	uint32_t ti_off_length = read_u32(stream);

	uint32_t hash_adj_offset = read_u32(stream);
	uint32_t hash_adj_length = read_u32(stream);

	// Type records, variable length, count = (maximum - minimum) from above header.
	uint32_t type_record_count = (maximum - minimum);

	tc.types = (Type *) allocate(&allocator, type_record_count * sizeof(Type));
	tc.names = (TypeName *) allocate(&allocator, type_record_count * sizeof(TypeName), true);
	tc.member_lists = (MemberList *) allocate(&allocator, type_record_count * sizeof(MemberList), true);
	tc.record_hashmap = hash_make(&allocator, type_record_count);

	uint64_t unnamed_tag_id = to_id64((unsigned)strlen("<unnamed-tag>"), "<unnamed-tag>");

	for (uint32_t i = 0; i < type_record_count; ++i) {
		uint16_t length = read_u16(stream);

		ASSERT((peek_u8(stream) & 0xF0) != 0xF0, "Invalid stream type!");

		size_t type_start = stream.cursor;
		uint16_t leaf_type = peek_u16(stream);

		int forward_declared = 0;

		tc.types[i].data = data(stream);
		switch (leaf_type) {
			case LF_ARGLIST: {
				lfArgList *type = (lfArgList *)read(stream, length);
#if 0
				log_info("PDB", "[argument list][%u]", i + CV_FIRST_NONPRIM);
				for (uint32_t arg_index = 0; arg_index < type->count; ++arg_index) {
					log_info("PDB", "%lu", type->arg[arg_index]);
					// if (arg_index < type->count - 1) {
						log_info("PDB", ",");
					// }
				}
				log_info("PDB", "]");
#endif
			} break;
			case LF_STRIDED_ARRAY: { ASSERT(false, "Not Implemented!"); } break;
			case LF_VECTOR: { ASSERT(false, "Not Implemented!"); } break;
			case LF_MATRIX: { ASSERT(false, "Not Implemented!"); } break;
			case LF_ARRAY: {
				lfArray *type = (lfArray *)read(stream, length);
				uint32_t size;
				char *name = extract_info(type->data, &size);
				tc.names[i] = make_type_name(name);

				// log_info("PDB", "[array][%u][%s(%u)][%lu,%lu]", i + CV_FIRST_NONPRIM, name, size, type->elemtype, type->idxtype);
			} break;
			case LF_BITFIELD: {
				ASSERT(length == (sizeof(lfBitfield) + sizeof(uint16_t)), "Length doesn't match struct size + padding!");
				lfBitfield *type = (lfBitfield *)read(stream, length);

				// log_info("PDB", "[bitfield][%u][%lu]", i + CV_FIRST_NONPRIM, type->type);
			} break;
			case LF_ENUM: {
				lfEnum *type = (lfEnum *)read(stream, length);
				char *name = (char *)type->Name;
				tc.names[i] = make_type_name(name);

				forward_declared = type->property.fwdref;

				// log_info("PDB", "[enum][%u][%s][%lu]", i + CV_FIRST_NONPRIM, name, type->field);
			} break;
			case LF_FIELDLIST: {
				MemberList &member_list = tc.member_lists[i];
				array_make(&allocator, member_list.types, 8);
				array_make(&allocator, member_list.names, 8);
				array_make(&allocator, member_list.flags, 8);

				// after the standard size and type fields, the body of the structure is made up of an arbitrary number of leaf types of type LF_MEMBER, LF_ENUMERATE, LF_BCLASS, LF_VFUNCTAB, LF_ONEMETHOD, LF_METHOD, or LF_NESTTYPE. This is somewhat annoying to parse, because the number of substructures is not known in advance, and so the only way to know when field list is finished is to see how many bytes have been parsed and compare it to the size of the overall structure.
				lfFieldList *type = (lfFieldList *)read(stream, sizeof(lfFieldList));
				while (stream.cursor < type_start + length) {
					parse_fieldlist(member_list, stream);
				}
				// log_info("PDB", "[field list][%u]", i + CV_FIRST_NONPRIM);
			} break;
			case LF_MFUNCTION: {
				ASSERT(length == sizeof(lfMFunc), "Length doesn't match struct size!");
				lfMFunc *type = (lfMFunc *)read(stream, length);

				// log_info("PDB", "[member function][%u][%lu, %lu, %lu, %lu]", i + CV_FIRST_NONPRIM, type->rvtype, type->classtype, type->thistype, type->arglist);
			} break;
			case LF_MODIFIER: {
				ASSERT(length == (sizeof(lfModifier) + sizeof(uint16_t)), "Length doesn't match struct size + padding!");
				lfModifier *type = (lfModifier *)read(stream, length);

				// log_info("PDB", "[modifier][%u][%lu]", i + CV_FIRST_NONPRIM, type->type);
			} break;
			case LF_POINTER: {
				ASSERT(length == sizeof(lfPointer::lfPointerBody), "Length doesn't match struct size!");
				lfPointer::lfPointerBody *type = (lfPointer::lfPointerBody *)read(stream, length);

				// log_info("PDB", "[pointer body][%u][%lu]", i + CV_FIRST_NONPRIM, type->utype);
			} break;
			case LF_PROCEDURE: {
				ASSERT(length == sizeof(lfProc), "Length doesn't match struct size!");
				lfProc *type = (lfProc *)read(stream, length);

				// log_info("PDB", "[procedure][%u][%lu,%lu]", i + CV_FIRST_NONPRIM, type->rvtype, type->arglist);
			} break;
			case LF_CLASS:
			case LF_INTERFACE:
			case LF_STRUCTURE: {
				lfClass *type = (lfClass *)read(stream, length);
				uint32_t size;
				char *name = extract_info(type->data, &size);
				tc.names[i] = make_type_name(name);

				forward_declared = type->property.fwdref;

				// log_info("PDB", "[structure][%u][%s(%u)][%lu,%lu,%lu]", i + CV_FIRST_NONPRIM, name, size, type->field, type->derived, type->vshape);
			} break;
			case LF_ALIAS: {
				lfAlias *type = (lfAlias *)read(stream, length);
				char *name = (char *)type->Name;
				tc.names[i] = make_type_name(name);

				// log_info("PDB", "[alias][%u][%s]", i + CV_FIRST_NONPRIM, name);
			} break;
			case LF_UNION: {
				lfUnion *type = (lfUnion *)read(stream, length);
				uint32_t size;
				char *name = extract_info(type->data, &size);
				tc.names[i] = make_type_name(name);

				forward_declared = type->property.fwdref;

				// log_info("PDB", "[union][%u][%s(%u)][%lu]", i + CV_FIRST_NONPRIM, name, size, type->field);
			} break;
			case LF_VTSHAPE: {
				lfVTShape *type = (lfVTShape *)read(stream, length);
			} break;
			case LF_METHODLIST: {
				stream.cursor += length;
			} break;
			default: {
				ASSERT(false, "Not Implemented!");
			} break;
		}

		ASSERT(stream.cursor == type_start + length, "Consumed length doesn't match specified length!");

		if (tc.names[i].id != 0 && !forward_declared && tc.names[i].id != unnamed_tag_id) {
			TypeRecord type_record = {};
			type_record.type_index = i + CV_FIRST_NONPRIM;

			HashEntry *hash_entry = hash_lookup(tc.record_hashmap, tc.names[i].id);
			if (hash_entry->key == tc.names[i].id) { // Forward declared thing
#if DEVELOPMENT
				// {
				CV_typ_t existing_type_index = ((TypeRecord*)&hash_entry->value)->type_index;
				existing_type_index -= CV_FIRST_NONPRIM;
				void *existing_data = tc.types[existing_type_index].data;
				// 	ASSERT(((lfClass*) existing_data)->property.fwdref == 1, "Already added structs should be forward declared!");
				// }
				// ASSERT(false, "Should never happen");
				log_error("PDB", "Found duplicated types!\
					existing=(type_index=%u, name=%s, type=0x%x),\
					new=(type_index=%u, name=%s, type=0x%x)",
					existing_type_index, tc.names[existing_type_index].name, ((lfClass*)tc.types[existing_type_index].data)->leaf,
					i, tc.names[i].name, ((lfClass*)tc.types[i].data)->leaf);
#endif // DEVELOPMENT
				hash_entry->value = *(uint64_t*)&type_record;
			} else {
				hash_add_entry(tc.record_hashmap, hash_entry, tc.names[i].id, *(uint64_t*)&type_record);
			}
		}
	}

	return tc;
}
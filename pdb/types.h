// Why is this needed?
#define _VC_VER_INC
#include "include/cvinfo.h"

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

void parse_fieldlist(Stream &stream) {
	uint16_t leaf_type = peek_u16(stream);
	// logf("FieldList type %#06hx\n", leaf_type);
	switch (leaf_type) {
		case LF_BCLASS: {
			lfBClass *field = (lfBClass*) read(stream, sizeof(lfBClass));
			uint32_t size;
			extract_size(stream, &size);

			logf("[bclass][%u][%lu]\n", size, field->index); // type index of base class
		} break;
		case LF_VBCLASS:
		case LF_IVBCLASS: {
			lfVBClass *field = (lfVBClass*) read(stream, sizeof(lfVBClass));
			uint32_t size;
			extract_size(stream, &size);

			logf("[vbclass][%u][%lu,%lu]\n", size,
				field->index, // type index of direct virtual base class
				field->vbptr); // type index of virtual base pointer
		} break;
		case LF_INDEX: {
			lfIndex *field = (lfIndex*) read(stream, sizeof(lfIndex));

			logf("[index][%lu]\n", field->index); // type index of referenced leaf
		} break;
		case LF_VFUNCTAB: {
			lfVFuncTab *field = (lfVFuncTab*) read(stream, sizeof(lfVFuncTab));

			logf("[vfunctab][%lu]\n", field->type); // type index of pointer
		} break;
		case LF_FRIENDCLS: {
			lfFriendCls *field = (lfFriendCls*) read(stream, sizeof(lfFriendCls));

			logf("[friend class][%lu]\n", field->index); // index to type record of friend class
		} break;
		case LF_VFUNCOFF: {
			lfVFuncOff *field = (lfVFuncOff*) read(stream, sizeof(lfVFuncOff));

			logf("[virtual function offset][%lu]\n", field->type); // type index of pointer
		} break;
		case LF_TYPESERVER: {
			lfTypeServer *field = (lfTypeServer*) read(stream, sizeof(lfTypeServer));
			PDB_ASSERT(false);
			// field->name // unsigned char   name[CV_ZEROLEN];     // length prefixed name of PDB
		} break;
		case LF_ENUMERATE: {
			lfEnumerate *field = (lfEnumerate*) read(stream, sizeof(lfEnumerate));
			uint32_t size;
			char *name = extract_info(stream, &size);

			logf("[enumerate][%s(%u)]\n", name, size);
		} break;
		case LF_ARRAY: {
			lfArray *field = (lfArray*) read(stream, sizeof(lfArray));
			uint32_t size;
			char *name = extract_info(stream, &size);

			logf("[array][%s(%u)][%lu,%lu]\n", name, size,
				field->elemtype, // type index of element type
				field->idxtype); // type index of indexing type
		} break;
		case LF_CLASS:
		case LF_STRUCTURE: {
			lfStructure *field = (lfStructure*) read(stream, sizeof(lfStructure));
			uint32_t size;
			char *name = extract_info(stream, &size);

			logf("[structure][%s(%u)][%lu,%lu,%lu]\n", name, size,
				field->field, // type index of LF_FIELD descriptor list
				field->derived, // type index of derived from list if not zero
				field->vshape); // type index of vshape table for this class
		} break;
		case LF_UNION: {
			lfUnion *field = (lfUnion*) read(stream, sizeof(lfUnion));
			uint32_t size;
			char *name = extract_info(stream, &size);

			logf("[union][%s(%u)][%lu,%lu,%lu]\n", name, size, field->field); // type index of LF_FIELD descriptor list
		} break;
		case LF_ENUM: {
			lfEnum *field = (lfEnum*) read(stream, sizeof(lfEnum));
			char *name = extract_name(stream, field->Name);

			logf("[enum][%s][%lu,%lu]\n", name,
				field->utype, // underlying type of the enum
				field->field); // type index of LF_FIELD descriptor list
		} break;
		case LF_DIMARRAY: {
			lfDimArray *field = (lfDimArray*) read(stream, sizeof(lfDimArray));
			char *name = extract_name(stream, field->name);

			logf("[dim-array][%s][%lu,%lu]\n", name,
				field->utype, // underlying type of the array
				field->diminfo); // dimension information
		} break;
		case LF_PRECOMP: {
			lfPreComp *field = (lfPreComp*) read(stream, sizeof(lfPreComp));
			PDB_ASSERT(false);
			// unsigned char   name[CV_ZEROLEN];     // length prefixed name of included type file
		} break;
		case LF_ALIAS: {
			lfAlias *field = (lfAlias*) read(stream, sizeof(lfAlias));
			char *name = extract_name(stream, field->Name);

			logf("[alias][%s][%lu]\n", name, field->utype); // underlying type
		} break;
		case LF_DEFARG: {
			lfDefArg *field = (lfDefArg*) read(stream, sizeof(lfDefArg));
			PDB_ASSERT(false);
			// unsigned char   expr[CV_ZEROLEN];   // length prefixed expression string
		} break;
		case LF_FRIENDFCN_ST:
		case LF_FRIENDFCN: {
			lfFriendFcn *field = (lfFriendFcn*) read(stream, sizeof(lfFriendFcn));
			char *name = extract_name(stream, field->Name);

			logf("[friend function][%s]\n", name, field->index); // index to type record of friend function
		} break;
		case LF_MEMBER_ST:
		case LF_MEMBER: {
			lfMember *field = (lfMember*) read(stream, sizeof(lfMember));
			uint32_t size;
			char *name = extract_info(stream, &size);

			logf("[member][%s(%u)][%lu]\n", name, size, field->index); // index of type record for field
		} break;
		case LF_STMEMBER_ST:
		case LF_STMEMBER: {
			lfSTMember *field = (lfSTMember*) read(stream, sizeof(lfSTMember));
			char *name = extract_name(stream, field->Name);

			logf("[st-member][%s][%lu]\n", name, field->index); // index of type record for field
		} break;
		case LF_METHOD_ST:
		case LF_METHOD: {
			lfMethod *field = (lfMethod*) read(stream, sizeof(lfMethod));
			char *name = extract_name(stream, field->Name);

			logf("[method][%s][%lu]\n", name, field->mList); // index to LF_METHODLIST record
		} break;
		case LF_NESTTYPE_ST:
		case LF_NESTTYPE: {
			lfNestType *field = (lfNestType*) read(stream, sizeof(lfNestType));
			char *name = extract_name(stream, field->Name);

			logf("[nest-type][%s][%lu]\n", name, field->index); // index of nested type definition
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

			logf("[one-method][%s(%u)][%lu]\n", name, size, field->index); // index to type record for procedure
		} break;
		case LF_NESTTYPEEX_ST:
		case LF_NESTTYPEEX: {
			lfNestTypeEx *field = (lfNestTypeEx*) read(stream, sizeof(lfNestTypeEx));
			char *name = extract_name(stream, field->Name);

			logf("[nest-type-ex][%s][%lu]\n", name, field->index); // index of nested type definition
		} break;
		case LF_MEMBERMODIFY_ST:
		case LF_MEMBERMODIFY: {
			lfMemberModify *field = (lfMemberModify*) read(stream, sizeof(lfMemberModify));
			char *name = extract_name(stream, field->Name);

			logf("[member-modify][%s][%lu]\n", name, field->index); // index of base class type definition
		} break;
		case LF_MANAGED_ST:
		case LF_MANAGED: {
			lfManaged *field = (lfManaged*) read(stream, sizeof(lfManaged));
			char *name = extract_name(stream, field->Name);

			logf("[managed][%s]\n", name);
		} break;
		case LF_TYPESERVER2: {
			lfTypeServer2 *field = (lfTypeServer2*) read(stream, sizeof(lfTypeServer2));
			PDB_ASSERT(false);
			// unsigned char   name[CV_ZEROLEN];     // length prefixed name of PDB
		} break;
		default: {
			PDB_ASSERT(false);
		}
	}
}

struct Type {
	void *data; // This is a lfXxx structure
};

// Padding scheme:
// 	(0xF0 | [number of bytes until next structure]).
// 	Less formally, the upper four bits are all set, and the lower four give the number of bytes to skip until the next structure. This results in patterns that look like "?? F3 F2 F1 [next structure]".

// Values
// 	If the value of the first word is less than LF_NUMERIC (0x8000), the value data is just the value of that word. The name begins at data[2] and is a C string.
//  Otherwise, the word refers to the type of the value data, one of LF_CHAR, LF_SHORT, LF_USHORT, LF_LONG, or LF_ULONG. Then comes the actual value, and then the name as a C string. The length of the value data is determined by the value type--one byte for LF_CHAR, 2 for LF_SHORT, and so on.

// Forward references
// - A forward reference is an empty structure in the types stream and "fwdref" bit set in its attributes (bit 7 of the word at offset 0x06 of an LF_STRUCTURE).
Type *parse_types(Stream &stream) {
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
	Type *types = (Type*)malloc(type_record_count * sizeof(Type));
	for (uint32_t j = 0; j < type_record_count; ++j) {
		uint16_t length = read_u16(stream);

		PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);

		size_t type_start = stream.cursor;
		uint16_t leaf_type = peek_u16(stream);
		types[j].data = data(stream);

		switch (leaf_type) {
			case LF_ARGLIST: {
				lfArgList *type = (lfArgList *)read(stream, length);
				logf("[argument list][%u][", j + 0x1000);
				for (uint32_t arg_index = 0; arg_index < type->count; ++arg_index) {
					logf("%lu", type->arg[arg_index]);
					if (arg_index < type->count - 1) {
						logf(",");
					}
				}
				logf("]\n");
			} break;
			case LF_STRIDED_ARRAY: { PDB_ASSERT(false); } break;
			case LF_VECTOR: { PDB_ASSERT(false); } break;
			case LF_MATRIX: { PDB_ASSERT(false); } break;
			case LF_ARRAY: {
				lfArray *type = (lfArray *)read(stream, length);
				uint32_t size;
				char *name = extract_info(type->data, &size);

				logf("[array][%u][%s(%u)][%lu,%lu]\n", j + 0x1000, name, size, type->elemtype, type->idxtype);
			} break;
			case LF_BITFIELD: {
				PDB_ASSERT(length == (sizeof(lfBitfield) + sizeof(uint16_t)));
				lfBitfield *type = (lfBitfield *)read(stream, length);

				logf("[bitfield][%u][%lu]\n", j + 0x1000, type->type);
			} break;
			case LF_ENUM: {
				lfEnum *type = (lfEnum *)read(stream, length);
				char *name = (char *)type->Name;

				logf("[enum][%u][%s][%lu]\n", j + 0x1000, name, type->field);
			} break;
			case LF_FIELDLIST: {
				// after the standard size and type fields, the body of the structure is made up of an arbitrary number of leaf types of type LF_MEMBER, LF_ENUMERATE, LF_BCLASS, LF_VFUNCTAB, LF_ONEMETHOD, LF_METHOD, or LF_NESTTYPE. This is somewhat annoying to parse, because the number of substructures is not known in advance, and so the only way to know when field list is finished is to see how many bytes have been parsed and compare it to the size of the overall structure.
				lfFieldList *type = (lfFieldList *)read(stream, sizeof(lfFieldList));
				while (stream.cursor < type_start + length) {
					parse_fieldlist(stream);
				}
				logf("[field list][%u]\n", j + 0x1000);
			} break;
			case LF_MFUNCTION: {
				PDB_ASSERT(length == sizeof(lfMFunc));
				lfMFunc *type = (lfMFunc *)read(stream, length);

				logf("[member function][%u][%lu, %lu, %lu, %lu]\n", j + 0x1000, type->rvtype, type->classtype, type->thistype, type->arglist);
			} break;
			case LF_MODIFIER: {
				PDB_ASSERT(length == (sizeof(lfModifier) + sizeof(uint16_t)));
				lfModifier *type = (lfModifier *)read(stream, length);

				logf("[modifier][%u][%lu]\n", j + 0x1000, type->type);
			} break;
			case LF_POINTER: {
				PDB_ASSERT(length == sizeof(lfPointer::lfPointerBody));
				lfPointer::lfPointerBody *type = (lfPointer::lfPointerBody *)read(stream, length);

				logf("[pointer body][%u][%lu]\n", j + 0x1000, type->utype);
			} break;
			case LF_PROCEDURE: {
				PDB_ASSERT(length == sizeof(lfProc));
				lfProc *type = (lfProc *)read(stream, length);

				logf("[procedure][%u][%lu,%lu]\n", j + 0x1000, type->rvtype, type->arglist);
			} break;
			case LF_CLASS:
			case LF_INTERFACE:
			case LF_STRUCTURE: {
				lfClass *type = (lfClass *)read(stream, length);
				uint32_t size;
				char *name = extract_info(type->data, &size);

				logf("[structure][%u][%s(%u)][%lu,%lu,%lu]\n", j + 0x1000, name, size, type->field, type->derived, type->vshape);
			} break;
			case LF_ALIAS: {
				lfAlias *type = (lfAlias *)read(stream, length);
				char *name = (char *)type->Name;

				logf("[alias][%u][%s]\n", j + 0x1000, name);
			} break;
			case LF_UNION: {
				lfUnion *type = (lfUnion *)read(stream, length);
				uint32_t size;
				char *name = extract_info(type->data, &size);

				logf("[union][%u][%s(%u)][%lu]\n", j + 0x1000, name, size, type->field);
			} break;
			case LF_VTSHAPE: {
				lfVTShape *type = (lfVTShape *)read(stream, length);
			} break;
			case LF_METHODLIST: {
				stream.cursor += length;
			} break;
			default: {
				PDB_ASSERT(false);
			} break;
		}

		PDB_ASSERT(stream.cursor == type_start + length);
	}

	return types;
}
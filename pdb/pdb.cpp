#include <windows.h>
#include <stdint.h>
#include <stdio.h>

// Why is this needed?
#define _VC_VER_INC
#include "include/cvinfo.h"

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#ifndef PDB_ASSERT
#include <assert.h>
#define PDB_ASSERT(x) assert(x)
#endif

struct PDB {
	uint8_t *source;
	uint32_t page_size;
};

struct PageList {
	uint32_t *list;
	uint32_t count;
	__forceinline uint32_t operator[](int index) { return list[index]; }
};

struct StreamData {
	uint8_t *data;
	size_t size;

	size_t cursor;
};

__forceinline uint8_t read_u8(uint8_t *buffer, size_t *cursor) {
	uint8_t result = *(uint8_t*)(buffer + *cursor);
	*cursor += sizeof(uint8_t);
	return result;
}
__forceinline uint16_t read_u16(uint8_t *buffer, size_t *cursor) {
	uint16_t result = *(uint16_t*)(buffer + *cursor);
	*cursor += sizeof(uint16_t);
	return result;
}
__forceinline uint32_t read_u32(uint8_t *buffer, size_t *cursor) {
	uint32_t result = *(uint32_t*)(buffer + *cursor);
	*cursor += sizeof(uint32_t);
	return result;
}

__forceinline uint8_t peek_u8(uint8_t *buffer, size_t cursor) {
	return *(uint8_t*)(buffer + cursor);
}
__forceinline uint16_t peek_u16(uint8_t *buffer, size_t cursor) {
	return *(uint16_t*)(buffer + cursor);
}
__forceinline uint32_t peek_u32(uint8_t *buffer, size_t cursor) {
	return *(uint32_t*)(buffer + cursor);
}

__forceinline uint8_t *read(uint8_t *buffer, size_t *cursor, size_t size) {
	uint8_t *result = (uint8_t*)(buffer + *cursor);
	*cursor += size;
	return result;
}

__forceinline uint8_t read_u8(StreamData &stream) {
	return read_u8(stream.data, &stream.cursor);
}
__forceinline uint16_t read_u16(StreamData &stream) {
	return read_u16(stream.data, &stream.cursor);
}
__forceinline uint32_t read_u32(StreamData &stream) {
	return read_u32(stream.data, &stream.cursor);
}

__forceinline uint8_t peek_u8(StreamData &stream) {
	return peek_u8(stream.data, stream.cursor);
}
__forceinline uint16_t peek_u16(StreamData &stream) {
	return peek_u16(stream.data, stream.cursor);
}
__forceinline uint32_t peek_u32(StreamData &stream) {
	return peek_u32(stream.data, stream.cursor);
}

__forceinline uint8_t *read(StreamData &stream, size_t size) {
	return read(stream.data, &stream.cursor, size);
}
__forceinline uint8_t *data(StreamData &stream) {
	return (uint8_t*)(stream.data + stream.cursor);
}

uint32_t _pages(uint32_t length, uint32_t page_size) {
	PDB_ASSERT(length > 0);
	uint32_t num_pages = 1u + ((length - 1) / page_size);
	return num_pages;
}



// struct Stream {
// 	uint32_t *pages;
// 	size_t size;
// };

#if 1
StreamData get_stream_data(PDB pdb, PageList list) {
	StreamData result = {};

	result.size = pdb.page_size * list.count;
	result.data = (uint8_t*)malloc(result.size);
#if DEBUG
	for (size_t i = 0; i < result.size; ++i) {
		result.data[i] = 0x66;
	}
#endif

	for (uint32_t i = 0; i < list.count; ++i) {
		uint32_t page_index = list[i];
		size_t at = page_index * pdb.page_size;
		uint8_t *data = read(pdb.source, &at, pdb.page_size);
		memcpy(result.data + i * pdb.page_size, data, pdb.page_size);
	}

	return result;
}
#else
StreamData get_stream_data(PDB pdb, PageList list) {
	uint32_t start = list[0];
	for (uint32_t i = 0; i < list.count; ++i) {
		PDB_ASSERT(start + i == list[i]);
	}

	StreamData result = {};

	size_t at = list[0] * pdb.page_size;

	result.size = pdb.page_size * list.count;
	result.data = pdb.source + at;

	return result;
}
#endif

/*
Stream 1 - is used to verify that the PDB is the same file referred to in an executable or object file stream.

Stream 2 and Stream 4 hold types information.
Actual type records define types used in the program.
The structure of these records can be found in the file cvinfo.h provided by Microsoft.
There are two flavors of records, each with its own set of index numbers:
	type IDs and types;
only types are stored in stream 2 and only type IDs are stored in stream 4.
The indices are used to refer to these records from within symbol records and other type records.

Stream 3 - is a directory for other streams. Note, it is not present in Version 2, nor in a PDB produced by a compiler. The stream starts with a header which is padded to be 64 bytes in total
*/

enum StreamType {
	StreamType_Root,
	StreamType_Info,
	StreamType_Types,
	StreamType_StreamDirectory,
	StreamType_TypeIDs,
};

#if 0
struct StreamDirectory {
0	4	Signature	Header identifier, == 0xFFFFFFFF
4	4	HeaderVersion	Version of the Header
8	4	Age
12	2	snGSSyms
14	2	usVerAll
union {
	   struct {
		   USHORT	  usVerPdbDllMin : 8; // minor version and
		   USHORT	  usVerPdbDllMaj : 7; // major version and
		   USHORT	  fNewVerFmt	 : 1; // flag telling us we have rbld stored elsewhere (high bit of original major version)
	   } vernew;						   // that built this pdb last.
	   struct {
		   USHORT	  usVerPdbDllRbld: 4;
		   USHORT	  usVerPdbDllMin : 7;
		   USHORT	  usVerPdbDllMaj : 5;
	   } verold;
	   USHORT		  usVerAll;
   };
16	2	snPSSyms
18	2	usVerPdbDllBuild	build version of the pdb dll that built this pdb last
20	2	snSymRecs
22	2	VerPdbDllRBld	rbld version of the pdb dll that built this pdb last
24	4	cbGpModi	size of rgmodi substream
28	4	cbSC	size of Section Contribution substream
32	4	cbSecMap	size of section map
36	4	cbFileInfo	size of file info stream
40	4	cbTSMap	size of the Type Server Map substream
44	4	iMFC	MFC Index
48	4	cbDbgHdr	size of optional DbgHdr info appended to the end of the stream
52	4	cbECInfo	number of bytes in EC substream, or 0 if no EC enabled Mods
56	2	flags
struct _flags {
	   USHORT  fIncLink:1;	 // true if linked incrmentally (really just if ilink thunks are present)
	   USHORT  fStripped:1;	// true if PDB::CopyTo stripped the private data out
	   USHORT  fCTypes:1;	  // true if this PDB is using CTypes.
	   USHORT  unused:13;	  // reserved, must be 0.
   } flags;
58	2	wMachine	Machine identifier, same as used in COFF object format, e.g., hex 8664 for Intel x86 64-bit
60	4	RESERVED	future expansion, pad to 64 bytes
}
#endif

void parse_fieldlist(StreamData &stream) {
	uint16_t leaf_type = peek_u16(stream);
	printf("FieldList type %#06hx\n", leaf_type);
	switch (leaf_type) {
		case LF_BCLASS: {
			lfBClass *field = (lfBClass*) read(stream, sizeof(lfBClass));
		} break;
		case LF_VBCLASS:
		case LF_IVBCLASS: {
			lfVBClass *field = (lfVBClass*) read(stream, sizeof(lfVBClass));
		} break;
		case LF_FRIENDFCN_ST: {
			lfFriendFcn *field = (lfFriendFcn*) read(stream, sizeof(lfFriendFcn));
		} break;
		case LF_INDEX: {
			lfIndex *field = (lfIndex*) read(stream, sizeof(lfIndex));
		} break;
		case LF_MEMBER_ST: {
			lfMember *field = (lfMember*) read(stream, sizeof(lfMember));
		} break;
		case LF_STMEMBER_ST: {
			lfSTMember *field = (lfSTMember*) read(stream, sizeof(lfSTMember));
		} break;
		case LF_METHOD_ST: {
			lfMethod *field = (lfMethod*) read(stream, sizeof(lfMethod));
		} break;
		case LF_NESTTYPE_ST: {
			lfNestType *field = (lfNestType*) read(stream, sizeof(lfNestType));
		} break;
		case LF_VFUNCTAB: {
			lfVFuncTab *field = (lfVFuncTab*) read(stream, sizeof(lfVFuncTab));
		} break;
		case LF_FRIENDCLS: {
			lfFriendCls *field = (lfFriendCls*) read(stream, sizeof(lfFriendCls));
		} break;
		case LF_ONEMETHOD_ST: {
			lfOneMethod *field = (lfOneMethod*) read(stream, sizeof(lfOneMethod));
		} break;
		case LF_VFUNCOFF: {
			lfVFuncOff *field = (lfVFuncOff*) read(stream, sizeof(lfVFuncOff));
		} break;
		case LF_NESTTYPEEX_ST: {
			lfNestTypeEx *field = (lfNestTypeEx*) read(stream, sizeof(lfNestTypeEx));
		} break;
		case LF_MEMBERMODIFY_ST: {
			lfMemberModify *field = (lfMemberModify*) read(stream, sizeof(lfMemberModify));
		} break;
		case LF_MANAGED_ST: {
			lfManaged *field = (lfManaged*) read(stream, sizeof(lfManaged));
		} break;

	LF_TYPESERVER       = 0x1501,       // not referenced from symbol
    LF_ENUMERATE        = 0x1502,
    LF_ARRAY            = 0x1503,
    LF_CLASS            = 0x1504,
    LF_STRUCTURE        = 0x1505,
    LF_UNION            = 0x1506,
    LF_ENUM             = 0x1507,
    LF_DIMARRAY         = 0x1508,
    LF_PRECOMP          = 0x1509,       // not referenced from symbol
    LF_ALIAS            = 0x150a,       // alias (typedef) type
    LF_DEFARG           = 0x150b,
    LF_FRIENDFCN        = 0x150c,
    LF_MEMBER           = 0x150d,
    LF_STMEMBER         = 0x150e,
    LF_METHOD           = 0x150f,
    LF_NESTTYPE         = 0x1510,
    LF_ONEMETHOD        = 0x1511,
    LF_NESTTYPEEX       = 0x1512,
    LF_MEMBERMODIFY     = 0x1513,
    LF_MANAGED          = 0x1514,
    LF_TYPESERVER2      = 0x1515,

		default: {
			PDB_ASSERT(false);
		}
	}
}

int main(int argc, char const *argv[]) {
	const char *test = "pdb.pdb";
	HANDLE handle = CreateFile(test, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD filesize = GetFileSize(handle, 0);
	// DWORD error = GetLastError();

	size_t cursor = 0;
	uint8_t *source = (uint8_t *)malloc(filesize);
	ReadFile(handle, source, filesize, 0, 0);

	const char signature[] = "Microsoft C/C++ MSF 7.00\r\n\x1A""DS\0\0\0";
	for (size_t i = 0; i < ARRAY_COUNT(signature) - 1; ++i) {
		if (source[cursor++] != signature[i]) {
			PDB_ASSERT(false);
			return 0;
		}
	}

/*
* Root Stream
* Info Stream
* Type Stream
* Debug Info Stream
* Global Symbol Stream
* OMAP Streams
* Section Header Streams
* FPOv1 Stream
* FPOv2 Stream
*/

	uint32_t page_size = read_u32(source, &cursor); // Page size, 4 bytes.
	uint32_t allocation_table_pointer = read_u32(source, &cursor); // Allocation table pointer, 4 bytes. The meaning of this is unknown. There appears to be an allocation table, an array of 65,536 bits (8,192 bytes), located at the end of the PDB, and a 1-bit means a page that is not being used.
	uint32_t num_file_pages = read_u32(source, &cursor); // Number of file pages, 4 bytes.
	uint32_t root_size = read_u32(source, &cursor); // Root stream size, 4 bytes.
	uint32_t reserved = read_u32(source, &cursor); // reserved, 4 bytes.

	PDB pdb = {};
	pdb.source = source;
	pdb.page_size = page_size;

	uint32_t num_root_pages = _pages(root_size, page_size); // How many pages in the root stream?

	// Root index page list - IDICES of the PAGES containing the Root Page List
	PageList root_index_page_list = {};

	root_index_page_list.count = _pages(num_root_pages * 4, page_size); // How many pages are needed to store the root page list?
	root_index_page_list.list = (uint32_t*) read(source, &cursor, root_index_page_list.count * sizeof(uint32_t));

	// Read in the root page list - INDICES of the PAGES containing the Root Stream Data
	StreamData root_page_list_stream = get_stream_data(pdb, root_index_page_list);

	PageList root_page_list = {};

	root_page_list.count = num_root_pages;
	root_page_list.list = (uint32_t*)root_page_list_stream.data;

	StreamData root_data = get_stream_data(pdb, root_page_list);

	uint32_t num_streams = read_u32(root_data);

	PageList *page_lists_stream = (PageList *)malloc(num_streams * sizeof(PageList));
	for (uint32_t i = 0; i < num_streams; ++i) {
		uint32_t stream_size = read_u32(root_data);

		int valid = stream_size != ~0u && stream_size != 0;

		PageList &pl = page_lists_stream[i];
		pl.count = valid ? _pages(stream_size, pdb.page_size) : 0;
		pl.list = 0;
	}

	// Next comes a list of the pages that make up each stream
	for (uint32_t i = 0; i < num_streams; ++i) {
		PageList &pl = page_lists_stream[i];
		pl.list = (uint32_t *)read(root_data, pl.count * sizeof(uint32_t));
	}

	for (uint32_t i = 0; i < num_streams; ++i) {
		PageList &pl = page_lists_stream[i];
		if (pl.count > 0) {
			StreamData stream = get_stream_data(pdb, pl);

			switch(i) {
				case StreamType_Root: {
				} break;
				case StreamType_Info: {
					uint32_t version = read_u32(stream);// Version, 4 bytes.
					uint32_t time_date_stamp = read_u32(stream); // Time date stamp, 4 bytes.
					uint32_t age = read_u32(stream); // Age, 4 bytes. This is the number of times this PDB has been modified since its creation.

					// GUID, 16 bytes.
					uint32_t guid1 = read_u32(stream);
					uint32_t guid2 = read_u32(stream);
					uint32_t guid3 = read_u32(stream);
					uint32_t guid4 = read_u32(stream);

					uint32_t name_string_length = read_u32(stream); // Total length of following names, 4 bytes. Followed by null-terminated character strings.
					char *buf = (char *)read(stream, name_string_length);

#if DEBUG
					printf("Version %u\n", version);
					printf("Time Stamp %u\n", time_date_stamp);
					printf("Age %u\n", age);
					printf("Guid %u, %u, %u, %u\n", guid1, guid2, guid3, guid4);

					printf("Names\n\t");
					for (uint32_t at = 0; at < name_string_length; ++at) {
						if (buf[at] == 0) {
							printf("\n\t");
						} else {
							printf("%c", buf[at]);
						}
					}
					printf("\n");
#endif
				} break;
				case StreamType_Types: {
					uint32_t version = read_u32(stream); // Version, 4 bytes.
					uint32_t header_size = read_u32(stream); // Header size, 4 bytes.
					uint32_t minimum = read_u32(stream); // Minimum index for type records, 4 bytes.
					uint32_t maximum = read_u32(stream); // Maximum (last + 1) index for type records, 4 bytes.
					uint32_t size = read_u32(stream); // Size of following data, 4 bytes, to the end of the stream.

					// Hash information:
					uint32_t stream_number = read_u32(stream) & 0x0000FFFF; // Stream number, 2 bytes with 2 bytes padding.
					uint32_t hash_key = read_u32(stream); // Hash key, 4 bytes.
					uint32_t buckets = read_u32(stream); // Buckets, 4 bytes.

					// Each composed of an offset and length, each 4 bytes.
					uint32_t hash_value_offset = read_u32(stream);
					uint32_t hash_value_length = read_u32(stream);

					uint32_t ti_off_offset = read_u32(stream);
					uint32_t ti_off_length = read_u32(stream);

					uint32_t hash_adj_offset = read_u32(stream);
					uint32_t hash_adj_length = read_u32(stream);

					// Type records, variable length, count = (maximum - minimum) from above header.
					uint32_t type_record_count = (maximum - minimum);
					for (uint32_t j = 0; j < type_record_count; ++j) {
						uint16_t length = read_u16(stream);

						size_t type_start = stream.cursor;
						uint16_t leaf_type = peek_u16(stream);
						printf("Leaf Type %#06hx\n", leaf_type);

						switch (leaf_type) {
							case LF_ARGLIST: {
								lfArgList *type = (lfArgList *)read(stream, sizeof(lfArgList));
								for (uint32_t arg_index = 0; arg_index < type->count; ++arg_index) {
									uint32_t arg_type = read_u32(stream);
									printf("Argument Type %#06hx\n", arg_type);
								}

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_ARRAY: {
								lfArray *type = (lfArray *)read(stream, sizeof(lfArray));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_BITFIELD: {
								lfBitfield *type = (lfBitfield *)read(stream, sizeof(lfBitfield));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_ENUM: {
								lfEnum *type = (lfEnum *)read(stream, sizeof(lfEnum));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_FIELDLIST: {
								// after the standard size and type fields, the body of the structure is made up of an arbitrary number of leaf types of type LF_MEMBER, LF_ENUMERATE, LF_BCLASS, LF_VFUNCTAB, LF_ONEMETHOD, LF_METHOD, or LF_NESTTYPE. This is somewhat annoying to parse, because the number of substructures is not known in advance, and so the only way to know when field list is finished is to see how many bytes have been parsed and compare it to the size of the overall structure.
								lfFieldList *type = (lfFieldList *)read(stream, sizeof(lfFieldList));
								while (stream.cursor < type_start + length) {
									parse_fieldlist(stream);
								}
							} break;
							case LF_MFUNCTION: {
								lfMFunc *type = (lfMFunc *)read(stream, sizeof(lfMFunc));
								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_MODIFIER: {
								lfModifier *type = (lfModifier *)read(stream, sizeof(lfModifier));
								uint16_t padding = read_u16(stream);

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_POINTER: {
								lfPointer::lfPointerBody *type = (lfPointer::lfPointerBody *)read(stream, sizeof(lfPointer::lfPointerBody));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_PROCEDURE: {
								lfProc *type = (lfProc *)read(stream, sizeof(lfProc));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_CLASS:
							case LF_INTERFACE:
							case LF_STRUCTURE: {
								lfClass *type = (lfClass *)read(stream, sizeof(lfClass));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_UNION: {
								lfUnion *type = (lfUnion *)read(stream, sizeof(lfUnion));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							case LF_VTSHAPE: {
								lfVTShape *type = (lfVTShape *)read(stream, sizeof(lfVTShape));

								PDB_ASSERT((peek_u8(stream) & 0xF0) != 0xF0);
							} break;
							default: {
								PDB_ASSERT(false);
							} break;
						}

						PDB_ASSERT(stream.cursor == type_start + length);

						// Padding scheme:
						// 	(0xF0 | [number of bytes until next structure]).
						// 	Less formally, the upper four bits are all set, and the lower four give the number of bytes to skip until the next structure. This results in patterns that look like "?? F3 F2 F1 [next structure]".

						// Values
						// 	If the value of the first word is less than LF_NUMERIC (0x8000), the value data is just the value of that word. The name begins at data[2] and is a C string.
						//  Otherwise, the word refers to the type of the value data, one of LF_CHAR, LF_SHORT, LF_USHORT, LF_LONG, or LF_ULONG. Then comes the actual value, and then the name as a C string. The length of the value data is determined by the value type--one byte for LF_CHAR, 2 for LF_SHORT, and so on.

						// Forward references
						// - A forward reference is an empty structure in the types stream and "fwdref" bit set in its attributes (bit 7 of the word at offset 0x06 of an LF_STRUCTURE).
					}

				} break;
				case StreamType_StreamDirectory: {
				} break;
				case StreamType_TypeIDs: {
				} break;
			}

			int a = 0;
		}
		// uint32_t type = streams[i].
		// try:
		// 	pdb_cls = self._stream_map[i]
		// except KeyError:
		// 	pdb_cls = PDBStream
		// stream_size, stream_pages = rs.streams[i]
		// self.streams.append(
		// 	pdb_cls(self.fp, stream_pages, i, size=stream_size,
		// 		page_size=self.page_size, fast_load=self.fast_load,
		// 		parent=self))
	}

	// // Sets up access to streams by name
	// self._update_names()

	// // Second stage init. Currently only used for FPO strings
	// if not self.fast_load:
	// 	for s in self.streams:
	// 		if hasattr(s, 'load2'):
	// 			s.load2()

	CloseHandle(handle);
	return 0;
}
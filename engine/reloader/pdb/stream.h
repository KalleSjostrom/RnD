struct PageList {
	uint32_t *list;
	uint32_t count;
	__forceinline uint32_t operator[](int index) { return list[index]; }
};

uint32_t _pages(uint32_t length, uint32_t page_size) {
	ASSERT(length > 0, "Length needs to be non-zero!");
	uint32_t num_pages = 1u + ((length - 1) / page_size);
	return num_pages;
}

struct Stream {
	uint8_t *data;
	size_t size;
	size_t cursor;
};

// Raw reads and peeks
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

// Wrappers for reading through the stream
__forceinline uint8_t read_u8(Stream &stream) {
	return read_u8(stream.data, &stream.cursor);
}
__forceinline uint16_t read_u16(Stream &stream) {
	return read_u16(stream.data, &stream.cursor);
}
__forceinline uint32_t read_u32(Stream &stream) {
	return read_u32(stream.data, &stream.cursor);
}

__forceinline uint8_t peek_u8(Stream &stream) {
	return peek_u8(stream.data, stream.cursor);
}
__forceinline uint16_t peek_u16(Stream &stream) {
	return peek_u16(stream.data, stream.cursor);
}
__forceinline uint32_t peek_u32(Stream &stream) {
	return peek_u32(stream.data, stream.cursor);
}

__forceinline uint8_t *read(Stream &stream, size_t size) {
	return read(stream.data, &stream.cursor, size);
}
__forceinline uint8_t *data(Stream &stream) {
	return (uint8_t*)(stream.data + stream.cursor);
}

#if 1
Stream get_stream_data(Allocator *a, uint8_t *source, size_t page_size, PageList list) {
	Stream result = {};

	result.size = page_size * list.count;
	result.data = (uint8_t*)allocate(a, result.size, false, 16);
#if DEBUG
	for (size_t i = 0; i < result.size; ++i) {
		result.data[i] = 0x66;
	}
#endif

	for (uint32_t i = 0; i < list.count; ++i) {
		uint32_t page_index = list[i];
		size_t at = page_index * page_size;
		uint8_t *data = read(source, &at, page_size);
		memcpy(result.data + i * page_size, data, page_size);
	}

	return result;
}
#else
// This is if we can assume that the list of pages are consecutive, then we can build the stream inplace
Stream get_stream_data(PDB pdb, PageList list) {
	uint32_t start = list[0];
	for (uint32_t i = 0; i < list.count; ++i) {
		PDB_ASSERT(start + i == list[i]);
	}

	Stream result = {};

	size_t at = list[0] * page_size;

	result.size = page_size * list.count;
	result.data = pdb.source + at;

	return result;
}
#endif

// Padding the steram to the nearest 4 byte boundary
void check_padding(Stream &stream, uint16_t *length = 0) {
	uint8_t b = peek_u8(stream);
#if 1
	static const size_t alignment = 4;
	size_t alignment_mask = alignment - 1;
	stream.cursor = (stream.cursor + alignment_mask) & ~alignment_mask;
#else
	switch (b) {
		case LF_PAD0:  { stream.cursor +=  0; } break;
		case LF_PAD1:  { stream.cursor +=  1; } break;
		case LF_PAD2:  { stream.cursor +=  2; } break;
		case LF_PAD3:  { stream.cursor +=  3; } break;
		case LF_PAD4:  { stream.cursor +=  4; } break;
		case LF_PAD5:  { stream.cursor +=  5; } break;
		case LF_PAD6:  { stream.cursor +=  6; } break;
		case LF_PAD7:  { stream.cursor +=  7; } break;
		case LF_PAD8:  { stream.cursor +=  8; } break;
		case LF_PAD9:  { stream.cursor +=  9; } break;
		case LF_PAD10: { stream.cursor += 10; } break;
		case LF_PAD11: { stream.cursor += 11; } break;
		case LF_PAD12: { stream.cursor += 12; } break;
		case LF_PAD13: { stream.cursor += 13; } break;
		case LF_PAD14: { stream.cursor += 14; } break;
		case LF_PAD15: { stream.cursor += 15; } break;
	}
#endif
}

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

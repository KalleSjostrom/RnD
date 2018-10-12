#include <windows.h>
#include <stdint.h>

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

unsigned read_unsigned(char *buffer, size_t *cursor) {
	unsigned result = *(unsigned*)(buffer + *cursor);
	*cursor += sizeof(unsigned);
	return result;
}

int main(int argc, char const *argv[]) {
	const char *test = "pdb.pdb";
	HANDLE handle = CreateFile(test, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD filesize = GetFileSize(handle, 0);
	// DWORD error = GetLastError();

	size_t cursor = 0;
	char *source = (char *)malloc(filesize);
	ReadFile(handle, source, filesize, 0, 0);

	const char signature[] = "Microsoft C/C++ MSF 7.00\r\n\x1A""DS\0\0\0";
	for (size_t i = 0; i < ARRAY_COUNT(signature) - 1; ++i) {
		if (source[cursor++] != signature[i]) {
			//ASSERT(false);
			return 0;
		}
	}

	unsigned page_size = read_unsigned(source, &cursor); // Page size, 4 bytes.
	unsigned allocation_table_pointer = read_unsigned(source, &cursor); // Allocation table pointer, 4 bytes. The meaning of this is unknown. There appears to be an allocation table, an array of 65,536 bits (8,192 bytes), located at the end of the PDB, and a 1-bit means a page that is not being used.
	unsigned num_file_pages = read_unsigned(source, &cursor); // Number of file pages, 4 bytes.
	unsigned root_stream_size = read_unsigned(source, &cursor); // Root stream size, 4 bytes.
	unsigned reserved = read_unsigned(source, &cursor); // reserved, 4 bytes.
	unsigned root_stream_page_number_list = read_unsigned(source, &cursor); // Root stream page number list, 4 bytes per page, enough to cover the above Root stream size.

	// Read in the root stream
	unsigned num_root_pages = root_stream_size / pagesize;
	// root_page_list num_root_pages * 4;

	CloseHandle(handle);
	return 0;
}
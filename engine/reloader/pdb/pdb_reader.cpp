int parse_pdb(Allocator &allocator, const char *pdb_file_path, TypeContext *type_context) {
	HANDLE handle = CreateFile(pdb_file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE) {
		log_error("PDB", "Unable to open PDB file! (filename=%s, error=%d)", pdb_file_path, GetLastError());
		return false;
	}

	ArenaAllocator temp_arena = {};
	Allocator temp_allocator = allocator_arena(&temp_arena, 8);

	size_t cursor = 0;
	DWORD filesize = GetFileSize(handle, 0);
	uint8_t *source = (uint8_t*) allocate(&temp_allocator, filesize);
	ReadFile(handle, source, filesize, 0, 0);

	const char signature[] = "Microsoft C/C++ MSF 7.00\r\n\x1A""DS\0\0\0";
	for (size_t i = 0; i < ARRAY_COUNT(signature) - 1; ++i) {
		if (source[cursor++] != signature[i]) {
			log_error("PDB", "Invalid PDB signature!");
			return false;
		}
	}

	uint32_t page_size = read_u32(source, &cursor); // Page size, 4 bytes.
	uint32_t allocation_table_pointer = read_u32(source, &cursor); // Allocation table pointer, 4 bytes. The meaning of this is unknown. There appears to be an allocation table, an array of 65,536 bits (8,192 bytes), located at the end of the PDB, and a 1-bit means a page that is not being used.
	uint32_t num_file_pages = read_u32(source, &cursor); // Number of file pages, 4 bytes.
	uint32_t root_size = read_u32(source, &cursor); // Root stream size, 4 bytes.
	uint32_t reserved = read_u32(source, &cursor); // reserved, 4 bytes.

	uint32_t num_root_pages = _pages(root_size, page_size); // How many pages in the root stream?

	// Root index page list - IDICES of the PAGES containing the Root Page List
	PageList root_index_page_list = {};

	root_index_page_list.count = _pages(num_root_pages * sizeof(uint32_t), page_size); // How many pages are needed to store the root page list?
	root_index_page_list.list = (uint32_t*) read(source, &cursor, root_index_page_list.count * sizeof(uint32_t));

	// Read in the root page list - INDICES of the PAGES containing the Root Stream Data
	Stream root_page_list_stream = get_stream_data(&allocator, source, page_size, root_index_page_list);

	PageList root_page_list = {};

	root_page_list.count = num_root_pages;
	root_page_list.list = (uint32_t*)root_page_list_stream.data;

	Stream root_data = get_stream_data(&allocator, source, page_size, root_page_list);

	uint32_t num_streams = read_u32(root_data);

	PageList *page_lists_stream = (PageList *) allocate(&allocator, num_streams * sizeof(PageList));
	for (uint32_t i = 0; i < num_streams; ++i) {
		uint32_t stream_size = read_u32(root_data);

		int valid = stream_size != ~0u && stream_size != 0;

		PageList &pl = page_lists_stream[i];
		pl.count = valid ? _pages(stream_size, page_size) : 0;
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
			Stream stream = get_stream_data(&allocator, source, page_size, pl);

			switch(i) {
				case StreamType_Root: {
				} break;
				case StreamType_Info: {
					uint32_t version = read_u32(stream); // Version, 4 bytes.
					uint32_t time_date_stamp = read_u32(stream); // Time date stamp, 4 bytes.
					uint32_t age = read_u32(stream); // Age, 4 bytes. This is the number of times this PDB has been modified since its creation.

					// GUID, 16 bytes.
					uint32_t guid1 = read_u32(stream);
					uint32_t guid2 = read_u32(stream);
					uint32_t guid3 = read_u32(stream);
					uint32_t guid4 = read_u32(stream);

					uint32_t name_string_length = read_u32(stream); // Total length of following names, 4 bytes. Followed by null-terminated character strings.
					char *buf = (char *)read(stream, name_string_length);
				} break;
				case StreamType_Types: {
					*type_context = parse_types(allocator, stream);
				} break;
				case StreamType_StreamDirectory: {
				} break;
				case StreamType_TypeIDs: {
				} break;
			}
		}
	}

	CloseHandle(handle);
	return true;
}
#ifdef OS_WINDOWS
	#define mkdir(name, mode) _mkdir(name)
#endif

u32 cache_lookup(Cache &cache, u64 key) {
	u32 index = key % cache.max_count;
	for (u32 i = 0; i < cache.max_count; ++i) {
		CacheEntry *entry = cache.entries + index;
		if (entry->key == key || entry->key == 0)
			return index;

		index++;
		if (index == cache.max_count)
			index = 0;
	}

	ASSERT(false, "Hash map is full!");
	return 0;
}

void make_sure_directory_exists(String directory) {
	bool success = CreateDirectory(*directory, NULL);
	if (!success) {
		// It's valid if the directory already exists, so assert only if there is some other error.
		bool already_exists = GetLastError() == ERROR_ALREADY_EXISTS;
		ASSERT(already_exists, "Error making directory! (dirpath=%s, error=%ld)", *directory, GetLastError());
	}
}

void register_ending(FileSystem &fs, String ending, u32 ending_id) {
	Ending *e = &fs.ending_lookup[ending[ending.length - 1]];

	bool inserted = false;
	while(!inserted) {
		if (e->id == ending_id) { // Already registered
			return;
		}

		if (e->id == 0) { // We are fine to use this slot
			e->string = ending;
			e->id = ending_id;
			inserted = true;
		} else {
			if (e->next == 0) {
				e->next = &fs.ending_storage[fs.ending_storage_count++];
			}
			e = e->next;
		}
	}
}

static String s_data = MAKE_STRING("data");

void init_filesystem(FileSystem &file_system, String root, String output_path, MemoryArena &arena) {
	Cache c = {};
	file_system.cache = c;
	file_system.cache.max_count = 8192;
	file_system.cache.entries = PUSH_STRUCTS(arena, file_system.cache.max_count, CacheEntry, true);
	file_system.cache.touched = PUSH_STRUCTS(arena, file_system.cache.max_count, bool, true);

	file_system.root = root;
	file_system.source_folder = root; // Should this be the asset path thing? What would we use that for?
	file_system.output_folder = make_path(arena, root, output_path);
	file_system.output_folder_id = make_string_id64(file_system.output_folder);
	make_sure_directory_exists(file_system.output_folder);

	file_system.data_folder = make_path(arena, file_system.output_folder, s_data);

	file_system.file_infos = PUSH_STRUCTS(arena, file_system.cache.max_count, FileInfo);
}

Ending *get_ending(FileSystem fs, String filename) {
	ASSERT(filename.length > 0, "");
	char key = filename[filename.length - 1];

	i32 at = filename.length - 1;
	for (; at >= 0; at--) {
        if (filename[at] == '.') {
            at++; // Ignore the dot
			break;
        }
	}

	String ending_string = make_string(filename.text + at, filename.length - at);

	Ending *e = &fs.ending_lookup[key];
	while (e && e->string.length > 0) {
		if (is_equal(ending_string, e->string)) {
			return e;
		} else {
			e = e->next;
		}
	}
	return 0;
}

bool ignored_directory(String &file_path, u64 file_path_id, String &filename, u32 filename_id) {
	(void)file_path_id;
	(void)filename;
	(void)filename_id;
	if (file_path[0] == '.')
		return true;
	return false;
}

u32 get_file_ending(String &filename) {
	i32 at = filename.length - 1;
	for (; at >= 0; at--) {
		if (filename[at] == '.')
			break;
	}

	String ending = make_string(filename.text + at, filename.length - at);
	u32 id = make_string_id32(ending);
	return id;
}

HANDLE find_first_file(const char *filename, WIN32_FIND_DATA *find_data) {
	return FindFirstFileEx(filename, FindExInfoBasic, find_data, FindExSearchNameMatch, NULL, FIND_FIRST_EX_CASE_SENSITIVE);
}

FILETIME get_current_module_filetime() {
	char buffer[MAX_PATH] = {};
	GetModuleFileName(0, buffer, ARRAY_COUNT(buffer));

	WIN32_FIND_DATA data;
	HANDLE handle = find_first_file(buffer, &data);

	FILETIME filetime = data.ftLastWriteTime;

	FindClose(handle);
	return filetime;
}

b32 remove_files(const char *search_string, const char *folder) {
	WIN32_FIND_DATA find_data = {};
	HANDLE handle = find_first_file(search_string, &find_data);
	b32 failed = false;

	if (handle != INVALID_HANDLE_VALUE) {
		do {
			char buffer[MAX_PATH];
			sprintf_s(buffer, "%s/%s", folder, find_data.cFileName);
			BOOL success = DeleteFile(buffer);
			if (!success) {
				failed = true;
				// fprintf(stderr, "Error in DeleteFile while removing `%s`\n\n`Windows error code: %ld`\n\n", find_data.cFileName, GetLastError());
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}

	return failed;
}

b32 has_locked_files(const char *search_string, const char *folder) {
	WIN32_FIND_DATA find_data = {};
	HANDLE handle = find_first_file(search_string, &find_data);
	b32 found = false;

	if (handle != INVALID_HANDLE_VALUE) {
		do {
			char buffer[MAX_PATH];
			sprintf_s(buffer, "%s/%s", folder, find_data.cFileName);
			HANDLE filehandle = CreateFile(buffer, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			found = filehandle == INVALID_HANDLE_VALUE;
			CloseHandle(filehandle);
		} while (!found && FindNextFile(handle, &find_data));

		FindClose(handle);
	}

	return found;
}

u32 read_cached_info(FileSystem &file_system, String &filepath, u64 key, FILETIME last_time_modified, Ending *ending) {
	Cache &cache = file_system.cache;
	u32 index = cache_lookup(cache, key);
	CacheEntry &entry = cache.entries[index];

	cache.touched[index] = true;

	CacheState state;
	if (entry.key == key) {
		b32 result = CompareFileTime(&entry.file_time, &last_time_modified);
        state = result ? CacheState_Modified : CacheState_Unmodified;
	} else {
		state = CacheState_Added;
	}

	entry.filetype = ending->id;
	entry.key = key;
	entry.file_time = last_time_modified;

	file_system.file_infos[index].filepath = filepath;
	file_system.file_infos[index].state = state;
	file_system.file_infos[index].filetype = ending->id;
	file_system.file_infos[index].key = key;

	return index;
}
void read_infocache_from_disc(FILETIME executable_modifed_time, String &cache_filepath, Cache &cache) {
	FILE *cache_file;
	fopen_s(&cache_file, *cache_filepath, "rb");
	if (!cache_file)
		return;

	FILETIME cached_executable_modifed_time;
	fread(&cached_executable_modifed_time, sizeof(FILETIME), 1, cache_file);
	if (CompareFileTime(&cached_executable_modifed_time, &executable_modifed_time) == 0) {
		fread(&cache.editor_mode, sizeof(bool), 1, cache_file);
		fread(cache.entries, sizeof(CacheEntry), cache.max_count, cache_file);
		cache.valid = true;
	}

	fclose(cache_file);
}
void write_infocache_to_disc(FILETIME executable_modifed_time, String &cache_filepath, Cache &cache) {
	FILE *cache_file;
	fopen_s(&cache_file, *cache_filepath, "wb");

	fwrite(&executable_modifed_time, sizeof(FILETIME), 1, cache_file);

	fwrite(&cache.editor_mode, sizeof(bool), 1, cache_file);
	fwrite(cache.entries, sizeof(CacheEntry), cache.max_count, cache_file);
	fclose(cache_file);
}




#ifdef OS_WINDOWS
	#define mkdir(name, mode) _mkdir(name)
#endif

enum CacheState {
	CacheState_Unmodified = 1 << 0,
	CacheState_Added      = 1 << 1,
	CacheState_Removed    = 1 << 2,
	CacheState_Modified   = 1 << 3,

	CacheState_All        = 0xffffffff,
};
struct CacheEntry {
	time_t file_time;
	u32 filetype;
	i32 __padding;
	u64 key;
};
struct Cache {
	CacheEntry *entries;
	bool *touched;
	u32 max_count;

	bool editor_mode;
	bool valid;
	u16 _padding;
};
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

struct FileInfo {
	String filepath;
	CacheState state;
	u32 filetype;
	u64 key;
};

// From settings
struct Ending {
	String string;

	u32 id;
	u32 _padding;
	u64 mask;

	Ending *next;
};

struct FileSystem {
	Ending ending_storage[64];
	Ending ending_lookup[128];

	String source_folder;
	String output_folder;

	Cache cache;
	FileInfo *file_infos;

	i32 ending_storage_count;
	i32 __padding;
};

void init_ending(Ending *ending, char *string, u32 length, u32 index) {
	ending->string = make_string(string, (i32)length);
	ending->mask = 1<<index;
	ending->id = to_id32(length, string);
}

void init_filesystem(FileSystem &file_system, char *source_folder, char *output_folder, MemoryArena &arena) {
	Cache c = {};
	file_system.cache = c;
	file_system.cache.max_count = 8192;
	file_system.cache.entries = PUSH_STRUCTS(arena, file_system.cache.max_count, CacheEntry, true);
	file_system.cache.touched = PUSH_STRUCTS(arena, file_system.cache.max_count, bool, true);

	file_system.source_folder = make_string(source_folder);
	file_system.output_folder = make_string(output_folder);

	struct stat st;
	if (stat(output_folder, &st) == -1) {
		mkdir(output_folder, 0700);
	}

	file_system.file_infos = PUSH_STRUCTS(arena, file_system.cache.max_count, FileInfo);

	u32 index = 0;

	FileSystem &fs = file_system;

	{
		Ending *top = &fs.ending_lookup['h'];
		init_ending(top, "h", sizeof("h") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['p'];
		init_ending(top, "cpp", sizeof("cpp") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['y'];
		init_ending(top, "entity", sizeof("entity") - 1, index++);

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "py", sizeof("py") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['s'];
		init_ending(top, "conversions", sizeof("conversions") - 1, index++);

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "particles", sizeof("particles") - 1, index++);
		top = top->next;

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "ability_nodes", sizeof("ability_nodes") - 1, index++);
		top = top->next;

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "behavior_nodes", sizeof("behavior_nodes") - 1, index++);
		top = top->next;

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "abilities", sizeof("abilities") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['e'];
		init_ending(top, "game_state_machine", sizeof("game_state_machine") - 1, index++);

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "package", sizeof("package") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['t'];
		init_ending(top, "unit", sizeof("unit") - 1, index++);

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "shading_environment", sizeof("shading_environment") - 1, index++);
		top = top->next;

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "font", sizeof("font") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['l'];
		init_ending(top, "material", sizeof("material") - 1, index++);

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "level", sizeof("level") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['w'];
		init_ending(top, "flow", sizeof("flow") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['r'];
		init_ending(top, "anim_controller", sizeof("anim_controller") - 1, index++);

		top->next = &fs.ending_storage[fs.ending_storage_count++];
		init_ending(top->next, "behavior", sizeof("behavior") - 1, index++);
	}

	{
		Ending *top = &fs.ending_lookup['o'];
		init_ending(top, "nfo", sizeof("nfo") - 1, index++);
	}
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
	//if (file_path[0] == '.')
	//	return true;
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

// // Go through the cache and check all the entries that weren't found when iterating over the file structure.
// // This will be the entries that corresponds to removed files.
// void prune_removed_files(FileSystem &file_system) {
// 	Cache &cache = file_system.cache;
// 	for (u32 i = 0; i < cache.max_count; ++i) {
// 		CacheEntry &entry = cache.entries[i];
// 		if (!cache.touched[i] && entry.key) { // If this cache entry wasn't touched, the file that produced it have been removed.
// 			// file_infos[i].filepath = entry.filepath;
// 			file_system.file_infos[i].state = CacheState_Removed;
// 			file_system.file_infos[i].filetype = entry.filetype;
// 			file_system.file_infos[i].key = entry.key;

// 			register_file_info_change(generator_array, file_infos[i], i);
// 			entry.key = 0;
// 		}
// 	}
// }


// FILETIME get_current_module_filetime() {
// 	char buffer[MAX_PATH] = {};
// 	GetModuleFileName(0, buffer, ARRAY_COUNT(buffer));

// 	WIN32_FIND_DATA data;
// 	HANDLE handle = find_first_file(buffer, &data);

// 	FILETIME filetime = data.ftLastWriteTime;

// 	FindClose(handle);
// 	return filetime;
// }

// ui3264_t key; CacheEntry *entry;
// 			Ending *ending = get_ending(file_system, filename_string);
// 				String file_path = clone_string(file_path_string, arena);

// 				u32 index = read_cached_info(cache, touched, file_infos, file_path, file_attributes.st_mtime, ending, current_location, &key, &entry);
// 				FileInfo &file_info = file_infos[index];

// 				if (file_info.state != CacheState_Unmodified) {
// 					entry->key = key;
// 					entry->file_time = find_data.ftLastWriteTime;
// 				}

u32 read_cached_info(FileSystem &file_system, String &filepath, u64 key, time_t last_time_modified, Ending *ending) {
	Cache &cache = file_system.cache;
	u32 index = cache_lookup(cache, key);
	CacheEntry &entry = cache.entries[index];

	cache.touched[index] = true;

	CacheState state;
	if (entry.key == key) {
		b32 result = entry.file_time == last_time_modified;
        state = result ? CacheState_Unmodified : CacheState_Modified;
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
void read_infocache_from_disc(time_t executable_modifed_time, String &cache_filepath, Cache &cache) {
	FILE *cache_file = fopen(*cache_filepath, "rb");
	if (!cache_file)
		return;

	time_t cached_executable_modifed_time;
	fread(&cached_executable_modifed_time, sizeof(time_t), 1, cache_file);
	if (cached_executable_modifed_time == executable_modifed_time) {
		fread(&cache.editor_mode, sizeof(bool), 1, cache_file);
		fread(cache.entries, sizeof(CacheEntry), cache.max_count, cache_file);
		cache.valid = true;
	}

	fclose(cache_file);
}
void write_infocache_to_disc(time_t executable_modifed_time, String &cache_filepath, Cache &cache) {
	FILE *cache_file = fopen(*cache_filepath, "wb");

	fwrite(&executable_modifed_time, sizeof(time_t), 1, cache_file);

	fwrite(&cache.editor_mode, sizeof(bool), 1, cache_file);
	fwrite(cache.entries, sizeof(CacheEntry), cache.max_count, cache_file);
	fclose(cache_file);
}




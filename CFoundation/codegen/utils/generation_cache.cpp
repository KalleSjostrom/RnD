struct CachedData {
	FILETIME file_time;
	unsigned size;
	void *buffer;
};
struct CacheHashEntry {
	CachedData value;
	unsigned header_type;
	uint64_t key;
	bool touched;
};

struct CacheHashMap {
	CacheHashEntry entries[2048];
};
struct CacheHeader {
	unsigned version;
	unsigned count;
};

bool pop_cache_entry_for(CacheHashMap &cache_hash_map, char *filename, WIN32_FIND_DATA &find_data, unsigned header_type, uint64_t *key, CacheHashEntry **out_cache_entry) {
	String file = make_string(filename);
	*key = make_string_id64(file);

	HASH_LOOKUP(entry, cache_hash_map.entries, ARRAY_COUNT(cache_hash_map.entries), *key);
	*out_cache_entry = entry;
	entry->touched = true;
	entry->header_type = header_type;

	if (entry->key == *key) {
		int result = CompareFileTime(&entry->value.file_time, &find_data.ftLastWriteTime);
		return result != 0;
	}

	return true;
}

inline void fill_hash_entry(CacheHashEntry *entry, uint64_t key, FILETIME &filetime) {
	entry->key = key;
	entry->value.file_time = filetime;
}

void read_cache_from_disc(MemoryArena &arena, CacheHashMap &cache_hash_map, unsigned version) {
	FILE *cache_file = fopen("cache.bin", "rb");

	CacheHeader cache_header = {};
	if (cache_file) {
		fread(&cache_header, sizeof(CacheHeader), 1, cache_file);
		if (cache_header.version != version)
			cache_header.count = 0;
	}

	for (int i = 0; i < cache_header.count; ++i) {
		CacheHashEntry cache_hash_entry;
		fread(&cache_hash_entry, sizeof(CacheHashEntry), 1, cache_file);

		CachedData &cached_data = cache_hash_entry.value;
		cached_data.buffer = allocate_memory(arena, cached_data.size);
		fread(cached_data.buffer, 1, cached_data.size, cache_file);

		HASH_LOOKUP(entry, cache_hash_map.entries, ARRAY_COUNT(cache_hash_map.entries), cache_hash_entry.key);
		ASSERT(entry->key != cache_hash_entry.key, "Found entry in hashmap while reading from disc");
		entry->value = cache_hash_entry.value;
		entry->header_type = cache_hash_entry.header_type;
		entry->key = cache_hash_entry.key;
		entry->touched = false;
	}

	if (cache_file)
		fclose(cache_file);
}

void write_cache_to_disc(CacheHashMap &cache_hash_map, unsigned version) {
	FILE *cache_out_file = fopen("cache.bin", "wb");
	unsigned cache_entry_count = 0;
	CacheHashEntry entries[ARRAY_COUNT(cache_hash_map.entries)];
	for (int i = 0; i < ARRAY_COUNT(cache_hash_map.entries); ++i) {
		CacheHashEntry &entry = cache_hash_map.entries[i];
		if (entry.key != 0 && entry.touched) {
			entries[cache_entry_count++] = entry;
		}
	}

	CacheHeader header;
	header.version = version;
	header.count = cache_entry_count;

	fwrite(&header, sizeof(CacheHeader), 1, cache_out_file);

	for (int i = 0; i < header.count; ++i) {
		CacheHashEntry &entry = entries[i];
		fwrite(&entry, sizeof(CacheHashEntry), 1, cache_out_file);

		CachedData &cached_data = entry.value;
		fwrite(cached_data.buffer, 1, cached_data.size, cache_out_file);
	}

	fclose(cache_out_file);
}

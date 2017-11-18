struct CachedData {
	void *buffer;
	size_t size;
};
struct CacheHashEntry {
	CachedData value;
	u64 key;
	b32 touched;
	b32 __padding;
};

HashMap_Make(CacheHashMap, CacheHashEntry);
struct CacheHeader {
	unsigned count;
};

b32 get_cache_entry_for(CacheHashMap &cache_hash_map, u64 key, CacheHashEntry **out_cache_entry) {
	HashMap_Lookup(entry, cache_hash_map, key, 0);
	*out_cache_entry = entry;
	entry->touched = true;
	b32 existed = entry->key == key;
	entry->key = key;
	return existed;
}

void read_cache_from_disc(char *filename, MemoryArena &arena, CacheHashMap &cache_hash_map) {
	FILE *cache_file = fopen(filename, "rb");

	CacheHeader cache_header = {};
	if (cache_file) {
		fread(&cache_header, sizeof(CacheHeader), 1, cache_file);
	}

	for (u32 i = 0; i < cache_header.count; ++i) {
		CacheHashEntry cache_hash_entry;
		fread(&cache_hash_entry, sizeof(CacheHashEntry), 1, cache_file);

		CachedData &cached_data = cache_hash_entry.value;
		cached_data.buffer = PUSH_SIZE(arena, cached_data.size);
		fread(cached_data.buffer, 1, cached_data.size, cache_file);

		HashMap_Lookup(entry, cache_hash_map, cache_hash_entry.key, 0);
		ASSERT(entry->key != cache_hash_entry.key, "Found entry in hashmap while reading from disc");
		entry->value = cache_hash_entry.value;
		entry->key = cache_hash_entry.key;
		entry->touched = false;
	}

	if (cache_file)
		fclose(cache_file);
}

void write_cache_to_disc(MemoryArena &arena, char *filename, CacheHashMap &cache_hash_map) {
	FILE *cache_out_file = fopen(filename, "wb");
	(void)cache_out_file;
	unsigned cache_entry_count = 0;
	TempAllocator ta(&arena);
	CacheHashEntry *entries = PUSH_STRUCTS(arena, cache_hash_map.capacity, CacheHashEntry);
	for (u32 i = 0; i < cache_hash_map.capacity; ++i) {
		CacheHashEntry &entry = cache_hash_map.data[i];
		if (entry.key != 0 && entry.touched) {
			entries[cache_entry_count++] = entry;
		}
	}

	CacheHeader header;
	header.count = cache_entry_count;

	fwrite(&header, sizeof(CacheHeader), 1, cache_out_file);

	for (u32 i = 0; i < header.count; ++i) {
		CacheHashEntry &entry = entries[i];
		fwrite(&entry, sizeof(CacheHashEntry), 1, cache_out_file);

		CachedData &cached_data = entry.value;
		fwrite(cached_data.buffer, 1, cached_data.size, cache_out_file);
	}

	fclose(cache_out_file);
}

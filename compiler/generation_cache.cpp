struct CachedData {
	void *buffer;
	size_t size;
};
struct CacheHashEntry {
	DECLARE_HASH_ENTRY(CachedData)
	bool touched;
};

struct CacheHeader {
	unsigned count;
};

bool get_cache_entry_for(CacheHashEntry *cache_hashmap, u64 key, CachedData *out_data) {
	CacheHashEntry *entry = inplace_hash_lookup(cache_hashmap, key);
	bool existed = entry->key == key;

	entry->key = key;
	entry->touched = true;

	*out_data = entry->value;
	return existed;
}

void read_cache_from_disc(ArenaAllocator &arena, char *filename, CacheHashEntry *cache_hashmap) {
	FILE *cache_file;
	fopen_s(&cache_file, filename, "rb");

	CacheHeader cache_header = {};
	if (cache_file) {
		fread(&cache_header, sizeof(CacheHeader), 1, cache_file);
	}

	for (u32 i = 0; i < cache_header.count; ++i) {
		CacheHashEntry read_entry;
		fread(&read_entry, sizeof(CacheHashEntry), 1, cache_file);

		CachedData &cached_data = read_entry.value;
		cached_data.buffer = PUSH(&arena, cached_data.size, char);
		fread(cached_data.buffer, 1, cached_data.size, cache_file);

		CacheHashEntry *entry = inplace_hash_lookup(cache_hashmap, read_entry.key);
		ASSERT(entry->key != read_entry.key, "Found entry in hashmap while reading from disc");
		entry->value = read_entry.value;
		entry->key = read_entry.key;
		entry->touched = false;
	}

	if (cache_file)
		fclose(cache_file);
}

void write_cache_to_disc(ArenaAllocator &arena, char *filename, CacheHashEntry *cache_hashmap) {
	FILE *cache_file;
	fopen_s(&cache_file, filename, "wb");
	(void)cache_file;
	unsigned cache_entry_count = 0;
	TempAllocator ta(&arena);

	InplaceHashHeader hash_header = _inplace_hash_header(cache_hashmap);
	CacheHashEntry *entries = PUSH(&arena, hash_header.capacity, CacheHashEntry);
	for (i32 i = 0; i < hash_header.capacity; ++i) {
		CacheHashEntry &entry = cache_hashmap[i];
		if (entry.key != 0 && entry.touched) {
			entries[cache_entry_count++] = entry;
		}
	}

	CacheHeader header;
	header.count = cache_entry_count;

	fwrite(&header, sizeof(CacheHeader), 1, cache_file);

	for (u32 i = 0; i < header.count; ++i) {
		CacheHashEntry &entry = entries[i];
		fwrite(&entry, sizeof(CacheHashEntry), 1, cache_file);

		CachedData &cached_data = entry.value;
		fwrite(cached_data.buffer, 1, cached_data.size, cache_file);
	}

	fclose(cache_file);
}

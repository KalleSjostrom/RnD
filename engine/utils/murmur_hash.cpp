u64 murmur_hash_64(const void * key, u32 len, u64 seed) {
	const u64 m = 0xc6a4a7935bd1e995ULL;
	const i32 r = 47;

	u64 h = seed ^ (len * m);

	const u64 * data = (const u64 *)key;
	const u64 * end = data + (len/8);

	while (data != end) {
		u64 k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const char * data2 = (const char*)data;

	switch (len & 7) {
		case 7: h ^= u64(data2[6]) << 48;
		[[clang::fallthrough]];
		case 6: h ^= u64(data2[5]) << 40;
		[[clang::fallthrough]];
		case 5: h ^= u64(data2[4]) << 32;
		[[clang::fallthrough]];
		case 4: h ^= u64(data2[3]) << 24;
		[[clang::fallthrough]];
		case 3: h ^= u64(data2[2]) << 16;
		[[clang::fallthrough]];
		case 2: h ^= u64(data2[1]) << 8;
		[[clang::fallthrough]];
		case 1: h ^= u64(data2[0]);
			h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

inline u32 to_id32(u32 len, const char *s) {
	u64 id64 = murmur_hash_64(s, len, 0);
	return (id64 >> 32);
}

inline u64 to_id64(u32 len, const char *s) {
	return murmur_hash_64(s, len, 0);
}

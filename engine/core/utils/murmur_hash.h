#pragma once

uint64_t murmur_hash_64(const void *key, unsigned len, uint64_t seed);

__forceinline unsigned to_id32(unsigned len, const char *s) {
	uint64_t id64 = murmur_hash_64(s, len, 0);
	return (id64 >> 32);
}

__forceinline uint64_t to_id64(unsigned len, const char *s) {
	return murmur_hash_64(s, len, 0);
}

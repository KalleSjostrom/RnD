#pragma once

// WARNING(kalle): These hash defines are deprecated!
// We want to move away from using defines, since they are harder to debug

#define DEPRECATED_HASH_ENTRY(type_name, key_type, value_type) \
	struct type_name { \
		key_type key; \
		value_type value; \
	};

#define DEPRECATED_HASH_MAP(name, entry_type, size) \
	struct name { \
		entry_type map[size]; \
		entry_type default_entry; \
	}; \

#define DEPRECATED_HASH_INIT(hashmap, type, default_key, default_value) \
	{ \
		type __default = { default_key, default_value }; \
		hashmap.default_entry = __default; \
		for (unsigned i = 0; i < ARRAY_COUNT(hashmap.map); ++i) { \
			hashmap.map[i] = __default; \
		} \
	}

#define DEPRECATED_HASH_LOOKUP(entry, hashmap, _key) \
	auto *entry = hashmap.map; \
	{ \
		unsigned __size = ARRAY_COUNT(hashmap.map); \
		unsigned __index = (_key) % __size; \
		for (unsigned __i = 0; __i < __size; ++__i) { \
			entry = (hashmap).map + __index; \
			if (entry->key == (_key) || entry->key == (hashmap).default_entry.key) { \
				break; \
			} \
			__index++; \
			if (__index == (__size)) \
				__index = 0; \
		} \
	}

#define DEPRECATED_HASH_LOOKUP_STATIC(entry, _hashmap, _key) \
	auto *entry = (_hashmap); \
	{ \
		unsigned __size = ARRAY_COUNT(_hashmap); \
		unsigned __index = (_key) % __size; \
		for (unsigned __i = 0; __i < __size; ++__i) { \
			entry = (_hashmap) + __index; \
			if (entry->key == (_key) || entry->key == 0) { \
				break; \
			} \
			__index++; \
			if (__index == (__size)) \
				__index = 0; \
		} \
	}

#define DEPRECATED_HASH_ADD(hashmap, _key, _value) \
	{ \
		DEPRECATED_HASH_LOOKUP(entry, hashmap, (_key)); \
		entry->key = (_key); \
		entry->value = (_value); \
	}

// TODO(kalle): Implement DEPRECATED_HASH_REMOVE
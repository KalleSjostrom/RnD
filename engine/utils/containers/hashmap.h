/*
Based on the "Hacker's Delight" version:
unsigned clp2(unsigned x) {
   x = x - 1;
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   x = x | (x >>16);
   return x + 1;
}*/
#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

#define DEFAULT_BUCKET_SIZE 2
#define HASH_SIZE_FOR(size) (next_power_of_2((size) * DEFAULT_BUCKET_SIZE)) // NOTE(kalle): We use a default of 2 as bucket_size!
#define HASH_SIZE_USING_BUCKET_SIZE(size, bucket_size) (next_power_of_2((size) * bucket_size))

/*
This is a simple hash map with open addressing (internally chained) and linear probing.

It is designed for storing a link between an entity and a component instance index.
NOTE(kalle): If using for anything else, make sure the usage agrees with the design.

The hash function is a simple & operation which requires the hash size to be a power of two.

There is an option to use a "bucket size", which will increase the memory footprint but lower risk of hash collisions (load factor).
If bucket size is 2, the storage array will be twice as large and have the following format
hash:      1     2     3     4
bucket: | 0 1 | 2 3 | 4 5 | 6 7 |
This means that we have one extra slot in place for one collision. Since we use this for entities which (for the most part) have monotonically increasing ids, having more than 2 collisions is unlikely.

NOTE(kalle): Remove is by far the most expensive operation to do. However, since remove is a rare operation (in contrast to add and lookup), this shouldn't matter much.
Remove is slow (or potentially slow) because we might need to shift entries that should have taken the removed entry's place.
NOTE(kalle): Right now it moves them in a bubble up sort of way, i.e. one at a time, no searching for the best fit, and moving that directly.
*/

// TODO(pp): Should we break out the "broadphase"-hashmap logic into its own thing? Currently
//           the HashMap is a bit muddied down with the different concepts and it's not even clear
//           why either works.
// TODO(kalle): Could the key & value be packed into 32 bits for less memory? Is it even beneficial?
struct HashEntry {
	union {
		u32 key; // e.g. Entity.  key == 0 means invalid
		u32 hash; // Used when hashing outside of the hashmap, e.g. for broadphases
	};
	union {
		u32 value; // e.g. index into the component instance array.
		u32 handle; // used by broadphase
	};
};
struct HashMap {
	HashEntry *entries;
	// NOTE(kalle): Must be a power of two! Use HASH_SIZE_FOR above
	u32 count;
	union {
		u32 invalid_key;
		u32 invalid_hash;
	};
	u32 bucket_size;
	u32 __padding;
};



enum HashMapOptions {
	HashMapOptions_ExistingMatchOnly,
};

#define HashMap_Make(map_name, entry_name) \
	struct map_name { \
		entry_name *data; \
		u32 capacity; \
		u32 bucket_size; \
		u64 invalid_key; \
	}

#define HashMap_Init(hashmap, _data, _capacity, _bucket_size, _invalid_key) do { \
	hashmap.data = (_data); \
	hashmap.capacity = (_capacity); \
	hashmap.bucket_size = (_bucket_size); \
	hashmap.invalid_key = (_invalid_key); \
} while(0)

#define HashMap_Lookup(entry, _hashmap, _key, _flags) \
	auto *entry = (_hashmap).data; \
	{ \
		u64 lookup_key = (_key) * (_hashmap).bucket_size; \
		u64 hash_mask = (u64)(_hashmap).capacity-1; \
		for (u64 __offset = 0; __offset < (_hashmap).capacity; __offset++) { \
			u64 index = (lookup_key + __offset) & hash_mask; \
			entry = (_hashmap).data + index; \
			\
			if ((_flags) & HashMapOptions_ExistingMatchOnly) { \
				if (entry->key == (_key)) { \
					break; \
				} \
			} else { \
				if (entry->key == (_key) || entry->key == (_hashmap).invalid_key) { \
					break; \
				} \
			} \
		} \
	}

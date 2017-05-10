#include "common.h"
#include "profiler.c"

#define MAX_AVATARS 48
// NOTE(kalle): Must be a power of two!
#define AVATAR_HASH_SIZE next_power_of_2(MAX_AVATARS)
#define AVATAR_HASH_MASK (AVATAR_HASH_SIZE-1)

enum ProfilerScopes {
	ProfilerScopes__add,
	ProfilerScopes__remove,
	ProfilerScopes__simulation,

	ProfilerScopes__count,
};

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

// We want to avoid high load factor so we use a BUCKET_SIZE > 1
// TODO(kalle): Make this a property of the HashMap?
#define BUCKET_SIZE 2


namespace testa {
	#define TKEY uint64_t
	#define TVALUE uint64_t
	#include "hashmap.h"
	#undef TKEY
	#undef TVALUE
}
typedef testa::HashEntry HashEntry64;
typedef testa::HashMap HashMap64;
HashMap<uint64_t, void*>


namespace test {
	#define TKEY unsigned
	#define TVALUE unsigned
	#include "hashmap.h"
	#undef TKEY
	#undef TVALUE
}
typedef test::HashEntry HashEntry;
typedef test::HashMap HashMap;

#define hash_size (AVATAR_HASH_SIZE*BUCKET_SIZE)

inline void test_add(HashMap &hashmap, unsigned *keys, unsigned &num_entries, unsigned i) {
	add(hashmap, i, 0);
	keys[num_entries++] = i;
}
inline void test_remove(HashMap &hashmap, unsigned *keys, unsigned &num_entries, unsigned &i, unsigned rand) {
	unsigned index = rand % num_entries;
	remove(hashmap, keys[index]);
	i--;
	keys[index] = keys[--num_entries];
}


int main(int argc, char const *argv[]) {
	HashEntry entries[hash_size] = {};
	HashMap hashmap;
	hashmap.entries = entries;
	hashmap.count = hash_size;
	hashmap.invalid_key = 0;

	unsigned num_keys = 65536*512;

	unsigned num_entries = 0;
	unsigned *keys = (unsigned*)malloc(MAX_AVATARS*sizeof(unsigned));

	bool *choices = (bool*)malloc(num_keys*sizeof(bool));
	for (unsigned i = 1; i < num_keys; ++i) {
		choices[i] = random()%256 < 128;
	}

	unsigned *randoms = (unsigned*)malloc(num_keys*sizeof(unsigned));
	for (unsigned i = 1; i < num_keys; ++i) {
		randoms[i] = random();
	}

	PROFILER_START(simulation);
	for (unsigned i = 1; i < num_keys; ++i) {
		if (num_entries < MAX_AVATARS-8) {
			test_add(hashmap, keys, num_entries, i);
		} else if (num_entries == MAX_AVATARS-1) {
			test_remove(hashmap, keys, num_entries, i, randoms[i]);
		} else if (num_entries < MAX_AVATARS/2) {
			if (choices[i]) {
				test_add(hashmap, keys, num_entries, i);
			} else {
				test_remove(hashmap, keys, num_entries, i, randoms[i]);
			}
		} else {
			if (choices[i]) {
				test_remove(hashmap, keys, num_entries, i, randoms[i]);
			} else {
				test_add(hashmap, keys, num_entries, i);
			}
		}
	}

	for (unsigned i = 0; i < num_entries; ++i) {
		test_remove(hashmap, keys, num_entries, i, randoms[i]);
	}
	PROFILER_STOP(simulation);

	/*printf("Adds %u\n", _add_itr);
	printf("Add lookups %u\n", _add_lookups_itr);
	printf("\tAdd ratio %.3f\n", (float)_add_lookups_itr / (float)_add_itr);
	printf("Removes %u\n", _remove_itr);
	printf("Remove lookups %u\n", _remove_lookups_itr);
	printf("\tRemove ratio %.3f\n", (float)_remove_lookups_itr / (float)_remove_itr);
	printf("Remove moves %u\n", _move_into_place_itr);
	printf("\tRemove moves ratio %.3f\n", _move_into_place_itr / (float)_remove_itr);
	printf("\n");*/

	PROFILER_PRINT(add);
	PROFILER_PRINT(remove);
	PROFILER_PRINT(simulation);

	return 0;
}








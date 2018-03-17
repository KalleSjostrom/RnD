#pragma warning(disable : 4255)
#pragma warning(disable : 4100) // unreferenced formal parameter

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdlib.h>
#include <stdint.h>

#include <assert.h>

#include "utils/array.h"

#define invalid_key 0
#include "utils/hashmap.h"

typedef struct {
	int key;
	int value;
} HashEntry;

int run(int argc, char *argv[]) {
	int *list = 0;
	_array_ensure_space(list, 16);
	array_push(list, 5);
	array_push(list, 3);
	array_push(list, 8);
	array_push(list, 1);
	array_push(list, 9);


	HashEntry *hash = 0;
 	hash_init(hash, 5, 4);

	int v[4] = { 3, 6, 2, 7 };
	hash_add(hash, 5, v[0]);
	hash_add(hash, 10, v[1]);
	hash_add(hash, 8, v[2]);
	hash_add(hash, 3, v[3]);

	HashEntry r[4];
	r[0] = *(HashEntry*)hash_lookup(hash, 5);
	r[1] = *(HashEntry*)hash_lookup(hash, 10);
	r[2] = *(HashEntry*)hash_lookup(hash, 8);
	r[3] = *(HashEntry*)hash_lookup(hash, 3);

	assert(r[0].value == v[0]);
	assert(r[1].value == v[1]);
	assert(r[2].value == v[2]);
	assert(r[3].value == v[3]);

	return 0;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
	(void) Instance;
	(void) PrevInstance;
	(void) ShowCode;

	char *argv[8];
	int argc = 0;
	argv[0] = CommandLine;

	int cursor = 0;
	while(1) {
		if (CommandLine[cursor] == ' ') {
			argv[argc][cursor] = '\0';
			argc++;
			cursor++;
			argv[argc] = CommandLine + cursor;
			continue;
		} else if (CommandLine[cursor] == '\0') {
			if (cursor > 0) {
				argc++;
			}
			break;
		}

		cursor++;
	}
	return run(argc, argv);
}

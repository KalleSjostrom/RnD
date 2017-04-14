#include <stdio.h>
#include "common.h"

inline long get_filesize(FILE *file) {
	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);
	return length;
}
inline FILE *open_file(const char *filename, size_t *filesize) {
	FILE *file = fopen(filename, "r");
	ASSERT(file, "Could not find file %s!", filename);
	*filesize = get_filesize(file);
	return file;
}

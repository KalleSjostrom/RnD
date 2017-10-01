#pragma once

inline u64 get_filesize(FILE *file) {
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	ASSERT(length != -1L, "Could not get filesize from file!");
	fseek(file, 0, SEEK_SET);
	return (u64)length;
}
inline FILE *open_file(const char *filename, size_t *filesize) {
	FILE *file = fopen(filename, "r");
	ASSERT(file, "Could not find file %s!", filename);
	*filesize = get_filesize(file);
	return file;
}

inline b32 file_exists(const char *filename) {
	// TODO(kalle): Replace with something sane.
	FILE *file = fopen(filename, "r");
	return file != 0;
}

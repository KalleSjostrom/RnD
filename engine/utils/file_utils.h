#pragma once

#include <io.h>

inline u64 get_filesize(FILE *file) {
	return (u64)_filelengthi64(_fileno(file));
}

inline FILE *open_file(const char *filename, size_t *filesize, char *mode = "rb") {
	FILE *file;
	fopen_s(&file, filename, mode);
	ASSERT(file, "Could not find file %s!", filename);
	*filesize = get_filesize(file);
	return file;
}

inline FILE *try_open_file(const char *filename, size_t *filesize, char *mode = "rb") {
	FILE *file;
	fopen_s(&file, filename, mode);
	if (!file)
		return 0;

	*filesize = get_filesize(file);
	return file;
}

inline bool file_exists(const char *filename) {
	// TODO(kalle): Replace with something sane.
	FILE *file;
	fopen_s(&file, filename, "rb");
	return file != 0;
}

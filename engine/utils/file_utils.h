#pragma once

#ifdef OS_WINDOWS
	#include <io.h>
	inline u64 get_filesize(FILE *file) {
		return (u64)_filelengthi64(_fileno(file));
	}
#else
	inline u64 get_filesize(FILE *file) {
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		ASSERT(length != -1L, "Could not get filesize from file!");
		fseek(file, 0, SEEK_SET);
		return (u64)length;
	}
#endif

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

inline b32 file_exists(const char *filename) {
	// TODO(kalle): Replace with something sane.
	FILE *file;
	fopen_s(&file, filename, "rb");
	return file != 0;
}

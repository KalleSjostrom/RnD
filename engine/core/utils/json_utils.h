#pragma once

#include "core/third_party/json/json.h"
#include "core/third_party/json/json.c"

__forceinline bool _json_string_compare(json_string_s *json_string, const char *cmp_string, unsigned str_len) {
	if (json_string->string_size != str_len)
		return false;
	return memcmp(json_string->string, cmp_string, str_len) == 0;
}
#define json_string_compare(json_string, string) _json_string_compare(json_string, string, sizeof(string))

void *json_alloc(void *user_data, size_t size) {
	ArenaAllocator *arena = (ArenaAllocator*)user_data;
	return allocate(arena, size);
}

json_value_s *open_json(const char *file_name, ArenaAllocator *arena) {
	HANDLE file_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD filesize = GetFileSize(file_handle, 0);
	DWORD error = GetLastError();

	char *source = (char*) allocate(arena, filesize);
	ReadFile(file_handle, source, filesize, 0, 0);

	json_parse_result_s parse_result = {};
	json_value_s *settings = json_parse_ex(source, filesize, json_parse_flags_allow_simplified_json | json_parse_flags_allow_c_style_comments, json_alloc, arena, &parse_result);
	if (parse_result.error == json_parse_error_none) {
		return settings;
	}
	return 0;
}
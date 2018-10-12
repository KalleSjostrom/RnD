#pragma once

#include <stdint.h>

#include "murmur_hash.h"
#include "string.h"
#include "dynamic_string.h"

__forceinline unsigned string_id32(String s) {
	return to_id32((unsigned)string_length(s), *s);
}
__forceinline uint64_t string_id64(String s) {
	return to_id64((unsigned)string_length(s), *s);
}
__forceinline unsigned string_id32(DynamicString ds) {
	return to_id32((unsigned)string_length(ds), ds.text);
}
__forceinline uint64_t string_id64(DynamicString ds) {
	return to_id64((unsigned)string_length(ds), ds.text);
}
__forceinline unsigned string_id32(const char *s) {
	return string_id32(string(s));
}
__forceinline uint64_t string_id64(const char *s) {
	return string_id64(string(s));
}
__forceinline unsigned string_id32(char *s) {
	return string_id32(string(s));
}
__forceinline uint64_t string_id64(char *s) {
	return string_id64(string(s));
}


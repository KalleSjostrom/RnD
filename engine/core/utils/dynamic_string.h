#pragma once

#include <stdio.h>

#include "core/utils/string.h"
#include "core/containers/array.h"

/** \addtogroup DynamicString
 *  @{
 */
struct DynamicString {
	char *text;

	__forceinline char operator[](int index) { return text[index]; }
	__forceinline char *operator*() { return text; }
};

__forceinline void dynamic_string_destroy(DynamicString &ds) {
	array_destroy(ds.text);
}

__forceinline int string_length(DynamicString &ds) {
	return array_count(ds.text);
}

__forceinline DynamicString & operator+=(DynamicString &a, const char *b) {
	int at = 0;
	while (b[at]) {
		array_push(a.text, b[at++]);
	}
	return a;
}
__forceinline DynamicString & operator+=(DynamicString &a, String s) {
	for (size_t i = 0; i < s.length; ++i) {
		array_push(a.text, s[i]);
	}
	return a;
}
__forceinline DynamicString & operator+=(DynamicString &a, DynamicString s) {
	for (int i = 0; i < string_length(s); ++i) {
		array_push(a.text, s[i]);
	}
	return a;
}

__forceinline DynamicString & operator-=(DynamicString &a, String s) {
	// TODO(kalle): This is very unsafe! If s length is greater than a length!
	array_count(a.text) -= (i32)s.length;
	return a;
}

__forceinline DynamicString dynamic_string(Allocator *a, int initial_size) {
	DynamicString ds = {};
	array_make(a, ds.text, initial_size);
	return ds;
}
__forceinline DynamicString dynamic_string(Allocator *a, String string) {
	DynamicString ds = {};
	array_make(a, ds.text, string.length);
	ds += string;
	return ds;
}
__forceinline DynamicString dynamic_string(Allocator *a, DynamicString string) {
	DynamicString ds = {};
	array_make(a, ds.text, string_length(string));
	ds += string;
	return ds;
}

__forceinline DynamicString dynamic_string(Allocator *a, const char *text) {
	return dynamic_string(a, string(text));
}
DynamicString dynamic_stringf(Allocator *a, const char *format, ...);

__forceinline String string(DynamicString ds) {
	String s = string(ds.text, string_length(ds));
	return s;
}
__forceinline void resize(DynamicString &ds, size_t new_size) {
	array_count(ds.text) = 0;
	_array_ensure_space(ds.text, new_size, 0);
	array_count(ds.text) = (int)new_size;
}

__forceinline void null_terminate(DynamicString &ds) {
	array_push(ds.text, '\0');
}

void printf(DynamicString &ds, const char *format, ...);

#define STR(s) s.length, s.text
#define DSTR(s) string_length(s), s.text

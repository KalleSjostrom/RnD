#pragma once

#include "engine/utils/memory/memory_arena.cpp"

struct String {
	char *text;
	i32 length;
	i32 __padding;
	inline char operator[](i32 index) { return text[index]; }
	inline char operator[](u32 index) { return text[index]; }
	inline char *operator*() { return text; }
};
typedef String* StringArray;

inline String make_string(char *string, i32 length) {
	String s = {};
	s.text = string;
	s.length = length;
	return s;
}

inline String make_string(char *string) {
	i32 length = 0;
	while (string[length] != '\0')
		length++;

	String s = {};
	s.text = string;
	s.length = length;
	return s;
}
inline void null_terminate(String s) { s.text[s.length] = '\0'; }

inline void copy_string(String &destination, String &source) {
	for (i32 i = 0; i < source.length; ++i) {
		destination.text[i] = source.text[i];
	}
	destination.length = source.length;
}

inline void append_string(String &destination, String &source) {
	for (i32 i = 0; i < source.length; ++i) {
		destination.text[i+destination.length] = source.text[i];
	}
	destination.length += source.length;
}

inline void append_strings(String &destination, String &A, String &B) {
	for (i32 i = 0; i < A.length; ++i) {
		destination.text[i] = A.text[i];
	}
	for (i32 i = 0; i < B.length; ++i) {
		destination.text[i+A.length] = B.text[i];
	}
	destination.length = A.length + B.length;
}

inline void append_path(String &path, i32 count, ...) {
	va_list arguments;
	va_start(arguments, count);

	for (i32 i = 0; i < count; ++i) {
		String *a = va_arg(arguments, String*);
		append_string(path, *a);
		if (i < count - 1)
			path.text[path.length++] = '/';
	}

	null_terminate(path);

	va_end(arguments);
}

inline void to_upper(String string) {
	for (i32 i = 0; i < string.length; ++i) {
		if (string.text[i] <= 'z' && string.text[i] >= 'a')
			string.text[i] += ('A' - 'a');
	}
}
inline void to_lower(String string) {
	for (i32 i = 0; i < string.length; ++i) {
		if (string.text[i] <= 'Z' && string.text[i] >= 'A')
			string.text[i] += ('a' - 'A');
	}
}

inline bool is_equal(String a, String b) {
	if (a.length != b.length)
		return false;

	for (i32 i = 0; i < a.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}

inline bool starts_with(String a, String b) {
	for (i32 i = 0; i < b.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}

inline bool ends_in(String string, String ending) {
	if (ending.length > string.length) // String doesn't end with ending if it's shorter!
		return false;

	for (i32 i = 0; i < ending.length; i++) {
		if (string.text[(string.length - 1) - i] != ending.text[(ending.length - 1) - i])
			return false;
	}
	return true;
}

b32 split(String string, String split, int *out_start, int *out_end) {
	*out_start = -1;

	// Search for the place to start the split
	for (i32 i = 0; i < string.length; ++i) {
		b32 found_mismatch = false;
		for (i32 j = 0; j < split.length && !found_mismatch; ++j) {
			found_mismatch = string.text[i + j] != split[j];
		}

		if (!found_mismatch) {
			*out_start = i;
			*out_end = i + split.length;
			return true;
		}
	}

	return false;
}

#define MAKE_STRING(text) make_string((char *)text, sizeof(text)-1)

// some_thing -> SomeThing
void to_pascal_case(String &dest, String &source) {
	dest.text[0] = source.text[0] + ('A' - 'a'); // Convert the first character to upper case.
	i32 target_index = 1;
	for (i32 i = 1; i < source.length; ++i) {
		// Look for an underscore
		if (source.text[i] == '_') {
			dest.text[target_index++] = source.text[++i] + ('A' - 'a');
		} else {
			dest.text[target_index++] = source.text[i];
		}
	}
	dest.length = target_index;
	null_terminate(dest);
}

// some_thing -> Some Thing
void prettify(String &string) {
	string.text[0] += ('A' - 'a'); // Convert the first character to upper case.
	for (i32 i = 1; i < string.length; ++i) {
		// Look for an underscore
		if (string.text[i] == '_') {
			string.text[i] = (' ');
			string.text[++i] += ('A' - 'a');
		} else {
			string.text[i] = string.text[i];
		}
	}
	null_terminate(string);
}

// SomeThing -> some_thing
String make_underscore_case(MemoryArena &arena, String string) {
	char buffer[512];

	i32 length = 0;
	for (i32 i = 0; i < string.length; ++i) {
		if (string.text[i] >= 'A' && string.text[i] <= 'Z') {
			if (i > 0)
				buffer[length++] = '_';
			buffer[length++] = string.text[i] + ('a' - 'A'); // Convert the to lower case.
		} else {
			buffer[length++] = string.text[i];
		}
	}

	char *new_string_buffer = PUSH_STRING(arena, (u32)length + 1);
	String s = make_string(new_string_buffer, length);
	for (i32 i = 0; i < length; ++i) {
		s.text[i] = buffer[i];
	}
	null_terminate(s);
	return s;
}

String clone_string(MemoryArena &arena, String &string) {
	char *buffer = PUSH_STRING(arena, (size_t)string.length + 1);
	buffer[string.length] = '\0';
	String newstring = make_string(buffer, string.length);
	copy_string(newstring, string);
	return newstring;
}

String allocate_string(MemoryArena &arena, size_t size) {
	String s = {};
	s.text = PUSH_STRING(arena, size);
	s.length = 0; // Not filled with anything useful!
	return s;
}

String append_strings(MemoryArena &arena, String &A, String &B, bool add_null_termination = false) {
	String result = allocate_string(arena, (size_t)(A.length + B.length + (add_null_termination?1:0)));
	append_strings(result, A, B);
	if (add_null_termination)
		null_terminate(result);

	return result;
}

inline String make_path(MemoryArena &arena, String &directory_path, String &filename) {
	String file_path_string = allocate_string(arena, (size_t)(directory_path.length + 1 + filename.length + 1));
	append_path(file_path_string, 2, &directory_path, &filename);
	return file_path_string;
}
String get_filename(String &filepath, bool strip_ending = false) {
	i32 at = filepath.length - 1;
	i32 end_of_string = filepath.length;
	for (; at >= 0; at--) {
		if (filepath[at] == '/')
			break;
		if (strip_ending && filepath[at] == '.')
			end_of_string = at;
	}
	at++;
	return make_string(filepath.text + at, end_of_string - at);
}

String get_directory(String filepath) {
	i32 at = filepath.length - 1;
	for (; at >= 0; at--) {
		if (filepath[at] == '/')
			break;
	}
	return make_string(filepath.text, at);
}

//// MOVE??? ////
#include "murmur_hash.cpp"
inline u32 make_string_id32(String &s) {
	return to_id32((u32)s.length, *s);
}
inline u64 make_string_id64(String &s) {
	return to_id64((u32)s.length, *s);
}

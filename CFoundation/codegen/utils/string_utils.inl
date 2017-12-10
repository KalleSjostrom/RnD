#pragma once

struct String {
	int length;
	char *text;
	inline char operator[](int index) { return text[index]; }
	inline char *operator*() { return text; }
};

inline String make_string(char *string, int length) {
	String s = {0};
	s.text = string;
	s.length = length;
	return s;
}

inline String make_string(char *string) {
	int length = 0;
	while (string[length] != '\0')
		length++;

	String s = { length, string };
	return s;
}

inline void copy_string(String &destination, String source) {
	for (unsigned i = 0; i < source.length; ++i) {
		destination.text[i] = source.text[i];
	}
	destination.length = source.length;
}

inline void append_string(String &destination, String source) {
	for (unsigned i = 0; i < source.length; ++i) {
		destination.text[i+destination.length] = source.text[i];
	}
	destination.length += source.length;
}

inline void append_strings(String &destination, String A, String B) {
	for (unsigned i = 0; i < A.length; ++i) {
		destination.text[i] = A.text[i];
	}
	for (unsigned i = 0; i < B.length; ++i) {
		destination.text[i+A.length] = B.text[i];
	}
	destination.length = A.length + B.length;
}

inline void to_upper(String string) {
	for (unsigned i = 0; i < string.length; ++i) {
		if (string.text[i] <= 'z' && string.text[i] >= 'a')
			string.text[i] += ('A' - 'a');
	}
}
inline void to_lower(String string) {
	for (unsigned i = 0; i < string.length; ++i) {
		if (string.text[i] <= 'Z' && string.text[i] >= 'A')
			string.text[i] += ('a' - 'A');
	}
}

inline bool string_ends_in(String string, String ending) {
	if (ending.length > string.length) // String doesn't end with ending if it's shorter!
		return false;

	for (unsigned i = 0; i < ending.length; i++) {
		if (string.text[(string.length - 1) - i] != ending.text[(ending.length - 1) - i])
			return false;
	}
	return true;
}

bool string_split(String string, String split, int *out_start, int *out_end) {
	*out_start = -1;

	// Search for the place to start the split
	for (unsigned i = 0; i < string.length; ++i) {
		bool found_mismatch = false;
		for (unsigned j = 0; j < split.length && !found_mismatch; ++j) {
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

inline void null_terminate(String &s) { s.text[s.length] = '\0'; }
inline char *printable_token(String &s) { null_terminate(s); return s.text; }
inline bool are_strings_equal(String a, String b) {
	if (a.length != b.length)
		return false;
	for (unsigned i = 0; i < a.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}
inline bool are_cstrings_equal(char *a, char *b) {
	while (*a != '\0' && *a == *b) { a++; b++; }
	return *a == *b;
}

bool starts_with(String &string, String &beginning) {
	if (beginning.length > string.length)
		return false;

	for (unsigned i = 0; i < beginning.length; ++i) {
		if (string.text[i] != beginning.text[i])
			return false;
	}
	return true;
}

inline String clone_string(String &string, MemoryArena &arena) {
	char *buffer = allocate_memory(arena, string.length + 1);
	buffer[string.length] = '\0';
	String newstring = make_string(buffer, string.length);
	copy_string(newstring, string);
	return newstring;
}

inline String allocate_string(MemoryArena &arena, size_t size) {
	String s = {};
	s.text = allocate_memory(arena, size);
	s.length = 0; // Not filled with anything useful!
	return s;
}

inline String make_upper_case(String string, MemoryArena &arena) {
	String s = clone_string(string, arena);
	to_upper(s);
	return s;
}

inline String make_lower_case(String string, MemoryArena &arena) {
	String s = clone_string(string, arena);
	to_lower(s);
	return s;
}

inline String slash_to_underscore(String string, MemoryArena &arena) {
	String s = clone_string(string, arena);
	for(unsigned i = 0; i < s.length; ++i) {
		if(s.text[i] == '/' || s.text[i] == '\\')
			s.text[i] = '_';
	}
	return s;
}

inline String to_idstring_identifier(String string, MemoryArena &arena) {
	return slash_to_underscore(make_lower_case(string, arena), arena);
}

#define MAKE_STRING(text) make_string((char *)text, sizeof(text)-1)

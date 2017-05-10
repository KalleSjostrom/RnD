#pragma once

struct String {
	char *text;
	i32 length;
	i32 __padding;
	inline char operator[](int index) { return text[index]; }
	inline char *operator*() { return text; }
};

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

#define MAKE_STRING(text) make_string((char *)text, sizeof(text)-1)


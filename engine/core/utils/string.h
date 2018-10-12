#pragma once

/** \addtogroup String
 * Acts as a window into an elswhere allocated string. If you need to allocate strings, see DynamicString, but consider using a simple char[].
 * It's usecases could be to record how long a char* string is or keep track of the directory part of a full path.
 *  @{
 */
struct String {
	char *text;
	size_t length;

	__forceinline char operator[](size_t index) { return text[index]; }
	__forceinline char *operator*() { return text; }
};

__forceinline size_t string_length(String s) {
	return s.length;
}

////////////// CONSTRUCTION / ALLOCATION //////////////
#define MAKE_STRING(text) string((char *)text, sizeof(text)-1)

__forceinline String string(char *text, size_t length) {
	String s = {};
	s.text = text;
	s.length = length;
	return s;
}
__forceinline String string(char *text) {
	size_t length = 0;
	while (text[length] != '\0')
		length++;

	return string(text, length);
}

__forceinline String string(const char *text, size_t length) {
	return string((char*)text, length);
}
__forceinline String string(const char *text) {
	return string((char*)text);
}

String string(struct Allocator *allocator, String source);

__forceinline void append_string(String &destination, String &source) {
	for (size_t i = 0; i < source.length; ++i) {
		destination.text[i+destination.length] = source.text[i];
	}
	destination.length += source.length;
}

////////////// QUERIES //////////////
__forceinline bool operator==(String a, String b) {
	if (a.length != b.length)
		return false;

	for (size_t i = 0; i < a.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}
__forceinline bool starts_with(String a, String b) {
	for (size_t i = 0; i < b.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}
__forceinline bool ends_with(String s, String ending) {
	if (ending.length > s.length) // String doesn't end with ending if it's shorter!
		return false;

	for (size_t i = 0; i < ending.length; i++) {
		if (s.text[(s.length - 1) - i] != ending.text[(ending.length - 1) - i])
			return false;
	}
	return true;
}

bool next_split_indices(String s, String split, size_t *out_start, size_t *out_end);

////////////// PATHS //////////////
__forceinline String get_filename(String path, bool strip_ending = false) {
	int at = (int)path.length - 1;
	int end_of_string = (int)path.length;
	for (; at >= 0; at--) {
		if (path[at] == '/' || path[at] == '\\')
			break;
		if (strip_ending && path[at] == '.')
			end_of_string = at;
	}
	at++;
	return string(path.text + at, end_of_string - at);
}
__forceinline String get_directory(String path) {
	int at = (int)path.length - 1;
	for (; at >= 0; at--) {
		if (path[at] == '/')
			break;
	}
	return string(path.text, at);
}
__forceinline String get_ending(String path) {
	int at = (int)path.length - 1;
	for (; at >= 0; at--) {
		if (path[at] == '.') {
			break;
		}
	}
	return string(path.text + at, path.length - at);
}


////////////// STYLE FORMATTING //////////////
// some_thing -> SomeThing
void to_pascal_case(String &dest, String &source);
// some_thing -> Some Thing
void prettify(String &s);

__forceinline String trim_quotes(String s) {
	s.text++;
	s.length -= 2; // Cut away the '"'
	return s;
}
__forceinline void to_upper(String &s) {
	for (size_t i = 0; i < s.length; ++i) {
		if (s.text[i] <= 'z' && s.text[i] >= 'a') {
			s.text[i] += ('A' - 'a');
		}
	}
}
__forceinline void to_lower(String &s) {
	for (size_t i = 0; i < s.length; ++i) {
		if (s.text[i] <= 'Z' && s.text[i] >= 'A') {
			s.text[i] += ('a' - 'A');
		}
	}
}
/** @}*/

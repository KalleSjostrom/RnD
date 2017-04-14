
struct String {
	u32 length;
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

inline void null_terminate(String s) { s.text[s.length] = '\0'; }
#if 0
inline bool is_equal(String s, char *text) {
	for (int i = 0; i < s.length; i++) {
		if (s.text[i] != text[i])
			return false;
	}
	return text[s.length] == '\0';
}
#endif
inline bool string_is_equal(String a, String b) {
	if (a.length != b.length)
		return false;
	for (int i = 0; i < a.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}
#if 0
static bool is_equal(const char *a, const char *b) {
	while (*a == *b) {
		a++; b++;
		if (*a == '\0' && *b == '\0')
			return true;
	}
	return false;
}
#endif

inline bool starts_with(String a, String b) {
	for (int i = 0; i < b.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}
	return true;
}

#define MAKE_STRING(text) make_string((char *)text, sizeof(text)-1)

/*
#define STRING_FPRINTF(file, format, ...) \
	fprintf(file, format, __VA_ARGS__) \
*/
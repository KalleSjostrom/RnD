inline String make_string(char *string, int length) {
	String s = {0};
	s.text = string;
	s.length = length;
	return s;
}
inline String make_string(char *string) {
	unsigned length = 0;
	while (string[length] != '\0')
		length++;

	String s = { length, string };
	return s;
}
#define MAKE_STRING(text) make_string((char *)text, sizeof(text)-1)

inline bool are_strings_equal(String a, String b) {
	if (a.length != b.length)
		return false;

	for (unsigned i = 0; i < a.length; ++i) {
		if (a.text[i] != b.text[i])
			return false;
	}

	return true;
}
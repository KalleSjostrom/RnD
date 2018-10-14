#include "string.h"

String string(Allocator *allocator, String source) {
	String result = source;
	result.text = (char*) allocate(allocator, source.length);
	for (size_t i = 0; i < source.length; ++i) {
		result.text[i] = source.text[i];
	}
	return result;
}

bool next_split_indices(String s, String split, size_t *out_start, size_t *out_end) {
	// Search for the place to start the split
	for (size_t i = 0; i < s.length; ++i) {
		bool found_mismatch = false;
		for (size_t j = 0; j < split.length && !found_mismatch; ++j) {
			found_mismatch = s.text[i + j] != split[j];
		}

		if (!found_mismatch) {
			*out_start = i;
			*out_end = i + split.length;
			return true;
		}
	}

	return false;
}

////////////// STYLE FORMATTING //////////////
// some_thing -> SomeThing
void to_pascal_case(String &dest, String &source) {
	dest.text[0] = source.text[0] + ('A' - 'a'); // Convert the first character to upper case.
	size_t target_index = 1;
	for (size_t i = 1; i < source.length; ++i) {
		if (source.text[i] == '_') {
			dest.text[target_index++] = source.text[++i] + ('A' - 'a');
		} else {
			dest.text[target_index++] = source.text[i];
		}
	}
	dest.length = target_index;
}

// some_thing -> Some Thing
void prettify(String &s) {
	s.text[0] += ('A' - 'a'); // Convert the first character to upper case.
	for (size_t i = 1; i < s.length; ++i) {
		if (s.text[i] == '_') {
			s.text[i] = (' ');
			s.text[++i] += ('A' - 'a');
		} else {
			s.text[i] = s.text[i];
		}
	}
}

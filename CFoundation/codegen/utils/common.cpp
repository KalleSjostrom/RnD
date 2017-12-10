#ifdef DEBUG
	#define HALT() (*(volatile int*)0 = 5)
#else
	#define HALT() (exit(-1))
#endif

#define sprintf_s sprintf
#define __forceinline inline
#define HANDLE int
#define FILETIME int
#define WIN32_FIND_DATA int
#define INVALID_HANDLE_VALUE 0
#define FindFirstFileEx(filepath, FindExInfoBasic, find_data, FindExSearchNameMatch, NULL, FIND_FIRST) 0
#define CreateDirectory(dirpath, x) false
#define GetLastError() false
#define ERROR_ALREADY_EXISTS false
#define FindClose(handle)
#include <string.h>


// d:\work\game\game\code\codegen\utils\memory_arena.cpp(32) : error C3861: 'ASSERT': identifier not found
		// char buf[1024];
		// sprintf_s(buf, ARRAY_COUNT(buf), (format), __VA_ARGS__);
		// printf("%s(%d) : error : %s\n", __FILE__, __LINE__, buf);
#define ASSERT(arg, format, ...) { \
	if (!(arg)) { \
		HALT(); \
	} \
}

#include <stdio.h>
#include <stdlib.h>

#define KB 1024
#define MB 1024 * KB
#define GB 1024 * MB

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))
#define ARRAY_CHECK_BOUNDS_STATIC(array) ASSERT(array.count < ARRAY_COUNT(array.entries), "Array index out of bounds!")
#define ARRAY_CHECK_BOUNDS(array) ASSERT(array.count < array.debug_max_count, "Array index out of bounds!")
#define ARRAY_CHECK_BOUNDS_COUNT(entries, count) ASSERT(count < ARRAY_COUNT(entries), "Array index out of bounds!")

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef short i16;

#include "memory_arena.cpp"
#include "parser.cpp"
#include "murmur_hash.cpp"

#if 0
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#include <windows.h>
#endif

#define MAKE_INPUT_FILE(variable_name, filename) FILE* variable_name = fopen(filename, "r"); ASSERT(variable_name, "Failed to open file for reading: '%s'", filename)
#define MAKE_OUTPUT_FILE(variable_name, filename) FILE* variable_name = fopen(filename, "w"); ASSERT(variable_name, "Failed to open file for writing: '%s'", filename)
#define MAKE_OUTPUT_FILE_WITH_HEADER(variable_name, filename) MAKE_OUTPUT_FILE(variable_name, filename); insert_header(variable_name)

//// ERROR HANDLING
parser::ParserContext *global_last_parser_context;
#define PARSER_ASSERT(arg, format, ...) { \
	if (!(arg)) { \
		char buf[1024]; \
		sprintf(buf, (format), __VA_ARGS__); \
		char filepath_buffer[1024]; \
		GetFullPathName(global_last_parser_context->filepath, 1024, filepath_buffer, 0); \
		int row, column; \
		parser::get_current_position(*global_last_parser_context, &row, &column); \
		printf("%s(%d) : error : %s\n\tAsserted at:\n", filepath_buffer, row, buf); \
		printf("%s(%d) : error : Assertion failed: %s\n", __FILE__, __LINE__, (#arg)); \
		HALT(); \
	} \
}

static char error_buffer[256];
inline char *printable_token(parser::Token &token) {
	if (token.type == TokenType_Number) {
		sprintf(error_buffer, "%.0f", token.number);
		return error_buffer;
	} else {
		null_terminate(token.string);
		return *token.string;
	}
}
inline char *printable_string(String &string) {
	null_terminate(string);
	return *string;
}
#define ASSERT_TOKEN(tok, token, expected_token) { \
	{ \
		bool equal = parser::is_equal(token, expected_token); \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(equal, "Unexpected token! (expected='%s', found='%s')", expected_token.string.text, printable_token(token)); \
	} \
}
#define ASSERT_NEXT_TOKEN(tok, expected_token) { \
	{ \
		parser::Token next_token = parser::next_token(&tok); \
		bool equal = parser::is_equal(next_token, expected_token); \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(equal, "Unexpected token! (expected='%s', found='%s')", expected_token.string.text, printable_token(next_token)); \
	} \
}
#define ASSERT_TOKEN_TYPE(token, expected_type) { \
	{ \
		char a[2]; \
		char b[2]; \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(token.type == expected_type, "Unexpected token! (expected='%s', found token='%s', found type='%s')", parser::type_tostring(expected_type, a), printable_token(token), parser::type_tostring(token.type, b)) \
	} \
}
#define ASSERT_NEXT_TOKEN_TYPE(tok, expected_type) { \
	{ \
		char a[2]; \
		char b[2]; \
		parser::Token next_token = parser::next_token(&tok); \
		global_last_parser_context->tokenizer = &tok; \
		PARSER_ASSERT(next_token.type == expected_type, "Unexpected token! (expected='%s', found token='%s', found type='%s')", parser::type_tostring(expected_type, a), printable_token(next_token), parser::type_tostring(next_token.type, b)) \
	} \
}


inline long get_filesize(FILE *file) {
	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);
	return length;
}

inline unsigned make_string_id(String &s) {
	return to_id32(s.length, *s);
}
inline uint64_t make_string_id64(String &s) {
	return to_id64(s.length, *s);
}

inline String trim_quotes(String &string) {
	String result = string;
	result.text++;
	result.length -= 2; // Cut away the '"'
	return result;
}

// some_thing -> SomeThing
void to_pascal_case(String &dest, String &source) {
	dest.text[0] = source.text[0] + ('A' - 'a'); // Convert the first character to upper case.
	int target_index = 1;
	for (int i = 1; i < source.length; ++i) {
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
	for (int i = 1; i < string.length; ++i) {
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
String make_underscore_case(String string, MemoryArena &arena) {
	char buffer[512];

	int length = 0;
	for (int i = 0; i < string.length; ++i) {
		if (string.text[i] >= 'A' && string.text[i] <= 'Z') {
			if (i > 0)
				buffer[length++] = '_';
			buffer[length++] = string.text[i] + ('a' - 'A'); // Convert the to lower case.
		} else {
			buffer[length++] = string.text[i];
		}
	}

	char *new_string_buffer = allocate_memory(arena, length + 1);
	String s = make_string(new_string_buffer, length);
	for (int i = 0; i < length; ++i) {
		s.text[i] = buffer[i];
	}
	null_terminate(s);
	return s;
}

static String COMPONENT_STRING = MAKE_STRING("Component");
String namespace_to_component_name(String namespace_string, MemoryArena &arena) {
	u32 name_length = namespace_string.length + COMPONENT_STRING.length;
	String component_name = allocate_string(arena, name_length + 1);
	to_pascal_case(component_name, namespace_string);
	append_string(component_name, COMPONENT_STRING);
	return component_name;
}

inline String make_filepath(MemoryArena &arena, String root_folder, String folder, String stem, String ending) {
	String filepath = allocate_string(arena,
		root_folder.length + 	// project folder
		1 + 					// path seperator
		folder.length + 		// folder inside project
		1 + 					// path seperator
		stem.length + 			// stem of the component name, i.e. motion, health
		ending.length + 		// file ending, e.g. _component.h
		1); 					// null termination

	append_string(filepath, root_folder);
	append_string(filepath, MAKE_STRING("/"));
	append_string(filepath, folder);
	append_string(filepath, MAKE_STRING("/"));
	append_string(filepath, stem);
	append_string(filepath, ending);
	null_terminate(filepath);

	return filepath;
}

inline String make_filename(MemoryArena &arena, String filepath) {
	int split_index = 0;
	for (int j = filepath.length-1; j > 0; j--) {
		if (filepath[j] == '/') {
			split_index = j + 1;
			break;
		}
	}
	ASSERT(split_index, "Couldn't find '/' in filepath!");
	int length = filepath.length - split_index;
	String filename = allocate_string(arena, length);
	filename.length = length;
	for (int j = 0; j < filename.length; ++j) {
		filename.text[j] = filepath[split_index + j];
	}
	null_terminate(filename);
	return filename;
}

inline String make_dirpath(MemoryArena &arena, String root_folder, String folder) {
	String filepath = allocate_string(arena,
		root_folder.length + 	// project folder
		1 + 					// path seperator
		folder.length + 		// folder inside project
		1); 					// null termination

	append_string(filepath, root_folder);
	append_string(filepath, MAKE_STRING("/"));
	append_string(filepath, folder);
	null_terminate(filepath);

	return filepath;
}

__forceinline HANDLE find_first_file(char *filepath, WIN32_FIND_DATA *find_data) {
	return FindFirstFileEx(filepath, FindExInfoBasic, find_data, FindExSearchNameMatch, NULL, FIND_FIRST_EX_CASE_SENSITIVE);
	// return FindFirstFile(filepath, find_data);
}

int file_exists(char *filepath) {
	WIN32_FIND_DATA data;
	HANDLE handle = find_first_file(filepath, &data) ;
	int found = handle != INVALID_HANDLE_VALUE;
	if(found)
		FindClose(handle);

	return found;
}

void make_sure_directory_exists(MemoryArena &arena, String root_folder, String folder) {
	String dirpath = make_dirpath(arena, root_folder, folder);
	bool success = CreateDirectory(*dirpath, NULL);
	if (!success) {
		// It's valid if the directory already exists, so assert only if there is some other error.
		bool already_exists = GetLastError() == ERROR_ALREADY_EXISTS;
		ASSERT(already_exists, "Error making directory! (dirpath=%s, error=%d)", *dirpath, GetLastError());
	}
}

void insert_header(FILE *file) {
	fprintf(file, "// Automatically generated file\n\n");
}

#define HASH_LOOKUP(entry, hashmap, _size, _key) \
	auto *entry = hashmap; \
	{ \
		unsigned __index = (_key) % (_size); \
		for (int __i = 0; __i < (_size); ++__i) { \
			entry = (hashmap) + __index; \
			if (entry->key == (_key) || entry->key == 0) { \
				break; \
			} \
			entry = 0; \
			__index++; \
			if (__index == (_size)) \
				__index = 0; \
		} \
		PARSER_ASSERT(entry, "Hash map is full!"); \
	}

#define HASH_LOOKUP_STATIC(entry, hashmap, _key) HASH_LOOKUP(entry, hashmap, ARRAY_COUNT(hashmap), _key)

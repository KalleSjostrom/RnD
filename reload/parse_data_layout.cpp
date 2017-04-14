#include <stdio.h>
#include <stdlib.h>

#include "../utils/parser.cpp"
#include "predefined_types.h"

struct Member {
	String name;
	String type;
	bool is_pointer;

	String optional_size;
	bool is_number;
};

struct Struct {
	String name;
	Member members[32];
	unsigned member_count;
};

inline FILE *open_file(char *filename) {
	FILE *file = fopen(filename, "r");
	printf("Parsing file: %s\n", filename);
	if (!file) {
		char buffer[256];
		sprintf(buffer, "No such file was found (path=%s)!", filename);
		ASSERT_MSG(false, buffer);
	}
	return file;
}

String build_path(String folder, String filename, MemoryArena &arena) {
	char *path = allocate_memory(arena, folder.length + filename.length + 1);
	int index = 0;
	for (int i = 0; i < folder.length; i++) {
		path[index++] = folder[i];
	}
	path[index++] = '/';
	for (int i = 0; i < filename.length; i++) {
		path[index++] = filename[i];
	}
	return make_string(path, index);
}

String coalesce_strings(String *strings, int count, MemoryArena &arena) {
	int total_length = 0;
	for (int i = 0; i < count; i++) {
		total_length += strings[i].length;
	}
	total_length++; // Include null terminator!
	String new_string = make_string(allocate_memory(arena, total_length), total_length-1);

	int index = 0;
	for (int i = 0; i < count; i++) {
		String &string = strings[i];
		for (int j = 0; j < string.length; j++) {
			new_string.text[index++] = string[j];
		}
	}
	new_string.text[index++] = '\0';
	ASSERT(index == total_length);

	return new_string;
}

void parse_file(String folder, String filename, FILE *output, MemoryArena &arena, Struct *structs, unsigned int *struct_count) {
	FILE *file;
	if (folder.length > 0) {
		file = open_file(*build_path(folder, filename, arena));
	} else {
		file = open_file(*filename);
	}

	size_t filesize = get_filesize(file);
	char *source = allocate_memory(arena, filesize+1);
	fread(source, 1, filesize, file);
	source[filesize] = '\0';

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize);
	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case '#': {
				token = parser::next_token(&tok);
				if (parser::is_equal(token, TOKENIZE("include"))) {
					parser::Token path_token = parser::next_token(&tok);
					if (path_token.type == TokenType_String) {
						String new_folder = {0};
						String new_filename = path_token.string;
						for (int i = new_filename.length-1; i >= 0; i--) {
							if (new_filename[i] == '/') {
								new_folder.text = new_filename.text;
								new_folder.length = i;
								new_filename.text = new_folder.text + i + 1;
								new_filename.length = new_filename.length - (i+1);
								break;
							}
						}

						if (new_folder.length > 0) {
							if (folder.length > 0) {
								String f = build_path(folder, new_folder, arena);
								parse_file(f, new_filename, output, arena, structs, struct_count);
							} else {
								parse_file(new_folder, new_filename, output, arena, structs, struct_count);
							}
						} else {
							parse_file(folder, new_filename, output, arena, structs, struct_count);
						}
					}
				}
			} break;
			case TokenType_CommandMarker: {
				token = parser::next_token(&tok);
				if (parser::is_equal(token, TOKENIZE("reloadable_struct"))) {
					token = parser::next_token(&tok);
					ASSERT(parser::is_equal(token, TOKENIZE("struct")));

					parser::Token name_token = parser::next_token(&tok);
					ASSERT(parser::is_equal(parser::next_token(&tok), TOKENIZE("{")));

					Struct s = { 0 };
					s.name = name_token.string;
					s.member_count = 0;

					token = parser::next_token(&tok);

					bool found_struct_end = false;
					while (!found_struct_end) {
						ASSERT(token.type == TokenType_Identifier);

						String member_strings[8];
						int member_string_counter = 0;
						member_strings[member_string_counter++] = token.string;

						parser::Token thing = parser::next_token(&tok);
						while (thing.type == TokenType_Namespace) {
							//thing.text[0] = '_';
							//thing.length = 1;
							member_strings[member_string_counter++] = thing.string;
							ASSERT(member_string_counter < 8);

							token = parser::next_token(&tok);
							member_strings[member_string_counter++] = token.string;
							ASSERT(member_string_counter <= 8);

							thing = parser::next_token(&tok);
						}
						bool is_pointer = false;
						if (thing.type == '*') {
							is_pointer = true;

	                        thing = parser::next_token(&tok);
						}

	                    parser::Token member_name = thing;
						ASSERT(member_name.type != '*');

						String member_type = coalesce_strings(member_strings, member_string_counter, arena);

						Member m = { 0 };
						m.name = member_name.string;
						m.type = member_type;
						m.is_pointer = is_pointer;

						token = parser::next_token(&tok);
						ASSERT(token.type == ';');

						token = parser::next_token(&tok);
						if (token.type == TokenType_CommandMarker) {
							token = parser::next_token(&tok);
							if (parser::is_equal(token, TOKENIZE("size"))) {
								token = parser::next_token(&tok);
								m.optional_size = token.string;
								m.is_number = token.type == TokenType_Number;

	                            token = parser::next_token(&tok);
							}
						}

	                    if (token.type == '}') {
							found_struct_end = true;
						}

						s.members[s.member_count++] = m;
					}

					structs[(*struct_count)++] = s;
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}
	fclose(file);
}

int main(int argc, char *argv[]) {
	int bytes = 32*MB;
	MemoryArena arena = init_memory(bytes);

	char *filename = argv[1];
	char *output_filename = argv[2];

	FILE *output = fopen(output_filename, "w");
	ASSERT_MSG(output, "No such file was found!");

	Struct structs[1024] = {0};
	unsigned struct_count = 0;

	// Find the index of the last /. Everything to the left of it is the folder, and the rest is the filename
	int cursor = 0;
	int last_path_sep = 0;
	while (filename[cursor] != '\0') {
		if (filename[cursor] == '/') {
			last_path_sep = cursor;
		}
		cursor++;
	}

	char *folder = 0;
	if (last_path_sep) { // Split the filename into two at the index of the last path seperator.
		folder = filename;
		folder[last_path_sep] = '\0';
		filename = folder + last_path_sep + 1;
	}
	parse_file(make_string(folder), make_string(filename), output, arena, structs, &struct_count);

	// Loop over all found structs and make sure the strings are null terminated.
	// Now that the parsing is over, we won't risk overwriting the read cursor.
	for (int i = 0; i < struct_count; ++i) {
		Struct s = structs[i];
		null_terminate(s.name);
		printf("struct=[%s] member_count=[%d]\n", *s.name, s.member_count);
		for (int j = 0; j < s.member_count; ++j) {
			Member m = s.members[j];
			null_terminate(m.name);
			null_terminate(m.type);
			if (m.optional_size.length > 0)
				null_terminate(m.optional_size);

			printf("\ttype=[%s] name=[%s] is_pointer=[%d] optional_size=[%s] is_number=[%d]\n", *m.type, *m.name, m.is_pointer, *m.optional_size, m.is_number);
		}
	}
	const char *entry_point = "AppMemory";

fprintf(output, "#include \"../generated/type_info.generated.cpp\"\n");
fprintf(output, "#include \"../reload/reloader.cpp\"\n");
fprintf(output, "\n");

fprintf(output, "#define __RELOAD_TYPE__generic 0\n");
fprintf(output, "#define __RELOAD_SIZE__generic %lu\n", sizeof(char));

for (int i = 0; i < predefined_type_count; ++i) {
	const char *name = predefined_types[i].name;
fprintf(output, "#define __RELOAD_TYPE__%s %d\n", name, i+1);
fprintf(output, "#define __RELOAD_SIZE__%s %d\n", name, predefined_types[i].size);
}

for (int i = 0; i < struct_count; ++i) {
	Struct s = structs[i];
fprintf(output, "#ifndef __RELOAD_TYPE__%s\n", *s.name);
fprintf(output, "	#define __RELOAD_TYPE__%s %d\n", *s.name, 1000000 + i);
fprintf(output, "	#define __RELOAD_SIZE__%s 0\n", *s.name);
fprintf(output, "#endif\n");
	for (int j = 0; j < s.member_count; ++j) {
		Member m = s.members[j];
fprintf(output, "#ifndef __RELOAD_OFFSET__%s_%s\n", *s.name, *m.name);
fprintf(output, "	#define __RELOAD_OFFSET__%s_%s 0\n", *s.name, *m.name);
fprintf(output, "#endif\n");
	}
}
fprintf(output, "\n");

fprintf(output, "void generate_layout() {\n");
fprintf(output, "	FILE *output = fopen(\"./generated/type_info.generated.cpp\", \"w\");\n");
fprintf(output, "\n");

for (int i = 0; i < struct_count; ++i) {
	Struct s = structs[i];
fprintf(output, "	fprintf(output, \"#define __RELOAD_TYPE__%s %d\\n\");\n", *s.name, CUSTOM_TYPE_BASE + i);
fprintf(output, "	fprintf(output, \"#define __RELOAD_SIZE__%s %%lu\\n\", sizeof(%s));\n", *s.name, *s.name);
	for (int j = 0; j < s.member_count; ++j) {
		Member m = s.members[j];
fprintf(output, "	fprintf(output, \"#define __RELOAD_OFFSET__%s_%s %%lu\\n\", offsetof(%s, %s));\n", *s.name, *m.name, *s.name, *m.name);
	}
}
fprintf(output, "\n");
fprintf(output, "	fclose(output);\n");
fprintf(output, "}\n");

for (int i = 0; i < struct_count; ++i) {
	Struct s = structs[i];
fprintf(output, "void set_%s(MemoryMapper *mapper, intptr_t base_old, intptr_t base_new) {\n", *s.name);
fprintf(output, "	unsigned size;\n");
	for (int j = 0; j < s.member_count; ++j) {
		Member m = s.members[j];

		bool known_reloadable_struct = false;
		for (int k = 0; k < struct_count; ++k) {
			if (string_is_equal(m.type, structs[k].name)) {
				known_reloadable_struct = true;
				break;
			}
		}

		if (m.is_pointer) {
			if (m.optional_size.length > 0) {
				if (m.is_number) {
fprintf(output, "	size = %s;\n", *m.optional_size);
				} else {
fprintf(output, "	size = *(unsigned*)((base_old + __RELOAD_OFFSET__%s_%s));\n", *s.name, *m.optional_size);
				}
			} else {
fprintf(output, "	size = sizeof(%s);\n", *m.type);
			}

			if (known_reloadable_struct) {
fprintf(output, "	set_pointer(mapper, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), __RELOAD_TYPE__%s, size);\n", *s.name, *m.name, *s.name, *m.name, *m.type);
			} else {
fprintf(output, "	set_pointer(mapper, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), __RELOAD_TYPE__generic, size);\n", *s.name, *m.name, *s.name, *m.name);
			}
		} else {
			if (known_reloadable_struct) {
fprintf(output, "	set_%s(mapper, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s));\n", *m.type, *s.name, *m.name, *s.name, *m.name);
			} else {
fprintf(output, "	set_generic(mapper, base_old + __RELOAD_OFFSET__%s_%s, base_new + offsetof(%s, %s), sizeof(%s));\n", *s.name, *m.name, *s.name, *m.name, *m.type);
			}
		}
	}
fprintf(output, "}\n\n");
}

fprintf(output, "void reload(char *old_memory, char *new_memory) {\n");
fprintf(output, "	AddressPair address_lookup[ADDRESS_LOOKUP_SIZE] = {0};\n");
fprintf(output, "	MemorySlot memory_slots[1024] = {0};\n");
fprintf(output, "\n");
fprintf(output, "	MemoryMapper mapper = {0};\n");
fprintf(output, "	mapper.address_lookup = address_lookup;\n");
fprintf(output, "	mapper.address_lookup_size = ADDRESS_LOOKUP_SIZE;\n");
fprintf(output, "	mapper.memory_slots = memory_slots;\n");
fprintf(output, "	mapper.memory_slot_counter = 0;\n");
fprintf(output, "	{\n");
fprintf(output, "		intptr_t base_new = (intptr_t) new_memory;\n");
fprintf(output, "		intptr_t base_old = (intptr_t) old_memory;\n");
fprintf(output, "\n");
fprintf(output, "		MemorySlot entry_point = {0};\n");
fprintf(output, "		entry_point.type = __RELOAD_TYPE__%s;\n", entry_point);
fprintf(output, "		entry_point.target_addr_old = base_old;\n");
fprintf(output, "		memory_slots[0] = entry_point;\n");
fprintf(output, "		mapper.memory_slot_counter = 1;\n");
fprintf(output, "		MemorySlot *top = memory_slots;\n");
fprintf(output, "\n");
fprintf(output, "		while (top) {\n");
fprintf(output, "			intptr_t distance = top->target_addr_old - base_old;\n");
fprintf(output, "			base_new += distance;\n");
fprintf(output, "			base_old += distance;\n");
fprintf(output, "\n");
fprintf(output, "			unsigned size = top->size > 0 ? top->size : 1;\n");
fprintf(output, "\n");
fprintf(output, "			switch (top->type) {\n");
for (int i = 0; i < struct_count; ++i) {
	Struct s = structs[i];
fprintf(output, "				case __RELOAD_TYPE__%s: {\n", *s.name);
fprintf(output, "					for (int i = 0; i < size; ++i) {\n");
fprintf(output, "						set_%s(&mapper, base_old, base_new);\n", *s.name);
fprintf(output, "						base_new += sizeof(%s);\n", *s.name);
fprintf(output, "						base_old += __RELOAD_SIZE__%s;\n", *s.name);
fprintf(output, "					}\n");
fprintf(output, "				} break;\n");
}
fprintf(output, "				case __RELOAD_TYPE__generic: {\n");
fprintf(output, "					set_generic(&mapper, top->target_addr_old, base_new, top->size);\n");
fprintf(output, "					base_new += top->size;\n");
fprintf(output, "					base_old += top->size;\n");
fprintf(output, "				} break;\n");
fprintf(output, "				default: {\n");
fprintf(output, "					ASSERT_MSG(false, \"Unknown reload type!\")\n");
fprintf(output, "				}\n");
fprintf(output, "			}\n");
fprintf(output, "\n");
fprintf(output, "			top = 0;\n");
fprintf(output, "			for (int i = mapper.memory_slot_counter-1; i >= 0; i--) {\n");
fprintf(output, "				MemorySlot *entry = memory_slots + i;\n");
fprintf(output, "				if (entry->target == 0) {\n");
fprintf(output, "					memory_slots[i] = memory_slots[mapper.memory_slot_counter-1];\n");
fprintf(output, "					mapper.memory_slot_counter--;\n");
fprintf(output, "				} else if (entry->target->addr_old != 0) {\n");
fprintf(output, "					*(intptr_t*)(entry->addr_new) = entry->target->addr_new;\n");
fprintf(output, "					memory_slots[i] = memory_slots[mapper.memory_slot_counter-1];\n");
fprintf(output, "					mapper.memory_slot_counter--;\n");
fprintf(output, "				} else {\n");
fprintf(output, "					if (top) {\n");
fprintf(output, "						top = top->target_addr_old < entry->target_addr_old ? top : entry;\n");
fprintf(output, "					} else {\n");
fprintf(output, "						top = entry;\n");
fprintf(output, "					}\n");
fprintf(output, "				}\n");
fprintf(output, "			}\n");
fprintf(output, "		}\n");
fprintf(output, "	}\n");
fprintf(output, "}\n");

	fclose(output);
}

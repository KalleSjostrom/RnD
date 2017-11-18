#include "engine/common.h"
#include "engine/utils/math/math.h"
#include "engine/utils/parser.cpp"
#include "engine/utils/file_utils.h"
#include "engine/utils/containers/dynamic_array.h"

/*
./external\ tools/obj_compiler/obj_compiler conetracer/assets/buddha.obj conetracer/out/data/buddha.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/bunny.obj conetracer/out/data/bunny.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/cornell.obj conetracer/out/data/cornell.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/cube.obj conetracer/out/data/cube.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/dragon.obj conetracer/out/data/dragon.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/quad.obj conetracer/out/data/quad.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/quadn.obj conetracer/out/data/quadn.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/sphere.obj conetracer/out/data/sphere.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/susanne.obj conetracer/out/data/susanne.cobj
./external\ tools/obj_compiler/obj_compiler conetracer/assets/teapot.obj conetracer/out/data/teapot.cobj
*/

parser::Tokenizer make_tokenizer(char *text) {
	parser::Tokenizer tok = { };
	tok.at = text;
	return tok;
}

inline float next_number(parser::Tokenizer &tok) {
	parser::Token token = parser::next_token(&tok);
	ASSERT_TOKEN_TYPE(token, TokenType_Number);
	return token.number;
}

#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define b32(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(b32(x-1) + 1)

typedef unsigned GLindex;

struct HashEntry {
	u64 key;
	GLindex value;
};

void compile_obj(const char *input_filepath, const char *output_filepath) {
	FILE *file = fopen(input_filepath, "r");
	ASSERT(file, "Could not open '%s'", input_filepath);

	size_t filesize = get_filesize(file);

	MemoryArena arena = {};

	char *source = (char *)PUSH_SIZE(arena, filesize);
	fread(source, 1, filesize, file);
	fclose(file);

	v3 *vertices = 0;
	v2 *coords = 0;
	v3 *normals = 0;

	GLindex *vertex_indices = 0;
	GLindex *coord_indices = 0;
	GLindex *normal_indices = 0;

	array_init(vertices, 1024*32);
	array_init(coords, 1024*32);
	array_init(normals, 1024*32);

	array_init(vertex_indices, 1024*32);
	array_init(coord_indices, 1024*32);
	array_init(normal_indices, 1024*32);

	v3 *output_vertices = 0;
	v2 *output_coords = 0;
	v3 *output_normals = 0;
	GLindex *output_indices = 0;

	array_init(output_vertices, 1024*32);
	array_init(output_coords, 1024*32);
	array_init(output_normals, 1024*32);
	array_init(output_indices, 1024*32);

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize, TokenizerFlags_KeepLineComments);
	parser::ParserContext pcontext;
	parser::set_parser_context(pcontext, &tok, (char*)input_filepath);

	HashEntry *vertex_cache = 0;
	unsigned hashmap_count = 0;
	unsigned hashmap_mask = 0;
	u64 invalid_key = -1u;
	GLindex element_index = 0;

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				// v - vertices
				if (parser::is_equal(token, TOKENIZE("v"))) {
					array_push(vertices, V3(next_number(tok), next_number(tok), next_number(tok)));
				}
				// vt - texture coordinates
				else if (parser::is_equal(token, TOKENIZE("vt"))) {
					array_push(coords, V2_f32(next_number(tok), next_number(tok)));
				}
				// vn - vertex normals
				else if (parser::is_equal(token, TOKENIZE("vn"))) {
					array_push(normals, V3(next_number(tok), next_number(tok), next_number(tok)));
				}
				// f - faces
				else if (parser::is_equal(token, TOKENIZE("f"))) {
					int vertex_count = (int)array_count(vertices);
					int coord_count = (int)array_count(coords);
					int normal_count = (int)array_count(normals);

					if (vertex_cache == 0) {
						int sum = vertex_count + coord_count + normal_count;
						hashmap_count = (unsigned)next_power_of_2(sum * 2);
						hashmap_mask = hashmap_count-1;
						vertex_cache = PUSH_STRUCTS(arena, hashmap_count, HashEntry);
						for (unsigned i = 0; i < hashmap_count; ++i) {
							vertex_cache[i].key = invalid_key;
						}
					}

					bool vertex_done = false;
					for (int vi = 0; vi < 3; ++vi) {
						token = parser::next_token(&tok);

						int vertex_index = 0;
						int coord_index = 0;
						int normal_index = 0;

						for (int j = 0; j < 3; ++j) {
							ASSERT_TOKEN_TYPE(token, TokenType_Number);

							switch (j) {
								case 0: { vertex_index = (int)token.number; } break;
								case 1: { coord_index = (int)token.number; } break;
								case 2: { normal_index = (int)token.number; } break;
							}

							token = parser::peek_next_token(&tok);
							if (token.type == TokenType_Number) {
								break;
							} else if (token.type == '/') {
								token = parser::next_token(&tok); // Consume all /
								token = parser::next_token(&tok);
								while (token.type == '/') {
									token = parser::next_token(&tok);
									j++;
								}
								continue;
							} else {
								// PARSER_ASSERT(false, "A face needs exactly 3 vertices!");
								break;
							}
						}

						u64 vi64 = 0;
						u64 ci64 = 0;
						u64 ni64 = 0;

						if (vertex_index != 0) {
							vi64 = (u64)(vertex_index < 0 ? vertex_count + vertex_index : vertex_index - 1);
						}
						if (coord_index != 0) {
							ci64 = (u64)(coord_index < 0 ? coord_count + coord_index : coord_index - 1);
						}
						if (normal_index != 0) {
							ni64 = (u64)(normal_index < 0 ? normal_count + normal_index : normal_index - 1);
						}

						static u64 channel_max = 1<<20;
						u64 key = vi64 + ci64 * channel_max + ni64 * channel_max * 2;

						bool found = false;
						for (u64 offset = 0; offset < hashmap_count; offset++) {
							u64 index = (key + offset) & hashmap_mask;

							HashEntry *entry = vertex_cache + index;
							if (entry->key == invalid_key) {
								// New entry!
								entry->key = key;

								if (vertex_index != 0) {
									array_push(output_vertices, vertices[vi64]);
								}
								if (coord_index != 0) {
									array_push(output_coords, coords[ci64]);
								}
								if (normal_index != 0) {
									array_push(output_normals, normals[ni64]);
								}

								entry->value = element_index;
								array_push(output_indices, element_index++);
								found = true;
								break;
							} else if (entry->key == key) {
								array_push(output_indices, entry->value);
								found = true;
								break;
							}
						}
						ASSERT(found, "Hash full!");
					}
				}
			} break;
			case '#': {
				parser::consume_line(&tok);
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}

	FILE *output = fopen(output_filepath, "wb");
	ASSERT(file, "Could not open '%s'", output_filepath);

	size_t vertex_count = (size_t)array_count(output_vertices);
	size_t coord_count = (size_t)array_count(output_coords);
	size_t normal_count = (size_t)array_count(output_normals);
	size_t index_count = (size_t)array_count(output_indices);

	fwrite(&vertex_count, sizeof(size_t), 1, output);
	fwrite(&coord_count, sizeof(size_t), 1, output);
	fwrite(&normal_count, sizeof(size_t), 1, output);
	fwrite(&index_count, sizeof(size_t), 1, output);

	fwrite(output_vertices, sizeof(v3), vertex_count, output);
	fwrite(output_coords, sizeof(v2), coord_count, output);
	fwrite(output_normals, sizeof(v3), normal_count, output);
	fwrite(output_indices, sizeof(GLindex), index_count, output);

	fclose(output);
}

int main(int argc, char const *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Needs 2 argument! Got %d\nUsage: ./obj_compiler input_filepath output_filepath\n", argc - 1);
		return -1;
	}

	compile_obj(argv[1], argv[2]);
}

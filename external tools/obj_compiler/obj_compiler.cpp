#include "engine/utils/platform.h"
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
./external\ tools/obj_compiler/obj_compiler conetracer/assets/sponza.obj conetracer/out/data/sponza.cobj
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

struct v3i {
	int vertex_index;
	int coord_index;
	int normal_index;
};

typedef unsigned GLindex;

struct HashEntry {
	u64 hash;
	u32 vi;
	u32 ci;
	u32 ni;
	GLindex value;
};

struct Group {
	v3i *face_vertices;
	GLindex *indices;
};

void compile_obj(const char *input_filepath, const char *output_filepath) {
	MemoryArena arena = {};

	size_t filesize;
	FILE *file = open_file(input_filepath, &filesize);

	char *source = (char *)PUSH_SIZE(arena, filesize);
	fread(source, filesize, 1, file);
	source[filesize] = '\0';
	fclose(file);

	v3 *vertices = 0;
	v2 *coords = 0;
	v3 *normals = 0;

	array_init(vertices, 1024*32);
	array_init(coords, 1024*32);
	array_init(normals, 1024*32);

	GLindex *vertex_indices = 0;
	GLindex *coord_indices = 0;
	GLindex *normal_indices = 0;

	array_init(vertex_indices, 1024*32);
	array_init(coord_indices, 1024*32);
	array_init(normal_indices, 1024*32);

	Group *groups = 0;
	array_init(groups, 32);

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize, TokenizerFlags_KeepLineComments);
	parser::ParserContext pcontext;
	parser::set_parser_context(pcontext, &tok, (char*)input_filepath);

	parser::Token t_v = TOKENIZE("v");
	parser::Token t_vt = TOKENIZE("vt");
	parser::Token t_vn = TOKENIZE("vn");
	parser::Token t_f = TOKENIZE("f");
	parser::Token t_g = TOKENIZE("g");
	parser::Token t_s = TOKENIZE("s");
	parser::Token t_usemtl = TOKENIZE("usemtl");
	parser::Token t_mtllib = TOKENIZE("mtllib");

	bool parsing = true;
	int mode = 0;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				// v - vertices
				if (parser::is_equal(token, t_v)) {
					mode = 1;
					array_push(vertices, V3(next_number(tok), next_number(tok), next_number(tok)));
				}
				// vt - texture coordinates
				else if (parser::is_equal(token, t_vt)) {
					mode = 2;
					array_push(coords, V2_f32(next_number(tok), next_number(tok)));

					// OPTIMIZE(kalle): Reuse the peeked token
					token = parser::peek_next_token(&tok);
					if (token.type == TokenType_Number) {
						next_number(tok);
					}
				}
				// vn - vertex normals
				else if (parser::is_equal(token, t_vn)) {
					mode = 3;
					array_push(normals, V3(next_number(tok), next_number(tok), next_number(tok)));
				}
				// f - faces
				else if (parser::is_equal(token, t_f)) {
					if (mode != 4) {
						array_new_entry(groups);
						Group &group = array_last(groups);
						group.face_vertices = 0;
						array_init(group.face_vertices, 1024 * 32);
					}
					mode = 4;
					int face_done = 0;
					int vi = 0;
					while (!face_done && tok.at[0] != '\0') {
						token = parser::peek_next_token(&tok);
						if (token.type == TokenType_Number) {
							token = parser::next_token(&tok);
						} else {
							face_done = true;
							break;
						}

						int vertex_index = 0;
						int coord_index = 0;
						int normal_index = 0;

						Group &group = array_last(groups);
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

						vi++;
						if (vi > 3) {
							int current_count = array_count(group.face_vertices);
							array_push(group.face_vertices, group.face_vertices[current_count - vi + 1]); // v0
							array_push(group.face_vertices, group.face_vertices[current_count - 1]); // v(n-1)
						}

						v3i face_vertex = { vertex_index, coord_index, normal_index };
						array_push(group.face_vertices, face_vertex); // v0
					}
				}
				// g - group
				else if (parser::is_equal(token, t_g)) {
				#if 0
					mode = 4;
					token = parser::next_token(&tok);

					array_new_entry(groups);
					Group &group = array_last(groups);
					group.face_vertices = 0;
					array_init(group.face_vertices, 1024*32);
				#endif
				}
				// s - smoothing groups - "Smooth shading across polygons is enabled by smoothing groups."
				else if (parser::is_equal(token, t_s)) {
					token = parser::next_token(&tok);
					if (token.type == TokenType_Number) {
						printf("Smoothing group: %g\n", token.number);
					} else {
						printf("Smoothing group: %.*s\n", token.length, *token.string);
					}
				}
				// usemtl - use material
				else if (parser::is_equal(token, t_usemtl)) {
					token = parser::next_line(&tok);
					printf("Using material: %.*s\n", token.length, *token.string);
				}
				// mtllib
				else if (parser::is_equal(token, t_mtllib)) {
					token = parser::next_token(&tok);
					printf("Mtl lib: %.*s\n", token.length, *token.string);
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

	int vertex_count = (int)array_count(vertices);
	int coord_count = (int)array_count(coords);
	int normal_count = (int)array_count(normals);
	int sum = vertex_count + coord_count + normal_count;

	v3 *out_vertices = 0;
	v2 *out_coords = 0;
	v3 *out_normals = 0;

	array_init(out_vertices, vertex_count);
	array_init(out_coords, coord_count);
	array_init(out_normals, normal_count);

	HashEntry *vertex_cache = 0;
	u64 hashmap_count = (u64)next_power_of_2(sum * 4);
	u64 hashmap_mask = hashmap_count-1;
	u64 invalid_hash = ~0u;
	GLindex element_index = 0;

	hashmap_mask = hashmap_count - 1;
	vertex_cache = PUSH_STRUCTS(arena, hashmap_count, HashEntry, true);
	for (unsigned i = 0; i < hashmap_count; ++i) {
		vertex_cache[i].hash = invalid_hash;
	}

	for (int i = 0; i < array_count(groups); ++i) {
		Group &group = groups[i];

		for (int j = 0; j < array_count(group.face_vertices); ++j) {
			v3i &fv = group.face_vertices[j];

			u32 vi = 0;
			u32 ci = 0;
			u32 ni = 0;

			if (fv.vertex_index != 0) {
				vi = (u32)(fv.vertex_index < 0 ? vertex_count + fv.vertex_index : fv.vertex_index - 1);
			}
			if (fv.coord_index != 0) {
				ci = (u32)(fv.coord_index < 0 ? coord_count + fv.coord_index : fv.coord_index - 1);
			}
			if (fv.normal_index != 0) {
				ni = (u32)(fv.normal_index < 0 ? normal_count + fv.normal_index : fv.normal_index - 1);
			}

			static int xprime = 492876863; /* arbitrary large prime */
			static int yprime = 633910099; /* arbitrary large prime */
			static int zprime = 805306457; /* arbitrary large prime */
			u64 hash = ((vi * xprime) ^ (ci * yprime) ^ (ni * zprime)) & hashmap_mask;

			bool found = false;
			for (u64 offset = 0; offset < hashmap_count; offset++) {
				u64 index = (hash + offset) & hashmap_mask;

				HashEntry *entry = vertex_cache + index;
				if (entry->hash == invalid_hash) {
					// New entry!
					entry->hash = hash;
					entry->vi = vi; entry->ci = ci; entry->ni = ni;

					if (fv.vertex_index != 0) {
						array_push(out_vertices, vertices[vi]);
					}
					if (fv.coord_index != 0) {
						array_push(out_coords, coords[ci]);
					}
					if (fv.normal_index != 0) {
						array_push(out_normals, normals[ni]);
					}

					entry->value = element_index;
					array_push(group.indices, element_index++);
					found = true;
					break;
				} else if (entry->hash == hash) {
					if (entry->vi == vi && entry->ci == ci && entry->ni == ni) {
						array_push(group.indices, entry->value);
						found = true;
						break;
					}
				}
			}
			ASSERT(found, "Hash full!");
		}
	}

	FILE *output;
	fopen_s(&output, output_filepath, "wb");
	ASSERT(file, "Could not open '%s'", output_filepath);

	int out_vertex_count = (int)array_count(out_vertices);
	int out_coord_count = (int)array_count(out_coords);
	int out_normal_count = (int)array_count(out_normals);
	int group_count = (int)array_count(groups);

	fwrite(&out_vertex_count, sizeof(int), 1, output);
	fwrite(&out_coord_count, sizeof(int), 1, output);
	fwrite(&out_normal_count, sizeof(int), 1, output);
	fwrite(&group_count, sizeof(int), 1, output);

	fwrite(out_vertices, sizeof(v3), (size_t)out_vertex_count, output);
	fwrite(out_coords, sizeof(v2), (size_t)out_coord_count, output);
	fwrite(out_normals, sizeof(v3), (size_t)out_normal_count, output);

	for (int i = 0; i < group_count; ++i) {
		Group &group = groups[i];
		int index_count = (int)array_count(group.indices);
		fwrite(&index_count, sizeof(int), 1, output);
		fwrite(group.indices, sizeof(GLindex), (size_t)index_count, output);
	}

	fclose(output);
}

int main(int argc, char const *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Needs 2 argument! Got %d\nUsage: ./obj_compiler input_filepath output_filepath\n", argc - 1);
		return -1;
	}

	compile_obj(argv[1], argv[2]);
}

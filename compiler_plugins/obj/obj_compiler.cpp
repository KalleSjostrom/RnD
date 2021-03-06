#include "engine/utils/platform.h"
#include "engine/common.h"
#include "engine/utils/math/math.h"
#include "engine/utils/parser.cpp"
#include "engine/utils/file_utils.h"
#include "engine/utils/containers/dynamic_array.h"
#include "engine/utils/logger.h"

inline float next_number(parser::Tokenizer &tok) {
	parser::Token token = parser::next_token(&tok);
	ASSERT_TOKEN_TYPE(token, TokenType_Number);
	return token.number;
}

#include "parse_mtllib.cpp"

#define b2(x)   (   (x) | (   (x) >> 1))
#define b4(x)   ( b2(x) | ( b2(x) >> 2))
#define b8(x)   ( b4(x) | ( b4(x) >> 4))
#define b16(x)  ( b8(x) | ( b8(x) >> 8))
#define bool(x)  (b16(x) | (b16(x) >>16))
#define next_power_of_2(x)(bool(x-1) + 1)

struct v3i {
	int vertex_index;
	int coord_index;
	int normal_index;
};

typedef unsigned GLindex;

struct VertexHashEntry {
	u64 hash;
	u32 vi;
	u32 ci;
	u32 ni;
	GLindex value;
};

struct Group {
	v3i *face_vertices;
	GLindex *indices;
	i32 material_index;
};

i32 pick_material(Material *materials, u32 name_id) {
	for (i32 i = 0; i < array_count(materials); ++i) {
		if (materials[i].name_id == name_id)
			return i;
	}
	return -1;
}

void write_string(String string, FILE *file) {
	fwrite(&string.length, sizeof(i32), 1, file);
	fwrite(string.text, sizeof(char), (size_t)string.length, file);
}

void compile_obj(String input_directory, String input_filename, String output_directory) {
	ArenaAllocator arena = {};

    size_t filesize;
    char buffer[1024];
    snprintf(buffer, ARRAY_COUNT(buffer), "%.*s/%s", input_directory.length, *input_directory, *input_filename);
    FILE *file = open_file(buffer, &filesize);

	char output_filename[MAX_PATH];
	snprintf(output_filename, MAX_PATH, "%.*s.cobj", input_filename.length-4, *input_filename);

	char *source = (char *)PUSH_SIZE(arena, filesize + 1);
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
	parser::set_parser_context(pcontext, &tok, (char*)*buffer);

	parser::Token t_v = TOKENIZE("v");
	parser::Token t_vt = TOKENIZE("vt");
	parser::Token t_vn = TOKENIZE("vn");
	parser::Token t_f = TOKENIZE("f");
	parser::Token t_g = TOKENIZE("g");
	parser::Token t_s = TOKENIZE("s");
	parser::Token t_usemtl = TOKENIZE("usemtl");
	parser::Token t_mtllib = TOKENIZE("mtllib");

	Material *materials = 0;
	i32 current_material_index = -1;

	bool parsing = true;
	int mode = 0;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				// v - vertices
				if (parser::is_equal(token, t_v)) {
					mode = 1;
					float x = next_number(tok);
					float y = next_number(tok);
					float z = next_number(tok);
					array_push(vertices, V3(x, y, z));
				}
				// vt - texture coordinates
				else if (parser::is_equal(token, t_vt)) {
					mode = 2;
					float u = next_number(tok);
					float v = next_number(tok);
					float w = 0.0f;

					// OPTIMIZE(kalle): Reuse the peeked token
					token = parser::peek_next_token(&tok);
					if (token.type == TokenType_Number) {
						w = next_number(tok);
					}

					// TODO(kalle): Why is the vertical flipped??
					array_push(coords, V2_f32(u, -v));
				}
				// vn - vertex normals
				else if (parser::is_equal(token, t_vn)) {
					mode = 3;
					float x = next_number(tok);
					float y = next_number(tok);
					float z = next_number(tok);
					array_push(normals, V3(x, y, z));
				}
				// f - faces
				else if (parser::is_equal(token, t_f)) {
					if (mode != 4) {
						array_new_entry(groups);
						Group &group = array_last(groups);
						group.face_vertices = 0;
						group.material_index = current_material_index;
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
					// if (token.type == TokenType_Number) {
					// 	printf("Smoothing group: %g\n", token.number);
					// } else {
					// 	printf("Smoothing group: %.*s\n", token.length, *token.string);
					// }
				}
				// usemtl - use material
				else if (parser::is_equal(token, t_usemtl)) {
					token = parser::next_line(&tok);
					if (materials) {
						u32 name_id = make_string_id32(token.string);
						current_material_index = pick_material(materials, name_id);
						if (current_material_index == -1) {
							LOG_WARNING("Compiler", "No material named '%.*s' loaded!\n", token.string.length, *token.string);
						}
					} else {
						LOG_WARNING("Compiler", "Trying to load '%.*s' when there are no materials loaded!\n", token.string.length, *token.string);
					}
				}
				// mtllib
				else if (parser::is_equal(token, t_mtllib)) {
					token = parser::next_line(&tok);
					snprintf(buffer, ARRAY_COUNT(buffer), "%.*s/%.*s", input_directory.length, *input_directory, token.string.length, *token.string);
					materials = parse_mtllib(arena, buffer);
					if (!materials) {
						LOG_WARNING("Compiler", "Couldn't load material file '%s'\n", buffer);
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

	VertexHashEntry *vertex_cache = 0;
	u64 hashmap_count = (u64)next_power_of_2(sum * 4);
	u64 hashmap_mask = hashmap_count-1;
	u64 invalid_hash = ~0u;
	GLindex element_index = 0;

	hashmap_mask = hashmap_count - 1;
	vertex_cache = PUSH_STRUCTS(arena, hashmap_count, VertexHashEntry, true);
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

			static u32 xprime = 492876863; /* arbitrary large prime */
			static u32 yprime = 633910099; /* arbitrary large prime */
			static u32 zprime = 805306457; /* arbitrary large prime */
			u64 hash = ((vi * xprime) ^ (ci * yprime) ^ (ni * zprime)) & hashmap_mask;

			bool found = false;
			for (u64 offset = 0; offset < hashmap_count; offset++) {
				u64 index = (hash + offset) & hashmap_mask;

				VertexHashEntry *entry = vertex_cache + index;
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
	snprintf(buffer, ARRAY_COUNT(buffer), "%.*s/%s", output_directory.length, *output_directory, output_filename);
	fopen_s(&output, buffer, "wb");
	ASSERT(file, "Could not open '%s'", buffer);

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
		fwrite(&group.material_index, sizeof(int), 1, output);

		int index_count = (int)array_count(group.indices);
		fwrite(&index_count, sizeof(int), 1, output);
		fwrite(group.indices, sizeof(GLindex), (size_t)index_count, output);
	}

	int material_count = array_count(materials);
	fwrite(&material_count, sizeof(int), 1, output);
	for (i32 i = 0; i < material_count; ++i) {
		Material &m = materials[i];

		fwrite(&m.Ns, sizeof(float), 4, output); // Ns to Tf
		fwrite(&m.illum, sizeof(u32), 1, output);
		fwrite(&m.Ka, sizeof(u32), 4, output); // Ka to Ke

		write_string(m.map_Ka, output);
		write_string(m.map_Kd, output);
		write_string(m.map_Ks, output);
		write_string(m.map_Ke, output);
		write_string(m.map_d, output);
		write_string(m.map_bump, output);
	}

	fclose(output);
}

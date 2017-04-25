// #include "../../utils/parser.cpp"
// #include "../../utils/math.h"

parser::Tokenizer make_tokenizer(char *text) {
	parser::Tokenizer tok = { text };
	return tok;
}

typedef union {
  float f;
  struct {
    unsigned int mantisa : 23;
    unsigned int exponent : 8;
    unsigned int sign : 1;
  } parts;
} double_cast;

inline float next_number(parser::Tokenizer *tok) {
	parser::Token token = parser::next_token(tok);
	ASSERT_TOKEN_TYPE(token, TokenType_Number);
	return token.number;
}

struct VertexData
{
	v3 *vertices;
	unsigned vertex_count;

	unsigned *vertex_indices;
	unsigned vertex_index_count;
};

VertexData read_pot()
{
	FILE *file = fopen("external tools/obj_compiler/teapot.obj", "r");
	ASSERT(file, "Could not open 'external tools/obj_compiler/teapot.obj'");

	size_t filesize = get_filesize(file);

	char *source = (char*) malloc(filesize);
	fread(source, 1, filesize, file);

	v3 *vertices = (v3*) malloc(16*1024*sizeof(v3));
	unsigned vertex_count = 0;

	v2 *tex_coords = (v2*) malloc(16*1024*sizeof(v2));
	unsigned tex_coord_count = 0;

	v3 *vertex_normals = (v3*) malloc(16*1024*sizeof(v3));
	unsigned vertex_normal_count = 0;

	v4 *vertex_colors = (v4*) malloc(16*1024*sizeof(v4));
	unsigned vertex_color_count = 0;

	unsigned *vertex_indices = (unsigned*) malloc(16*1024*sizeof(unsigned));
	unsigned vertex_index_count = 0;

	parser::Tokenizer tok = { source };
	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				// v - vertices
				if (parser::is_equal(token, TOKENIZE("v"))) {
					// ASSERT(vertex_count < 1024);
					vertices[vertex_count++] = V3(
						next_number(&tok),
						next_number(&tok),
						next_number(&tok));
				}
				// vt - texture coordinates
				else if (parser::is_equal(token, TOKENIZE("vt"))) {
					ASSERT(tex_coord_count < 1024, "tex_coord_count out of bounds");
					tex_coords[tex_coord_count++] = V2(
						next_number(&tok),
						next_number(&tok));
				}
				// vn - vertex normals
				else if (parser::is_equal(token, TOKENIZE("vn"))) {
					ASSERT(vertex_normal_count < 1024, "vertex_normal_count out of bounds");
					vertex_normals[vertex_normal_count++] = V3(
						next_number(&tok),
						next_number(&tok),
						next_number(&tok));
				}
				// vc - vertex colors
				else if (parser::is_equal(token, TOKENIZE("vc"))) {
					ASSERT(vertex_color_count < 1024, "vertex_color_count out of bounds");
					vertex_colors[vertex_color_count++] = V4(
						next_number(&tok),
						next_number(&tok),
						next_number(&tok),
						next_number(&tok));
				}
				// f - faces MUST BE TRIANGLES!
				else if (parser::is_equal(token, TOKENIZE("f"))) {
					// ASSERT(vertex_index_count < 1024);
					vertex_indices[vertex_index_count++] = next_number(&tok);
					vertex_indices[vertex_index_count++] = next_number(&tok);
					vertex_indices[vertex_index_count++] = next_number(&tok);
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

	VertexData data = { };

	data.vertices = vertices;
	data.vertex_count = vertex_count;
	data.vertex_indices = vertex_indices;
	data.vertex_index_count = vertex_index_count;

	return data;
}
/*
int main(int argc, char const *argv[])
{
	read_pot();
}*/
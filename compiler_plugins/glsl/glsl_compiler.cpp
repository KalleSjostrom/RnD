// #include "engine/utils/platform.h"
#include "engine/common.h"
#include "engine/utils/math/math.h"
#include "engine/utils/parser.cpp"
#include "engine/utils/file_utils.h"
#include <gl/gl.h>
#include "engine/utils/win32_setup_gl.h"
#include "engine/opengl/gl_program_builder.cpp"
// #include "engine/utils/containers/dynamic_array.h"
// #include "engine/utils/logger.h"

char *parse_shader(parser::Tokenizer &tok) {
	ASSERT_NEXT_TOKEN_TYPE(tok, '=');
	ASSERT_NEXT_TOKEN_TYPE(tok, '{');
	i32 bracket_count = 1;
	char *start = tok.at;
	while (bracket_count) {
		if (tok.at[0] == '{') {
			bracket_count++;
		} else if (tok.at[0] == '}') {
			bracket_count--;
		} else if (tok.at[0] == '\0') {
			ASSERT(false, "Reached end of file before finding a closing bracket!");
			return 0;
		}
		tok.at++;
	}
	tok.at[-1] = '\0';
	return start;
}

void compile_glsl(String input_directory, String input_filename, String output_directory) {
	MemoryArena arena = {};

    size_t filesize;
    char buffer[1024];
    snprintf(buffer, ARRAY_COUNT(buffer), "%.*s/%s", input_directory.length, *input_directory, *input_filename);
    FILE *file = open_file(buffer, &filesize);

	char output_filename[MAX_PATH];
	snprintf(output_filename, MAX_PATH, "%.*s.cglsl", input_filename.length-4, *input_filename);

	char *source = (char *)PUSH_SIZE(arena, filesize + 1);
	fread(source, filesize, 1, file);
	source[filesize] = '\0';
	fclose(file);

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize, TokenizerFlags_Verbatim);
	parser::ParserContext pcontext;
	parser::set_parser_context(pcontext, &tok, (char*)*buffer);

	parser::Token t_vertex = TOKENIZE("vertex");
	parser::Token t_fragment = TOKENIZE("fragment");
	parser::Token t_geometry = TOKENIZE("geometry");

	char *vertex = 0;
	char *fragment = 0;
	char *geometry = 0;

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				if (parser::is_equal(token, t_vertex)) {
					vertex = parse_shader(tok);
				} else if (parser::is_equal(token, t_fragment)) {
					fragment = parse_shader(tok);
					} else if (parser::is_equal(token, t_geometry)) {
					geometry = parse_shader(tok);
				}
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}

	GLuint program = gl_program_builder::create_from_strings(vertex, fragment, geometry);
	gl_program_builder::write_to_binary_file(arena, program, output_filename);
}

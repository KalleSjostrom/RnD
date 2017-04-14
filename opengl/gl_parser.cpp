#include "../utils/parser.cpp"

struct Uniform {
	String value;
	String type;
	String name;

	int line_count;
	String lines[32];
};
struct InVariables {
	String location;
	String type;
	String name;
};

int main(int argc, char *argv[]) {
	FILE *output = fopen("../generated/setup_vertex_array.cpp", "w");
	ASSERT(output);
	fprintf(output, "#include \"gl_preamble.cpp\"\n");
	fprintf(output, "namespace gl_manager {\n");

	{
		FILE *file = fopen("../shaders/shader.vert", "r");
		size_t filesize = get_filesize(file);

		char source[filesize];
		fread(source, 1, filesize, file);

	    Uniform uniforms[32] = {0};
		int uniform_counter = 0;

		InVariables in_variables[32] = {0};
		int in_variables_counter = 0;

		parser::Tokenizer tok = { source };
		bool parsing = true;
		while (parsing) {
			parser::Token token = parser::next_token(&tok);
			switch (token.type) {
				case TokenType_Identifier: {
					if (parser::is_equal(token, TOKENIZE("layout"))) {
						InVariables in_var = {0};

						ASSERT_NEXT_TOKEN(tok, "(");
						ASSERT_NEXT_TOKEN(tok, "location");
						ASSERT_NEXT_TOKEN(tok, "=");
						in_var.location = parser::next_token(&tok).string;
						ASSERT_NEXT_TOKEN(tok, ")");
						ASSERT_NEXT_TOKEN(tok, "in");

						in_var.type = parser::next_token(&tok).string;
						in_var.name = parser::next_token(&tok).string;

						in_variables[in_variables_counter++] = in_var;
					}
				} break;
				case TokenType_CommandMarker: {
					parser::Token token = parser::next_token(&tok);
					if (parser::is_equal(token, TOKENIZE("uniform"))) {
						Uniform uniform = {0};
						bool reached_end = false;
						while (!reached_end) {
							token = parser::next_token(&tok);
							switch(token.type) {
								case TokenType_Identifier: {
									ASSERT(parser::is_equal(token, TOKENIZE("uniform")));
									uniform.type = parser::next_token(&tok).string;
									uniform.name = parser::next_token(&tok).string;
									reached_end = true;
								} break;
								case TokenType_CommandMarker: {
									token = parser::next_token(&tok);
									if (parser::is_equal(token, TOKENIZE("passthrough"))) {
										uniform.lines[uniform.line_count++] = parser::next_line(&tok).string;
									} else if (parser::is_equal(token, TOKENIZE("value"))) {
										uniform.value = parser::next_line(&tok).string;
									} else {
										ASSERT_MSG(false, "Unknown command!")
									}
								}
							}
						}
						uniforms[uniform_counter++] = uniform;
					}
				} break;
				case '\0': {
					parsing = false;
				} break;
			}
		}
		fclose(file);

		// TODO(kalle): THIS IS BULLSHIT! Find a better way of null terminating the strings without the parser to fuckup..
		for (int i = 0; i < uniform_counter; ++i) {
			Uniform &u = uniforms[i];
			null_terminate(u.value);
			null_terminate(u.type);
			null_terminate(u.name);
			for (int j = 0; j < u.line_count; ++j) {
				null_terminate(u.lines[j]);
			}
		}
		for (int i = 0; i < in_variables_counter; ++i) {
			null_terminate(in_variables[i].location);
			null_terminate(in_variables[i].type);
			null_terminate(in_variables[i].name);
		}

		fprintf(output, "\
	void setup_uniforms(GLuint program) {\n\
		glUseProgram(program);\n\n");

		String mat4 = MAKE_STRING("mat4");
		for (int i = 0; i < uniform_counter; ++i) {
			Uniform u = uniforms[i];
			for (int j = 0; j < u.line_count; ++j) {
				fprintf(output, "\t\t%s\n", *u.lines[j]);
			}

			fprintf(output, "\t\tGLint %s_location = glGetUniformLocation(program, \"%s\");\n", *u.name, *u.name);
			if (string_is_equal(u.type, mat4)) {
				fprintf(output, "\t\tglUniformMatrix4fv(%s_location, 1, GL_FALSE, (GLfloat*)(%s));\n", *u.name, *u.value);
			} else {
				ASSERT_MSG(false, "Only mat4 is supperted atm...");
			}
		}

		fprintf(output, "\t}\n");

		fprintf(output, "\
	void setup_vertex_array(GLuint *vao, Buffer *buffers) {\n\
		glGenVertexArrays(1, vao);\n\
		glBindVertexArray(*vao);\n\n");
		for (int i = 0; i < in_variables_counter; ++i) {
			InVariables ivar = in_variables[i];

			fprintf(output, "\t\t__GL_BIND_BUFFER(buffers, %s);\n", *ivar.name);
			fprintf(output, "\t\tglVertexAttribPointer(%s, 2, GL_FLOAT, GL_FALSE, 0, 0);\n", *ivar.location);
			fprintf(output, "\t\tglEnableVertexAttribArray(%s);\n\n", *ivar.location);
		}
		fprintf(output, "\t}\n");
	}
	{
		FILE *file = fopen("../shaders/shader.frag", "r");
		size_t filesize = get_filesize(file);
		fclose(file);
	}

	fprintf(output, "}\n");

	fclose(output);
}

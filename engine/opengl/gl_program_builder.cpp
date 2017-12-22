#include "engine/utils/file_utils.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/logger.h"
#include "gl_errors.cpp"

namespace gl_program_builder {
	static GLuint compile(GLenum shader_type, const char *shader_source) {
		GLuint shader = glCreateShader(shader_type);
		GL_CHECK_ERROR(glCreateShader);

		glShaderSource(shader, 1, &shader_source, NULL);
		GL_CHECK_ERROR(glShaderSource);
		glCompileShader(shader);
		GL_CHECK_ERROR(glCompileShader);

		char buffer[2048];
		GLsizei length;
		glGetShaderInfoLog(shader, 2048, &length, buffer);
		GL_CHECK_ERROR(glGetShaderInfoLog);
		if (length > 0) {
			LOG_INFO("GlProgram", "Failed compiling shader!: %s\n", buffer);
			ASSERT(false, "Failed compiling shader!: %s\n", buffer);
		} else {
			LOG_INFO("GlProgram", "Shader compile successful!\n");
		}

		return shader;
	}

	static GLuint load_and_compile(MemoryArena &arena, GLenum shader_type, const char *filename) {
		size_t size;
		FILE *file = open_file(filename, &size);
		char *source_buffer = (char*)PUSH_SIZE(arena, size);
		fread(source_buffer, sizeof(char), (size_t)size, file);
		fclose(file);
		source_buffer[size] = '\0';

		return compile(shader_type, source_buffer);
	}

	GLuint create_from_source_files(MemoryArena &arena, const char *vertex_shader_filename, const char *fragment_shader_filename, const char *geometry_shader_filename) {
		GLuint program = glCreateProgram();
		GL_CHECK_ERROR_RETVAL(glCreateProgram, program);

		MemoryBlockHandle handle = begin_block(arena);
			GLuint vs = load_and_compile(arena, GL_VERTEX_SHADER, vertex_shader_filename);
			glAttachShader(program, vs);
			GL_CHECK_ERROR(glAttachShader);

			GLuint fs = load_and_compile(arena, GL_FRAGMENT_SHADER, fragment_shader_filename);
			glAttachShader(program, fs);
			GL_CHECK_ERROR(glAttachShader);

			if (geometry_shader_filename) {
				GLuint gs = load_and_compile(arena, GL_GEOMETRY_SHADER, geometry_shader_filename);
				glAttachShader(program, gs);
				GL_CHECK_ERROR(glAttachShader);
			}
		end_block(arena, handle);

		glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		GL_CHECK_ERROR(glProgramParameteri);

		glLinkProgram(program);
		GL_CHECK_ERROR(glLinkProgram);

		GLint link_status;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);
		GL_CHECK_ERROR(glGetProgramiv);

		if (link_status == GL_TRUE) {
			LOG_INFO("GlProgram", "Gl link successful!\n");
		} else {
			GLchar info_log[2048];
			glGetProgramInfoLog(program, 2048, 0, info_log);
			GL_CHECK_ERROR(glGetProgramInfoLog);
			LOG_INFO("GlProgram", "glGetProgramInfoLog: %s\n", info_log);
		}

		return program;
	}

	GLuint create_from_strings(const char *vertex_shader_source, const char *fragment_shader_source, const char *geometry_shader_source) {
		GLuint program = glCreateProgram();
		GL_CHECK_ERROR_RETVAL(glCreateProgram, program);

		GLuint vs = compile(GL_VERTEX_SHADER, vertex_shader_source);
		glAttachShader(program, vs);
		GL_CHECK_ERROR(glAttachShader);

		GLuint fs = compile(GL_FRAGMENT_SHADER, fragment_shader_source);
		glAttachShader(program, fs);
		GL_CHECK_ERROR(glAttachShader);

		if (geometry_shader_source) {
			GLuint gs = compile(GL_GEOMETRY_SHADER, geometry_shader_source);
			glAttachShader(program, gs);
			GL_CHECK_ERROR(glAttachShader);
		}

		glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		GL_CHECK_ERROR(glProgramParameteri);

		glLinkProgram(program);
		GL_CHECK_ERROR(glLinkProgram);

		GLint link_status;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);
		GL_CHECK_ERROR(glGetProgramiv);

		if (link_status == GL_TRUE) {
			LOG_INFO("GlProgram", "Gl link successful!\n");
		} else {
			GLchar info_log[2048];
			glGetProgramInfoLog(program, 2048, 0, info_log);
			GL_CHECK_ERROR(glGetProgramInfoLog);
			LOG_INFO("GlProgram", "glGetProgramInfoLog: %s\n", info_log);
		}

		return program;
	}

	GLuint create_from_binary_file(const char *filename) {
		(void)filename;
		LOG_INFO("GlProgram", "create_from_binary_file not implemented!\n");
		return 0;
	}

	bool validate_program(GLuint program) {
		glValidateProgram(program);
		GL_CHECK_ERROR(glValidateProgram);

		GLint validate_status;
		glGetProgramiv(program, GL_VALIDATE_STATUS, &validate_status);
		GL_CHECK_ERROR(glGetProgramiv);

		bool is_valid = validate_status == GL_TRUE;

		if (is_valid) {
			LOG_INFO("GlProgram", "Gl program is valid!\n");
		} else {
			GLchar info_log[2048];
			glGetProgramInfoLog(program, 2048, 0, info_log);
			GL_CHECK_ERROR(glGetProgramInfoLog);
			LOG_INFO("GlProgram", "glGetProgramInfoLog: %s\n", info_log);
		}
		return is_valid;
	}

	void write_to_binary_file(MemoryArena &arena, GLuint program, const char *filename) {
		GLint binary_length;
		glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binary_length);
		GL_CHECK_ERROR(glGetProgramiv);

		GLenum binary_format;
		char *binary = PUSH_STRING(arena, (u32)binary_length);
		glGetProgramBinary(program, binary_length, 0, &binary_format, binary);
		GL_CHECK_ERROR(glGetProgramBinary);

		{
			FILE *file;
			fopen_s(&file, filename, "w");
			fwrite(&binary_format, sizeof(GLenum), 1, file);
			fwrite(&binary_length, sizeof(GLint), 1, file);
			fwrite(&binary, sizeof(char), (u32)binary_length, file);
			fclose(file);
		}

		free(binary);
	}
}
// Have you tried adding [code]glProgramParameteri(program, PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);[/code] before linking the program? [/size]
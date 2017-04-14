#include "gl_preamble.cpp"
namespace gl_manager {
	void setup_uniforms(GLuint program) {
		glUseProgram(program);

		matrix4x4 view = look_at(V3(0.0f, 0.0f, 20.0f), V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
		matrix4x4 projection = perspective_fov(100, RES_WIDTH / RES_HEIGHT, 1.0f, 100.0f);
		GLint model_view_location = glGetUniformLocation(program, "model_view");
		glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat*)((projection * view).m));
	}
	void setup_vertex_array(GLuint *vao, Buffer *buffers) {
		glGenVertexArrays(1, vao);
		glBindVertexArray(*vao);

		__GL_BIND_BUFFER(buffers, positions);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		__GL_BIND_BUFFER(buffers, density_pressure);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

	}
}

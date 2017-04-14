namespace gl_manager {
	#define __GL_BIND_BUFFER(buffers, id) glBindBuffer(GL_ARRAY_BUFFER, buffers[BufferIndex__##id].vbo);
}
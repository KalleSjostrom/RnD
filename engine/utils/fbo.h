struct FBO {
	GLuint framebuffer_object;
	i32 count;
	GLuint render_texture[8];
};

static GLenum draw_buffers[] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7,
};

void setup_fbo(FBO &fbo, i32 width, i32 height, i32 count = 1) {
	glGenFramebuffers(1, &fbo.framebuffer_object);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.framebuffer_object);

	fbo.count = count;
	for (i32 i = 0; i < count; ++i) {
		glGenTextures(1, &fbo.render_texture[i]);
		glBindTexture(GL_TEXTURE_2D, fbo.render_texture[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, draw_buffers[i], GL_TEXTURE_2D, fbo.render_texture[i], 0);
	}

	glDrawBuffers(count, draw_buffers);

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "glCheckFramebufferStatus failed!");
}

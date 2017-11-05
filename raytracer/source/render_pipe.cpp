struct FBO {
	GLuint framebuffer_object;
	i32 count;
	GLuint render_texture[8];
};

static const u32 fsaa_num_samples = 8;

struct RenderPipe {
	i32 screen_width;
	i32 screen_height;

	Entity fullscreen_quad;

	GLuint passthrough_program;
	GLint passthrough_render_texture_location;

	GLuint ray_texture;
	// GLuint buffer;

	GLuint fsaa_tex;
	GLuint fsaa_fbo;

	ImageData image_data;

	// Move
	Program programs[4];
	i32 program_count;
	i32 __padding;
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

void update_image(RenderPipe &r) {
	glBindTexture(GL_TEXTURE_2D, r.ray_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, r.image_data.width, r.image_data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, r.image_data.pixels);
}

void alloc_image(MemoryArena &arena, RenderPipe &r) {
	r.image_data = {};

	r.image_data.width = 1024;
	r.image_data.height = 768;
	r.image_data.bytes_per_pixel = 4;
	r.image_data.pixels = PUSH_SIZE(arena, (size_t)(r.image_data.width * r.image_data.height * r.image_data.bytes_per_pixel));

	glGenTextures(1, &r.ray_texture);

	glBindTexture(GL_TEXTURE_2D, r.ray_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, r.image_data.width, r.image_data.height, 0, GL_RGBA, GL_FLOAT, 0);


	glGenTextures(1, &r.fsaa_tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, r.fsaa_tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fsaa_num_samples, GL_SRGB8_ALPHA8, r.screen_width, r.screen_height, false);

	glGenFramebuffers(1, &r.fsaa_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, r.fsaa_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, r.fsaa_tex, 0);
}

void setup_render_pipe(EngineApi *engine, MemoryArena &arena, RenderPipe &r, ComponentGroup &components, i32 screen_width, i32 screen_height) {
	if (r.fullscreen_quad.type != EntityType_Fullscreen) {
		Context c = {};
		spawn_entity(components, r.fullscreen_quad, EntityType_Fullscreen, c);
	}

	r.screen_width = screen_width;
	r.screen_height = screen_height;

	// glDisable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_FRAMEBUFFER_SRGB);

	alloc_image(arena, r);

	{
		r.passthrough_program = gl_program_builder::create_from_strings(shader_passthrough::vertex, shader_passthrough::fragment, 0);
		r.passthrough_render_texture_location = glGetUniformLocation(r.passthrough_program, "render_texture");
		gl_program_builder::validate_program(r.passthrough_program);

		ASSERT(r.passthrough_render_texture_location != -1, "passthrough_render_texture_location not found");
	}
}

void render(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	// Push viewport and blend state
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (i32)(r.screen_width), (i32)(r.screen_height));

	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, r.fsaa_fbo);

	if (1) {
		// Get the fullscreen quad and set it up for rendering
		i32 model_id = r.fullscreen_quad.model_id;
		Renderable &renderable = components.model.instances[model_id].renderable;
		glBindVertexArray(renderable.vertex_array_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.element_array_buffer);

		glUseProgram(r.passthrough_program);
		glBindTexture(GL_TEXTURE_2D, r.ray_texture);
		glUniform1i(r.passthrough_render_texture_location, 0);
		glDrawElements(renderable.draw_mode, renderable.index_count, GL_UNSIGNED_SHORT, (void*)0);


		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   // Make sure no FBO is set as the draw framebuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, r.fsaa_fbo); // Make sure your multisampled FBO is the read framebuffer
		glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer
		glBlitFramebuffer(0, 0, r.screen_width, r.screen_height, 0, 0, r.screen_width, r.screen_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	} else {
		render(components.renderer, camera, RenderMask_ShadowCasters);
	}
}

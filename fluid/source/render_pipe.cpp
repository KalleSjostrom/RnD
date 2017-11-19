static const u32 fsaa_num_samples = 4;

struct RenderPipe {
	i32 screen_width;
	i32 screen_height;

	Entity fullscreen_quad;

	GLuint fsaa_tex;
	GLuint fsaa_fbo;
};

void setup_render_pipe(EngineApi *engine, RenderPipe &r, ComponentGroup &components, i32 screen_width, i32 screen_height) {
	if (r.fullscreen_quad.type != EntityType_Fullscreen) {
		Context c = {};
		spawn_entity(components, r.fullscreen_quad, EntityType_Fullscreen, c);
	}

	r.screen_width = screen_width;
	r.screen_height = screen_height;

	glGenTextures(1, &r.fsaa_tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, r.fsaa_tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fsaa_num_samples, GL_RGBA8, r.screen_width, r.screen_height, false);

	glGenFramebuffers(1, &r.fsaa_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, r.fsaa_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, r.fsaa_tex, 0);
}

void render_combine(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	(void) camera;

	glBindFramebuffer(GL_FRAMEBUFFER, r.fsaa_fbo);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	components.model.render(r.fullscreen_quad.model_id, -1);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   // Make sure no FBO is set as the draw framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r.fsaa_fbo); // Make sure your multisampled FBO is the read framebuffer
	glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer
	glBlitFramebuffer(0, 0, r.screen_width, r.screen_height, 0, 0, r.screen_width, r.screen_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void render(RenderPipe &render_pipe, ComponentGroup &components, Camera &camera) {
	render(components.renderer, camera, RenderMask_Fluid);
}

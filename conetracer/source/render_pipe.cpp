struct RenderPipe {
	int screen_width;
	int screen_height;
};

void setup_render_pipe(ArenaAllocator &arena, EngineApi *engine, RenderPipe &r, ComponentGroup &components, int screen_width, int screen_height) {
	r.screen_width = screen_width;
	r.screen_height = screen_height;
}

void render(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (int)(r.screen_width), (int)(r.screen_height));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	render(components.renderer, camera, RenderMask_ShadowCasters);
}

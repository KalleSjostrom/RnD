struct RenderPipe {
	i32 screen_width;
	i32 screen_height;
};

void setup_render_pipe(MemoryArena &arena, EngineApi *engine, RenderPipe &r, ComponentGroup &components, i32 screen_width, i32 screen_height) {
	r.screen_width = screen_width;
	r.screen_height = screen_height;
}

void render(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (i32)(r.screen_width), (i32)(r.screen_height));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	render(components.renderer, camera, RenderMask_ShadowCasters);
}

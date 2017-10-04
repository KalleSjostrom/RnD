#include "shaders/fullscreen_effects.shader.cpp"

enum RenderMask {
	RenderMask_ShadowCasters = 1 << 0,
	RenderMask_Lights = 1 << 1,
	RenderMask_Rest = 1 << 2,

	RenderMask_All = 0xFFFFFFFF,
};

struct FBO {
	GLuint framebuffer_object;
	int count;
	GLuint render_texture[8];
};

struct RenderPipe {
	int screen_width;
	int screen_height;

	Entity fullscreen_quad;

	FBO scene;
	FBO shadowmap;
	FBO lightmap[2];
	FBO bloom[2];

	v3 light_colors[4];
	v2 light_positions[4];
	float light_radii[4];

	GLuint passthrough_program;
	GLint passthrough_render_texture_location;

	// 1D Shadow map
	GLuint shadowmap_program;
	GLint shadowmap_scene_location;
	GLint shadowmap_light_positions_location;
	GLint shadowmap_light_radii_location;

	// Lightmap
	GLuint lightmap_program;
	GLint lightmap_shadowmap_location;
	GLint lightmap_light_positions_location;
	GLint lightmap_light_radii_location;
	GLint lightmap_light_colors_location;

	// Bloom filter
	GLuint bloom_program;
	GLint bloom_render_texture_location;
	GLint bloom_direction_location;

	// Produces the final image, by blending the scene with the ligth
	GLuint blend_program;
	GLint blend_hatch_location;
	GLint blend_bloom_location;
	GLint blend_lightmap_info_location;
	GLint blend_lightmap_color_location;
	GLint blend_scene_location;

	GLuint hatch_texture;

	// Move
	Program programs[4];
	i32 program_count;
	i32 __padding;
};

static GLenum draw_buffers[] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7,
};

void setup_fbo(FBO &fbo, int width, int height, int count = 1) {
	glGenFramebuffers(1, &fbo.framebuffer_object);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.framebuffer_object);

	fbo.count = count;
	for (int i = 0; i < count; ++i) {
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

void load_image(EngineApi *engine, RenderPipe &r) {
	ImageData image_data;
	b32 success = engine->image_load("../../game/assets/hatch.png", image_data);

	glGenTextures(1, &r.hatch_texture);

	glBindTexture(GL_TEXTURE_2D, r.hatch_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// The png is stored as ARGB, appearently
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data.width, image_data.height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image_data.pixels);

	// #include "../assets/hatch_3.c"
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hatch.width, hatch.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, hatch.pixel_data);
}

void setup(EngineApi *engine, RenderPipe &r, int screen_width, int screen_height) {
	if (r.fullscreen_quad.type != EntityType_Fullscreen)
		spawn_entity(*(ComponentGroup*)globals::components, r.fullscreen_quad, EntityType_Fullscreen);

	r.screen_width = screen_width;
	r.screen_height = screen_height;

	setup_fbo(r.scene, screen_width, screen_height);
	setup_fbo(r.shadowmap, 360*2, 1);

	setup_fbo(r.lightmap[0], screen_width/4, screen_height/4, 2);
	setup_fbo(r.lightmap[1], screen_width/4, screen_height/4, 2);

	setup_fbo(r.bloom[0], screen_width/4, screen_height/4);
	setup_fbo(r.bloom[1], screen_width/4, screen_height/4);

	r.passthrough_program = gl_program_builder::create_from_strings(shader_passthrough::vertex, shader_passthrough::fragment, 0);
	r.passthrough_render_texture_location = glGetUniformLocation(r.passthrough_program, "render_texture");
	gl_program_builder::validate_program(r.passthrough_program);

	r.shadowmap_program = gl_program_builder::create_from_strings(shader_shadowmap::vertex, shader_shadowmap::fragment, 0);
	r.shadowmap_scene_location = glGetUniformLocation(r.shadowmap_program, "scene");
	r.shadowmap_light_positions_location = glGetUniformLocation(r.shadowmap_program, "light_positions");
	r.shadowmap_light_radii_location = glGetUniformLocation(r.shadowmap_program, "light_radii");
	gl_program_builder::validate_program(r.shadowmap_program);

	r.lightmap_program = gl_program_builder::create_from_strings(shader_lightmap::vertex, shader_lightmap::fragment, 0);
	r.lightmap_shadowmap_location = glGetUniformLocation(r.lightmap_program, "shadowmap");
	r.lightmap_light_positions_location = glGetUniformLocation(r.lightmap_program, "light_positions");
	r.lightmap_light_radii_location = glGetUniformLocation(r.lightmap_program, "light_radii");
	r.lightmap_light_colors_location = glGetUniformLocation(r.lightmap_program, "light_colors");
	gl_program_builder::validate_program(r.lightmap_program);

	r.bloom_program = gl_program_builder::create_from_strings(shader_bloom::vertex, shader_bloom::fragment, 0);
	r.bloom_render_texture_location = glGetUniformLocation(r.bloom_program, "render_texture");
	r.bloom_direction_location = glGetUniformLocation(r.bloom_program, "direction");
	gl_program_builder::validate_program(r.bloom_program);

	r.blend_program = gl_program_builder::create_from_strings(shader_combine::vertex, shader_combine::fragment, 0);
	r.blend_hatch_location = glGetUniformLocation(r.blend_program, "hatch");
	r.blend_bloom_location = glGetUniformLocation(r.blend_program, "bloom");
	r.blend_lightmap_info_location = glGetUniformLocation(r.blend_program, "lightmap_info");
	r.blend_lightmap_color_location = glGetUniformLocation(r.blend_program, "lightmap_color");
	r.blend_scene_location = glGetUniformLocation(r.blend_program, "scene");
	gl_program_builder::validate_program(r.blend_program);

	load_image(engine, r);

	r.light_radii[0] = 1.0f; r.light_radii[1] = 1.0f; r.light_radii[2] = 1.0f; r.light_radii[3] = 1.0f;

	r.light_colors[0] = V3(0.55f, 0.4f, 0.4f); r.light_colors[1] = V3(0.4f, 0.4f, 0.55f);
	r.light_colors[2] = V3(0.4f, 0.55f, 0.4f); r.light_colors[3] = V3(0.55f, 0.55f, 0.4f);

	r.light_positions[0] = V2_f32(0, -0.8f); r.light_positions[1] = V2_f32(0.8f, 0);
	r.light_positions[2] = V2_f32(0, 0.8f); r.light_positions[3] = V2_f32(-0.8f, 0);
}

void set_light_position(RenderPipe &r, Camera &camera, v3 light_position) {
	camera.view_projection = camera.projection * camera.view;
	v3 lp = multiply_perspective(camera.view_projection, light_position);
	r.light_positions[0] = V2_f32(lp.x, lp.y);
}

void render_bloom(RenderPipe &r) {
	// Set a smaller viewport for down-sampling
	glViewport(0, 0, r.screen_width/4, r.screen_height/4);

	glBindFramebuffer(GL_FRAMEBUFFER, r.bloom[1].framebuffer_object);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Render the scene into our (downscaled) bloom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, r.bloom[0].framebuffer_object);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Get the fullscreen quad and set it up for rendering
	int model_id = r.fullscreen_quad.model_id;
	Renderable &renderable = ((ComponentGroup*)globals::components)->model.instances[model_id].renderable;
	glBindVertexArray(renderable.vertex_array_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.element_array_buffer);

	// Use the pass through shader to render the scene into the first bloom framebuffer
	glUseProgram(r.passthrough_program);
	glBindTexture(GL_TEXTURE_2D, r.scene.render_texture[0]);
	glUniform1i(r.passthrough_render_texture_location, 0);
	glDrawElements(renderable.draw_mode, renderable.index_count, GL_UNSIGNED_SHORT, (void*)0);

	// Start ping-pong rendering of the gaussian blur
	glUseProgram(r.bloom_program);
	for (int i = 0; i < 15; ++i) {
		// Render to the second framebuffer_object
		glBindFramebuffer(GL_FRAMEBUFFER, r.bloom[1].framebuffer_object);
		glBindTexture(GL_TEXTURE_2D, r.bloom[0].render_texture[0]);
		glUniform2f(r.bloom_direction_location, 0, 1);
		glDrawElements(renderable.draw_mode, renderable.index_count, GL_UNSIGNED_SHORT, (void*)0);

		// Render to the first framebuffer_object
		glBindFramebuffer(GL_FRAMEBUFFER, r.bloom[0].framebuffer_object);
		glBindTexture(GL_TEXTURE_2D, r.bloom[1].render_texture[0]);
		glUniform2f(r.bloom_direction_location, 1, 0);
		glDrawElements(renderable.draw_mode, renderable.index_count, GL_UNSIGNED_SHORT, (void*)0);
	}

	// Restore the viewport
	glViewport(0, 0, r.screen_width, r.screen_height);
}

void render_shadowmap(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, r.scene.framebuffer_object);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Rendering all shadow-casters to the scene fbo
	render(components.renderer, camera, RenderMask_ShadowCasters);

	glDisable(GL_BLEND);
	glViewport(0, 0, 360*2, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, r.shadowmap.framebuffer_object);

	glUseProgram(r.shadowmap_program);
	glBindTexture(GL_TEXTURE_2D, r.scene.render_texture[0]);
	glUniform1i(r.shadowmap_scene_location, 0);
	glUniform2fv(r.shadowmap_light_positions_location, 4, (float*)r.light_positions);
	glUniform1fv(r.shadowmap_light_radii_location, 4, (float*)r.light_radii);

	CALL(r.fullscreen_quad, model, render, -1);

	glViewport(0, 0, r.screen_width, r.screen_height);
	glEnable(GL_BLEND);
}

void render_test(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, r.scene.framebuffer_object);

	glDisable(GL_BLEND);

	render(components.renderer, camera, RenderMask_Rest);

	glEnable(GL_BLEND);
}

void render_lightmap(RenderPipe &r, ComponentGroup &components, Camera &camera) {
	// Push viewport and blend state
	glDisable(GL_BLEND);
	glViewport(0, 0, r.screen_width/4, r.screen_height/4);

	glBindFramebuffer(GL_FRAMEBUFFER, r.lightmap[0].framebuffer_object);

	glUseProgram(r.lightmap_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r.shadowmap.render_texture[0]);
	glUniform1i(r.lightmap_shadowmap_location, 0);
	glUniform2fv(r.lightmap_light_positions_location, 4, (float*)r.light_positions);
	glUniform1fv(r.lightmap_light_radii_location, 4, (float*)r.light_radii);
	glUniform3fv(r.lightmap_light_colors_location, 4, (float*)r.light_colors);

	CALL(r.fullscreen_quad, model, render, -1);

	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	render(components.renderer, camera, RenderMask_Lights); // Rendering all (non shadow-casting) lights to the lightmap

	glViewport(0, 0, r.screen_width, r.screen_height);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_combine(RenderPipe &r) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(r.blend_program);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, r.hatch_texture);
	glUniform1i(r.blend_hatch_location, 4);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, r.bloom[0].render_texture[0]);
	glUniform1i(r.blend_bloom_location, 3);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, r.lightmap[0].render_texture[1]);
	glUniform1i(r.blend_lightmap_info_location, 2);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, r.lightmap[0].render_texture[0]);
	glUniform1i(r.blend_lightmap_color_location, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r.scene.render_texture[0]);
	glUniform1i(r.blend_scene_location, 0);

	CALL(r.fullscreen_quad, model, render, -1);
}

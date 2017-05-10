struct Program {
	GLuint program;
	GLint model_location;
	GLint view_projection_location;

	i32 count;
	Renderable *renderables[8];

	u32 render_mask;
	i32 __padding;
};

struct Renderer {
	Program programs[4];
	i32 program_count;
	i32 __padding;
};

i32 add_program(Renderer &r, GLuint shader_program, u32 render_mask) {
	ASSERT((u32)r.program_count < ARRAY_COUNT(r.programs), "Too many programs added!");
	i32 id = r.program_count++;
	Program &p = r.programs[id];
	p.program = shader_program;
	glUseProgram(shader_program);
	p.model_location = glGetUniformLocation(shader_program, "model");
	p.view_projection_location = glGetUniformLocation(shader_program, "view_projection");
	p.render_mask = render_mask;
	return id;
}

void add_to_program(Renderer &r, i32 id, Renderable *renderable) {
	Program &p = r.programs[id];
	p.renderables[p.count++] = renderable;
}

void render(Renderer &r, Camera &camera, u32 render_mask = 0xFFFFFFFF) {
	for (i32 i = 0; i < r.program_count; ++i) {
		Program &p = r.programs[i];
		if (p.render_mask & render_mask) {
			glUseProgram(p.program);
			begin_frame(camera, p.view_projection_location);

			for (i32 j = 0; j < p.count; ++j) {
				// model.render(p.model_ids[j], p.model_location);
				// model_component::Instance &instance = model.instances[p.model_ids[j]];

				Renderable &re = *p.renderables[j];

				// set_translation(re.pose, V3(0.2f, 0, 0));

				glUniformMatrix4fv(p.model_location, 1, GL_FALSE, (GLfloat*)(re.pose.m));

				glBindVertexArray(re.vertex_array_object);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, re.element_array_buffer);
				glDrawElements(re.draw_mode, re.index_count, GL_INDEX, (void*)0);
			}
		}
	}
}

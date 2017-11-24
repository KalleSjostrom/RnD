struct Program {
	GLuint program;
	GLint projection_location;
	GLint view_location;
	GLint model_location;

	i32 count;
	Renderable *renderables[8];

	u32 render_mask;
	i32 __padding;
};

struct Renderer {
	Program programs[32];
	i32 program_count;
	i32 __padding;
};

i32 add_program(Renderer &r, GLuint shader_program, u32 render_mask) {
	ASSERT((u32)r.program_count < ARRAY_COUNT(r.programs), "Too many programs added!");
	i32 id = r.program_count++;
	Program &p = r.programs[id];
	p.program = shader_program;
	glUseProgram(shader_program);
	p.projection_location = glGetUniformLocation(shader_program, "projection");
	p.view_location = glGetUniformLocation(shader_program, "view");
	p.model_location = glGetUniformLocation(shader_program, "model");
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
			begin_frame(camera, p.projection_location, p.view_location);

			for (i32 j = 0; j < p.count; ++j) {
				Renderable &re = *p.renderables[j];
				glUniformMatrix4fv(p.model_location, 1, GL_FALSE, (GLfloat*)(re.pose.m));

				Mesh &mesh = re.mesh;
				glBindVertexArray(mesh.vertex_array_object);

				for (i32 k = 0; k < mesh.group_count; ++k) {
					Group &group = mesh.groups[k];
					Material *m = group.material;
					if (m) {
						GLint diffuse = glGetUniformLocation(p.program, "diffuse");

						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, m->map_Kd);
						glUniform1i(diffuse, 0);
					}

					switch (re.datatype) {
						case RenderableDataType_Arrays: {
							glDrawArrays(re.draw_mode, 0, group.index_count);
						} break;
						case RenderableDataType_Elements: {
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.element_array_buffer);
							glDrawElements(re.draw_mode, group.index_count, GL_UNSIGNED_INT, (void*)0);
						} break;
					}
				}
			}
		}
	}
}

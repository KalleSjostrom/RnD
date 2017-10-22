namespace fluid_component {
	struct Instance {
		Renderable renderable;

		GLenum buffer_type; // e.g. GL_STATIC_DRAW;
		// GLuint positions_vbo;
		// GLuint density_pressure_vbo;

		i32 vertex_count;
		i32 __padding;
	};

	struct FluidComponent {
		Instance instances[8];
		i32 count;
		GLuint positions_vbo;
		GLuint density_pressure_vbo;
		i32 __padding;

		void set_vbos(GLuint p, GLuint dp) {
			positions_vbo = p;
			density_pressure_vbo = dp;
		}

		i32 add() {
			ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
			i32 id = count++;
			Instance &instance = instances[id];
			Renderable &renderable = instance.renderable;

			renderable.pose = identity();
			// set_position(id, position);

			// renderable.index_count = index_count;
			// instance.vertex_count = vertex_count;

			// instance.buffer_type = buffer_type;
			// renderable.draw_mode = draw_mode;

			// glGenBuffers(1, &renderable.element_array_buffer);
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.element_array_buffer);
			// glBufferData(GL_ELEMENT_ARRAY_BUFFER, (u32)index_count * sizeof(GLindex), indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices

			// glGenBuffers(1, &instance.vertex_buffer_object);
			// glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			// glBufferData(GL_ARRAY_BUFFER, (u32)vertex_count * sizeof(v3), vertices, buffer_type);

			// glGenVertexArrays(1, &renderable.vertex_array_object);
			// glBindVertexArray(renderable.vertex_array_object);

			// glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			// glEnableVertexAttribArray(0);

			// instance.positions_vbo = positions_vbo;
			// instance.density_pressure_vbo = density_pressure_vbo;

			glGenVertexArrays(1, &renderable.vertex_array_object);
			glBindVertexArray(renderable.vertex_array_object);

			glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, density_pressure_vbo);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			return id;
		}

		void render(i32 id, GLint model_location) {
			Renderable &re = instances[id].renderable;

			m4 view = look_at(V3(0.0f, 0.0f, 20.0f), V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
			m4 projection = perspective_fov(100, RES_WIDTH / RES_HEIGHT, 1.0f, 100.0f);
			// GLint model_view_location = glGetUniformLocation(program, "model_view");
			glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat*)((projection * view).m));
			// glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat*)(re.pose.m));

			glBindVertexArray(re.vertex_array_object);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, re.element_array_buffer);
			glDrawElements(re.draw_mode, re.index_count, GL_UNSIGNED_SHORT, (void*)0);
		}
	};
}

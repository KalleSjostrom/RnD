namespace model_component {
	struct Instance {
		m4 pose;

		int32_t index_count;
		int32_t vertex_count;

		GLenum buffer_type; // e.g. GL_STATIC_DRAW;
		GLenum draw_mode; // e.g. GL_TRIANGLE_STRIP;

		GLuint element_array_buffer;
		GLuint vertex_buffer_object;
		GLuint vertex_array_object;
	};

	struct ModelComponent {
		int count;
		Instance instances[8];

		int add(v3 position, GLindex *indices, int32_t index_count, v3 *vertices, int32_t vertex_count, GLenum buffer_type = GL_STATIC_DRAW, GLenum draw_mode = GL_TRIANGLE_STRIP) {
			ASSERT(count < ARRAY_COUNT(instances), "Component full!");
			int id = count++;
			Instance &instance = instances[id];

			instance.pose = identity();
			set_position(id, position);

			instance.index_count = index_count;
			instance.vertex_count = vertex_count;

			instance.buffer_type = buffer_type;
			instance.draw_mode = draw_mode;

			glGenBuffers(1, &instance.element_array_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.element_array_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLindex), indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices

			glGenBuffers(1, &instance.vertex_buffer_object);
			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, sizeof(v3) * vertex_count, vertices, buffer_type);

			glGenVertexArrays(1, &instance.vertex_array_object);
			glBindVertexArray(instance.vertex_array_object);

			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			return id;
		}

		inline void set_position(int id, v3 position) {
			Instance &instance = instances[id];
			set_translation(instance.pose, position);
		}

		inline void rotate_around(int id, float angle, float x, float y) {
			Instance &instance = instances[id];

			float ca = cos(angle);
			float sa = sin(angle);

			float rx = -ca*x + sa*y + x;
			float ry = -sa*x - ca*y + y;

			m4 rotation = identity();

			rotation.m[INDEX(0, 3)] = rx;
			rotation.m[INDEX(1, 3)] = ry;

			rotation.m[INDEX(0, 0)] = ca;
			rotation.m[INDEX(0, 1)] = -sa;
			rotation.m[INDEX(1, 0)] = sa;
			rotation.m[INDEX(1, 1)] = ca;

			instance.pose = instance.pose * rotation;
		}

		inline void rotate(int id, float angle) {
			Instance &instance = instances[id];

			float ca = cos(angle);
			float sa = sin(angle);

			m4 rotation = identity();

			rotation.m[INDEX(0, 0)] = ca;
			rotation.m[INDEX(0, 1)] = -sa;
			rotation.m[INDEX(1, 0)] = sa;
			rotation.m[INDEX(1, 1)] = ca;

			instance.pose = instance.pose * rotation;
		}

		void update_vertices(int id, v3 *vertices) {
			Instance &instance = instances[id];

			ASSERT(instance.buffer_type == GL_DYNAMIC_DRAW, "Should not update vertices with static buffer type!");
			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, sizeof(v3) * instance.vertex_count, vertices, GL_DYNAMIC_DRAW);
		}

		void update_vertices(int id, GLindex *indices, int32_t index_count, v3 *vertices, int32_t vertex_count) {
			Instance &instance = instances[id];

			ASSERT(instance.buffer_type == GL_DYNAMIC_DRAW, "Should not update vertices with static buffer type!");

			instance.index_count = index_count;
			instance.vertex_count = vertex_count;
			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, sizeof(v3) * vertex_count, vertices, instance.buffer_type);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.element_array_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLindex), indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices
		}

		v3 transform_vertex(int id, v3 &vertex) {
			Instance &instance = instances[id];

			return multiply_perspective(instance.pose, vertex);
		}

		void render(int id, GLuint model_location) {
			Instance &instance = instances[id];

			glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat*)(instance.pose.m));

			glBindVertexArray(instance.vertex_array_object);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.element_array_buffer);
			glDrawElements(instance.draw_mode, instance.index_count, GL_INDEX, (void*)0);
		}
	};
}

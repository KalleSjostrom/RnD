enum RenderableDataType {
	RenderableDataType_Elements,
	RenderableDataType_Arrays,
};

struct Renderable {
	m4 pose;
	i32 index_count;
	RenderableDataType datatype;
	GLenum draw_mode; // e.g. GL_TRIANGLE_STRIP;
	GLuint element_array_buffer;
	GLuint vertex_array_object;
};

namespace model_component {
	struct Instance {
		Renderable renderable;

		GLenum buffer_type; // e.g. GL_STATIC_DRAW;
		GLuint vertex_buffer_object;

		i32 vertex_count;
		i32 __padding;
	};

	struct ModelComponent {
		Instance instances[8];
		i32 count;
		i32 __padding;
		i64 ___padding;

		i32 add(v3 position, GLindex *indices, i32 index_count, v3 *vertices, i32 vertex_count, GLenum buffer_type = GL_STATIC_DRAW, GLenum draw_mode = GL_TRIANGLE_STRIP) {
			ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
			i32 id = count++;
			Instance &instance = instances[id];
			Renderable &renderable = instance.renderable;

			renderable.pose = identity();
			set_position(id, position);

			renderable.index_count = index_count;
			instance.vertex_count = vertex_count;

			instance.buffer_type = buffer_type;
			renderable.draw_mode = draw_mode;

			glGenBuffers(1, &renderable.element_array_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.element_array_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (u32)index_count * sizeof(GLindex), indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices

			glGenBuffers(1, &instance.vertex_buffer_object);
			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, (u32)vertex_count * sizeof(v3), vertices, buffer_type);

			glGenVertexArrays(1, &renderable.vertex_array_object);
			glBindVertexArray(renderable.vertex_array_object);

			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			return id;
		}

		inline void set_position(i32 id, v3 position) {
			Instance &instance = instances[id];
			set_translation(instance.renderable.pose, position);
		}

		inline v3 get_position(i32 id) {
			Instance &instance = instances[id];
			return translation(instance.renderable.pose);
		}

		inline m4 &get_pose(i32 id) {
			Instance &instance = instances[id];
			return instance.renderable.pose;
		}

		inline void rotate_around(i32 id, float angle, float x, float y) {
			Instance &instance = instances[id];

			float ca = cosf(angle);
			float sa = sinf(angle);

			float rx = -ca*x + sa*y + x;
			float ry = -sa*x - ca*y + y;

			m4 rotation = identity();

			rotation.m[INDEX(0, 3)] = rx;
			rotation.m[INDEX(1, 3)] = ry;

			rotation.m[INDEX(0, 0)] = ca;
			rotation.m[INDEX(0, 1)] = -sa;
			rotation.m[INDEX(1, 0)] = sa;
			rotation.m[INDEX(1, 1)] = ca;

			instance.renderable.pose *= rotation;
		}

		inline void rotate(i32 id, float angle) {
			Instance &instance = instances[id];

			float ca = cosf(angle);
			float sa = sinf(angle);

			m4 rotation = identity();

			rotation.m[INDEX(0, 0)] = ca;
			rotation.m[INDEX(0, 1)] = -sa;
			rotation.m[INDEX(1, 0)] = sa;
			rotation.m[INDEX(1, 1)] = ca;

			instance.renderable.pose *= rotation;
		}

		inline void set_rotation(i32 id, float angle) {
			Instance &instance = instances[id];

			float ca = cosf(angle);
			float sa = sinf(angle);

			m4 &pose = instance.renderable.pose;

			pose.m[INDEX(0, 0)] = ca;
			pose.m[INDEX(0, 1)] = -sa;
			pose.m[INDEX(1, 0)] = sa;
			pose.m[INDEX(1, 1)] = ca;

			// instance.renderable.pose = rotation;
		}

		inline void set_scale(i32 id, v3 scale) {
			Instance &instance = instances[id];

			m4 &pose = instance.renderable.pose;

			pose.m[INDEX(0, 0)] = scale.x;
			pose.m[INDEX(1, 1)] = scale.y;
			pose.m[INDEX(2, 2)] = scale.z;
		}

		void update_vertices(i32 id, v3 *vertices) {
			Instance &instance = instances[id];

			ASSERT(instance.buffer_type == GL_DYNAMIC_DRAW, "Should not update vertices with static buffer type!");
			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, (u32)instance.vertex_count * sizeof(v3), vertices, GL_DYNAMIC_DRAW);
		}

		void update_vertices(i32 id, GLindex *indices, i32 index_count, v3 *vertices, i32 vertex_count) {
			Instance &instance = instances[id];

			ASSERT(instance.buffer_type == GL_DYNAMIC_DRAW, "Should not update vertices with static buffer type!");

			instance.renderable.index_count = index_count;
			instance.vertex_count = vertex_count;
			glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
			glBufferData(GL_ARRAY_BUFFER, (u32)vertex_count * sizeof(v3), vertices, instance.buffer_type);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.renderable.element_array_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (u32)index_count * sizeof(GLindex), indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices
		}

		v3 transform_vertex(i32 id, v3 &vertex) {
			Instance &instance = instances[id];

			return multiply_perspective(instance.renderable.pose, vertex);
		}

		void render(i32 id, GLint model_location) {
			Renderable &re = instances[id].renderable;

			glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat*)(re.pose.m));

			glBindVertexArray(re.vertex_array_object);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, re.element_array_buffer);
			glDrawElements(re.draw_mode, re.index_count, GL_UNSIGNED_SHORT, (void*)0);
		}
	};
}

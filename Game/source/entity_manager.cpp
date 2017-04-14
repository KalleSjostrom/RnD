static const short quad_vertex_indices[] = { 0, 1, 2, 3 };

void spawn_entity(Entity &entity, EntityType type, int program_id = -1, v3 position = V3(0,0,0)) {
	ComponentManager *components = (ComponentManager *)globals::components;

	entity.type = type;

	switch (type) {
		case EntityType_Avatar: {
			// Animation
			entity.animation_id = components->animation.add(&animation::walk::data);

			// Model
			v3 *vertices = CALL(entity, animation, get_skeleton_vertices);
			int vertex_count = CALL(entity, animation, get_skeleton_vertex_count);

			entity.model_id = components->model.add(position, skeleton_default_indices, ARRAY_COUNT(skeleton_default_indices), vertices, vertex_count, GL_DYNAMIC_DRAW, GL_LINES);
			components->add_to_program(program_id, entity.model_id);
		} break;
		case EntityType_BlockAvatar: {
			static v3 vertex_buffer_data[] = {
				{  0.0f,  0.0f, 0.0f },
				{ 32.0f,  0.0f, 0.0f },
				{  0.0f, 64.0f, 0.0f },
				{ 32.0f, 64.0f, 0.0f },
			};

			entity.mover_id = components->mover.add(position);

			// Model
			entity.model_id = components->model.add(position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
			components->add_to_program(program_id, entity.model_id);

			// Actor
			m4 &pose = components->model.instances[entity.model_id].pose;
			entity.actor_id = components->actor.add(ShapeType_AABox, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		case EntityType_Sphere: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
			};

			// Model
			entity.model_id = components->model.add(position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data), GL_STATIC_DRAW, GL_POINTS);
			components->add_to_program(program_id, entity.model_id);

			// Actor
			static v3 bounding_box[] = {
				{ -50.0f, -50.0f, 0.0f },
				{  50.0f, -50.0f, 0.0f },
				{  50.0f,  50.0f, 0.0f },
				{ -50.0f,  50.0f, 0.0f },
			};
			m4 &pose = components->model.instances[entity.model_id].pose;
			entity.actor_id = components->actor.add(ShapeType_Sphere, pose, bounding_box, ARRAY_COUNT(bounding_box));
		} break;
		case EntityType_Block: {
			static v3 vertex_buffer_data[] = {
				{   0.0f,   0.0f, 0.0f },
				{ 100.0f,   0.0f, 0.0f },
				{   0.0f, 100.0f, 0.0f },
				{ 100.0f, 100.0f, 0.0f },
			};

			// Model
			entity.model_id = components->model.add(position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
			components->add_to_program(program_id, entity.model_id);

			// Actor
			m4 &pose = components->model.instances[entity.model_id].pose;
			entity.actor_id = components->actor.add(ShapeType_Box, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		case EntityType_Ray: {
			static v3 vertex_buffer_data[] = {
				{   0.0f, 0.0f, 0.0f },
				{ 100.0f, 0.0f, 0.0f },
			};

			short vertex_indices[] = { 0, 1 };

			// Model
			entity.model_id = components->model.add(position, (GLindex*)vertex_indices, ARRAY_COUNT(vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data), GL_DYNAMIC_DRAW, GL_LINE_STRIP);
			components->add_to_program(program_id, entity.model_id);
		} break;
		case EntityType_Fullscreen: {
			static v3 vertex_buffer_data[] = {
				{ -1.0f, -1.0f, 0.0f },
				{  1.0f, -1.0f, 0.0f },
				{ -1.0f,  1.0f, 0.0f },
				{  1.0f,  1.0f, 0.0f },
			};

			// Model
			entity.model_id = components->model.add(position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		default: {
			ASSERT(false, "Unknown entity type! (type=%u)", type);
		} break;
	}
}
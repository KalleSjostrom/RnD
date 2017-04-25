#include "engine/components/model_component.cpp"
#include "engine/components/mover_component.cpp"
#include "engine/components/actor_component.cpp"

#include "engine/utils/renderer.cpp"

struct ComponentGroup {
	mover_component::MoverComponent mover;
	model_component::ModelComponent model;
	actor_component::ActorComponent actor;

	Renderer renderer;
	i32 __padding;
	i32 ___padding;
};

void update_components(ComponentGroup &component_group, float dt) {
	component_group.mover.update(dt);
}

struct Entity {
	u32 type;

	i32 mover_id;
	i32 model_id;
	i32 actor_id;
};

enum EntityType : u32 {
	EntityType_Block = 0,

	EntityType_Count,
};

static const short quad_vertex_indices[] = { 0, 1, 2, 3 };

void spawn_entity(ComponentGroup &components, Entity &entity, EntityType type, int program_id = -1, v3 position = V3(0,0,0)) {
	entity.type = type;

	switch (type) {
		case EntityType_Block: {
			static v3 vertex_buffer_data[] = {
				{   0.0f,   0.0f, 0.0f },
				{ 100.0f,   0.0f, 0.0f },
				{   0.0f, 100.0f, 0.0f },
				{ 100.0f, 100.0f, 0.0f },
			};

			// Model
			entity.model_id = components.model.add(position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
			add_to_program(components.renderer, program_id, &components.model.instances[entity.model_id].renderable);

			// Actor
			m4 &pose = components.model.instances[entity.model_id].renderable.pose;
			entity.actor_id = components.actor.add(ShapeType_Box, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		default: {
			ASSERT(false, "Unknown entity type! (type=%u)", type);
		} break;
	}
}


namespace component_glue {
	void update(ComponentGroup &components, Entity *entites, int count, float dt) {
		(void)dt;
		for (int i = 0; i < count; ++i) {
			Entity &entity = entites[i];
			switch (entity.type) {
				case EntityType_Block: {
					m4 &pose = GET(components, entity, model, renderable.pose);
					CALL(components, entity, actor, set_pose, pose);
				} break;
				default: {};
			}
		}
	}
}

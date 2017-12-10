typedef u16 cid;

#define GLSL(src) "#version 410\n" #src
typedef u32 GLindex;

#include "engine/components/input_component.cpp"
#include "engine/components/animation_component.cpp"
#include "engine/components/model_component.cpp"
#include "engine/components/mover_component.cpp"
#include "engine/components/actor_component.cpp"
#include "engine/components/fluid_component.cpp"
#include "engine/components/material_component.cpp"

#include "engine/utils/renderer.cpp"

#include "shaders/default.shader.cpp"
#include "shaders/avatar.shader.cpp"
#include "shaders/fluid.shader.cpp"
#include "shaders/model.shader.cpp"
#include "shaders/fullscreen_effects.shader.cpp"

struct Entity {
	u32 type;

	cid input_id;

	cid animation_id;
	cid mover_id;

	cid model_id;
	cid actor_id;
	cid fluid_id;
	cid material_id;
};

struct ComponentGroup {
	InputComponent input;

	AnimationComponent animation;
	MoverComponent mover;

	ModelComponent model;
	ActorComponent actor;
	FluidComponent fluid;
	MaterialComponent material;

	Renderer renderer;
	MemoryArena *arena;

	i32 entity_count;
	Entity entities[512];
};

struct Context {
	// InputContext *input;
	// AnimationContext *animation;
	// MoverContext *mover;
	ModelCC *model;
	// ActorContext *actor;
	MaterialCC *material;
};

void update_components(ComponentGroup &component_group, float dt) {
    component_group.input.update(dt);
	component_group.animation.update(dt);
	component_group.mover.update(dt);
	component_group.fluid.update(dt);
}

enum EntityType : u32 {
	EntityType_Avatar = 0,
	EntityType_BlockAvatar,
	EntityType_Sphere,
	EntityType_Block,
	EntityType_Fullscreen,
	EntityType_Plane,
	EntityType_Model,
	EntityType_Fluid,
	EntityType_Ruler,

	EntityType_Count,
};

// @move_me
enum RenderMask {
	RenderMask_ShadowCasters = 1 << 0,
	RenderMask_Lights = 1 << 1,
	RenderMask_Rest = 1 << 2,
	RenderMask_Fluid = 1 << 3,

	RenderMask_All = 0xFFFFFFFF,
};

enum ProgramType {
	ProgramType_default = 0,
	ProgramType_avatar,
	ProgramType_sphere,
	ProgramType_model,
	ProgramType_ray,
	ProgramType_fluid,
};
void setup_programs(ComponentGroup &components) {
	GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
	GLuint avatar_program = gl_program_builder::create_from_strings(shader_avatar::vertex, shader_avatar::fragment, shader_avatar::geometry);
	GLuint sphere_program = gl_program_builder::create_from_strings(shader_sphere::vertex, shader_sphere::fragment, shader_sphere::geometry);
	GLuint model_program = gl_program_builder::create_from_strings(shader_model::vertex, shader_model::fragment, 0);
	GLuint ray_program = gl_program_builder::create_from_strings(shader_ray::vertex, shader_ray::fragment, shader_ray::geometry);
	GLuint fluid_program = gl_program_builder::create_from_strings(fluid::vertex, fluid::fragment, 0);

	components.renderer.program_count = 0;

	add_program(components.renderer, default_program, RenderMask_ShadowCasters);
	add_program(components.renderer, avatar_program, RenderMask_ShadowCasters);
	add_program(components.renderer, sphere_program, RenderMask_ShadowCasters);
	add_program(components.renderer, model_program, RenderMask_ShadowCasters);
	add_program(components.renderer, ray_program, RenderMask_Rest);
	add_program(components.renderer, fluid_program, RenderMask_Fluid);
}

void reload_programs(ComponentGroup &components) {
	components.renderer.program_count = 0;
	setup_programs(components);
}

static const GLindex quad_vertex_indices[] = { 0, 1, 2, 3 };

v3 *animation__get_skeleton_vertices(ComponentGroup &components, Entity &entity) {
	return components.animation.get_skeleton_vertices(entity.animation_id);
}
i32 animation__get_skeleton_vertex_count(ComponentGroup &components, Entity &entity) {
	return components.animation.get_skeleton_vertex_count(entity.animation_id);
}
// void model__update_vertices(ComponentGroup &components, Entity &entity, v3 *vertices) {
// 	components.model.update_vertices(entity.model_id, vertices);
// }
void model__set_position(ComponentGroup &components, Entity &entity, v3 position) {
	components.model.set_position(entity.model_id, position);
}
m4 &model__get_pose(ComponentGroup &components, Entity &entity) {
	return components.model.get_pose(entity.model_id);
}
void actor__set_pose(ComponentGroup &components, Entity &entity, m4 &pose) {
	components.actor.set_pose(entity.actor_id, pose);
}
void model__set_rotation(ComponentGroup &components, Entity &entity, float angle) {
	components.model.set_rotation(entity.model_id, angle);
}
void model__set_scale(ComponentGroup &components, Entity &entity, v3 scale) {
	components.model.set_scale(entity.model_id, scale);
}
v3 &mover__get_position(ComponentGroup &components, Entity &entity) {
	return components.mover.movers[entity.mover_id].position;
}
v3 &mover__get_wanted_translation(ComponentGroup &components, Entity &entity) {
	return components.mover.movers[entity.mover_id].wanted_translation;
}
v3 &mover__get_velocity(ComponentGroup &components, Entity &entity) {
	return components.mover.movers[entity.mover_id].velocity;
}
void mover__set_velocity(ComponentGroup &components, Entity &entity, v3 &velocity) {
	components.mover.movers[entity.mover_id].velocity = velocity;
}
void mover__add_acceleration(ComponentGroup &components, Entity &entity, v3 &acceleration) {
	components.mover.add_acceleration(entity.mover_id, acceleration);
}
void mover__add_impulse(ComponentGroup &components, Entity &entity, v3 &impulse) {
	components.mover.add_impulse(entity.mover_id, impulse);
}
v3 input__get_move(ComponentGroup &components, Entity &entity) {
	return components.input.inputs[entity.input_id].move;
}
b32 input__get_jump(ComponentGroup &components, Entity &entity) {
	return components.input.inputs[entity.input_id].jump;
}

Entity *spawn_entity(EngineApi *engine, ComponentGroup &components, EntityType type, Context &context, v3 position = V3(0,0,0)) {
	Entity &entity = components.entities[components.entity_count++];
	entity.type = type;

	switch (type) {
		case EntityType_BlockAvatar: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 1.0f, 1.0f, 0.0f },
			};

			// Mover
			entity.input_id = components.input.add();

			// Mover
			entity.mover_id = components.mover.add(position);

			ModelCC model_cc = {};

			MeshData &md = model_cc.mesh_data;
			md.vertices = vertex_buffer_data;
			md.vertex_count = ARRAY_COUNT(vertex_buffer_data);

			md.group_count = 1;
			md.groups = PUSH_STRUCTS(*components.arena, 1, GroupData);
			md.groups[0].indices = (GLindex*) quad_vertex_indices;
			md.groups[0].index_count = ARRAY_COUNT(quad_vertex_indices);

			model_cc.buffer_type = GL_STATIC_DRAW;
			model_cc.draw_mode = GL_TRIANGLE_STRIP;

			// Model
			entity.model_id = components.model.add(engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_default, &components.model.instances[entity.model_id]);

			// Actor
			m4 &pose = components.model.instances[entity.model_id].pose;
			entity.actor_id = components.actor.add(ShapeType_AABox, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		case EntityType_Sphere: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
			};

			ModelCC model_cc = {};

			MeshData &md = model_cc.mesh_data;
			md.vertices = vertex_buffer_data;
			md.vertex_count = ARRAY_COUNT(vertex_buffer_data);

			md.group_count = 1;
			md.groups = PUSH_STRUCTS(*components.arena, 1, GroupData);
			md.groups[0].indices = (GLindex*) quad_vertex_indices;
			md.groups[0].index_count = 1;

			model_cc.buffer_type = GL_STATIC_DRAW;
			model_cc.draw_mode = GL_POINTS;

			// Model
			entity.model_id = components.model.add(engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_sphere, &components.model.instances[entity.model_id]);

			// Material
			entity.material_id = components.material.add(context.material);

			// Actor
			static v3 bounding_box[] = {
				{ -50.0f, -50.0f, 0.0f },
				{  50.0f, -50.0f, 0.0f },
				{  50.0f,  50.0f, 0.0f },
				{ -50.0f,  50.0f, 0.0f },
			};
			m4 &pose = components.model.instances[entity.model_id].pose;
			entity.actor_id = components.actor.add(ShapeType_Sphere, pose, bounding_box, ARRAY_COUNT(bounding_box));
		} break;
		case EntityType_Block: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 1.0f, 1.0f, 0.0f },
			};

			ModelCC model_cc = {};

			MeshData &md = model_cc.mesh_data;
			md.vertices = vertex_buffer_data;
			md.vertex_count = ARRAY_COUNT(vertex_buffer_data);

			md.group_count = 1;
			md.groups = PUSH_STRUCTS(*components.arena, 1, GroupData);
			md.groups[0].indices = (GLindex*) quad_vertex_indices;
			md.groups[0].index_count = ARRAY_COUNT(quad_vertex_indices);

			model_cc.buffer_type = GL_STATIC_DRAW;
			model_cc.draw_mode = GL_TRIANGLE_STRIP;

			// Model
			entity.model_id = components.model.add(engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_default, &components.model.instances[entity.model_id]);

			// Actor
			m4 &pose = components.model.instances[entity.model_id].pose;
			entity.actor_id = components.actor.add(ShapeType_Box, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		case EntityType_Fullscreen: {
			static v3 vertex_buffer_data[] = {
				{ -1.0f, -1.0f, 0.0f },
				{  1.0f, -1.0f, 0.0f },
				{ -1.0f,  1.0f, 0.0f },
				{  1.0f,  1.0f, 0.0f },
			};

			ModelCC model_cc = {};

			MeshData &md = model_cc.mesh_data;
			md.vertices = vertex_buffer_data;
			md.vertex_count = ARRAY_COUNT(vertex_buffer_data);

			md.group_count = 1;
			md.groups = PUSH_STRUCTS(*components.arena, 1, GroupData);
			md.groups[0].indices = (GLindex*) quad_vertex_indices;
			md.groups[0].index_count = ARRAY_COUNT(quad_vertex_indices);

			model_cc.buffer_type = GL_STATIC_DRAW;
			model_cc.draw_mode = GL_TRIANGLE_STRIP;

			// Model
			entity.model_id = components.model.add(engine, *components.arena, position, &model_cc);
		} break;

		case EntityType_Model: {
			ASSERT(context.model, "Cannot create a model without creation context");

			// Model
			entity.model_id = components.model.add(engine, *components.arena, position, context.model);
			add_to_program(components.renderer, context.model->program_type, &components.model.instances[entity.model_id]);
		} break;
		case EntityType_Ruler: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 2.0f, 0.0f },
			};

			ModelCC model_cc = {};

			MeshData &md = model_cc.mesh_data;
			md.vertices = vertex_buffer_data;
			md.vertex_count = ARRAY_COUNT(vertex_buffer_data);

			md.group_count = 1;
			md.groups = PUSH_STRUCTS(*components.arena, 1, GroupData);
			md.groups[0].indices = (GLindex*) quad_vertex_indices;
			md.groups[0].index_count = 2;

			model_cc.buffer_type = GL_DYNAMIC_DRAW;
			model_cc.draw_mode = GL_LINE_STRIP;

			// Model
			entity.model_id = components.model.add(engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_default, &components.model.instances[entity.model_id]);
		} break;
		case EntityType_Plane: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
			};

			// Model
			// entity.model_id = components.model.add(position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
			add_to_program(components.renderer, ProgramType_default, &components.model.instances[entity.model_id]);

			// Material
			entity.material_id = components.material.add(context.material); // V3(0.5f, 0.5f, 0.5f), V3(0, 0, 0), 1);

			// Actor
			// m4 &pose = components.model.instances[entity.model_id].renderable.pose;
			// entity.actor_id = components.actor.add(ShapeType_AABox, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		case EntityType_Fluid: {
			entity.fluid_id = components.fluid.add(*components.arena);
			add_to_program(components.renderer, ProgramType_fluid, &components.fluid.instances[entity.fluid_id].renderable);
		} break;
		default: {
			ASSERT(false, "Unknown entity type! (type=%u)", type);
		} break;
	}
	return &entity;
}
namespace component_glue {
	void update(ComponentGroup &components, float dt) {
		(void)dt;
		for (i32 i = 0; i < components.entity_count; ++i) {
			Entity &entity = components.entities[i];
			switch (entity.type) {
				case EntityType_Avatar: {
					// v3 *vertices = animation__get_skeleton_vertices(components, entity);
					// model__update_vertices(components, entity, vertices);
				} break;
				case EntityType_BlockAvatar: {
					v3 move = input__get_move(components, entity) * 10;
					mover__add_acceleration(components, entity, move);

					if (input__get_jump(components, entity)) {
						v3 up = V3(0, 200, 0);
						mover__add_impulse(components, entity, up);
					}

					v3 &wanted_translation = mover__get_wanted_translation(components, entity);
					SweepResults result = components.actor.sweep(entity.actor_id, wanted_translation);

					v3 &position = mover__get_position(components, entity);
					position += result.constrained_translation;

					model__set_position(components, entity, position);

					m4 &pose = model__get_pose(components, entity);
					actor__set_pose(components, entity, pose);

					if (result.id) {
						v3 &velocity = mover__get_velocity(components, entity);
						if (velocity.y <= 0)
							velocity.y = 0;
					}
				} break;
				case EntityType_Block: {
					m4 &pose = model__get_pose(components, entity);
					actor__set_pose(components, entity, pose);
				} break;
				default: {};
			}
		}
	}
}

typedef u16 cid;

#define GLSL(src) "#version 410\n" #src
typedef u32 GLindex;

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
	EntityType_Roomba,
	EntityType_Line,

	EntityType_Count,
};

struct Entity {
	EntityType type;

	cid input_id;

	cid animation_id;
	cid mover_id;

	cid model_id;
	cid actor_id;
	cid fluid_id;
	cid material_id;
};

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
#include "shaders/roomba.shader.cpp"

struct ComponentGroup {
	InputComponent input;

	AnimationComponent animation;
	MoverComponent mover;

	ModelComponent model;
	ActorComponent actor;
	FluidComponent fluid;
	MaterialComponent material;

	Renderer renderer;
	Allocator *allocator;

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
    update(component_group.input, dt);
	update(component_group.animation, dt);
	update(component_group.mover, dt);
	update(component_group.fluid, dt);
}

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
	ProgramType_roomba,
	ProgramType_line,
};
void setup_programs(ComponentGroup &components) {
	GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
	GLuint avatar_program = gl_program_builder::create_from_strings(shader_avatar::vertex, shader_avatar::fragment, shader_avatar::geometry);
	GLuint sphere_program = gl_program_builder::create_from_strings(shader_sphere::vertex, shader_sphere::fragment, shader_sphere::geometry);
	GLuint model_program = gl_program_builder::create_from_strings(shader_model::vertex, shader_model::fragment, 0);
	GLuint ray_program = gl_program_builder::create_from_strings(shader_ray::vertex, shader_ray::fragment, shader_ray::geometry);
	GLuint fluid_program = gl_program_builder::create_from_strings(fluid::vertex, fluid::fragment, 0);
	GLuint roomba_program = gl_program_builder::create_from_strings(shader_roomba::vertex, shader_roomba::fragment, shader_roomba::geometry);
	GLuint line_program = gl_program_builder::create_from_strings(shader_line::vertex, shader_line::fragment, shader_line::geometry);

	components.renderer.program_count = 0;

	add_program(components.renderer, default_program, RenderMask_ShadowCasters);
	add_program(components.renderer, avatar_program, RenderMask_ShadowCasters);
	add_program(components.renderer, sphere_program, RenderMask_ShadowCasters);
	add_program(components.renderer, model_program, RenderMask_ShadowCasters);
	add_program(components.renderer, ray_program, RenderMask_Rest);
	add_program(components.renderer, fluid_program, RenderMask_Fluid);
	add_program(components.renderer, roomba_program, RenderMask_Rest);
	add_program(components.renderer, line_program, RenderMask_Rest);
}

void reload_programs(ComponentGroup &components) {
	components.renderer.program_count = 0;
	setup_programs(components);
}

static const GLindex quad_vertex_indices[] = { 0, 1, 2, 3 };

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

			// Input
			add(components.input, entity);

			// Mover
			add(components.mover, entity, position);

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
			add(components.model, entity, engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_default, get_model(components.model, entity));

			// Actor
			m4 &pose = components.model.models[entity.model_id].pose;
			add(components.actor, entity, ShapeType_AABox, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
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
			add(components.model, entity, engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_sphere, get_model(components.model, entity));

			// Material
			add(components.material, entity, context.material);

			// Actor
			static v3 bounding_box[] = {
				{ -50.0f, -50.0f, 0.0f },
				{  50.0f, -50.0f, 0.0f },
				{  50.0f,  50.0f, 0.0f },
				{ -50.0f,  50.0f, 0.0f },
			};
			m4 &pose = get_pose(components.model, entity);
			add(components.actor, entity, ShapeType_Sphere, pose, bounding_box, ARRAY_COUNT(bounding_box));
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
			add(components.model, entity, engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_default, get_model(components.model, entity));

			// Actor
			m4 &pose = get_pose(components.model, entity);
			add(components.actor, entity, ShapeType_Box, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
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
			add(components.model, entity, engine, *components.arena, position, &model_cc);
		} break;

		case EntityType_Model: {
			ASSERT(context.model, "Cannot create a model without creation context");

			// Model
			add(components.model, entity, engine, *components.arena, position, context.model);
			add_to_program(components.renderer, context.model->program_type, get_model(components.model, entity));
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
			add(components.model, entity, engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_line, get_model(components.model, entity));
		} break;
		case EntityType_Roomba: {
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
			add(components.model, entity, engine, *components.arena, position, &model_cc);
			add_to_program(components.renderer, ProgramType_roomba, get_model(components.model, entity));

			// Material
			// add(components.material, entity, context.material);

			// Actor
			static v3 bounding_box[] = {
				{ -50.0f, -50.0f, 0.0f },
				{  50.0f, -50.0f, 0.0f },
				{  50.0f,  50.0f, 0.0f },
				{ -50.0f,  50.0f, 0.0f },
			};
			m4 &pose = get_pose(components.model, entity);
			add(components.actor, entity, ShapeType_Sphere, pose, bounding_box, ARRAY_COUNT(bounding_box));
		} break;
		case EntityType_Plane: {
			static v3 vertex_buffer_data[] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
			};

			// Model
			// add(components.model, entity, position, (GLindex*)quad_vertex_indices, ARRAY_COUNT(quad_vertex_indices), vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
			add_to_program(components.renderer, ProgramType_default, get_model(components.model, entity));

			// Material
			add(components.material, entity, context.material); // V3(0.5f, 0.5f, 0.5f), V3(0, 0, 0), 1);

			// Actor
			m4 &pose = get_pose(components.model, entity);
			add(components.actor, entity, ShapeType_AABox, pose, vertex_buffer_data, ARRAY_COUNT(vertex_buffer_data));
		} break;
		case EntityType_Fluid: {
			add(components.fluid, entity, *components.arena);
			add_to_program(components.renderer, ProgramType_fluid, &components.fluid.fluids[entity.fluid_id].renderable);
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
					v3 move = get_move(components.input, entity) * 10;
					add_acceleration(components.mover, entity, move);

					if (get_jump(components.input, entity)) {
						v3 up = V3(0, 200, 0);
						add_impulse(components.mover, entity, up);
					}

					v3 &wanted_translation = get_wanted_translation(components.mover, entity);
					SweepResults result = sweep(components.actor, entity, wanted_translation);

					v3 &position = get_position(components.mover, entity);
					position += result.constrained_translation;

					m4 &pose = get_pose(components.model, entity);
					translation(pose) = position;

					set_pose(components.actor, entity, pose);

					if (result.id) {
						v3 &velocity = get_velocity(components.mover, entity);
						if (velocity.y <= 0)
							velocity.y = 0;
					}
				} break;
				case EntityType_Block: {
					m4 &pose = get_pose(components.model, entity);
					set_pose(components.actor, entity, pose);
				} break;
				default: {};
			}
		}
	}
}

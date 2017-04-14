#include "components/animation_component.cpp"
#include "components/model_component.cpp"
#include "components/mover_component.cpp"
#include "components/actor_component.cpp"

enum RenderMask {
	RenderMask_ShadowCasters = 1 << 0,
	RenderMask_Lights = 1 << 1,
	RenderMask_Rest = 1 << 2,

	RenderMask_All = 0xFFFFFFFF,
};

struct Program {
	GLuint program;
	GLuint model_location;
	GLuint view_projection_location;

	int count;
	int model_ids[8];

	RenderMask render_mask;
};

struct ComponentManager {
	animation_component::AnimationComponent animation;
	model_component::ModelComponent model;
	mover_component::MoverComponent mover;
	actor_component::ActorComponent actor;

	int program_count;
	Program programs[4];

	void update(float dt) {
		animation.update(dt);
		mover.update(dt);
	}

	int add_program(GLuint shader_program, RenderMask render_mask) {
		ASSERT(program_count < ARRAY_COUNT(programs), "Too many programs added!");
		int id = program_count++;
		Program &p = programs[id];
		p.program = shader_program;
		glUseProgram(shader_program);
		p.model_location = glGetUniformLocation(shader_program, "model");
		p.view_projection_location = glGetUniformLocation(shader_program, "view_projection");
		p.render_mask = render_mask;
		return id;
	}

	void add_to_program(int id, GLuint model_id) {
		Program &p = programs[id];
		p.model_ids[p.count++] = model_id;
	}

	void render(Camera &camera, RenderMask render_mask = RenderMask_All) {
		for (int i = 0; i < program_count; ++i) {
			Program &p = programs[i];
			glUseProgram(p.program);
			begin_frame(camera, p.view_projection_location);

			if (p.render_mask & render_mask) {
				for (int i = 0; i < p.count; ++i) {
					model.render(p.model_ids[i], p.model_location);
				}
			}
		}
	}
};

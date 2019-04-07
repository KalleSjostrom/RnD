struct FluidPlugin;

#define PLUGIN_DATA FluidPlugin*

#define DATA_FOLDER "../../game/out/data"
#define ASSET_FOLDER ""

#define CL_ERROR_CHECKING 1

#define FEATURE_RELOAD
#define SYSTEM_OPENGL
#define SYSTEM_AUDIO
#define SYSTEM_GRAPHICS
#define SYSTEM_COMPONENTS
#include "systems.h"
#include "render_pipe.cpp"
#include "levels.cpp"

struct FluidPlugin {
	RenderPipe render_pipe;
};

void plugin_setup(Application *application) {
	FluidPlugin *fluid = (FluidPlugin *)allocate(application->allocator, sizeof(FluidPlugin));
	application->plugin_data = fluid;

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(12);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];
		Entity *entity = spawn_entity(application->engine, application->components, data.type, data.context, data.offset);

		Matrix4x4 &pose = get_pose(application->components.model, *entity);
		translation(pose) = data.offset;
		set_rotation(pose, data.rotation);
		set_scale(pose, data.size);
	}

	i32 screen_width, screen_height;
	application->engine->screen_dimensions(screen_width, screen_height);
	setup_render_pipe(application->engine, fluid->render_pipe, application->components, screen_width, screen_height);
}

void plugin_reloaded(Application *application) {
	// Plugin &application->= *(Plugin*) memory;

	// setup_gl();

	// ArenaAllocator empty = {};
	// application->transient_arena = empty;
	// reset_arena(application->transient_arena, MB);
	// globals::transient_arena = &application->transient_arena;

	// reload_programs(application->components);
	// setup_render_pipe(application->engine, application->render_pipe, application->components, screen_width, screen_height);
	// application->components.input.set_input_data(&input);

	// Level level = make_level();
	// for (i32 i = 0; i < level.count; ++i) {
	// 	EntityData &data = level.entity_data[i];

	// 	Entity *entity = 0;
	// 	if (i < application->entity_count) {
	// 		entity = application->entities + i;
	// 	} else {
	// 		entity = application->entities + application->entity_count++;
	// 	}

	// 	model__set_position(application->components, *entity, data.offset);
	// 	model__set_rotation(application->components, *entity, data.rotation);
	// 	model__set_scale(application->components, *entity, data.size);
	// }
	// application->entity_count = level.count;
}

void plugin_update(Application *application, float dt) {
	if (is_held(*application->components.input.input_data, InputKey_Space)) {
		Entity &entity = application->components.entities[application->components.entity_count-1];
		rotate(application->components.fluid, entity, dt);
	}
}

void plugin_render(Application *application) {
	FluidPlugin *fp = (FluidPlugin*)application->plugin_data;
	render(fp->render_pipe, application->components, application->camera);
}

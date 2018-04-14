struct Conetracer;
#define RELOAD_ENTRY_POINT Conetracer

#define SYSTEM_OPENGL
#define SYSTEM_GRAPHICS
#define SYSTEM_COMPONENTS
#include "engine/systems.h"

#include "shaders/voxel_cone_tracing.cpp"

#include "render_pipe.cpp"
#include "levels.cpp"
#include "engine/utils/obj_reader.cpp"

struct Conetracer {
	RenderPipe render_pipe;
	Random random;
};

void plugin_reloaded(Application &application) {
}

#define DATA_FOLDER "../../conetracer/out/data"
#define ASSET_FOLDER "../../conetracer/assets"

void plugin_setup(Application &application) {
	Conetracer &conetracer = *PUSH_STRUCT(application.persistent_arena, Conetracer);
	application.user_data = &conetracer;

	conetracer.random = {};
	random_init(conetracer.random, rdtsc(), 54u);

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];
		Entity *entity = spawn_entity(application.engine, application.components, data.type, data.context, data.offset);

		m4 &pose = get_pose(application.components.model, *entity);
		translation(pose) = data.offset;
		set_rotation(pose, data.rotation);
		set_scale(pose, data.size);
	}
	{
		Context c = {};
		ModelCC model_cc = load_model(application.persistent_arena, DATA_FOLDER"/sponza.cobj");
		c.model = &model_cc;
		Entity *entity = spawn_entity(application.engine, application.components, EntityType_Model, c, V3(0, 0, 0));
		(void)entity;
	}


	i32 screen_width, screen_height;
	application.engine->screen_dimensions(screen_width, screen_height);
	setup_render_pipe(application.persistent_arena, application.engine, conetracer.render_pipe, application.components, screen_width, screen_height);
}

void plugin_update(Application &application, float dt) {
	InputData &input = *application.components.input.input_data;
	float translation_speed = 128.0f;
	float rotation_speed = 2.5f;
	move(application.camera, input, translation_speed, rotation_speed, dt);
}
void plugin_render(Application &application) {
	Conetracer &conetracer = *(Conetracer*)application.user_data;
	render(conetracer.render_pipe, application.components, application.camera);
}

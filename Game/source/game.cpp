struct Game;

#define PLUGIN_DATA Game*

#define DATA_FOLDER "../../game/out/data"
#define ASSET_FOLDER "../../game/assets"

#define FEATURE_RELOAD
#define SYSTEM_OPENGL
#define SYSTEM_AUDIO
#define SYSTEM_GRAPHICS
#define SYSTEM_COMPONENTS
#include "systems.h"
#include "render_pipe.cpp"
#include "levels.cpp"

struct Game {
	RenderPipe render_pipe;
};

struct MemoryGarbage {
	uint8_t garbage[128];
};

void plugin_setup(Application *application) {
	Game *game = (Game *)allocate(application->allocator, sizeof(Game));
	application->plugin_data = game;

	MemoryGarbage *mg = (MemoryGarbage *)allocate(application->allocator, sizeof(MemoryGarbage));
	for (uint8_t i = 0; i < 128; i++) {
		mg->garbage[i] = i;
	}

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);

	Level level = make_level(*application);
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
	setup_render_pipe(application->engine, game->render_pipe, application->components, screen_width, screen_height);

	application->audio_manager.play(application->engine, "../../game/assets/test.wav");
}

void plugin_reloaded(Application *application) {
}

void plugin_update(Application *application, float dt) {
	if (is_pressed(*application->components.input.input_data, InputKey_MouseLeft)) {
		Matrix4x4 &pose = get_pose(application->components.model, application->components.entities[0]);

		Vector3 &x = *(Vector3*)(pose.m + 0);
		Vector3 &y = *(Vector3*)(pose.m + 4);
		Vector3 &z = *(Vector3*)(pose.m + 8);

		Quaternion q = quaternion(vector3(0, 1, 0), 2*dt);

		x = ::rotate_around(q, x);
		y = ::rotate_around(q, y);
		z = ::rotate_around(q, z);
	}
}

void plugin_render(Application *application) {
	Game *game = (Game*)application->plugin_data;
	render(game->render_pipe, application->components, application->camera);
}

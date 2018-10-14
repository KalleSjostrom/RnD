struct Game;
#define RELOAD_ENTRY_POINT Game

#define ASSERT(...)

#define DATA_FOLDER "../../game/out/data"
#define ASSET_FOLDER "../../game/assets"

#define SYSTEM_OPENGL
#define SYSTEM_AUDIO
#define SYSTEM_GRAPHICS
#define SYSTEM_COMPONENTS
#include "engine/systems.h"
#include "render_pipe.cpp"
#include "levels.cpp"

struct Game {
	RenderPipe render_pipe;
};

void plugin_setup(Application &application) {
	Game &game = *PUSH_STRUCT(application.persistent_arena, Game);
	application.user_data = &game;

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);

	Level level = make_level(application);
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];
		Entity *entity = spawn_entity(application.engine, application.components, data.type, data.context, data.offset);

		m4 &pose = get_pose(application.components.model, *entity);
		translation(pose) = data.offset;
		set_rotation(pose, data.rotation);
		set_scale(pose, data.size);
	}

	i32 screen_width, screen_height;
	application.engine->screen_dimensions(screen_width, screen_height);
	setup_render_pipe(application.engine, game.render_pipe, application.components, screen_width, screen_height);

	application.audio_manager.play(application.engine, "../../game/assets/test.wav");
}

void plugin_update(Application &application, float dt) {
	// if (IS_PRESSED(*application.components.input.input_data, InputKey_MouseLeft)) {
		// m4 &pose = get_pose(application.components.model, application.components.entities[0]);

		// v3 &x = *(v3*)(pose.m + 0);
		// v3 &y = *(v3*)(pose.m + 4);
		// v3 &z = *(v3*)(pose.m + 8);

		// q4 q = Quaternion(V3(0, 1, 0), dt);

		// x = ::rotate_around(q, x);
		// y = ::rotate_around(q, y);
		// z = ::rotate_around(q, z);
	// }
}

void plugin_render(Application &application) {
	Game &game = *(Game*)application.user_data;
	render(game.render_pipe, application.components, application.camera);
}

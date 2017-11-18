#include "includes.h"
#include "levels.cpp"

struct Application {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;

	EngineApi *engine;

	b32 initialized;

	i32 entity_count;
	Entity entities[512];
};

EXPORT PLUGIN_RELOAD(reload) {
	Application &application = *(Application*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
	#endif

	MemoryArena empty = {};
	application.transient_arena = empty;
	reset_arena(application.transient_arena, MB);
	globals::transient_arena = &application.transient_arena;

	reload_programs(application.components);
	setup_render_pipe(application.engine, application.render_pipe, application.components, screen_width, screen_height);
	application.components.input.set_input_data(&input);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];

		Entity *entity = 0;
		if (i < application.entity_count) {
			entity = application.entities + i;
		} else {
			entity = application.entities + application.entity_count++;
		}

		model__set_position(application.components, *entity, data.offset);
		model__set_rotation(application.components, *entity, data.rotation);
		model__set_scale(application.components, *entity, data.size);
	}
	application.entity_count = level.count;
}

EXPORT PLUGIN_UPDATE(update) {
	Application &application = *(Application*) memory;

	if (!application.initialized) {
		application.initialized = true;

		#ifdef OS_WINDOWS
			setup_gl();
		#endif
		MemoryArena empty = {};
		application.persistent_arena = empty;
		application.transient_arena = empty;
		setup_arena(application.transient_arena, MB);
		globals::transient_arena = &application.transient_arena;

		application.engine = &engine;

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		setup_programs(application.components);
		setup_render_pipe(application.engine, application.render_pipe, application.components, screen_width, screen_height);
		application.components.input.set_input_data(&input);

		Level level = make_level();
		for (i32 i = 0; i < level.count; ++i) {
			EntityData &data = level.entity_data[i];
			Entity &entity = application.entities[application.entity_count++];

			spawn_entity(application.components, entity, data.type, data.context, data.offset);

			model__set_position(application.components, entity, data.offset);
			model__set_rotation(application.components, entity, data.rotation);
			model__set_scale(application.components, entity, data.size);
		}

		// 	// Audio
		// 	application.audio_manager.play(application.engine, "../../application/assets/test.wav");
		setup_camera(application.camera, V3(0, 0, 500), ASPECT_RATIO);
	}

	{ // Update the application
		// Update all the components
		update_components(application.components, dt);
		// Handle component/component communication.
		component_glue::update(application.components, application.entities, application.entity_count, dt);
		// Update sound
		application.audio_manager.update(*globals::transient_arena, application.engine, dt);
	}

	{ // Render
		render(application.render_pipe, application.components, application.camera);
	}

	reset_transient_memory(*globals::transient_arena);
	return 0;
}

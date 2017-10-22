#define CL_ERROR_CHECKING 1

#include "includes.h"
#include "levels.cpp"

#include "../generated/fluid_kernel.generated.cpp"
#include "opencl/cl_manager.cpp"
#include "fluid_.cpp"

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

	cl_manager::ClInfo info;
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
	application.components.input.set_input(&input);

	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];

		Entity *entity = 0;
		if (i < application.entity_count) {
			entity = application.entities + i;
		} else {
			entity = application.entities + application.entity_count++;
		}

		model__set_position(application.components, *entity, V3(data.x, data.y, 0));
		model__set_rotation(application.components, *entity, data.rotation);
		model__set_scale(application.components, *entity, V3(data.w, data.h, 0));
	}
	application.entity_count = level.count;
}

EXPORT PLUGIN_UPDATE(update) {
	Application &application = *(Application*) memory;

	// unsigned flag = detect_cpu_features();

	if (!application.initialized) {
		application.initialized = true;

		#ifdef OS_WINDOWS
			setup_gl();
			setup_cl();
		#endif
		MemoryArena empty = {};
		application.persistent_arena = empty;
		application.transient_arena = empty;
		setup_arena(application.transient_arena, MB);
		globals::transient_arena = &application.transient_arena;

		application.engine = &engine;

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		setup_programs(application.components);
		setup_render_pipe(application.engine, application.render_pipe, application.components, screen_width, screen_height);
		application.components.input.set_input(&input);

		application.info = cl_manager::init(application.transient_arena);
		cl_manager::ClInfo &info = application.info;
		cl_manager::create_program_and_kernels(application.transient_arena, info);
		cl_manager::setup_buffers(info);
		cl_manager::setup_kernels(info);

		application.components.fluid.set_vbos(info.buffers[BufferIndex__positions].vbo, info.buffers[BufferIndex__density_pressure].vbo);

		for (i32 i = 0; i < level.count; ++i) {
			EntityData &data = level.entity_data[i];
			Entity &entity = application.entities[application.entity_count++];

			spawn_entity(application.components, entity, data.type, V3(data.x, data.y, 0));

			model__set_position(application.components, entity, V3(data.x, data.y, 0));
			model__set_rotation(application.components, entity, data.rotation);
			model__set_scale(application.components, entity, V3(data.w, data.h, 0));
		}

		// 	// Audio
		// 	application.audio_manager.play(application.engine, "../../application/assets/test.wav");
		setup_camera(application.camera, V3(0, 0, 500), ASPECT_RATIO);
	}

	run_fluid(application.info);

	{ // Update the application
		// Update all the components
		update_components(application.components, dt);
		// Handle component/component communication.
		component_glue::update(application.components, application.entities, application.entity_count, dt);
		// Update sound
		application.audio_manager.update(*globals::transient_arena, application.engine, dt);
	}

	{ // Render
		//// Render pipe ////
		render(application.render_pipe, application.components, application.camera);
	}

	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// glUseProgram(am.shader_program);
	// glBindVertexArray(am.vao);
	// glDrawArrays(GL_POINTS, 0, NR_PARTICLES);


	// globals::transient_arena.offset = 0;
	return 0;
}

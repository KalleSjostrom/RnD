#include "includes.h"

#include "shaders/voxel_cone_tracing.cpp"

#include "levels.cpp"
#include "engine/utils/math/random.h"
#include "engine/utils/profiler.c"
#include "engine/utils/obj_reader.cpp"

struct Application {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;
	Random random;

	EngineApi *engine;

	b32 initialized;

	i32 entity_count;
	Entity entities[512];
};

EXPORT PLUGIN_RELOAD(reload) {
	Application &application = *(Application*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
		setup_cl();
	#endif

	MemoryArena empty = {};
	application.transient_arena = empty;
	reset_arena(application.transient_arena, MB);
	globals::transient_arena = &application.transient_arena;

	application.components.arena = &application.persistent_arena;

	reload_programs(application.components);
	setup_render_pipe(application.persistent_arena, application.engine, application.render_pipe, application.components, screen_width, screen_height);
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

#define DATA_FOLDER "../../conetracer/out/data/"

ModelCC load_model(MemoryArena &arena, const char *path) {
	MeshData mesh_data = read_obj(arena, path);
	ModelCC model_cc = {};
	model_cc.mesh_data = mesh_data;

	model_cc.buffer_type = GL_STATIC_DRAW;
	model_cc.draw_mode = GL_TRIANGLES;

	model_cc.program_type = ProgramType_model;

	return model_cc;
}

EXPORT PLUGIN_UPDATE(update) {
	Application &application = *(Application*) memory;

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

		application.components.arena = &application.persistent_arena;

		application.engine = &engine;
		application.random = {};
		random_init(application.random, rdtsc(), 54u);

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);

		setup_programs(application.components);
		setup_render_pipe(application.persistent_arena, application.engine, application.render_pipe, application.components, screen_width, screen_height);
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

		{
			Context c = {};
			ModelCC model_cc = load_model(application.persistent_arena, DATA_FOLDER"cube.cobj");
			c.model = &model_cc;
			Entity &entity = application.entities[application.entity_count++];
			spawn_entity(application.components, entity, EntityType_Model, c, V3(0, 0, 0));
		}

		// Audio
		//	application.audio_manager.play(application.engine, "../../application/assets/test.wav");
		setup_camera(application.camera, V3(0, 0, 10), ASPECT_RATIO);
	}

	{
		v2 m = {};
		if (IS_HELD(input, InputKey_A)) {
			m.x = -1;
		}
		if (IS_HELD(input, InputKey_S)) {
			m.y = -1;
		}
		if (IS_HELD(input, InputKey_D)) {
			m.x = 1;
		}
		if (IS_HELD(input, InputKey_W)) {
			m.y = 1;
		}

		float camera_speed = 64.0f;
		float camera_rotation_speed = 0.5f;

		if (IS_HELD(input, InputKey_Shift)) {
			camera_speed *= 2;
		}

		m4 &pose = application.camera.pose;

		v3 &x = *(v3*)(pose.m + 0);
		v3 &y = *(v3*)(pose.m + 4);
		v3 &z = *(v3*)(pose.m + 8);
		v3 &position = translation(pose);

		position += x * (m.x * dt * camera_speed);
		position += z * (m.y * dt * camera_speed);

		if (IS_HELD(input, InputKey_MouseLeft)) {
			v3 world_up = V3(0, 1, 0);
			q4 qx = Quaternion(world_up, -input.mouse_xrel * dt * camera_rotation_speed);
			q4 qy = Quaternion(x, -input.mouse_yrel * dt * camera_rotation_speed);
			q4 q = qx * qy;

			x = ::rotate_around(q, x);
			y = ::rotate_around(q, y);
			z = ::rotate_around(q, z);
		}

		update_view(application.camera);
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

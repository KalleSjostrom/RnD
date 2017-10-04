#include "includes.h"

struct Game {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;

	EngineApi *engine;

	b32 initialized;

	i32 default_program_id;
	i32 avatar_program_id;
	i32 sphere_program_id;
	i32 ray_program_id;

	i32 entity_count;
	Entity entities[8];
};

#define TRANSIENT_ARENA_SIZE (MB*32)
// #define PERSISTENT_ARENA_SIZE (MB*32)

EXPORT PLUGIN_RELOAD(reload) {
	Game &game = *(Game*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
	#endif

	globals::transient_arena = &game.transient_arena;
	reset_arena(game.transient_arena, TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.

	globals::components = &game.components;

	GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
	GLuint avatar_program = gl_program_builder::create_from_strings(shader_avatar::vertex, shader_avatar::fragment, shader_avatar::geometry);
	GLuint sphere_program = gl_program_builder::create_from_strings(shader_light::vertex, shader_light::fragment, shader_light::geometry);
	GLuint ray_program = gl_program_builder::create_from_strings(shader_ray::vertex, shader_ray::fragment, shader_ray::geometry);

	Renderer &renderer = game.components.renderer;

	Program &dp = renderer.programs[game.default_program_id];
	Program &ap = renderer.programs[game.avatar_program_id];
	Program &sp = renderer.programs[game.sphere_program_id];
	Program &rp = renderer.programs[game.ray_program_id];

	dp.program = default_program;
	dp.view_projection_location = glGetUniformLocation(default_program, "view_projection");
	dp.model_location = glGetUniformLocation(default_program, "model");

	ap.program = avatar_program;
	ap.view_projection_location = glGetUniformLocation(avatar_program, "view_projection");
	ap.model_location = glGetUniformLocation(avatar_program, "model");

	sp.program = sphere_program;
	sp.view_projection_location = glGetUniformLocation(sphere_program, "view_projection");
	sp.model_location = glGetUniformLocation(sphere_program, "model");

	rp.program = ray_program;
	rp.view_projection_location = glGetUniformLocation(ray_program, "view_projection");
	rp.model_location = glGetUniformLocation(ray_program, "model");

	setup(game.engine, game.render_pipe, screen_width, screen_height);
}

static float time = 0;

EXPORT PLUGIN_UPDATE(update) {
	Game &game = *(Game*) memory;

	if (!game.initialized) {
		#ifdef OS_WINDOWS
			setup_gl();
		#endif
		MemoryArena ma = {};
		game.persistent_arena = ma; // init_from_existing((char*)memory + sizeof(Game), PERSISTENT_ARENA_SIZE);
		setup_arena(game.transient_arena, TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.
	}

	TempAllocator ta(&game.transient_arena);
	globals::transient_arena = &game.transient_arena;

	if (!game.initialized) {
		game.initialized = true;

		globals::components = &game.components;
		game.engine = &engine;

		GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
		GLuint avatar_program = gl_program_builder::create_from_strings(shader_avatar::vertex, shader_avatar::fragment, shader_avatar::geometry);
		GLuint sphere_program = gl_program_builder::create_from_strings(shader_sphere::vertex, shader_sphere::fragment, shader_sphere::geometry);
		GLuint ray_program = gl_program_builder::create_from_strings(shader_ray::vertex, shader_ray::fragment, shader_ray::geometry);

		Renderer &renderer = game.components.renderer;

		game.default_program_id = add_program(renderer, default_program, RenderMask_ShadowCasters);
		game.avatar_program_id = add_program(renderer, avatar_program, RenderMask_ShadowCasters);
		game.sphere_program_id = add_program(renderer, sphere_program, RenderMask_ShadowCasters);
		game.ray_program_id = add_program(renderer, ray_program, RenderMask_Rest);

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		setup(game.engine, game.render_pipe, screen_width, screen_height);

		// spawn_entity(game.entities[game.entity_count++], EntityType_Avatar, game.avatar_program_id, V3(-400, -400, 0));
		spawn_entity(game.components, game.entities[game.entity_count++], EntityType_BlockAvatar, game.default_program_id, V3(-500, 0, 0));
		spawn_entity(game.components, game.entities[game.entity_count++], EntityType_Ray, game.ray_program_id, V3(0, 0, 0));

		spawn_entity(game.components, game.entities[game.entity_count++], EntityType_Block, game.default_program_id, V3(100, 150, 0));
		spawn_entity(game.components, game.entities[game.entity_count++], EntityType_Block, game.default_program_id, V3(-200, -350, 0));
		spawn_entity(game.components, game.entities[game.entity_count++], EntityType_Sphere, game.sphere_program_id, V3(-300, 200, 0));

		// CALL(game.entities[game.entity_count-1], model, set_rotation_around, 1, 50, 50);

		#define ASPECT_RATIO ((float)RES_WIDTH/(float)RES_HEIGHT)
		setup_camera(game.camera, V3(0, 0, 500), ASPECT_RATIO);

		if (!gl_program_builder::validate_program(default_program))
			return -1;

		game.audio_manager.play(game.engine, "../../game/assets/test.wav");
	}

	{ // Update the game
		// Input handling
		if (IS_HELD(input, InputKey_Space)) {
			time += dt;
		}

		Entity &ray_entity = game.entities[1];

		i32 count = 2;
		v3 ray_vertices[] = {
			{ 0.0f, 0.0f, 0.0f },
			{ 500.0f * cosf(time), 500.0f * sinf(time), 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },

			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
		};

		GLindex ray_indices[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

		Ray ray = make_ray(ray_vertices[0], ray_vertices[1]);

		bool ray_hit = true;
		while (ray_hit && count < (i32)ARRAY_COUNT(ray_vertices)) {
			RaycastResults rr = game.components.actor.raycast(ray);
			ray_hit = rr.id > 0;
			if (ray_hit) {
				ray_vertices[count-1] = rr.position;
				v3 projected = dot(ray.delta, rr.normal) * rr.normal;
				v3 reflection = ray.delta - projected * 2;
				ray_vertices[count] = rr.position + normalize(reflection) * 500;

				ray = make_ray(ray_vertices[count-1], ray_vertices[count]);

				count++;
			}
		}

		CALL(ray_entity, model, update_vertices, ray_indices, count, ray_vertices, count);

		CALL(game.entities[game.entity_count-2], model, rotate, 0.01f);
		CALL(game.entities[game.entity_count-1], model, rotate, 0.02f);

		Entity &avatar = game.entities[0];
		v2 direction = V2_f32(0, 0);
		if (IS_HELD(input, InputKey_D)) {
			direction.x = 1;
		}
		if (IS_HELD(input, InputKey_A)) {
			direction.x = -1;
		}
		if (IS_HELD(input, InputKey_W)) {
			direction.y = 1;
		}
		if (IS_HELD(input, InputKey_S)) {
			direction.y = -1;
		}
		direction = normalize_or_zero(direction);
		CALL(avatar, mover, add_acceleration, V3(direction.x * 2000, direction.y * 2000, 0));

		// Update all the components
		update_components(game.components, dt);

		// Handle component/component communication.
		component_glue::update(game.entities, game.entity_count, dt);

		game.audio_manager.update(*globals::transient_arena, game.engine, dt);
	}

	{ // Render
		render_shadowmap(game.render_pipe, game.components, game.camera);
		render_lightmap(game.render_pipe, game.components, game.camera);

		render_test(game.render_pipe, game.components, game.camera);

		render_bloom(game.render_pipe);
		render_combine(game.render_pipe);
	}
	return 0;
}

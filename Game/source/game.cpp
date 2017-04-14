// #include <SDL.h>
#include <assert.h>
#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>

#define GLSL(src) "#version 410\n" #src
typedef uint16_t GLindex;
#define GL_INDEX GL_UNSIGNED_SHORT

#include "../assets/hatch.c"

#include "game.h"
#include "utils/memory_arena.cpp"
#include "gl_program_builder.cpp"
#include "utils/math.inl"

namespace globals {
	static MemoryArena transient_arena;
	static void *components;
};
#define SCRATCH_ALLOCATE_STRUCT(type, count) (type*)allocate_memory(globals::transient_arena, sizeof(type)*count)
#define SCRATCH_ALLOCATE(type, size) (type*)allocate_memory(globals::transient_arena, size)
#define CALL(owner, compname, command, ...) ((ComponentManager*)globals::components)->compname.command(owner.compname ## _id, ## __VA_ARGS__)
#define GET(owner, compname, member) ((ComponentManager*)globals::components)->compname.instances[owner.compname ## _id].member

enum EntityType {
	EntityType_Avatar = 0,
	EntityType_BlockAvatar,
	EntityType_Sphere,
	EntityType_Block,
	EntityType_Ray,
	EntityType_Fullscreen,

	EntityType_Count,
};

struct Entity {
	EntityType type;
	int animation_id;
	int model_id;
	int mover_id;
	int actor_id;
};

#include "camera.cpp"
#include "animation.h"

#include "component_manager.cpp"

#include "default.shader.cpp"
#include "avatar.shader.cpp"
#include "entity_manager.cpp"

#include "component_glue.cpp"
#include "renderer.cpp"

#include "audio_manager.cpp"

struct Game {
	bool initialized;

	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	EngineApi *engine;
	AudioManager audio_manager;

	int default_program_id;
	int avatar_program_id;
	int sphere_program_id;
	int ray_program_id;

	Renderer renderer;

	ComponentManager components;

	Camera camera;
	int entity_count;
	Entity entities[8];
};

#define TRANSIENT_ARENA_SIZE (MB*2)
#define PERSISTENT_ARENA_SIZE (MB*2)

#define CTC(condition, msg) typedef int my_static_assert[condition ? 1 : -1]

CTC(sizeof(int) == 4, "Some message");

EXPORT GAME_RELOAD(reload) {
	Game &game = *(Game*) memory;

	globals::transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.
	globals::components = &game.components;

	GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
	GLuint avatar_program = gl_program_builder::create_from_strings(shader_avatar::vertex, shader_avatar::fragment, shader_avatar::geometry);
	GLuint sphere_program = gl_program_builder::create_from_strings(shader_light::vertex, shader_light::fragment, shader_light::geometry);
	GLuint ray_program = gl_program_builder::create_from_strings(shader_ray::vertex, shader_ray::fragment, shader_ray::geometry);

	Program &dp = game.components.programs[game.default_program_id];
	Program &ap = game.components.programs[game.avatar_program_id];
	Program &sp = game.components.programs[game.sphere_program_id];
	Program &rp = game.components.programs[game.ray_program_id];

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

	setup(game.renderer, screen_width, screen_height);
}

static float time = 0;
static float time2 = 0;

EXPORT GAME_UPDATE(update) {
	Game &game = *(Game*) memory;
	if (!game.initialized) {
		game.initialized = true;
		// TEST(sizeof(int) == 4);

		game.persistent_arena = init_from_existing((char*)memory + sizeof(Game), MB*8);
		globals::transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.
		globals::components = &game.components;
		game.engine = &engine;

		GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
		GLuint avatar_program = gl_program_builder::create_from_strings(shader_avatar::vertex, shader_avatar::fragment, shader_avatar::geometry);
		GLuint sphere_program = gl_program_builder::create_from_strings(shader_sphere::vertex, shader_sphere::fragment, shader_sphere::geometry);
		GLuint ray_program = gl_program_builder::create_from_strings(shader_ray::vertex, shader_ray::fragment, shader_ray::geometry);

		game.default_program_id = game.components.add_program(default_program, RenderMask_ShadowCasters);
		game.avatar_program_id = game.components.add_program(avatar_program, RenderMask_ShadowCasters);
		game.sphere_program_id = game.components.add_program(sphere_program, RenderMask_ShadowCasters);
		game.ray_program_id = game.components.add_program(ray_program, RenderMask_Rest);

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		setup(game.renderer, screen_width, screen_height);

		// spawn_entity(game.entities[game.entity_count++], EntityType_Avatar, game.avatar_program_id, V3(-400, -400, 0));
		spawn_entity(game.entities[game.entity_count++], EntityType_BlockAvatar, game.default_program_id, V3(-500, 0, 0));
		spawn_entity(game.entities[game.entity_count++], EntityType_Ray, game.ray_program_id, V3(0, 0, 0));

		spawn_entity(game.entities[game.entity_count++], EntityType_Block, game.default_program_id, V3(100, 150, 0));
		spawn_entity(game.entities[game.entity_count++], EntityType_Block, game.default_program_id, V3(-200, -350, 0));
		spawn_entity(game.entities[game.entity_count++], EntityType_Sphere, game.sphere_program_id, V3(-300, 200, 0));

		// CALL(game.entities[game.entity_count-1], model, set_rotation_around, 1, 50, 50);

		setup_camera(game.camera, ASPECT_RATIO, V3(0, 0, 500));

		if (!gl_program_builder::validate_program(default_program))
			return -1;

		game.audio_manager.play(game.engine, "../assets/test.wav");
	}

	{ // Update the game


		// Input handling
		if (IS_HELD(input, InputKey_Jump)) {
			time += dt;
		}
		time2 += dt;
			Entity &ray_entity = game.entities[1];

			int count = 2;
			v3 ray_vertices[] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 500.0f * (float)cos(time), 500.0f * (float)sin(time), 0.0f },
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
			while (ray_hit && count < ARRAY_COUNT(ray_vertices)) {
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

			CALL(game.entities[game.entity_count-2], model, rotate, 0.01);
			CALL(game.entities[game.entity_count-1], model, rotate, 0.02);
		// }
			// game.audio_manager.play(game.engine, "../assets/test_effect.wav");


		Entity &avatar = game.entities[0];
		v2 direction = V2(0, 0);
		if (IS_HELD(input, InputKey_Right)) {
			direction.x = 1;
		}
		if (IS_HELD(input, InputKey_Left)) {
			direction.x = -1;
		}
		if (IS_HELD(input, InputKey_Up)) {
			direction.y = 1;
		}
		if (IS_HELD(input, InputKey_Down)) {
			direction.y = -1;
		}
		direction = normalize_or_zero(direction);
		CALL(avatar, mover, add_acceleration, V3(direction.x * 2000, direction.y * 2000, 0));


		// set_light_position(game.renderer, game.camera, V3(cos(time) * 800, sin(time)*0.0f, 0));
		// float x = (game.renderer.light_position.x + 1) * 0.5f;
		// if (x < 0) {
		// 	game.audio_manager.set_volume(0, 1-x, x);
		// } else {
		// 	game.audio_manager.set_volume(0, 1-x, x);
		// }

		// Update all the components
		game.components.update(dt);

		// Handle component/component communication.
		component_glue::update(game.entities, game.entity_count, dt);

		game.audio_manager.update(game.engine, dt);
	}

	{ // Render
		render_shadowmap(game.renderer, game.components, game.camera);
		render_lightmap(game.renderer, game.components, game.camera);

		render_test(game.renderer, game.components, game.camera);

		render_bloom(game.renderer);
		render_combine(game.renderer);
	}

	globals::transient_arena.offset = 0;
	return 0;
}

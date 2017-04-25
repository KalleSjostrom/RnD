#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>

#define GLSL(src) "#version 410\n" #src
typedef uint16_t GLindex;
#define GL_INDEX GL_UNSIGNED_SHORT

#include "engine/plugin.h"
#include "utils/memory_arena.cpp"
#include "utils/file_utils.h"
#include "opengl/gl_program_builder.cpp"

#define USE_INTRINSICS 1
#include "utils/math.h"

#include "engine/utils/audio_manager.cpp"
#include "engine/utils/camera.cpp"

///// PLUGIN SPECIFICS
#include "default.shader.cpp"

#define CALL(components, owner, compname, command, ...) (components.compname.command(owner.compname ## _id, ## __VA_ARGS__))
#define GET(components, owner, compname, member) (components.compname.instances[owner.compname ## _id].member)
#include "component_group.cpp"

struct State {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;
	ComponentGroup components;

	Camera camera;

	EngineApi *engine;

	b32 initialized;
	i32 default_program_id;

	i32 entity_count;
	Entity entities[8];

	i32 __padding;
	i32 ___padding;
	i32 ____padding;
};

#define TRANSIENT_ARENA_SIZE (MB*32)
#define PERSISTENT_ARENA_SIZE (MB*32)

EXPORT PLUGIN_RELOAD(reload) {
	(void)screen_width;
	(void)screen_height;
	(void)memory;
	// State &state = *(State*) memory;
}

EXPORT PLUGIN_UPDATE(update) {
	(void)screen_width;
	(void)screen_height;
	(void)input;
	(void)dt;

	State &state = *(State*) memory;
	if (!state.initialized) {
		state.initialized = true;

		state.engine = &engine;

		state.persistent_arena = init_from_existing((char*)memory + sizeof(State), PERSISTENT_ARENA_SIZE);
		state.transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.

		GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);

		Renderer &renderer = state.components.renderer;
		state.default_program_id = add_program(renderer, default_program, 1);

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		spawn_entity(state.components, state.entities[state.entity_count++], EntityType_Block, state.default_program_id, V3(-500, 0, 0));

		setup_camera(state.camera, ASPECT_RATIO, V3(0, 0, 500));

		if (!gl_program_builder::validate_program(default_program))
			return -1;
	}

	{
		// Update all the components
		update_components(state.components, dt);
		// Handle component/component communication.
		component_glue::update(state.components, state.entities, state.entity_count, dt);
	}

	{
		render(state.components.renderer, state.camera);
	}

	state.transient_arena.offset = 0;

	return 0;
}

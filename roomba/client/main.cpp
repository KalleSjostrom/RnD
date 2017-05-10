#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>

#define GLSL(src) "#version 410\n" #src
typedef uint16_t GLindex;
#define GL_INDEX GL_UNSIGNED_SHORT

#include "engine/plugin.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/file_utils.h"
#include "opengl/gl_program_builder.cpp"

#define USE_INTRINSICS 1
#include "engine/utils/math/math.h"

#include "engine/utils/string.h"
#include "engine/utils/audio_manager.cpp"
#include "engine/utils/camera.cpp"
#include "engine/utils/gui.cpp"

///// PLUGIN SPECIFICS
#include "roomba.shader.cpp"

#define CALL(components, owner, compname, command, ...) (components.compname.command(owner.compname ## _id, ## __VA_ARGS__))
#define GET(components, owner, compname, member) (components.compname.instances[owner.compname ## _id].member)
#include "component_group.cpp"

#include <fcntl.h>
#include "../roomba_common.cpp"
#include "roomba.cpp"

struct State {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;
	ComponentGroup components;
	RoombaConnection connection;

	Camera camera;
	gui::GUI gui;

	EngineApi *engine;

	Entity entities[8];
	i32 entity_count;

	b32 initialized;
	i32 default_program_id;
	i32 roomba_program_id;
	i32 line_program_id;
	i32 __padding;

	f64 rotation;

	char *fps_string;
	i32 fps_job_handle;
	i32 fps_frames;
	f32 fps_timer;

	i32 ___padding;
};

#define TRANSIENT_ARENA_SIZE (MB*32)
#define PERSISTENT_ARENA_SIZE (MB*256)

EXPORT PLUGIN_RELOAD(reload) {
	(void)screen_width;
	(void)screen_height;
	(void)memory;

	State &state = *(State*) memory;

	state.transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.

	GLuint roomba_program = gl_program_builder::create_from_strings(shader_roomba::vertex, shader_roomba::fragment, shader_roomba::geometry);
	GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
	GLuint line_program = gl_program_builder::create_from_strings(shader_line::vertex, shader_line::fragment, shader_line::geometry);

	Renderer &renderer = state.components.renderer;

	Program &rp = renderer.programs[state.roomba_program_id];
	Program &dp = renderer.programs[state.default_program_id];
	Program &lp = renderer.programs[state.line_program_id];

	rp.program = roomba_program;
	dp.program = default_program;
	lp.program = line_program;

	// setup_camera(state.camera, ASPECT_RATIO, V3(0, 0, 500));
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

		GLuint roomba_program = gl_program_builder::create_from_strings(shader_roomba::vertex, shader_roomba::fragment, shader_roomba::geometry);
		GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
		GLuint line_program = gl_program_builder::create_from_strings(shader_line::vertex, shader_line::fragment, shader_line::geometry);

		Renderer &renderer = state.components.renderer;
		state.roomba_program_id = add_program(renderer, roomba_program, 1);
		state.default_program_id = add_program(renderer, default_program, 1);
		state.line_program_id = add_program(renderer, line_program, 1);

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		spawn_entity(state.components, state.entities[state.entity_count++], EntityType_Roomba, state.roomba_program_id, V3(0, 0, 0));
		spawn_entity(state.components, state.entities[state.entity_count++], EntityType_Block, state.default_program_id, V3(-500, 0, 0));
		spawn_entity(state.components, state.entities[state.entity_count++], EntityType_Ruler, state.line_program_id, V3(0, 0, 0));

		setup_camera(state.camera, V3(0, 0, 4), screen_width/1000.f, screen_height/1000.f);

		gui::GUISettings settings = {};
		settings.font_path = "../../assets/font.gamefont";
		settings.text_vertex_shader = "../../assets/text.vert";
		settings.text_fragment_shader = "../../assets/text.frag";
		gui::init(state.gui, state.persistent_arena, state.transient_arena, settings);

		state.fps_string = allocate_memory(state.persistent_arena, 32);
		sprintf(state.fps_string, "Framerate: N/A");
		String string = make_string(state.fps_string);
		state.fps_job_handle = gui::create_string(state.gui, true, string);

		setup_connection(state.connection, ConnectionMode_Playback, state.persistent_arena);
		update_transmit(state.connection);

		if (!gl_program_builder::validate_program(default_program))
			return -1;
	}

	i32 distance = 0;
	i32 angle = 0;
	update_recieve(state.connection, distance, angle);

	{
		Entity &ray_entity = state.entities[2];
		static f32 time = 0;
		time += dt;

		i32 count = 2;
		v3 ray_vertices[] = {
			{ 0.0f, -2.0f, 0.0f },
			// { 500.0f * cosf(time), 500.0f * sinf(time), 0.0f },
			{ 0.0f, 2.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },

			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
		};

		GLindex ray_indices[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
		CALL(state.components, ray_entity, model, update_vertices, ray_indices, count, ray_vertices, count);

		Entity &roomba = state.entities[0];

		i16 speed = 0;
		i16 radius = 0;

		if (IS_HELD(input, InputKey_W)) {
			speed = 200; // mm / s
		}
		if (IS_HELD(input, InputKey_D)) {
			radius = 100; // mm
		}
		if (IS_HELD(input, InputKey_A)) {
			radius = -100; // mm
		}

		u8 header = COMMAND_DRIVE;
		u32 packet = (((u32)speed << 16) & 0xFFFF0000) | ((u32)radius & 0x0000FFFF);
		send_command(state.connection, header, (u8*)&packet, 4);

		// To drive in reverse at a velocity of -200 mm/s while turning at
		// a radius of 500mm, you would send the serial byte sequence
		// [137] [255] [56] [1] [244].
		// Velocity = -200 = hex FF38 = [hex FF] [hex 38] = [255] [56]
		// Radius = 500 = hex 01F4 = [hex 01] [hex F4] = [1] [244]

		v3 position = CALL(state.components, roomba, model, get_position);
		v2_f64 p = V2_f64((f64)position.x, (f64)position.y);
		convert(angle, distance, p, state.rotation);
		position.x = (f32)p.x; position.y = (f32)p.y;
		CALL(state.components, roomba, model, set_position, position);

		CALL(state.components, roomba, model, set_rotatation, (f32) state.rotation);

		// Update all the components
		// update_components(state.components, dt);

		// Handle component/component communication.
		// component_glue::update(state.components, state.entities, state.entity_count, dt);
	}

	{
		glClear(GL_COLOR_BUFFER_BIT);
		render(state.components.renderer, state.camera);

		String string = MAKE_STRING("Hejja");
		i32 handle = gui::create_string(state.gui, true, string);

		static v4 color = {0.5f, 0.75f, 1.0, 1.0f};

		state.fps_frames++;
		state.fps_timer += dt;
		if (state.fps_timer > 2) {
			sprintf(state.fps_string, "Framerate: %.1f", (f64)state.fps_frames/(f64)state.fps_timer);
			String fps_string = make_string(state.fps_string);
			gui::update_string(state.gui, (i16)state.fps_job_handle, fps_string);

			state.fps_frames = 0;
			state.fps_timer = 0;
		}

		gui::begin_render(state.gui);
		gui::render_dynamic(state.gui, state.fps_job_handle, 0, 16, 0.4f, color);
		gui::render_dynamic(state.gui, handle, 0, 32, 0.4f, color);

		gui::render_all_static(state.gui, 0, 64, 0.4f, color);

		gui::destroy_string(state.gui, handle);
	}

	state.transient_arena.offset = 0;

	update_transmit(state.connection);

	if (IS_HELD(input, InputKey_Escape)) {
		save_stream(state.connection);
		engine.quit();
	}

	return 0;
}

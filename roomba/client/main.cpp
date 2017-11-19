#include "includes.h"

struct StatString {
	char buf[64];
};

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

	StatString stats[Sensor_Count];
};

#define TRANSIENT_ARENA_SIZE (MB*32)
#define PERSISTENT_ARENA_SIZE (MB*256)

EXPORT PLUGIN_RELOAD(reload) {
	(void)screen_width;
	(void)screen_height;
	(void)memory;

	State &state = *(State*) memory;

	// state.transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.

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

		setup_arena(state.persistent_arena, PERSISTENT_ARENA_SIZE);
		setup_arena(state.transient_arena, TRANSIENT_ARENA_SIZE);
		// state.persistent_arena = init_from_existing((char*)memory + sizeof(State), PERSISTENT_ARENA_SIZE);
		// state.transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.

		GLuint roomba_program = gl_program_builder::create_from_strings(shader_roomba::vertex, shader_roomba::fragment, shader_roomba::geometry);
		GLuint default_program = gl_program_builder::create_from_strings(shader_default::vertex, shader_default::fragment, 0);
		GLuint line_program = gl_program_builder::create_from_strings(shader_line::vertex, shader_line::fragment, shader_line::geometry);

		Renderer &renderer = state.components.renderer;
		state.roomba_program_id = add_program(renderer, roomba_program, 1);
		state.default_program_id = add_program(renderer, default_program, 1);
		state.line_program_id = add_program(renderer, line_program, 1);

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		spawn_entity(state.components, state.entities[state.entity_count++], EntityType_Model, state.roomba_program_id, V3(0, 0, 0));
		spawn_entity(state.components, state.entities[state.entity_count++], EntityType_Ruler, state.line_program_id, V3(0, 0, 0));

		setup_camera(state.camera, V3(0, 0, 4), screen_width/1000.f, screen_height/1000.f);

		gui::GUISettings settings = {};
		settings.font_path = "../../assets/font.gamefont";
		settings.text_vertex_shader = "../../assets/text.vert";
		settings.text_fragment_shader = "../../assets/text.frag";
		gui::init(state.gui, state.persistent_arena, state.transient_arena, settings);

		state.fps_string = PUSH_STRING(state.persistent_arena, 32);
		sprintf(state.fps_string, "Framerate: N/A");
		String string = make_string(state.fps_string);
		state.fps_job_handle = gui::create_string(state.gui, true, string);

		setup_connection(state.connection, ConnectionMode_Server, state.persistent_arena);
		send_command(state.connection, COMMAND_FULL);
		// send_command(state.connection, COMMAND_SENSORS);

		u8 header = COMMAND_SENSORS;
		u8 packet[1] = {};
		packet[0] = 0;
		send_command(state.connection, header, packet, 1);

		if (!gl_program_builder::validate_program(default_program))
			return -1;
	}

	// u8 a = 0b11111111;
	// u8 b = 0b11110111;
    u16 a = (0xFF << 8) | 0xF7;
    u16 b = (0xF7 << 8) | 0xFF;

	// i16 c = (i16)(((u32)a << 8) | (u32)b);

	// TempAllocator ta(&state.transient_arena);
	MemoryBlockHandle transient_block_handle = begin_block(state.transient_arena);

	static SensorData sensor_data = {};
	SensorData data = {};
	i32 distance = 0;
	i32 angle = 0;
	if (update_recieve(state.connection, distance, angle, data)) {
		sensor_data = data;
		printf("Recieved\n");
	}
	static float time = 0;
	static const float update_time = 1.00f;
    time += dt;
	if (time >= update_time) {
		time -= update_time;

		i16 speed = 0;
		i16 radius = 0;

		if (IS_HELD(input, InputKey_W)) {
			speed = 200; // mm / s
		}
		if (IS_HELD(input, InputKey_S)) {
			speed = -200; // mm / s
		}
		if (IS_HELD(input, InputKey_D)) {
			radius = -100; // mm
		}
		if (IS_HELD(input, InputKey_A)) {
			radius = 100; // mm
		}

		{
			u8 header = COMMAND_DRIVE;
			u8 packet[4] = {};
			packet[0] = (u8)((u32)speed >> 8);
			packet[1] = (u8)((u32)speed);
			packet[2] = (u8)((u32)radius >> 8);
			packet[3] = (u8)((u32)radius);

			send_command(state.connection, header, packet, 4);
		}

		{
			u8 header = COMMAND_SENSORS;
			u8 packet[1] = {};
			packet[0] = 0;

			send_command(state.connection, header, packet, 1);
		}

		printf("Sending\n");
	}

	// printf("%f\n", (f64)time);

	{
#if 1

#endif

		if (IS_PRESSED(input, InputKey_Space)) {
			send_command(state.connection, COMMAND_DOCK);
		} else if (IS_PRESSED(input, InputKey_Enter)) {
			send_command(state.connection, COMMAND_CLEAN);
		}

		// To drive in reverse at a velocity of -200 mm/s while turning at
		// a radius of 500mm, you would send the serial byte sequence
		// [137] [255] [56] [1] [244].
		// Velocity = -200 = hex FF38 = [hex FF] [hex 38] = [255] [56]
		// Radius = 500 = hex 01F4 = [hex 01] [hex F4] = [1] [244]

		Entity &roomba = state.entities[0];
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


		sprintf(state.stats[Sensor_Bumpwheeldrops].buf,    "Bumpwheeldrops: %d",    sensor_data.Bumpwheeldrops);
		sprintf(state.stats[Sensor_Wall].buf,              "Wall: %d",              sensor_data.Wall);
		sprintf(state.stats[Sensor_CliffLeft].buf,         "CliffLeft: %d",         sensor_data.CliffLeft);
		sprintf(state.stats[Sensor_CliffFrontLeft].buf,    "CliffFrontLeft: %d",    sensor_data.CliffFrontLeft);
		sprintf(state.stats[Sensor_CliffFrontRight].buf,   "CliffFrontRight: %d",   sensor_data.CliffFrontRight);
		sprintf(state.stats[Sensor_CliffRight].buf,        "CliffRight: %d",        sensor_data.CliffRight);
		sprintf(state.stats[Sensor_VirtualWall].buf,       "VirtualWall: %d",       sensor_data.VirtualWall);
		sprintf(state.stats[Sensor_MotorOvercurrents].buf, "MotorOvercurrents: %d", sensor_data.MotorOvercurrents);
		sprintf(state.stats[Sensor_DirtDetectorLeft].buf,  "DirtDetectorLeft: %d",  sensor_data.DirtDetectorLeft);
		sprintf(state.stats[Sensor_DirtDetectorRight].buf, "DirtDetectorRight: %d", sensor_data.DirtDetectorRight);
		sprintf(state.stats[Sensor_RemoteOpcode].buf,      "RemoteOpcode: %d",      sensor_data.RemoteOpcode);
		sprintf(state.stats[Sensor_Buttons].buf,           "Buttons: %d",           sensor_data.Buttons);
		sprintf(state.stats[Sensor_DistanceMSB].buf,       "Distance: %d",          sensor_data.distance);
		sprintf(state.stats[Sensor_AngleMSB].buf,          "Angle: %d",             sensor_data.angle);
		sprintf(state.stats[Sensor_ChargingState].buf,     "ChargingState: %d",     sensor_data.ChargingState);
		sprintf(state.stats[Sensor_VoltageMSB].buf,        "Voltage: %d",           sensor_data.voltage);
		sprintf(state.stats[Sensor_CurrentMSB].buf,        "Current: %d",           sensor_data.current);
		sprintf(state.stats[Sensor_Temperature].buf,       "Temperature: %d",       sensor_data.Temperature);
		sprintf(state.stats[Sensor_ChargeMSB].buf,         "BatteryLevel: %.2f",    (f64)sensor_data.BatteryLevel);

		String strings[] = {
			make_string(state.stats[Sensor_Bumpwheeldrops].buf),
			make_string(state.stats[Sensor_Wall].buf),
			make_string(state.stats[Sensor_CliffLeft].buf),
			make_string(state.stats[Sensor_CliffFrontLeft].buf),
			make_string(state.stats[Sensor_CliffFrontRight].buf),
			make_string(state.stats[Sensor_CliffRight].buf),
			make_string(state.stats[Sensor_VirtualWall].buf),
			make_string(state.stats[Sensor_MotorOvercurrents].buf),
			make_string(state.stats[Sensor_DirtDetectorLeft].buf),
			make_string(state.stats[Sensor_DirtDetectorRight].buf),
			make_string(state.stats[Sensor_RemoteOpcode].buf),
			make_string(state.stats[Sensor_Buttons].buf),
			make_string(state.stats[Sensor_DistanceMSB].buf),
			make_string(state.stats[Sensor_AngleMSB].buf),
			make_string(state.stats[Sensor_ChargingState].buf),
			make_string(state.stats[Sensor_VoltageMSB].buf),
			make_string(state.stats[Sensor_CurrentMSB].buf),
			make_string(state.stats[Sensor_Temperature].buf),
			make_string(state.stats[Sensor_ChargeMSB].buf),
		};



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

		for (u32 i = 0; i < ARRAY_COUNT(strings); ++i) {
			i32 handle = gui::create_string(state.gui, true, strings[i]);
			gui::render_dynamic(state.gui, handle, 0, 32 + 16 * i, 0.4f, color);
		}

		gui::render_all_static(state.gui, 0, 64, 0.4f, color);
	}

	if (IS_HELD(input, InputKey_Escape)) {
		save_stream(state.connection);
		engine.quit();
	}

	end_block(state.transient_arena, transient_block_handle);

	return 0;
}

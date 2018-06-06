struct Roomba;
#define RELOAD_ENTRY_POINT Roomba

#define SYSTEM_OPENGL
#define SYSTEM_AUDIO
#define SYSTEM_GRAPHICS
#define SYSTEM_GUI
#define SYSTEM_COMPONENTS
#include "engine/systems.h"
#include "render_pipe.cpp"
#include "levels.cpp"

#include "../sci.h"
#include "../tcp_socket.cpp"

#include "../roomba_common.cpp"
#include "roomba.cpp"

struct StatString {
	char buf[64];
};

struct Roomba {
	RenderPipe render_pipe;
	RoombaConnection connection;

	EngineApi *engine;

	i32 default_program_id;
	i32 roomba_program_id;
	i32 line_program_id;

	f64 rotation;

	char *fps_string;
	i32 fps_job_handle;
	i32 fps_frames;
	f32 fps_timer;

	StatString stats[Sensor_Count];
};

void plugin_setup(Application &application) {
	Roomba &roomba = *PUSH_STRUCT(application.persistent_arena, Roomba);
	application.user_data = &roomba;

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);

	Level level = make_level();
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
	setup_render_pipe(application.persistent_arena, application.engine, roomba.render_pipe, application.components, screen_width, screen_height);

	{ // gui
		GUISettings settings = {};
		settings.font_path = "../../roomba/assets/font.gamefont";
		settings.text_vertex_shader = "../../roomba/assets/text.vert";
		settings.text_fragment_shader = "../../roomba/assets/text.frag";
		init(application.gui, application.persistent_arena, application.transient_arena, settings);

		roomba.fps_string = PUSH_STRING(application.persistent_arena, 32);
		sprintf_s(roomba.fps_string, 32, "Framerate: N/A");
		String string = make_string(roomba.fps_string);
		roomba.fps_job_handle = create_string(application.gui, true, string);
	}

	setup_connection(roomba.connection, ConnectionMode_Server, application.persistent_arena);
	// send_command(roomba.connection, COMMAND_FULL);
}

void plugin_update(Application &application, float dt) {
	MemoryBlockHandle transient_block_handle = begin_block(application.transient_arena);
	Roomba &roomba = *(Roomba*)application.user_data;

	static bool sent = 0;

	static SensorData sensor_data = {};
	SensorData data = {};
	if (sent && update_recieve(roomba.connection, data)) {
		sent = 0;
		sensor_data = data;
		printf("Recieved\n");
	}
	static float time = 0;
	static const float update_time = 1.00f;
	time += dt;
	if (time >= update_time && !sent) {
		time = 0;

#if 0
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

			send_command(roomba.connection, header, packet, 4);
		}
#endif

		{
			u8 header = COMMAND_SENSORS;
			u8 packet[1] = {};
			packet[0] = 0;

			send_command(roomba.connection, header, packet, 1);
			sent = 1;
		}

		printf("Sending\n");
	}

	{
#if 1
		if (IS_PRESSED(*application.input, InputKey_Space)) {
			send_command(roomba.connection, COMMAND_DOCK);
		} else if (IS_PRESSED(*application.input, InputKey_Enter)) {
			send_command(roomba.connection, COMMAND_CLEAN);
		}
#endif

		// To drive in reverse at a velocity of -200 mm/s while turning at
		// a radius of 500mm, you would send the serial byte sequence
		// [137] [255] [56] [1] [244].
		// Velocity = -200 = hex FF38 = [hex FF] [hex 38] = [255] [56]
		// Radius = 500 = hex 01F4 = [hex 01] [hex F4] = [1] [244]

		ComponentGroup &components = application.components;
		Entity &entity = components.entities[0];
		m4 &pose = get_pose(components.model, entity);
		v3 &position = translation(pose);
		v2_f64 p = V2_f64((f64)position.x, (f64)position.y);
		// convert(angle, distance, p, roomba.rotation);
		// position.x = (f32)p.x; position.y = (f32)p.y;
		// CALL(roomba.components, roomba, model, set_rotatation, (f32) roomba.rotation);
	}

	u32 bufsize = ARRAY_COUNT(roomba.stats[0].buf);
	sprintf_s(roomba.stats[Sensor_Bumpwheeldrops].buf,   bufsize, "Bumpwheeldrops: %d",    sensor_data.Bumpwheeldrops);
	sprintf_s(roomba.stats[Sensor_Wall].buf,             bufsize, "Wall: %d",              sensor_data.Wall);
	sprintf_s(roomba.stats[Sensor_CliffLeft].buf,        bufsize, "CliffLeft: %d",         sensor_data.CliffLeft);
	sprintf_s(roomba.stats[Sensor_CliffFrontLeft].buf,   bufsize, "CliffFrontLeft: %d",    sensor_data.CliffFrontLeft);
	sprintf_s(roomba.stats[Sensor_CliffFrontRight].buf,  bufsize, "CliffFrontRight: %d",   sensor_data.CliffFrontRight);
	sprintf_s(roomba.stats[Sensor_CliffRight].buf,       bufsize, "CliffRight: %d",        sensor_data.CliffRight);
	sprintf_s(roomba.stats[Sensor_VirtualWall].buf,      bufsize, "VirtualWall: %d",       sensor_data.VirtualWall);
	sprintf_s(roomba.stats[Sensor_MotorOvercurrents].buf,bufsize, "MotorOvercurrents: %d", sensor_data.MotorOvercurrents);
	sprintf_s(roomba.stats[Sensor_DirtDetectorLeft].buf, bufsize, "DirtDetectorLeft: %d",  sensor_data.DirtDetectorLeft);
	sprintf_s(roomba.stats[Sensor_DirtDetectorRight].buf,bufsize, "DirtDetectorRight: %d", sensor_data.DirtDetectorRight);
	sprintf_s(roomba.stats[Sensor_RemoteOpcode].buf,     bufsize, "RemoteOpcode: %d",      sensor_data.RemoteOpcode);
	sprintf_s(roomba.stats[Sensor_Buttons].buf,          bufsize, "Buttons: %d",           sensor_data.Buttons);
	sprintf_s(roomba.stats[Sensor_DistanceMSB].buf,      bufsize, "Distance: %d",          sensor_data.distance);
	sprintf_s(roomba.stats[Sensor_AngleMSB].buf,         bufsize, "Angle: %d",             sensor_data.angle);
	sprintf_s(roomba.stats[Sensor_ChargingState].buf,    bufsize, "ChargingState: %d",     sensor_data.ChargingState);
	sprintf_s(roomba.stats[Sensor_VoltageMSB].buf,       bufsize, "Voltage: %d",           sensor_data.voltage);
	sprintf_s(roomba.stats[Sensor_CurrentMSB].buf,       bufsize, "Current: %d",           sensor_data.current);
	sprintf_s(roomba.stats[Sensor_Temperature].buf,      bufsize, "Temperature: %d",       sensor_data.Temperature);
	sprintf_s(roomba.stats[Sensor_ChargeMSB].buf,        bufsize, "BatteryLevel: %.2f",    (f64)sensor_data.battery_level);

	roomba.fps_frames++;
	roomba.fps_timer += dt;
	if (roomba.fps_timer > 2) {
		sprintf_s(roomba.fps_string, 32, "Framerate: %.1f", (f64)roomba.fps_frames/(f64)roomba.fps_timer);
		String fps_string = make_string(roomba.fps_string);
		update_string(application.gui, (i16)roomba.fps_job_handle, fps_string);

		roomba.fps_frames = 0;
		roomba.fps_timer = 0;
	}

	if (IS_HELD(*application.input, InputKey_Escape)) {
		save_stream(roomba.connection);
		application.engine->quit();
	}

	end_block(application.transient_arena, transient_block_handle);
}

void plugin_render(Application &application) {
	Roomba &roomba = *(Roomba*)application.user_data;
	render(roomba.render_pipe, application.components, application.camera);

	String strings[] = {
		make_string(roomba.stats[Sensor_Bumpwheeldrops].buf),
		make_string(roomba.stats[Sensor_Wall].buf),
		make_string(roomba.stats[Sensor_CliffLeft].buf),
		make_string(roomba.stats[Sensor_CliffFrontLeft].buf),
		make_string(roomba.stats[Sensor_CliffFrontRight].buf),
		make_string(roomba.stats[Sensor_CliffRight].buf),
		make_string(roomba.stats[Sensor_VirtualWall].buf),
		make_string(roomba.stats[Sensor_MotorOvercurrents].buf),
		make_string(roomba.stats[Sensor_DirtDetectorLeft].buf),
		make_string(roomba.stats[Sensor_DirtDetectorRight].buf),
		make_string(roomba.stats[Sensor_RemoteOpcode].buf),
		make_string(roomba.stats[Sensor_Buttons].buf),
		make_string(roomba.stats[Sensor_DistanceMSB].buf),
		make_string(roomba.stats[Sensor_AngleMSB].buf),
		make_string(roomba.stats[Sensor_ChargingState].buf),
		make_string(roomba.stats[Sensor_VoltageMSB].buf),
		make_string(roomba.stats[Sensor_CurrentMSB].buf),
		make_string(roomba.stats[Sensor_Temperature].buf),
		make_string(roomba.stats[Sensor_ChargeMSB].buf),
	};

	static v4 color = {0.5f, 0.75f, 1.0, 1.0f};

	begin_render(application.gui);
	render_dynamic(application.gui, roomba.fps_job_handle, 0, 16, 0.4f, color);

	for (u32 i = 0; i < ARRAY_COUNT(strings); ++i) {
		i32 handle = create_string(application.gui, true, strings[i]);
		render_dynamic(application.gui, handle, 0, 32 + 16 * i, 0.4f, color);
	}

	render_all_static(application.gui, 0, 64, 0.4f, color);
}

void plugin_reloaded(Application &application) {
	// Roomba &roomba = *(Roomba*)application.user_data;
}

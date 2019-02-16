#define SYSTEM_OPENGL
#define SYSTEM_AUDIO
#define SYSTEM_GRAPHICS
#define SYSTEM_GUI
#define SYSTEM_COMPONENTS

#define STREAM_NAME "../../roomba/server/out/roomba_2018-3-25_22-52-30"
#define DATA_FOLDER "../../roomba/out"
#define ASSET_FOLDER "../../roomba/assets"

#include "engine/systems.h"
#include "render_pipe.cpp"
#include "levels.cpp"

#include "../sensor_data.c"

SensorData next_sensor_data(SensorStream *stream) {
	SensorData sensor_data = *(SensorData*)(stream->memory + stream->cursor);
	stream->cursor += sizeof(SensorData);
	return sensor_data;
}

uint64_t open_stream(SensorStream *stream, const char *filename) {
	uint64_t filesize;
	FILE *file = open_file(filename, &filesize);
	stream->memory = (char*)malloc(filesize);
	fread(stream->memory, filesize, 1, file);
	fclose(file);
	return filesize;
}

struct StatString {
	char buf[64];
};

struct Roomba {
	RenderPipe render_pipe;
	EngineApi *engine;

	SensorStream stream;
	uint64_t streamsize;

	int default_program_id;
	int roomba_program_id;
	int line_program_id;

	double rotation;

	char *fps_string;
	int fps_job_handle;
	int fps_frames;
	float fps_timer;

	StatString stats[Sensor_Count];
};

void plugin_setup(Application &application) {
	setup_camera(application.camera, V3(0, 0, 2), 60, ASPECT_RATIO);

	Roomba &roomba = *PUSH_STRUCT(application.persistent_arena, Roomba);
	application.user_data = &roomba;

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);

	Level level = make_level(application);
	for (int i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];
		Entity *entity = spawn_entity(application.engine, application.components, data.type, data.context, data.offset);

		m4 &pose = get_pose(application.components.model, *entity);
		translation(pose) = data.offset;
		set_rotation(pose, data.rotation);
		set_scale(pose, data.size);
	}

	int screen_width, screen_height;
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

	roomba.streamsize = open_stream(&roomba.stream, STREAM_NAME".stream");


#if 1
	{
		FILE *file;
		fopen_s(&file, STREAM_NAME".txt", "w");
		assert(file);
		double total_time = 0;
		double total_distance = 0;
		while (roomba.stream.cursor + sizeof(SensorData) < roomba.streamsize) {
			SensorData sensor_data = next_sensor_data(&roomba.stream);
			print_sensor_data(file, sensor_data);

			total_time += sensor_data.time;
			total_distance += abs(sensor_data.distance);
		}
		roomba.stream.cursor = 0;
	}

	{
		FILE *file;
		fopen_s(&file, STREAM_NAME".bin", "rb");
		assert(file);
		size_t filesize = get_filesize(file);
		uint8_t *buf = (uint8_t *)malloc(filesize);
		fread(buf, filesize, 1, file);

		size_t cursor = 0;
		while (cursor < filesize) {
			uint8_t *buffer = buf + cursor;

			SensorData sensor_data;
			sensor_data.Bumpwheeldrops    = buffer[(int)Sensor_Bumpwheeldrops];
			sensor_data.Wall              = buffer[(int)Sensor_Wall];
			sensor_data.CliffLeft         = buffer[(int)Sensor_CliffLeft];
			sensor_data.CliffFrontLeft    = buffer[(int)Sensor_CliffFrontLeft];
			sensor_data.CliffFrontRight   = buffer[(int)Sensor_CliffFrontRight];
			sensor_data.CliffRight        = buffer[(int)Sensor_CliffRight];
			sensor_data.VirtualWall       = buffer[(int)Sensor_VirtualWall];
			sensor_data.MotorOvercurrents = buffer[(int)Sensor_MotorOvercurrents];
			sensor_data.DirtDetectorLeft  = buffer[(int)Sensor_DirtDetectorLeft];
			sensor_data.DirtDetectorRight = buffer[(int)Sensor_DirtDetectorRight];
			sensor_data.RemoteOpcode      = buffer[(int)Sensor_RemoteOpcode];
			sensor_data.Buttons           = buffer[(int)Sensor_Buttons];
			sensor_data.ChargingState     = buffer[(int)Sensor_ChargingState];
			sensor_data.Temperature       = (int8_t) buffer[(int)Sensor_Temperature];

			sensor_data.distance = (int16_t)(buffer[(int)Sensor_DistanceMSB] << 8 | buffer[(int)Sensor_DistanceLSB]);
			sensor_data.angle    = (int16_t)(buffer[(int)Sensor_AngleMSB]    << 8 | buffer[(int)Sensor_AngleLSB]);
			sensor_data.voltage  = buffer[(int)Sensor_VoltageMSB]  << 8 | buffer[(int)Sensor_VoltageLSB];
			sensor_data.current  = buffer[(int)Sensor_CurrentMSB]  << 8 | buffer[(int)Sensor_CurrentLSB];

			int Charge   = buffer[(int)Sensor_ChargeMSB]   << 8 | buffer[(int)Sensor_ChargeLSB];
			int Capacity = buffer[(int)Sensor_CapacityMSB] << 8 | buffer[(int)Sensor_CapacityLSB];

			sensor_data.battery_level = (float) Charge / (float) Capacity;

			print_sensor_data(stdout, sensor_data);

			cursor += 26;
		}
	}
#endif
}

struct MotionDelta {
	v2_f64 position;
	double rotation;
};

MotionDelta convert(int angle, int distance, v2_f64 position, double rotation) {
	MotionDelta delta = {};
	if (angle == 0 && distance == 0) { // No movement
		return delta;
	}

	/* The angle that Roomba has turned through since the angle was
	last requested. The angle is expressed as the difference in
	the distance traveled by Roomba’s two wheels in millimeters,
	specifically the right wheel distance minus the left wheel
	distance, divided by two. This makes counter-clockwise angles
	positive and clockwise angles negative. This can be used to
	directly calculate the angle that Roomba has turned through
	since the last request. Since the distance between Roomba’s
	wheels is 258mm, the equations for calculating the angles in
	familiar units are:
	Angle in radians = (2 * difference) / 258
	Angle in degrees = (360 * difference) / (258 * Pi).
	If the value is not polled frequently enough, it will be capped at
	its minimum or maximum.*/

#if 0
	d = (r + l) / 2
	a = (r - l) / 2 => 2*a + l = r
	// substitute r in the equation for distance
	d = (2*a + l + l) / 2 => d = (a + l)
	// Move all known to left side
	d - a = l
	// Left is distance - angle
	// Plug this in the eqution for angle
	d = (r - (d - a)) / 2 => d = r/2 - d/2 + a/2 => 3d - a = r
	a = (r - (d - a)) / 2 => a = r/2 - d/2 + a/2 => a + d = r
	// right is angle + distance
#endif

	if (angle == 0) { // Haven't turned
		v2_f64 direction = V2_f64(cos(rotation), sin(rotation));
		delta.position = direction * distance / 1000.0;
	} else if (distance == 0) { // Turned on the spot
		double angle_in_radians = (2.0 * angle) / 258.0;
		delta.rotation = angle_in_radians;
	} else { // Moved while turning
		double angle_in_radians = (2.0 * angle) / 258.0; // The angle that the roomba has turned through
		double outer_arc_length;
		if (angle > 0) { // counter clockwise, or left, right wheel is outer
			outer_arc_length = angle + distance; // right_wheel_distance;
		} else {
			// Need to negate the length in order to use the same calculations for the right wheel.
			outer_arc_length = -(distance - angle); // left_wheel_distance;
		}

		// pido => pi * 2 * r * angle_in_radians/(2*pi) = outer_arc_length
		// pido => r * angle_in_radians = outer_arc_length
		double total_radius = outer_arc_length / angle_in_radians;
		double triangle_side_length = total_radius - 258.0/2.0;

		// Law of cosines - b^2 = a^2 + c^2 - 2ac * cos(beta);
		// Since a and c are equal in length (radius of the circle) this becomes
		// b^2 = 2*a^2 - 2*a^2 * cos(beta) or:
		// b^2 = 2*a^2(1 - cos(beta))
		double distance_sq = 2 * triangle_side_length * triangle_side_length * (1.0 - cos(angle_in_radians));
		double straight_distance = sqrt(distance_sq);

		double angle_of_direction = (M_PI - angle_in_radians) / 2;
		angle_of_direction = (M_PI / 2) - angle_of_direction;
		double rotation_for_direction = rotation + angle_of_direction;
		v2_f64 direction = V2_f64(cos(rotation_for_direction), sin(rotation_for_direction));

		delta.position = direction * straight_distance / 1000.0;
		delta.rotation = angle_in_radians;
	}

	return delta;
}

void plugin_update(Application &application, float dt) {
	MemoryBlockHandle transient_block_handle = begin_block(application.transient_arena);
	Roomba &roomba = *(Roomba*)application.user_data;

	InputData &input = *application.components.input.input_data;
	float translation_speed = 1.0f;
	float rotation_speed = 0.2f;
	move(application.camera, input, translation_speed, rotation_speed, dt);

	SensorData sensor_data = {};
	if (roomba.stream.cursor + sizeof(SensorData) < roomba.streamsize) {
		sensor_data = next_sensor_data(&roomba.stream);

		u32 bufsize = ARRAY_COUNT(roomba.stats[0].buf);
		sprintf_s(roomba.stats[Sensor_Bumpwheeldrops].buf, bufsize, "Bumpwheeldrops: %d", sensor_data.Bumpwheeldrops);
		sprintf_s(roomba.stats[Sensor_Wall].buf, bufsize, "Wall: %d", sensor_data.Wall);
		sprintf_s(roomba.stats[Sensor_CliffLeft].buf, bufsize, "CliffLeft: %d", sensor_data.CliffLeft);
		sprintf_s(roomba.stats[Sensor_CliffFrontLeft].buf, bufsize, "CliffFrontLeft: %d", sensor_data.CliffFrontLeft);
		sprintf_s(roomba.stats[Sensor_CliffFrontRight].buf, bufsize, "CliffFrontRight: %d", sensor_data.CliffFrontRight);
		sprintf_s(roomba.stats[Sensor_CliffRight].buf, bufsize, "CliffRight: %d", sensor_data.CliffRight);
		sprintf_s(roomba.stats[Sensor_VirtualWall].buf, bufsize, "VirtualWall: %d", sensor_data.VirtualWall);
		sprintf_s(roomba.stats[Sensor_MotorOvercurrents].buf, bufsize, "MotorOvercurrents: %d", sensor_data.MotorOvercurrents);
		sprintf_s(roomba.stats[Sensor_DirtDetectorLeft].buf, bufsize, "DirtDetectorLeft: %d", sensor_data.DirtDetectorLeft);
		sprintf_s(roomba.stats[Sensor_DirtDetectorRight].buf, bufsize, "DirtDetectorRight: %d", sensor_data.DirtDetectorRight);
		sprintf_s(roomba.stats[Sensor_RemoteOpcode].buf, bufsize, "RemoteOpcode: %d", sensor_data.RemoteOpcode);
		sprintf_s(roomba.stats[Sensor_Buttons].buf, bufsize, "Buttons: %d", sensor_data.Buttons);
		sprintf_s(roomba.stats[Sensor_DistanceMSB].buf, bufsize, "Distance: %d", sensor_data.distance);
		sprintf_s(roomba.stats[Sensor_AngleMSB].buf, bufsize, "Angle: %d", sensor_data.angle);
		sprintf_s(roomba.stats[Sensor_ChargingState].buf, bufsize, "ChargingState: %d", sensor_data.ChargingState);
		sprintf_s(roomba.stats[Sensor_VoltageMSB].buf, bufsize, "Voltage: %d", sensor_data.voltage);
		sprintf_s(roomba.stats[Sensor_CurrentMSB].buf, bufsize, "Current: %d", sensor_data.current);
		sprintf_s(roomba.stats[Sensor_Temperature].buf, bufsize, "Temperature: %d", sensor_data.Temperature);
		sprintf_s(roomba.stats[Sensor_ChargeMSB].buf, bufsize, "BatteryLevel: %.2f", (double)sensor_data.battery_level);
	}

		// To drive in reverse at a velocity of -200 mm/s while turning at
		// a radius of 500mm, you would send the serial byte sequence
		// [137] [255] [56] [1] [244].
		// Velocity = -200 = hex FF38 = [hex FF] [hex 38] = [255] [56]
		// Radius = 500 = hex 01F4 = [hex 01] [hex F4] = [1] [244]

	// SensorData sensor_data;

	ComponentGroup &components = application.components;
	Entity &entity = components.entities[0];
	m4 &pose = get_pose(components.model, entity);
	v3 &position = translation(pose);
	v2_f64 p = V2_f64((double)position.x, (double)position.y);

	// sensor_data.distance = 7;
	// sensor_data.angle = 0;

	MotionDelta delta = convert(sensor_data.angle, sensor_data.distance, p, roomba.rotation);

	position.x += (float)delta.position.x; position.y += (float)delta.position.y;
	roomba.rotation += delta.rotation;

	rotate(pose, delta.rotation);

	roomba.fps_frames++;
	roomba.fps_timer += dt;
	if (roomba.fps_timer > 2) {
		sprintf_s(roomba.fps_string, 32, "Framerate: %.1f", (double)roomba.fps_frames/(double)roomba.fps_timer);
		String fps_string = make_string(roomba.fps_string);
		update_string(application.gui, (i16)roomba.fps_job_handle, fps_string);

		roomba.fps_frames = 0;
		roomba.fps_timer = 0;
	}

	Sleep(sensor_data.time * 1000);

	if (IS_HELD(*application.input, InputKey_Escape)) {
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
		int handle = create_string(application.gui, true, strings[i]);
		render_dynamic(application.gui, handle, 0, 32 + 16 * i, 0.4f, color);
	}

	render_all_static(application.gui, 0, 64, 0.4f, color);
}

void plugin_reloaded(Application &application) {
	// Roomba &roomba = *(Roomba*)application.user_data;
}

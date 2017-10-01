enum ConnectionMode {
	ConnectionMode_Server,
	ConnectionMode_Playback,
};

struct SensorData {
	u64 time;

	u8 Bumpwheeldrops;
	u8 Wall;
	u8 CliffLeft;
	u8 CliffFrontLeft;
	u8 CliffFrontRight;
	u8 CliffRight;
	u8 VirtualWall;
	u8 MotorOvercurrents;
	u8 DirtDetectorLeft;
	u8 DirtDetectorRight;
	u8 RemoteOpcode;
	u8 Buttons;
	u8 ChargingState;
	i8 Temperature;

	u8 __padding;
	u8 ___padding;

	i16 distance;
	i16 angle;
	i32 voltage;
	i32 current;
	f32 BatteryLevel;
	u32 ____padding;
	u32 _____padding;
};

struct RoombaConnection {
	char *output_stream;
	u64 output_cursor;
	i32 socket_handle;
	ConnectionMode mode;

	i32 distance;
	i32 angle;
};

void set_sensor_data(SensorData &sensor_data, u8 *buffer) {
	sensor_data.time = mach_absolute_time();

	sensor_data.Bumpwheeldrops    = buffer[Sensor_Bumpwheeldrops];
	sensor_data.Wall              = buffer[Sensor_Wall];
	sensor_data.CliffLeft         = buffer[Sensor_CliffLeft];
	sensor_data.CliffFrontLeft    = buffer[Sensor_CliffFrontLeft];
	sensor_data.CliffFrontRight   = buffer[Sensor_CliffFrontRight];
	sensor_data.CliffRight        = buffer[Sensor_CliffRight];
	sensor_data.VirtualWall       = buffer[Sensor_VirtualWall];
	sensor_data.MotorOvercurrents = buffer[Sensor_MotorOvercurrents];
	sensor_data.DirtDetectorLeft  = buffer[Sensor_DirtDetectorLeft];
	sensor_data.DirtDetectorRight = buffer[Sensor_DirtDetectorRight];
	sensor_data.RemoteOpcode      = buffer[Sensor_RemoteOpcode];
	sensor_data.Buttons           = buffer[Sensor_Buttons];
	sensor_data.ChargingState     = buffer[Sensor_ChargingState];
	sensor_data.Temperature       = (i8) buffer[Sensor_Temperature];

	sensor_data.distance = (i16)((buffer[Sensor_DistanceMSB] << 8) | buffer[Sensor_DistanceLSB]);
	sensor_data.angle    = (i16)((buffer[Sensor_AngleMSB] << 8) | buffer[Sensor_AngleLSB]);
	sensor_data.voltage  = buffer[Sensor_VoltageMSB]  << 8 | buffer[Sensor_VoltageLSB];
	sensor_data.current  = buffer[Sensor_CurrentMSB]  << 8 | buffer[Sensor_CurrentLSB];

	printf("----\n");
	printf("%d\n", buffer[Sensor_DistanceMSB]);
	printf("%d\n", buffer[Sensor_DistanceLSB]);
	printf("%d\n", (i32)(buffer[Sensor_DistanceLSB] << 8 | buffer[Sensor_DistanceMSB]));
	printf("%d\n", (i32)(buffer[Sensor_DistanceMSB] << 8 | buffer[Sensor_DistanceLSB]));

	i32 Charge = buffer[Sensor_ChargeMSB]   << 8 | buffer[Sensor_ChargeLSB];
	i32 Capacity = buffer[Sensor_CapacityMSB] << 8 | buffer[Sensor_CapacityLSB];

	sensor_data.BatteryLevel = (f32) Charge / (f32) Capacity;
}

static void send_command(RoombaConnection &connection, u8 header, u8 *data = 0, i32 size = 0) {
	if (connection.mode == ConnectionMode_Server) {
		u32 buffer_count = 0;
		static u8 send_buffer[512];

		send_buffer[buffer_count++] = header;
		for (i32 i = 0; i < size; ++i) {
			send_buffer[buffer_count++] = data[i];
		}

		i32 sent_size = send_socket(connection.socket_handle, send_buffer, buffer_count);
		if (sent_size <= 0) {
			close_socket(connection.socket_handle);
		}
	}
}

static void update_transmit(RoombaConnection &connection) {
	if (connection.mode == ConnectionMode_Server) {
		u8 data = 0;
		send_command(connection, COMMAND_SENSORS, &data, 0);
	}
}

static bool update_recieve(RoombaConnection &connection, i32 &distance, i32 &angle, SensorData &sensor_data) {
	if (connection.mode == ConnectionMode_Server) {
		static u8 receive_buffer[512];
		i32 recieved_size = recieve_socket(connection.socket_handle, receive_buffer, ARRAY_COUNT(receive_buffer));
		if (recieved_size <= 0) {
			distance = 0;
			angle = 0;
			return false;
		}

		printf("%d\n", recieved_size);

		set_sensor_data(sensor_data, receive_buffer);

		distance = sensor_data.distance;
		angle = sensor_data.angle;

		u8 header = 0;
		if (connection.distance != distance) {
			header |= 1<<0;
		}
		if (connection.angle != angle) {
			header |= 1<<1;
		}

		connection.distance = distance;
		connection.angle = angle;

		memcpy(connection.output_stream + connection.output_cursor, &header, sizeof(header));
		connection.output_cursor += sizeof(header);

		if (header & 1<<0) {
			i16 data = (i16)distance;
			memcpy(connection.output_stream + connection.output_cursor, &data, sizeof(data));
			connection.output_cursor += sizeof(data);
		}

		if (header & 1<<1) {
			i16 data = (i16)angle;
			memcpy(connection.output_stream + connection.output_cursor, &data, sizeof(data));
			connection.output_cursor += sizeof(data);
		}

		ASSERT(connection.output_cursor <= 128*MB, "Out of memory");
	} else {
		u8 header = *(u8*)(connection.output_stream + connection.output_cursor);
		connection.output_cursor += sizeof(header);

		distance = 0;
		angle = 0;

		if (header & 1<<0) {
			i16 data = *(i16*)(connection.output_stream + connection.output_cursor);
			connection.output_cursor += sizeof(data);
			distance = data;
		}

		if (header & 1<<1) {
			i16 data = *(i16*)(connection.output_stream + connection.output_cursor);
			connection.output_cursor += sizeof(data);
			angle = data;
		}
	}

	return true;
}

// #define DEFAULT_IP "127.0.0.1"
#define DEFAULT_IP "10.0.0.1"

void setup_connection(RoombaConnection &connection, ConnectionMode mode, MemoryArena &arena) {
	connection.mode = mode;

	if (mode == ConnectionMode_Server) {
		if (!make_socket(connection.socket_handle))
			return;

		if (!setup_client_socket(connection.socket_handle, DEFAULT_IP, DEFAULT_PORT))
			return;

		int file_descriptor_flags = fcntl(connection.socket_handle, F_GETFL);
		file_descriptor_flags |= O_NONBLOCK;
		fcntl(connection.socket_handle, F_SETFL, file_descriptor_flags);
	}
	connection.output_stream = (char*)PUSH_SIZE(arena, 128*MB);
	connection.output_cursor = 0;

	if (mode == ConnectionMode_Playback) {
		FILE *file = fopen("./roomba.stream", "rb");
		u64 filesize = get_filesize(file);
		fread(connection.output_stream, filesize, 1, file);
		fclose(file);
	}
}

void save_stream(RoombaConnection &connection) {
	ASSERT(connection.output_stream, "Trying to save a null stream!");

	FILE *file = fopen("./roomba.stream", "wb");
	fwrite(connection.output_stream, connection.output_cursor, 1, file);
	fclose(file);
}

#include "sensor_data.c"

void set_sensor_data(SensorData *sensor_data, uint8_t *buffer) {
	sensor_data->Bumpwheeldrops    = buffer[Sensor_Bumpwheeldrops];
	sensor_data->Wall              = buffer[Sensor_Wall];
	sensor_data->CliffLeft         = buffer[Sensor_CliffLeft];
	sensor_data->CliffFrontLeft    = buffer[Sensor_CliffFrontLeft];
	sensor_data->CliffFrontRight   = buffer[Sensor_CliffFrontRight];
	sensor_data->CliffRight        = buffer[Sensor_CliffRight];
	sensor_data->VirtualWall       = buffer[Sensor_VirtualWall];
	sensor_data->MotorOvercurrents = buffer[Sensor_MotorOvercurrents];
	sensor_data->DirtDetectorLeft  = buffer[Sensor_DirtDetectorLeft];
	sensor_data->DirtDetectorRight = buffer[Sensor_DirtDetectorRight];
	sensor_data->RemoteOpcode      = buffer[Sensor_RemoteOpcode];
	sensor_data->Buttons           = buffer[Sensor_Buttons];
	sensor_data->ChargingState     = buffer[Sensor_ChargingState];
	sensor_data->Temperature       = (int8_t) buffer[Sensor_Temperature];

	sensor_data->distance = (int16_t)(buffer[Sensor_DistanceMSB] << 8 | buffer[Sensor_DistanceLSB]);
	sensor_data->angle    = (int16_t)(buffer[Sensor_AngleMSB]    << 8 | buffer[Sensor_AngleLSB]);
	sensor_data->voltage  = (int16_t)(buffer[Sensor_VoltageMSB]  << 8 | buffer[Sensor_VoltageLSB]);
	sensor_data->current  = (int16_t)(buffer[Sensor_CurrentMSB]  << 8 | buffer[Sensor_CurrentLSB]);
	int16_t charge        = (int16_t)(buffer[Sensor_ChargeMSB]   << 8 | buffer[Sensor_ChargeLSB]);
	int16_t capacity      = (int16_t)(buffer[Sensor_CapacityMSB] << 8 | buffer[Sensor_CapacityLSB]);

	sensor_data->battery_level = (float) charge / (float) capacity;
}

typedef struct {
	SOCKET socket_handle;
	SensorStream stream;

	FILE *debug_output;
	FILE *debug_bin_output;
} RoombaConnection;

void send_data(RoombaConnection *connection, uint8_t *data, int size) {
	int sent_size = send_socket(connection->socket_handle, data, size);
	if (sent_size <= 0) {
		close_socket(connection->socket_handle);
	}
}

void send_command(RoombaConnection *connection, uint8_t header) {
	send_data(connection, &header, 1);
}

SensorData recieve_sensor_data(RoombaConnection *connection, uint8_t *receive_buffer, Stopwatch *timer) {
	SensorData sensor_data;
	set_sensor_data(&sensor_data, receive_buffer);

	double time = stopwatch_stop(timer);
	sensor_data.time = (float)time;

	// LOG_INFO("Data", "%g\n", time);

	SensorStream *stream = &connection->stream;
	memcpy(stream->memory + stream->cursor, &sensor_data, sizeof(SensorData));
	stream->cursor += sizeof(SensorData);

	if (connection->debug_output) {
		print_sensor_data(connection->debug_output, sensor_data);
	}

	if (connection->debug_bin_output) {
		fwrite(receive_buffer, 26, 1, connection->debug_bin_output);
	}

	return sensor_data;
}

static bool update_recieve(RoombaConnection *connection, Stopwatch *timer) {
	static uint8_t receive_buffer[512];
	static int cursor = 0;

	int received = recieve_socket(connection->socket_handle, receive_buffer + cursor, ARRAY_COUNT(receive_buffer));
	if (received <= 0) {
		return false;
	}

	LOG_INFO("Data", "Receive: %d %d\n", received, cursor);

	// I have at least received _some_ data
	int total_received = received + cursor;
	if (total_received < 26) {
		// Not done, update cursor
		cursor += received;
		return false;
	} else {
		int offset = 0;
		while (total_received >= 26) {
			assert(cursor < 26 && "Reminder should always be less than a full receive!");
			SensorData sensor_data = recieve_sensor_data(connection, receive_buffer + offset, timer);
			(void)sensor_data;
			total_received -= 26;
			offset += 26;

			LOG_INFO("Data", "Sensor data: %d %d %d %g %d\n", total_received, offset, cursor, sensor_data.battery_level, sensor_data.Temperature);
			LOG_INFO("Data", "   Bumpwheeldrops: %d\n", sensor_data.Bumpwheeldrops);
			LOG_INFO("Data", "   Wall: %d\n", sensor_data.Wall);
			LOG_INFO("Data", "   CliffLeft: %d\n", sensor_data.CliffLeft);
			LOG_INFO("Data", "   CliffFrontLeft: %d\n", sensor_data.CliffFrontLeft);
			LOG_INFO("Data", "   CliffFrontRight: %d\n", sensor_data.CliffFrontRight);
			LOG_INFO("Data", "   CliffRight: %d\n", sensor_data.CliffRight);
			LOG_INFO("Data", "   VirtualWall: %d\n", sensor_data.VirtualWall);
			LOG_INFO("Data", "   MotorOvercurrents: %d\n", sensor_data.MotorOvercurrents);
			LOG_INFO("Data", "   DirtDetectorLeft: %d\n", sensor_data.DirtDetectorLeft);
			LOG_INFO("Data", "   DirtDetectorRight: %d\n", sensor_data.DirtDetectorRight);
			LOG_INFO("Data", "   RemoteOpcode: %d\n", sensor_data.RemoteOpcode);
			LOG_INFO("Data", "   Buttons: %d\n", sensor_data.Buttons);
			LOG_INFO("Data", "   ChargingState: %d\n", sensor_data.ChargingState);
			LOG_INFO("Data", "   Temperature: %d\n", sensor_data.Temperature);
			LOG_INFO("Data", "   distance: %d\n", sensor_data.distance);
			LOG_INFO("Data", "   angle: %d\n", sensor_data.angle);
			LOG_INFO("Data", "   voltage: %d\n", sensor_data.voltage);
			LOG_INFO("Data", "   current: %d\n", sensor_data.current);
			LOG_INFO("Data", "   battery_level: %g\n", sensor_data.battery_level);
			LOG_INFO("Data", "\n");
		}
		cursor = total_received;
		for (int i = 0; i < cursor; ++i) {
			receive_buffer[i] = receive_buffer[offset + i];
		}

		return true;
	}
}

// #define DEFAULT_IP "10.0.0.1"
#define DEFAULT_IP "192.168.1.112"

// 26 bytes @ 60 Hz for 6 hours is ((60 * 26) * 60 * 60 * 6) = 33696000 bytes or 33.696 MB, so reserving 128 MB should be sufficient.
#define STREAM_MEMORY_SIZE (128*1024*1024)

void setup_connection(RoombaConnection *connection) {
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	assert(result == 0 && "WSAStartup failed!");
	if (!make_socket(&connection->socket_handle))
		return;

	if (!setup_client_socket(connection->socket_handle, DEFAULT_IP, DEFAULT_PORT))
		return;

	connection->stream.memory = VirtualAlloc(0, STREAM_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	connection->stream.cursor = 0;
}

void save_stream(RoombaConnection *connection) {
	assert(connection->stream.memory && "Trying to save a null stream!");

	const char *output_path = "../out";

	SYSTEMTIME time;	
	GetLocalTime(&time);

	char buf[512];
	snprintf(buf, ARRAY_COUNT(buf), "%s/roomba_%d-%d-%d_%d-%d-%d.stream", output_path, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

	FILE *file;
	fopen_s(&file, buf, "wb");
	assert(file);
	fwrite(connection->stream.memory, connection->stream.cursor, 1, file);
	fclose(file);
}

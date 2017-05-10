#include <time.h>
#include <mach/mach_time.h>
#include "../sci.h"
#include "engine/utils/common.h"

#include "../tcp_socket.cpp"

#include "engine/utils/platform.h"
#include "engine/utils/threading/threading.cpp"

// #define DEFAULT_IP   "10.0.0.1"

struct State {
	// Set by the simulation
	f64 distance;
	f64 angle;

	// Set by the server code
	i16 speed;
	i16 radius;

	i32 __padding;
};

void set_sensor_data(State &state, u8 *buffer) {
	u32 distance = (u32) state.distance;
	u32 angle = (u32) state.angle;

	buffer[Sensor_Bumpwheeldrops]    = 0;
	buffer[Sensor_Wall]              = 0;
	buffer[Sensor_CliffLeft]         = 0;
	buffer[Sensor_CliffFrontLeft]    = 0;
	buffer[Sensor_CliffFrontRight]   = 0;
	buffer[Sensor_CliffRight]        = 0;
	buffer[Sensor_VirtualWall]       = 0;
	buffer[Sensor_MotorOvercurrents] = 0;
	buffer[Sensor_DirtDetectorLeft]  = 0;
	buffer[Sensor_DirtDetectorRight] = 0;
	buffer[Sensor_RemoteOpcode]      = 0;
	buffer[Sensor_Buttons]           = 0;
	buffer[Sensor_ChargingState]     = 0;
	buffer[Sensor_Temperature]       = 0;

	buffer[Sensor_DistanceMSB] = (u8)(distance >> 8);
	buffer[Sensor_DistanceLSB] = (u8)distance;
	buffer[Sensor_AngleMSB]    = (u8)(angle    >> 8);
	buffer[Sensor_AngleLSB]    = (u8)angle;

	buffer[Sensor_VoltageMSB]  = 0;
	buffer[Sensor_VoltageLSB]  = 0;
	buffer[Sensor_CurrentMSB]  = 0;
	buffer[Sensor_CurrentLSB]  = 0;
	buffer[Sensor_ChargeMSB]   = 0;
	buffer[Sensor_ChargeLSB]   = 0;
	buffer[Sensor_CapacityMSB] = 0;
	buffer[Sensor_CapacityLSB] = 0;
}

static void *simulate_roomba(void *user_data) {
	State &state = *(State*)user_data;

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);

	f64 time_resolution = (f64) timebase_info.numer / (timebase_info.denom * 1.0e9);
	u64 previous_time = mach_absolute_time();

	bool running = true;
	while (running) {
		u64 current_time = mach_absolute_time();
		f64 dt = (current_time - previous_time) * time_resolution;
		previous_time = current_time;

		f64 distance = state.speed * dt;
		state.distance += distance;

		if (state.radius == 0) {
			state.angle = 0;
		} else {
			f64 angle_in_radians = state.distance / state.radius;
			f64 difference = (angle_in_radians * 258.0) / 2.0;
			state.angle = difference;
		}

		f64 delta = (mach_absolute_time() - previous_time) * time_resolution;
		f64 seconds_to_sleep = 1.0/100.0; // 100 Hz
		if (delta < seconds_to_sleep) {
			timespec t = {};

			t.tv_sec = (i32)seconds_to_sleep;
			seconds_to_sleep -= (i32)seconds_to_sleep;
			t.tv_nsec = (i64)(1e+9 * (seconds_to_sleep - delta));

			nanosleep(&t, 0);
		}
	}

	return 0;
}

i32 main(i32 argc, char **argv) {
	(void) argc;
	(void) argv;

	i32 host_socket;
	if (!make_socket(host_socket))
		return -1;

	if (!setup_host_socket(host_socket, DEFAULT_PORT, 8))
		return -1;

	State state = {};

	ThreadType thread;
	if (!create_thread(simulate_roomba, &state, &thread)) {
		printf("Error: Failed to create all the worker threads");
		return -1;
	}

	i32 client_socket = -1;
	while (true) {

		if (client_socket == -1) {
			// printf("No client, listen for connections.\n");
			accept_client(host_socket, client_socket);
		} else {
			// printf("Have client, try to recieve.\n");

			static u8 receive_buffer[512];
			i32 recieved_size = recieve_socket(client_socket, receive_buffer, ARRAY_COUNT(receive_buffer));
			if (recieved_size <= 0) {
				close_socket(client_socket);
			} else {
				if (receive_buffer[0] == COMMAND_SENSORS) {
					// printf("Sending sensor data.\n");
					u32 buffer_count = Sensor_Count;
					static u8 send_buffer[512];

					set_sensor_data(state, send_buffer);

					state.distance = 0;
					state.angle = 0;

					i32 sent_size = send_socket(client_socket, send_buffer, buffer_count);
					if (sent_size <= 0) {
						close_socket(client_socket);
					}

					// printf("Sent %d bytes\n", (i32) sent_size);
				} else if (receive_buffer[0] == COMMAND_DRIVE) {
					u32 data = *(u32*) (&receive_buffer[1]);

					state.speed = (i16)((data >> 16) & 0xFFFF);
					state.radius = (i16)(data & 0xFFFF);
				} else {
					fprintf(stderr, "Invalid command.\n");
				}
			}
		}
	}
}

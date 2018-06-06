#pragma warning(disable : 4255)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4189) // local variable is initialized but not referenced
#pragma warning(disable : 4820) // 'x' bytes padding added after data member 'name'
#pragma warning(disable : 4668)
#pragma warning(disable : 4710)

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <signal.h>

#define OS_WINDOWS

#include "utils/logger.h"
#include "utils/time.c"

#include "sci.h"
#include "tcp_socket.c"
#include "roomba_connection.c"

static RoombaConnection *_connection = 0;
void ctrlc_handler(int dummy) {
	if (_connection) {
		send_command(_connection, COMMAND_STOP);
		save_stream(_connection);

		if (_connection->debug_output) {
			fclose(_connection->debug_output);
			_connection->debug_output = 0;
		}

		if (_connection->debug_bin_output) {
			fclose(_connection->debug_bin_output);
			_connection->debug_bin_output = 0;
		}
	}
	exit(0);
}

void setup(RoombaConnection *connection) {
	{
		// HACK(kalle): I can't get a sleeping roomba to wake unless I terminate a connection... So I create a temp connection, that I immediately shut down.
		setup_connection(connection);
		close_socket(connection->socket_handle);
	}
	setup_connection(connection);

	{
		FILE *file;
		fopen_s(&file, "../out/test.txt", "w");
		assert(file);
		connection->debug_output = file;
	}
	{
		FILE *file;
		fopen_s(&file, "../out/test.bin", "wb");
		assert(file);
		connection->debug_bin_output = file;
	}
}

void run() {
	bool sent = 0;
	bool started = 0;
	bool running = true;

	double time_running = 0;

	signal(SIGINT, ctrlc_handler);

	RoombaConnection connection;
	setup(&connection);
	_connection = &connection;

	double update_frequency = 2; // 1 time per second

	Stopwatch sw;
	stopwatch_init(&sw);

	Stopwatch timer;
	stopwatch_init(&timer);

	while (running) {
		stopwatch_start(&sw);

		if (!started) {
			// static bool sent_start = false;
			// static bool sent_full = false;
			// static bool sent_drive = false;
			// if (time_running >= 1.0 && !sent_start) {
			// 	LOG_INFO("Server", "Sending start");
			// 	send_command(&connection, COMMAND_START);
			// 	send_command(&connection, COMMAND_SAFE);
			// 	sent_start = true;
			// 	started = true;
			// }

			if (time_running >= 2.0) {
				send_data(&connection, (uint8_t[]){COMMAND_START, COMMAND_SAFE, COMMAND_CLEAN}, 3);
				// send_command(&connection, COMMAND_SAFE);
				// send_command(&connection, COMMAND_CLEAN);
				started = true;

				// 10 times per second if sending the sensor packet 28 * 10 * 8 = 2240 bits.
				// 30 times per second if sending the sensor packet 28 * 30 * 8 = 6720 bits.
				// 60 times per second if sending the sensor packet 28 * 60 * 8 = 13440 bits.
				// update_frequency = 2.0; 
			}
		} else {
			// send_data(&connection, (uint8_t[]){137, 0, 200, 0, 0}, 5);
			if (update_recieve(&connection, &timer)) {
				sent = 0;
			}

			if (!sent) {
				send_data(&connection, (uint8_t[]){COMMAND_SENSORS, 0}, 2);
				stopwatch_start(&timer);
				sent = 1;
			}
		}

		double actual_time = stopwatch_stop(&sw);
		double wanted_time = 1.0 / update_frequency;
		double time_to_sleep = wanted_time - actual_time;
		if (time_to_sleep > 0) {
			int ms_to_sleep = (int)(time_to_sleep * 1000);
			LOG_INFO("Server", "Sleep %d\n", ms_to_sleep);
			Sleep(ms_to_sleep);
		}

		time_running += wanted_time;
	}

	send_command(&connection, COMMAND_STOP);
	save_stream(&connection);
	if (_connection->debug_output) {
		fclose(_connection->debug_output);
		_connection->debug_output = 0;
	}
}

int main(int argc, char const *argv[]) {
	run();
	return 0;
}

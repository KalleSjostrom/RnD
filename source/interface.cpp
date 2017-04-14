#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <time.h>
#include <mach/mach_time.h>

#include "app.h"
#include "../mygl/mygl.h"

#define KB 1024
#define MB 1024 * KB
#define GB 1024 * MB

struct AppCode {
	void* lib_handle;
	app_update_t* app_update;
	app_reload_t* app_reload;
	bool valid;
	time_t timestamp;
};
static bool running = false;

static time_t get_timestamp(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) != -1) {
		return statbuf.st_mtime;
	}
	return 0;
}

APP_UPDATE(app_update_stub) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return 0;
}

static AppCode load_app_code(char const *lib_path, time_t timestamp) {
	AppCode app = {0};

	void *lib_handle = dlopen(lib_path, RTLD_NOW | RTLD_NODELETE);
	if (lib_handle) {
		void* update = dlsym(lib_handle, "app_update");
		if (update) {
			app.lib_handle = lib_handle;
			app.app_update = (app_update_t*) update;
			app.app_reload = (app_reload_t*) dlsym(lib_handle, "app_reload");
			app.valid = true;
			app.timestamp = timestamp;
		} else {
			dlclose(lib_handle);
		}
	}

	if (app.app_update == 0) {
		app.app_update = (app_update_t*) app_update_stub;
		app.valid = false;
		printf("Loading code failed!\n");
	} else {
		printf("code loaded!\n");
	}

	return app;
}

static void unload_app_code(AppCode &app) {
	int ret = dlclose(app.lib_handle);
	if (ret != 0)
		printf("Error closing lib handle (code=%d)\n", ret);

	app.lib_handle = 0;
	app.valid = false;
	printf("code unloaded!\n");
 	app.app_update = (app_update_t*) app_update_stub;
}

#define APP_MEMORY_SIZE (MB*16)
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))

static void run() {
	running = true;

	void *memory_chunks[2] = {
		malloc(APP_MEMORY_SIZE),
		malloc(APP_MEMORY_SIZE),
	};
	unsigned memory_index = 0;
	void *memory = memory_chunks[memory_index];

	mygl_init();
	GLWindowHandle *window = mygl_create_window(1024, 768, "RnD");
	ASSERT(window);

	char const *lockfile_path = "./out/__lockfile";
	time_t timestamp = get_timestamp(lockfile_path);

	char const *lib_path = "./out/app.dylib";
	// AppCode app = load_app_code(lib_path, timestamp);

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);

	double time_resolution = (double) timebase_info.numer / (timebase_info.denom * 1.0e9);
	uint64_t current_time;
	uint64_t previous_time = mach_absolute_time();

	double fps_current_seconds = 0;
	u32 fps_frame_count = 0;

	GLuint vertex_array_object;
	GLuint vertex_buffer_object;

	glGenVertexArrays(1, &vertex_array_object);
	glBindVertexArray(vertex_array_object);

	static const GLfloat g_vertex_buffer_data[] = {
	   -1.0f, -1.0f, 0.0f,
	   1.0f, -1.0f, 0.0f,
	   0.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &vertex_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	while (running) {
		// if (app.valid) {
		// 	time_t timestamp = get_timestamp(lockfile_path);
		// 	if (timestamp > app.timestamp) {
		// 		unload_app_code(app);

		// 		app = load_app_code(lib_path, timestamp);

		// 		void *old_memory = memory_chunks[memory_index];
		// 		memory_index = (memory_index + 1) % ArrayCount(memory_chunks);
		// 		void *new_memory = memory_chunks[memory_index];
		// 		app.app_reload(old_memory, new_memory);

		// 		memory = memory_chunks[memory_index];
		// 	}
		// }

		current_time = mach_absolute_time();
		double dt = (current_time - previous_time) * time_resolution;
		previous_time = current_time;
		{ // _update_fps_counter(window, dt);
			fps_current_seconds += dt;
			fps_frame_count++;
			if (fps_current_seconds > 0.25) {
				double fps = fps_frame_count / fps_current_seconds;
				char tmp[128];
				sprintf(tmp, "opengl @ fps: %.2f", fps);
				mygl_set_window_title(window, tmp);
				fps_current_seconds = 0;
				fps_frame_count = 0;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
		glVertexAttribPointer(
		   0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		   3,                  // size
		   GL_FLOAT,           // type
		   GL_FALSE,           // normalized?
		   0,                  // stride
		   (void*)0            // array buffer offset
		);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		// if (app.app_update(memory, dt) < 0) {
		// 	unload_app_code(app);
		// }

#ifdef USE_OPENGL
		mygl_swap_buffers(window);
		mygl_poll_events();
#endif
	}

	for (int i = 0; i < ArrayCount(memory_chunks); ++i) {
		free(memory_chunks[i]);
	}
}

int main(int argc, char *argv[]) {
	run();
	return 0;
}
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <time.h>
#include <mach/mach_time.h>

#include "plugin.h"
#include "mygl/mygl.h"

// Ignore some warnings for third party stuff.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
	// #define GL3_PROTOTYPES 1
	#include <OpenGL/gl3.h>
	#include <SDL2/SDL.h>
#pragma clang diagnostic pop

static float ENGINE_TIME = 0;

#include "engine/input.cpp"
#include "engine/audio.cpp"

struct Plugin {
	void* lib_handle;
	plugin_update_t* update;
	plugin_reload_t* reload;
	time_t timestamp;
	b32 valid;
	i32 __padding;
};
static bool running = false;

static void quit() {
	running = false;
}

static time_t get_timestamp(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) != -1) {
		return statbuf.st_mtime;
	}
	return 0;
}

PLUGIN_UPDATE(plugin_update_stub) {
	(void)memory;
	(void)engine;
	(void)screen_width;
	(void)screen_height;
	(void)input;
	(void)dt;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return 0;
}

static Plugin load_plugin_code(char const *lib_path, time_t timestamp) {
	Plugin plugin = {};

	void *lib_handle = dlopen(lib_path, RTLD_NOW | RTLD_NODELETE);
	if (lib_handle) {
		void *update = dlsym(lib_handle, "update");
		if (update) {
			plugin.lib_handle = lib_handle;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
			plugin.update = (plugin_update_t*) update;
			plugin.reload = (plugin_reload_t*) dlsym(lib_handle, "reload");
#pragma clang diagnostic pop

			plugin.valid = true;
			plugin.timestamp = timestamp;
		} else {
			dlclose(lib_handle);
		}
	}

	if (plugin.update == 0) {
		plugin.update = (plugin_update_t*) plugin_update_stub;
		plugin.valid = false;
		printf("Loading code failed!\n");
	} else {
		printf("code loaded!\n");
	}

	return plugin;
}

static void unload_plugin_code(Plugin &plugin) {
	int ret = dlclose(plugin.lib_handle);
	if (ret != 0)
		printf("Error closing lib handle (code=%d)\n", ret);

	plugin.lib_handle = 0;
	plugin.valid = false;
	printf("code unloaded!\n");
	plugin.update = (plugin_update_t*) plugin_update_stub;
}

#define PLUGIN_MEMORY_SIZE (MB*16)

static void run(const char *plugin_path) {
	running = true;

	void *memory_chunks[2] = {
		malloc(PLUGIN_MEMORY_SIZE),
		malloc(PLUGIN_MEMORY_SIZE),
	};
	int32_t memory_index = 0;
	void *memory = memory_chunks[memory_index];

	GLWindowHandle *window = mygl_setup(RES_WIDTH, RES_HEIGHT, "Engine");
	mygl_enter_fullscreen_mode(window);

	InputApi input_api = {};
	input_api.key_down = key_down;
	input_api.key_up = key_up;
	mygl_set_input_api(window, &input_api);

	int width, height;
	mygl_get_framebuffer_size(window, &width, &height);
	printf("%d, %d\n", width, height);

	char const *lockfile_path = "./__lockfile";

	Plugin plugin = load_plugin_code(plugin_path, get_timestamp(lockfile_path));

	mach_timebase_info_data_t timebase_info;
	mach_timebase_info(&timebase_info);

	double time_resolution = (double) timebase_info.numer / (timebase_info.denom * 1.0e9);
	uint64_t current_time;
	uint64_t previous_time = mach_absolute_time();

	double fps_current_seconds = 0;
	int32_t fps_frame_count = 0;

	SDL_Init(SDL_INIT_AUDIO); // Only audio for now
	audio::open();

	EngineApi engine;

	engine.audio_load = audio::load;
	engine.audio_queue = audio::queue;
	engine.audio_queued_size = audio::queued_size;
	engine.audio_free = audio::free;
	engine.audio_set_playing = audio::set_playing;

	engine.quit = quit;

	engine.audio_set_playing(true);

	while (running) {
		if (plugin.valid) {
			time_t timestamp = get_timestamp(lockfile_path);
			if (timestamp > plugin.timestamp) {
				unload_plugin_code(plugin);

				plugin = load_plugin_code(plugin_path, timestamp);

				// void *old_memory = memory_chunks[memory_index];
				// memory_index = (memory_index + 1) % ARRAY_COUNT(memory_chunks);
				// void *new_memory = memory_chunks[memory_index];
				// plugin.reload(old_memory, new_memory);
				// memory = memory_chunks[memory_index];

				plugin.reload(memory, width, height);
				printf("Plugin reloaded\n");
			}
		}

		current_time = mach_absolute_time();
		double dt = (current_time - previous_time) * time_resolution;
		previous_time = current_time;
		{
			fps_current_seconds += dt;
			fps_frame_count++;
			if (fps_current_seconds > 0.25) {
				double fps = fps_frame_count / fps_current_seconds;
				char tmp[128];
				sprintf(tmp, "opengl @ fps: %.2f (valid=%d)", fps, plugin.valid);
				mygl_set_window_title(window, tmp);
				fps_current_seconds = 0;
				fps_frame_count = 0;
			}
		}

		if (plugin.update(memory, engine, width, height, _input, (float)dt) < 0) {
			unload_plugin_code(plugin);
		}
		for (int32_t i = 0; i < InputKey_Count; ++i) {
			_input.pressed[i] = false;
			_input.released[i] = false;
		}
		ENGINE_TIME += dt;

		mygl_swap_buffers(window);
		mygl_poll_events();
	}

	for (u32 i = 0; i < ARRAY_COUNT(memory_chunks); ++i) {
		free(memory_chunks[i]);
	}
}

int main(int argc, char *argv[]) {
	(void)argc;
	run(argv[1]);
	return 0;
}

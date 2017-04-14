#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <time.h>
#include <mach/mach_time.h>

static float GAME_TIME = 0;

#include "game.h"
#include "../mygl/mygl.h"
#include "gl_program_builder.cpp"

// #define GL3_PROTOTYPES 1
#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>

#include "engine/input.cpp"
#include "engine/audio.cpp"

struct GameCode {
	void* lib_handle;
	game_update_t* update;
	game_reload_t* reload;
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

GAME_UPDATE(game_update_stub) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return 0;
}

static GameCode load_game_code(char const *lib_path, time_t timestamp) {
	GameCode game = {0};

	void *lib_handle = dlopen(lib_path, RTLD_NOW | RTLD_NODELETE);
	if (lib_handle) {
		void* update = dlsym(lib_handle, "update");
		if (update) {
			game.lib_handle = lib_handle;
			game.update = (game_update_t*) update;
			game.reload = (game_reload_t*) dlsym(lib_handle, "reload");
			game.valid = true;
			game.timestamp = timestamp;
		} else {
			dlclose(lib_handle);
		}
	}

	if (game.update == 0) {
		game.update = (game_update_t*) game_update_stub;
		game.valid = false;
		printf("Loading code failed!\n");
	} else {
		printf("code loaded!\n");
	}

	return game;
}

static void unload_game_code(GameCode &game) {
	int ret = dlclose(game.lib_handle);
	if (ret != 0)
		printf("Error closing lib handle (code=%d)\n", ret);

	game.lib_handle = 0;
	game.valid = false;
	printf("code unloaded!\n");
	game.update = (game_update_t*) game_update_stub;
}

#define GAME_MEMORY_SIZE (MB*16)
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))

static void run() {
	running = true;

	void *memory_chunks[2] = {
		malloc(GAME_MEMORY_SIZE),
		malloc(GAME_MEMORY_SIZE),
	};
	int32_t memory_index = 0;
	void *memory = memory_chunks[memory_index];

	GLWindowHandle *window = mygl_setup(RES_WIDTH, RES_HEIGHT, "In Shadows");
	mygl_enter_fullscreen_mode(window);

	InputApi input_api = {};
	input_api.key_down = key_down;
	input_api.key_up = key_up;
	mygl_set_input_api(window, &input_api);

	int width, height;
	mygl_get_framebuffer_size(window, &width, &height);
	printf("%d, %d\n", width, height);

	char const *lockfile_path = "./out/__lockfile";
	time_t timestamp = get_timestamp(lockfile_path);

	char const *lib_path = "./out/game.dylib";
	GameCode game = load_game_code(lib_path, timestamp);

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

	engine.audio_set_playing(true);

	while (running) {
// #ifndef USE_MYGL
// 		while (SDL_PollEvent(&event)) {
// 			if (event.type == SDL_QUIT) {
// 				running = false;
// 				break;
// 			}
// 		}
// #endif // USE_MYGL

			if (game.valid) {
				time_t timestamp = get_timestamp(lockfile_path);
				if (timestamp > game.timestamp) {
					unload_game_code(game);

					game = load_game_code(lib_path, timestamp);

					// void *old_memory = memory_chunks[memory_index];
					// memory_index = (memory_index + 1) % ArrayCount(memory_chunks);
					// void *new_memory = memory_chunks[memory_index];
					// game.reload(old_memory, new_memory);
					// memory = memory_chunks[memory_index];

					game.reload(memory, width, height);
					printf("Game reloaded\n");
				}
			}

			current_time = mach_absolute_time();
			double dt = (current_time - previous_time) * time_resolution;
			previous_time = current_time;
			{ // _update_fps_counter(window, dt);
				fps_current_seconds += dt;
				fps_frame_count++;
				if (fps_current_seconds > 0.25) {
					double fps = fps_frame_count / fps_current_seconds;
					char tmp[128];
					sprintf(tmp, "opengl @ fps: %.2f (valid=%d)", fps, game.valid);
					mygl_set_window_title(window, tmp);
					fps_current_seconds = 0;
					fps_frame_count = 0;
				}
			}

			if (game.update(memory, engine, width, height, input, dt) < 0) {
				unload_game_code(game);
			}
			for (int32_t i = 0; i < InputKey_Count; ++i) {
				input.pressed[i] = false;
				input.released[i] = false;
			}
			GAME_TIME += dt;

		mygl_swap_buffers(window);
		mygl_poll_events();
	}

	for (int i = 0; i < ArrayCount(memory_chunks); ++i) {
		free(memory_chunks[i]);
	}
}

int main(int argc, char *argv[]) {
	run();
	return 0;
}
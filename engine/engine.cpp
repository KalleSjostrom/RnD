#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

#include "common.h"
#include "plugin.h"

#include "include/SDL.h"

#include "modules/error.h"
#include "modules/logging.h"
#include "modules/input.h"
#include "modules/audio.h"
#include "modules/image.h"
#include "modules/reloader.h"

#include "core/utils/string_id.h"

#include "core/memory/allocator.cpp"
#include "core/memory/mspace_allocator.cpp"
#include "core/memory/arena_allocator.cpp"
#include "core/utils/murmur_hash.cpp"
#include "core/containers/array.cpp"
#include "core/containers/hashmap.cpp"
#include "core/utils/dynamic_string.cpp"
#include "core/utils/string.cpp"

#include "plugin.cpp"

__forceinline u64 absolute_time(void) {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
}

struct TimeInfo {
	i64 performance_count_frequency;
};

void setup_time(TimeInfo &t) {
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	t.performance_count_frequency = PerfCountFrequencyResult.QuadPart;
}
double time_in_seconds(TimeInfo &t, u64 time) {
	return  ((double)(time) / (double) t.performance_count_frequency);
}

static bool running = false;
void quit() {
	running = false;
}

static i32 _screen_width;
static i32 _screen_height;
static void screen_dimensions(i32 &screen_width, i32 &screen_height) {
	screen_width = _screen_width;
	screen_height = _screen_height;
}

static void run(const char *plugin_directory, const char *plugin_name) {
	log_init(); // We need logging asap!
	error_init();

	running = true;

	Allocator allocator = allocator_mspace();

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		ASSERT(0, "SDL_Init failed: %s", SDL_GetError());
	}
	image_init();
	audio_init();

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	i32 resolution_width = 1024;
	i32 resolution_height = 768;
	SDL_Window *window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution_width, resolution_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	(void)glContext;

	SDL_GL_GetDrawableSize(window, &_screen_width, &_screen_height);
	// SDL_GL_SetSwapInterval(0);

	EngineApi engine;

	engine.audio.load = audio_load;
	engine.audio.queue = audio_queue;
	engine.audio.queued_size = audio_queued_size;
	engine.audio.free = audio_free;
	engine.audio.set_playing = audio_set_playing;

	engine.image.load = image_load;
	engine.screen_dimensions = screen_dimensions;

	engine.error.report_assert_failure = report_script_assert_failure;

	engine.logging.log_info = log_info;
	engine.logging.log_warning = log_warning;
	engine.logging.log_error = log_error;

	engine.quit = quit;

	InputData input = {};

	Plugin plugin = make_plugin(&allocator, plugin_directory, plugin_name);
	plugin.setup(&plugin.allocator, &engine, &input);

	TimeInfo time = {};
	setup_time(time);
	u64 engine_time = 0;
	u64 current_time = 0;
	u64 previous_time = absolute_time();

	double fps_current_seconds = 0;
	i32 fps_frame_count = 0;
	while (running) {
#if !MYGL
		SDL_Event sdl_event = {};
		input.mouse_xrel = 0;
		input.mouse_yrel = 0;
		while (SDL_PollEvent(&sdl_event) != 0) {
			switch (sdl_event.type) {
				case SDL_QUIT: {
					running = false;
				} break;
				case SDL_MOUSEMOTION: {
					mouse_motion(input, sdl_event.motion.x, sdl_event.motion.y, sdl_event.motion.xrel, sdl_event.motion.yrel);
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					mouse_down(input, engine_time, sdl_event.button.button);
				} break;
				case SDL_MOUSEBUTTONUP: {
					mouse_up(input, engine_time, sdl_event.button.button);
				} break;
				case SDL_KEYDOWN: {
					if (sdl_event.key.repeat == 0)
						key_down(input, engine_time, sdl_event.key.keysym.sym, sdl_event.key.keysym.mod);
				} break;
				case SDL_KEYUP: {
					if (sdl_event.key.repeat == 0)
						key_up(input, engine_time, sdl_event.key.keysym.sym, sdl_event.key.keysym.mod);
				} break;
			}
		}
#endif

		if (plugin.valid) {
			check_for_reloads(plugin);
		}

		current_time = absolute_time();
		u64 delta = current_time - previous_time;
		double dt = time_in_seconds(time, delta);
		previous_time = current_time;

		{
			fps_current_seconds += dt;
			fps_frame_count++;
			if (fps_current_seconds > 0.25) {
				double fps = fps_frame_count / fps_current_seconds;
				char tmp[128];
				snprintf(tmp, ARRAY_COUNT(tmp), "opengl @ fps: %.2f (valid=%d)", fps, plugin.valid);
				SDL_SetWindowTitle(window, tmp);
				fps_current_seconds = 0;
				fps_frame_count = 0;
			}
		}

		plugin.update(&plugin.allocator, (float) dt);
		log_update();

		for (i32 i = 0; i < InputKey_Count; ++i) {
			input.pressed[i] = false;
			input.released[i] = false;
		}
		engine_time += delta;

		SDL_GL_SwapWindow(window);
	}

	image_deinit();
	SDL_Quit();

	destroy_plugin(plugin);

	error_deinit();
	log_deinit();
}

#include <shellapi.h>
int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
	(void) Instance;
	(void) PrevInstance;
	(void) ShowCode;

	char *argv[8] = {};
	int argc = 0;
	argv[0] = CommandLine;

	int cursor = 0;
	while(1) {
		if (CommandLine[cursor] == ' ') {
			argv[argc][cursor] = '\0';
			argc++;
			cursor++;
			argv[argc] = CommandLine + cursor;
			continue;
		} else if (CommandLine[cursor] == '\0') {
			if (cursor > 0) {
				argc++;
			}
			break;
		}

		cursor++;
	}
	// ASSERT(argc == 2, "Usage engine.exe path/to/plugin name_of_plugin")
	run(argv[0], argv[1]);
}

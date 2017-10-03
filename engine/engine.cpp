#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include "include/SDL.h"
#include "include/SDL_image.h"
#pragma clang diagnostic pop

#include "plugin.h"

#ifdef OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"

	#define LIB_HANDLE HMODULE
	#define load_library(lib_path) LoadLibrary(lib_path)
	#define get_symbol_address(lib_handle, symbol) GetProcAddress(lib_handle, symbol)
	#define unload_library(lib_handle) FreeLibrary(lib_handle)

	FORCE_INLINE LARGE_INTEGER absolute_time(void) {
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		return time;
	}

	struct Time {
		i64 performance_count_frequency;
	};

	void setup_time(Time &t) {
		LARGE_INTEGER PerfCountFrequencyResult;
		QueryPerformanceFrequency(&PerfCountFrequencyResult);
		t.performance_count_frequency = PerfCountFrequencyResult.QuadPart;
	}
	double time_in_seconds(Time &t, u64 ticks) {
		return  ((double)(ticks) / (double) t.performance_count_frequency);
	}
#else
	#include <mach/mach_time.h>
	#include <unistd.h>
	#include <dlfcn.h>

	#define LIB_HANDLE void*
	#define load_library(lib_path) dlopen(lib_path, RTLD_NOW | RTLD_NODELETE)
	#define get_symbol_address(lib_handle, symbol) dlsym(lib_handle, symbol)
	#define unload_library(lib_handle) dlclose(lib_handle)

	FORCE_INLINE u64 absolute_time(void) {
		return mach_absolute_time();
	}

	struct Time {
		double resolution;
	};

	void setup_time(Time &t) {
		mach_timebase_info_data_t timebase_info;
		mach_timebase_info(&timebase_info);

		t.resolution = (double) timebase_info.numer / (timebase_info.denom * 1.0e9);
	}
	double time_in_seconds(Time &t, u64 ticks) {
		return ticks * t.resolution;
	}
#endif

#define PLUGIN_MEMORY_SIZE (MB*16)

static u64 ENGINE_TIME;

#include "engine/input.cpp"
#include "engine/audio.cpp"

struct Plugin {
	LIB_HANDLE lib_handle;
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
	return 0;
}

static Plugin load_plugin_code(const char *lib_path, time_t timestamp) {
	Plugin plugin = {};

	LIB_HANDLE lib_handle = load_library(lib_path);
	if (lib_handle) {
		void *update = get_symbol_address(lib_handle, "update");
		if (update) {
			plugin.lib_handle = lib_handle;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
			plugin.update = (plugin_update_t*) update;
			plugin.reload = (plugin_reload_t*) get_symbol_address(lib_handle, "reload");
#pragma clang diagnostic pop

			plugin.valid = true;
			plugin.timestamp = timestamp;
		} else {
			unload_library(lib_handle);
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
	int ret = unload_library(plugin.lib_handle);
	if (ret != 0)
		printf("Error closing lib handle (code=%d)\n", ret);

	plugin.lib_handle = 0;
	plugin.valid = false;
	printf("code unloaded!\n");
	plugin.update = (plugin_update_t*) plugin_update_stub;
}

static b32 image_load(const char *filepath, ImageData &data) {
	SDL_Surface *surface = IMG_Load(filepath);
	if (!surface) {
		printf("Image load failed: %s", IMG_GetError());
		return false;
	}

	data.bytes_per_pixel = surface->format->BytesPerPixel;
	data.width = surface->w;
	data.height = surface->h;
	data.pixels = surface->pixels;

	return true;
}

static void run(const char *plugin_path) {
	running = true;

	void *memory_chunks[2] = {
		malloc(PLUGIN_MEMORY_SIZE),
		malloc(PLUGIN_MEMORY_SIZE),
	};
	i32 memory_index = 0;
	void *memory = memory_chunks[memory_index];

	char const *lockfile_path = "./__lockfile";

	Plugin plugin = load_plugin_code(plugin_path, get_timestamp(lockfile_path));

	Time time = {};
	setup_time(time);
	u64 current_time = 0;
	u64 previous_time = absolute_time();

	double fps_current_seconds = 0;
	i32 fps_frame_count = 0;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) {
		ASSERT(0, "SDL_Init failed: %s", IMG_GetError());
	}
	if (!IMG_Init(IMG_INIT_JPG)) {
		ASSERT(0, "IMG_Init failed: %s", IMG_GetError());
	}
	audio::open();

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Window *window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RES_WIDTH, RES_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	(void)glContext;

	EngineApi engine;

	engine.audio_load = audio::load;
	engine.audio_queue = audio::queue;
	engine.audio_queued_size = audio::queued_size;
	engine.audio_free = audio::free;
	engine.audio_set_playing = audio::set_playing;

	engine.image_load = image_load;

	engine.quit = quit;
	engine.audio_set_playing(true);

	int width = RES_WIDTH;
	int height = RES_HEIGHT;

	SDL_Event sdl_event;
	while (running) {
		while (SDL_PollEvent(&sdl_event) != 0) {
			if (sdl_event.type == SDL_QUIT) {// Esc button is pressed
				running = false;
			}
		}

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
				sprintf(tmp, "opengl @ fps: %.2f (valid=%d)", fps, plugin.valid);
				SDL_SetWindowTitle(window, tmp);
				fps_current_seconds = 0;
				fps_frame_count = 0;
			}
		}

		if (plugin.update(memory, engine, width, height, _input, (float)dt) < 0) {
			unload_plugin_code(plugin);
		}
		for (i32 i = 0; i < InputKey_Count; ++i) {
			_input.pressed[i] = false;
			_input.released[i] = false;
		}
		ENGINE_TIME += delta;

		SDL_GL_SwapWindow(window);
	}

	IMG_Quit();
	SDL_Quit();

	for (u32 i = 0; i < ARRAY_COUNT(memory_chunks); ++i) {
		free(memory_chunks[i]);
	}
}

int main(int argc, char *argv[]) {
	(void)argc;
	run(argv[1]);
	return 0;
}

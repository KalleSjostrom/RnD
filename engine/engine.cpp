#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include "include/SDL.h"
#include "include/SDL_image.h"
#pragma clang diagnostic pop

#include "mygl/mygl.h"

#include "plugin.h"

#define MYGL 0

#include <stdio.h>
#include "utils/memory/memory_arena.cpp"
#include "utils/string.h"
struct ReloadInfo {
	String reload_marker_path;
	String directory_path;
	String plugin_path;
	String plugin_pattern;
};

#ifdef OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"

	#define LIB_HANDLE HMODULE
	#define load_library(lib_path) LoadLibrary(lib_path)
	#define get_symbol_address(lib_handle, symbol) GetProcAddress(lib_handle, symbol)
	#define unload_library(lib_handle) FreeLibrary(lib_handle)

	void update_plugin_path(ReloadInfo &reload) {
		WIN32_FIND_DATA best_data = {};
		WIN32_FIND_DATA find_data;
		HANDLE handle = FindFirstFile(*reload.plugin_pattern, &find_data);
		if (handle == INVALID_HANDLE_VALUE)
			return;
		do {
			FILETIME time = find_data.ftCreationTime;
			FILETIME best_time = best_data.ftCreationTime;
			if (time.dwHighDateTime > best_time.dwHighDateTime || (time.dwHighDateTime == best_time.dwHighDateTime && time.dwLowDateTime > best_time.dwLowDateTime)) {
				best_data = find_data;
			}

		} while (FindNextFile(handle, &find_data));

		FindClose(handle);

		String plugin_name_string = make_string((char*)best_data.cFileName);

		free(reload.plugin_path.text);
		reload.plugin_path.text = (char*) malloc((size_t)(reload.directory_path.length + 1 + plugin_name_string.length + 1));
		reload.plugin_path.length = 0;
		append_path(reload.plugin_path, 2, &reload.directory_path, &plugin_name_string);
	}

	FORCE_INLINE u64 absolute_time(void) {
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

	struct TimeInfo {
		double resolution;
	};

	typedef u64 u64;

	void setup_time(TimeInfo &t) {
		mach_timebase_info_data_t timebase_info;
		mach_timebase_info(&timebase_info);

		t.resolution = (double) timebase_info.numer / (timebase_info.denom * 1.0e9);
	}
	double time_in_seconds(TimeInfo &t, u64 ticks) {
		return ticks * t.resolution;
	}
	void update_plugin_path(ReloadInfo &reload) {
		(void) reload;
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
	(void)ret;
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

	printf("%s\n", SDL_GetPixelFormatName(surface->format->format));

	data.pixels = surface->pixels;
	data.bytes_per_pixel = surface->format->BytesPerPixel;
	data.width = surface->w;
	data.height = surface->h;

	switch(surface->format->format) {
		case SDL_PIXELFORMAT_RGBA32: { data.format = PixelFormat_RGBA; } break;
		case SDL_PIXELFORMAT_BGRA32: { data.format = PixelFormat_ARGB; } break;
		default: {
			ASSERT(0, "Unsuported pixel format!");
			return false;
		}
	};

	return true;
}

static void run(const char *plugin_directory, const char *plugin_name) {
	running = true;

	void *memory_chunks[2] = {
		malloc(PLUGIN_MEMORY_SIZE),
		malloc(PLUGIN_MEMORY_SIZE),
	};
	i32 memory_index = 0;
	void *memory = memory_chunks[memory_index];

	String plugin_directory_string = make_string((char *)plugin_directory);
	String plugin_name_string = make_string((char *)plugin_name);

	MemoryArena arena = {};

	String reload_marker_string = MAKE_STRING("__reload_marker");
	String plugin_patter_string = MAKE_STRING("*.dll");

	ReloadInfo reload = {};
	reload.reload_marker_path = make_path(arena, plugin_directory_string, reload_marker_string);
	reload.plugin_pattern = make_path(arena, plugin_directory_string, plugin_patter_string);
	reload.directory_path = plugin_directory_string;

	reload.plugin_path.text = (char*) malloc((size_t)(reload.directory_path.length + 1 + plugin_name_string.length + 1));
	reload.plugin_path.length = 0;
	append_path(reload.plugin_path, 2, &reload.directory_path, &plugin_name_string);

	Plugin plugin = load_plugin_code(*reload.plugin_path, get_timestamp(*reload.reload_marker_path));

	TimeInfo time = {};
	setup_time(time);
	u64 current_time = 0;
	u64 previous_time = absolute_time();

	double fps_current_seconds = 0;
	i32 fps_frame_count = 0;

	u32 sdl_subsystems = SDL_INIT_AUDIO;

	int width, height;

#if MYGL
	GLWindowHandle *window = mygl_setup(RES_WIDTH, RES_HEIGHT, "Engine");
	mygl_get_framebuffer_size(window, &width, &height);
	printf("%d %d\n", width, height);
#else
	sdl_subsystems = SDL_INIT_AUDIO | SDL_INIT_VIDEO;
#endif

	if (SDL_Init(sdl_subsystems)) {
		ASSERT(0, "SDL_Init failed: %s", IMG_GetError());
	}
	if (!IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG)) {
		ASSERT(0, "IMG_Init failed: %s", IMG_GetError());
	}
	audio::open();

#if !MYGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Window *window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RES_WIDTH, RES_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	(void)glContext;

	SDL_GL_GetDrawableSize(window, &width, &height);
#endif

	EngineApi engine;

	engine.audio_load = audio::load;
	engine.audio_queue = audio::queue;
	engine.audio_queued_size = audio::queued_size;
	engine.audio_free = audio::free;
	engine.audio_set_playing = audio::set_playing;

	engine.image_load = image_load;

	engine.quit = quit;
	engine.audio_set_playing(true);

	while (running) {
#if !MYGL
		SDL_Event sdl_event;
		while (SDL_PollEvent(&sdl_event) != 0) {
			switch (sdl_event.type) {
				case SDL_QUIT: {
					running = false;
				} break;
				case SDL_KEYDOWN: {
					if (sdl_event.key.repeat == 0)
						key_down(sdl_event.key.keysym.sym, 0);
				} break;
				case SDL_KEYUP: {
					if (sdl_event.key.repeat == 0)
						key_up(sdl_event.key.keysym.sym, 0);
				} break;
			}
		}
#endif

		if (plugin.valid) {
			time_t timestamp = get_timestamp(*reload.reload_marker_path);
			if (timestamp > plugin.timestamp) {
				unload_plugin_code(plugin);

				update_plugin_path(reload);
				plugin = load_plugin_code(*reload.plugin_path, timestamp);

				// void *old_memory = memory_chunks[memory_index];
				// memory_index = (memory_index + 1) % ARRAY_COUNT(memory_chunks);
				// void *new_memory = memory_chunks[memory_index];
				// plugin.reload(old_memory, new_memory);
				// memory = memory_chunks[memory_index];

				plugin.reload(memory, engine, width, height, _input);
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
				snprintf(tmp, ARRAY_COUNT(tmp), "opengl @ fps: %.2f (valid=%d)", fps, plugin.valid);
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

#if MYGL
		mygl_swap_buffers(window);
		mygl_poll_events();
#else
		SDL_GL_SwapWindow(window);
#endif
	}

	IMG_Quit();
	SDL_Quit();

	for (u32 i = 0; i < ARRAY_COUNT(memory_chunks); ++i) {
		free(memory_chunks[i]);
	}
}

#ifdef OS_WINDOWS
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
	ASSERT(argc == 2, "Usage engine.exe path/to/plugin name_of_plugin")
	run(argv[0], argv[1]);
}
#else
int main(int argc, char *argv[]) {
	(void)argc;
	run(argv[1], argv[2]);
	return 0;
}
#endif

#include <sys/types.h>
// #include <unistd.h>
// #include <dlfcn.h>
#include <sys/stat.h>

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "plugin.h"

// #include "mygl/mygl.h"
#include "include/SDL.h"
#include "include/SDL_Image.h"

// #define GET_SDL_FUNC_LOCAL(name) type_##name *name = (type_##name*) GetProcAddress(sdl_handle, #name);
// #define GET_SDL_FUNC_GLOBAL(name) _##name = (type_##name*) GetProcAddress(sdl_handle, #name);

// typedef int SDLCALL type_SDL_Init(Uint32 flags);
// typedef int SDLCALL type_SDL_GL_SetAttribute(SDL_GLattr attr, int value);
// typedef SDL_Window* SDLCALL type_SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
// typedef SDL_GLContext SDLCALL type_SDL_GL_CreateContext(SDL_Window *window);
// typedef int SDLCALL type_SDL_PollEvent(SDL_Event * event);
// typedef void SDLCALL type_SDL_SetWindowTitle(SDL_Window * window, const char *title);
// typedef void SDLCALL type_SDL_GL_SwapWindow(SDL_Window * window);
// typedef void SDLCALL type_SDL_Quit(void);

// typedef SDL_AudioDeviceID SDLCALL type_SDL_OpenAudioDevice(const char *device, int iscapture, const SDL_AudioSpec * desired, SDL_AudioSpec * obtained, int allowed_changes);
// typedef int SDLCALL type_SDL_OpenAudio(SDL_AudioSpec * desired, SDL_AudioSpec * obtained);
// typedef const char *SDLCALL type_SDL_GetError(void);
// typedef SDL_AudioSpec *SDLCALL type_SDL_LoadWAV_RW(SDL_RWops * src, int freesrc, SDL_AudioSpec * spec, Uint8 ** audio_buf, Uint32 * audio_len);
// typedef SDL_RWops *SDLCALL type_SDL_RWFromFile(const char *file, const char *mode);
// typedef int SDLCALL type_SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len);
// typedef Uint32 SDLCALL type_SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev);
// typedef void SDLCALL type_SDL_FreeWAV(Uint8 * audio_buf);
// typedef void SDLCALL type_SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);

// static type_SDL_OpenAudioDevice *_SDL_OpenAudioDevice;
// static type_SDL_OpenAudio *_SDL_OpenAudio;
// static type_SDL_GetError *_SDL_GetError;
// static type_SDL_LoadWAV_RW *_SDL_LoadWAV_RW;
// static type_SDL_RWFromFile *_SDL_RWFromFile;
// static type_SDL_QueueAudio *_SDL_QueueAudio;
// static type_SDL_GetQueuedAudioSize *_SDL_GetQueuedAudioSize;
// static type_SDL_FreeWAV *_SDL_FreeWAV;
// static type_SDL_PauseAudioDevice *_SDL_PauseAudioDevice;

inline LARGE_INTEGER absolute_time(void) {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time;
}

static float ENGINE_TIME = 0;

#include "engine/input.cpp"
#include "engine/audio.cpp"

struct Plugin {
	HMODULE lib_handle;
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

static Plugin load_plugin_code(char const *lib_path, time_t timestamp) {
	Plugin plugin = {};

	HMODULE lib_handle = LoadLibraryA(lib_path);
	if (lib_handle) {
		void *update = GetProcAddress(lib_handle, "update");
		if (update) {
			plugin.lib_handle = lib_handle;

			plugin.update = (plugin_update_t*) update;
			plugin.reload = (plugin_reload_t*) GetProcAddress(lib_handle, "reload");

			plugin.valid = true;
			plugin.timestamp = timestamp;
		} else {
			FreeLibrary(lib_handle);
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
	int ret = FreeLibrary(plugin.lib_handle);
	if (ret != 0)
		printf("Error closing lib handle (code=%d)\n", ret);

	plugin.lib_handle = 0;
	plugin.valid = false;
	printf("code unloaded!\n");
	plugin.update = (plugin_update_t*) plugin_update_stub;
}

#define PLUGIN_MEMORY_SIZE (MB*16)

static ImageData image_load(const char *filepath) {
	ImageData data = {};

	SDL_Surface *surface = IMG_Load(filepath);
	ASSERT(surface, "Image load failed: %s", IMG_GetError());
	data.bytes_per_pixel = surface->format->BytesPerPixel;
	data.width = surface->w;
	data.height = surface->h;
	data.pixels = surface->pixels;

	return data;
}

static void run(const char *plugin_path) {
	running = true;

	void *memory_chunks[2] = {
		malloc(PLUGIN_MEMORY_SIZE),
		malloc(PLUGIN_MEMORY_SIZE),
	};
	int32_t memory_index = 0;
	void *memory = memory_chunks[memory_index];

	// GLWindowHandle *window = mygl_setup(RES_WIDTH, RES_HEIGHT, "Engine");
	// mygl_enter_fullscreen_mode(window);

	// InputApi input_api = {};
	// input_api.key_down = key_down;
	// input_api.key_up = key_up;
	// mygl_set_input_api(window, &input_api);

	// int width, height;
	// mygl_get_framebuffer_size(window, &width, &height);
	// printf("%d, %d\n", width, height);

	char const *lockfile_path = "./__lockfile";

	Plugin plugin = load_plugin_code(plugin_path, get_timestamp(lockfile_path));

	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	int64_t performance_count_frequency = PerfCountFrequencyResult.QuadPart;

	LARGE_INTEGER current_time;
	LARGE_INTEGER previous_time = absolute_time();

	double fps_current_seconds = 0;
	int32_t fps_frame_count = 0;

	// HMODULE sdl_handle = LoadLibraryA("../../sdl/sdl2.dll");
	// GET_SDL_FUNC_LOCAL(SDL_Init);
	// GET_SDL_FUNC_LOCAL(SDL_GL_SetAttribute);
	// GET_SDL_FUNC_LOCAL(SDL_CreateWindow);
	// GET_SDL_FUNC_LOCAL(SDL_GL_CreateContext);
	// GET_SDL_FUNC_LOCAL(SDL_PollEvent);
	// GET_SDL_FUNC_LOCAL(SDL_SetWindowTitle);
	// GET_SDL_FUNC_LOCAL(SDL_GL_SwapWindow);
	// GET_SDL_FUNC_LOCAL(SDL_Quit);

	// GET_SDL_FUNC_GLOBAL(SDL_OpenAudioDevice);
	// GET_SDL_FUNC_GLOBAL(SDL_OpenAudio);
	// GET_SDL_FUNC_GLOBAL(SDL_GetError);
	// GET_SDL_FUNC_GLOBAL(SDL_LoadWAV_RW);
	// GET_SDL_FUNC_GLOBAL(SDL_RWFromFile);
	// GET_SDL_FUNC_GLOBAL(SDL_QueueAudio);
	// GET_SDL_FUNC_GLOBAL(SDL_GetQueuedAudioSize);
	// GET_SDL_FUNC_GLOBAL(SDL_FreeWAV);
	// GET_SDL_FUNC_GLOBAL(SDL_PauseAudioDevice);

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
	int result = IMG_Init(IMG_INIT_JPG);
	ASSERT(result, "IMG_Init failed: %s", IMG_GetError());

	audio::open();

	 //Use OpenGL 3.1 core
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

				plugin.reload(memory, RES_WIDTH, RES_HEIGHT);
				printf("Plugin reloaded\n");
			}
		}

		current_time = absolute_time();
		double dt = ((float)(current_time.QuadPart - previous_time.QuadPart) / (float)performance_count_frequency);
		previous_time = current_time;
		{
			fps_current_seconds += dt;
			fps_frame_count++;
			if (fps_current_seconds > 0.25) {
				double fps = fps_frame_count / fps_current_seconds;
				char tmp[128];
				sprintf_s(tmp, ARRAY_COUNT(tmp), "opengl @ fps: %.2f (valid=%d)", fps, plugin.valid);
				SDL_SetWindowTitle(window, tmp);
				fps_current_seconds = 0;
				fps_frame_count = 0;
			}
		}

		if (plugin.update(memory, engine, RES_WIDTH, RES_HEIGHT, _input, (float)dt) < 0) {
			unload_plugin_code(plugin);
		}
		for (int32_t i = 0; i < InputKey_Count; ++i) {
			_input.pressed[i] = false;
			_input.released[i] = false;
		}

		SDL_GL_SwapWindow(window);
	}

	IMG_Quit();
	SDL_Quit();

	for (u32 i = 0; i < ARRAY_COUNT(memory_chunks); ++i) {
		free(memory_chunks[i]);
	}
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
	(void) Instance;
	(void) PrevInstance;
	(void) ShowCode;
	run(CommandLine);
}

// int main(int argc, char *argv[]) {
// 	(void)argc;
// 	run(argv[1]);
// 	return 0;
// }

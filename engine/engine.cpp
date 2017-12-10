#include "engine/utils/platform.h"
#include "engine/utils/logger.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include "include/SDL.h"
#include "include/SDL_image.h"
#pragma clang diagnostic pop

#include "plugin.h"

#include <stdio.h>
#include "utils/memory/memory_arena.cpp"
#include "utils/string.h"

#if DEVELOPMENT
	#include <dbghelp.h>
	#include <psapi.h>

	struct SymbolInfoPackage : public SYMBOL_INFO_PACKAGEW {
		SymbolInfoPackage() {
			si.SizeOfStruct = sizeof(SYMBOL_INFOW);
			si.MaxNameLen = sizeof(name);
		}
	};

	// TODO(kalle): This is supposed to be shared between the boot.cpp and this!
	struct ReloadHeader {
		void *old_mspace;
		void *new_mspace;
		size_t old_memory_size;
	};

	enum SymTagEnum {
		SymTagNull,
		SymTagExe,
		SymTagCompiland,
		SymTagCompilandDetails,
		SymTagCompilandEnv,
		SymTagFunction,
		SymTagBlock,
		SymTagData,
		SymTagAnnotation,
		SymTagLabel,
		SymTagPublicSymbol, // 10
		SymTagUDT,
		SymTagEnum,
		SymTagFunctionType,
		SymTagPointerType,
		SymTagArrayType, // 15
		SymTagBaseType,
		SymTagTypedef,
		SymTagBaseClass,
		SymTagFriend,
		SymTagFunctionArgType, // 20
		SymTagFuncDebugStart,
		SymTagFuncDebugEnd,
		SymTagUsingNamespace,
		SymTagVTableShape,
		SymTagVTable,
		SymTagCustom,
		SymTagThunk,
		SymTagCustomType,
		SymTagManagedType,
		SymTagDimension,
		SymTagMax
	};

	#include "reloader.cpp"
#endif //DEVELOPMENT

struct ReloadInfo {
	String reload_marker_path;
	String directory_path;
	String plugin_path;
	String plugin_pattern;

#ifdef DEVELOPMENT
	ReloadHeader reload_header;
#endif // DEVELOPMENT
};

#ifdef OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"

	#define LIB_HANDLE HMODULE
#else
	#define LIB_HANDLE void*
#endif

struct Plugin {
	LIB_HANDLE module;
	plugin_setup_t *setup;
	plugin_update_t *update;
	plugin_reload_t *reload;
	time_t timestamp;
	b32 valid;
};

#ifdef OS_WINDOWS
	#define load_library(lib_path) LoadLibrary(lib_path)
	#define get_symbol_address(module, symbol) GetProcAddress(module, symbol)
	#define unload_library(module) FreeLibrary(module)

	void find_newest_plugin(ReloadInfo &reload) {
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

	void init_symbols(Plugin &plugin, ReloadInfo &reload_info) {
		HANDLE process = GetCurrentProcess();
		BOOL success = SymInitialize(process, *reload_info.directory_path, FALSE);
		if (!success) {
			DWORD last_error = GetLastError();
			LOG_ERROR("Reload", "Could not init symbols for plugin dll! (error=%ld).", last_error);
			return;
		}

		MODULEINFO module_info = {};
		if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
			DWORD64 image_base = SymLoadModuleEx(process, 0, *reload_info.plugin_path, 0, (DWORD64)module_info.lpBaseOfDll, module_info.SizeOfImage, 0, 0);
			if (image_base == 0) {
				DWORD last_error = GetLastError();
				LOG_ERROR("Reload", "Could not load symbols for plugin dll! (error=%ld, file=%s).", last_error, *reload_info.plugin_path);
			}
		}
	}

	void unload_symbols(Plugin &plugin) { // Unload symbols
		MODULEINFO module_info = {};
		HANDLE process = GetCurrentProcess();
		if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
			BOOL success = SymUnloadModule64(process, (DWORD64)module_info.lpBaseOfDll);
			if (!success) {
				DWORD last_error = GetLastError();
				LOG_ERROR("Reload", "Could not unload symbols for plugin dll! (error=%ld).", last_error);
			}
		}
	}

	void load_symbols(Plugin &plugin, ReloadInfo &reload_info) { // Load symbols
		MODULEINFO module_info = {};
		HANDLE process = GetCurrentProcess();
		if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
			DWORD64 image_base = SymLoadModuleEx(process, 0, *reload_info.plugin_path, 0, (DWORD64)module_info.lpBaseOfDll, module_info.SizeOfImage, 0, 0);
			if (image_base == 0) {
				DWORD last_error = GetLastError();
				LOG_ERROR("Reload", "Could not load symbols for plugin dll! (error=%ld, file=%s).", last_error, *reload_info.plugin_path);
			} else {
				SymbolInfoPackage symbol_info_package;
				BOOL result = SymGetTypeFromNameW(process, (DWORD64)module_info.lpBaseOfDll, L"Application", &symbol_info_package.si);

				if (result) {
					SYMBOL_INFOW *symbol_info = &symbol_info_package.si;
					symbased_memory_patching(process, symbol_info->ModBase, symbol_info, reload_info.reload_header);
				}
			}
		}
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

	#define load_library(lib_path) dlopen(lib_path, RTLD_NOW | RTLD_NODELETE)
	#define get_symbol_address(module, symbol) dlsym(module, symbol)
	#define unload_library(module) dlclose(module)

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
	void find_newest_plugin(ReloadInfo &reload) {
		(void) reload;
	}
#endif

#define PLUGIN_MEMORY_SIZE (MB*16)

static u64 ENGINE_TIME;

#include "engine/input.cpp"
#include "engine/audio.cpp"


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
	(void)_mspace;
	(void)dt;
	return 0;
}
PLUGIN_SETUP(plugin_setup_stub) {
	(void)_mspace;
	(void)engine;
	(void)input;
}

static Plugin load_plugin_code(const char *lib_path, time_t timestamp) {
	Plugin plugin = {};

	LIB_HANDLE module = load_library(lib_path);
	if (module) {
		void *update = get_symbol_address(module, "update");
		if (update) {
			plugin.module = module;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
			plugin.setup = (plugin_setup_t*) get_symbol_address(module, "setup");
			plugin.update = (plugin_update_t*) update;
			plugin.reload = (plugin_reload_t*) get_symbol_address(module, "reload");
#pragma clang diagnostic pop

			plugin.valid = true;
			plugin.timestamp = timestamp;
		} else {
			unload_library(module);
		}
	}

	if (plugin.update == 0) {
		plugin.setup = (plugin_setup_t*) plugin_setup_stub;
		plugin.update = (plugin_update_t*) plugin_update_stub;
		plugin.valid = false;
		LOG_INFO("Engine", "Loading code failed!\n");
	} else {
		LOG_INFO("Engine", "code loaded!\n");
	}

	return plugin;
}

static void unload_plugin_code(Plugin &plugin) {
	int ret = unload_library(plugin.module);
	(void)ret;
	plugin.module = 0;
	plugin.valid = false;
	LOG_INFO("Engine", "code unloaded!\n");
	plugin.update = (plugin_update_t*) plugin_update_stub;
}

static b32 image_load(const char *filepath, ImageData &data) {
	SDL_Surface *surface = IMG_Load(filepath);
	if (!surface) {
		LOG_INFO("Engine", "Image load failed: %s\n", IMG_GetError());
		return false;
	}

	LOG_INFO("Engine", "Loading image '%s'. Pixel format: %s\n", filepath, SDL_GetPixelFormatName(surface->format->format));

	data.pixels = surface->pixels;
	data.bytes_per_pixel = surface->format->BytesPerPixel;
	data.width = surface->w;
	data.height = surface->h;

	switch (surface->format->format) {
		case SDL_PIXELFORMAT_RGBA32: { data.format = PixelFormat_RGBA; } break;
		case SDL_PIXELFORMAT_BGRA32: { data.format = PixelFormat_ARGB; } break;
		case SDL_PIXELFORMAT_RGB24: { data.format = PixelFormat_RGB; } break;
		case SDL_PIXELFORMAT_RGB888: { data.format = PixelFormat_RGB; } break;
		case SDL_PIXELFORMAT_BGR24: { data.format = PixelFormat_BGR; } break;
		case SDL_PIXELFORMAT_UNKNOWN: { LOG_INFO("Engine", "SDL_PIXELFORMAT_UNKNOWN\n"); } break;
		case SDL_PIXELFORMAT_INDEX1LSB: { LOG_INFO("Engine", "SDL_PIXELFORMAT_INDEX1LSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX1MSB: { LOG_INFO("Engine", "SDL_PIXELFORMAT_INDEX1MSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX4LSB: { LOG_INFO("Engine", "SDL_PIXELFORMAT_INDEX4LSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX4MSB: { LOG_INFO("Engine", "SDL_PIXELFORMAT_INDEX4MSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX8: { LOG_INFO("Engine", "SDL_PIXELFORMAT_INDEX8\n"); } break;
		case SDL_PIXELFORMAT_RGB332: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGB332\n"); } break;
		case SDL_PIXELFORMAT_RGB444: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGB444\n"); } break;
		case SDL_PIXELFORMAT_RGB555: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGB555\n"); } break;
		case SDL_PIXELFORMAT_BGR555: { LOG_INFO("Engine", "SDL_PIXELFORMAT_BGR555\n"); } break;
		case SDL_PIXELFORMAT_ARGB4444: { LOG_INFO("Engine", "SDL_PIXELFORMAT_ARGB4444\n"); } break;
		case SDL_PIXELFORMAT_RGBA4444: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGBA4444\n"); } break;
		case SDL_PIXELFORMAT_ABGR4444: { LOG_INFO("Engine", "SDL_PIXELFORMAT_ABGR4444\n"); } break;
		case SDL_PIXELFORMAT_BGRA4444: { LOG_INFO("Engine", "SDL_PIXELFORMAT_BGRA4444\n"); } break;
		case SDL_PIXELFORMAT_ARGB1555: { LOG_INFO("Engine", "SDL_PIXELFORMAT_ARGB1555\n"); } break;
		case SDL_PIXELFORMAT_RGBA5551: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGBA5551\n"); } break;
		case SDL_PIXELFORMAT_ABGR1555: { LOG_INFO("Engine", "SDL_PIXELFORMAT_ABGR1555\n"); } break;
		case SDL_PIXELFORMAT_BGRA5551: { LOG_INFO("Engine", "SDL_PIXELFORMAT_BGRA5551\n"); } break;
		case SDL_PIXELFORMAT_RGB565: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGB565\n"); } break;
		case SDL_PIXELFORMAT_BGR565: { LOG_INFO("Engine", "SDL_PIXELFORMAT_BGR565\n"); } break;
		case SDL_PIXELFORMAT_RGBX8888: { LOG_INFO("Engine", "SDL_PIXELFORMAT_RGBX8888\n"); } break;
		case SDL_PIXELFORMAT_BGR888: { LOG_INFO("Engine", "SDL_PIXELFORMAT_BGR888\n"); } break;
		case SDL_PIXELFORMAT_BGRX8888: { LOG_INFO("Engine", "SDL_PIXELFORMAT_BGRX8888\n"); } break;
		case SDL_PIXELFORMAT_ARGB2101010: { LOG_INFO("Engine", "SDL_PIXELFORMAT_ARGB2101010\n"); } break;
		case SDL_PIXELFORMAT_YV12: { LOG_INFO("Engine", "SDL_PIXELFORMAT_YV12\n"); } break;
		case SDL_PIXELFORMAT_IYUV: { LOG_INFO("Engine", "SDL_PIXELFORMAT_IYUV\n"); } break;
		case SDL_PIXELFORMAT_YUY2: { LOG_INFO("Engine", "SDL_PIXELFORMAT_YUY2\n"); } break;
		case SDL_PIXELFORMAT_UYVY: { LOG_INFO("Engine", "SDL_PIXELFORMAT_UYVY\n"); } break;
		case SDL_PIXELFORMAT_YVYU: { LOG_INFO("Engine", "SDL_PIXELFORMAT_YVYU\n"); } break;
		case SDL_PIXELFORMAT_NV12: { LOG_INFO("Engine", "SDL_PIXELFORMAT_NV12\n"); } break;
		case SDL_PIXELFORMAT_NV21		: { LOG_INFO("Engine", "SDL_PIXELFORMAT_NV21\n"); } break;
		default: {
			ASSERT(0, "Unsuported pixel format!");
			return false;
		}
	};

	return true;
}

static i32 _screen_width;
static i32 _screen_height;

static void screen_dimensions(i32 &screen_width, i32 &screen_height) {
	screen_width = _screen_width;
	screen_height = _screen_height;
}

static void run(const char *plugin_directory, const char *plugin_name) {
	running = true;

	mspace plugin_mspace = create_mspace(PLUGIN_MEMORY_SIZE, 0);

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
	init_symbols(plugin, reload);

	TimeInfo time = {};
	setup_time(time);
	u64 current_time = 0;
	u64 previous_time = absolute_time();

	double fps_current_seconds = 0;
	i32 fps_frame_count = 0;

	u32 sdl_subsystems = SDL_INIT_AUDIO;
	sdl_subsystems = SDL_INIT_AUDIO | SDL_INIT_VIDEO;

	if (SDL_Init(sdl_subsystems)) {
		ASSERT(0, "SDL_Init failed: %s", IMG_GetError());
	}
	if (!IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG)) {
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

	SDL_Window *window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RES_WIDTH, RES_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	(void)glContext;

	SDL_GL_GetDrawableSize(window, &_screen_width, &_screen_height);
	SDL_GL_SetSwapInterval(0);

	EngineApi engine;

	engine.audio_load = audio::load;
	engine.audio_queue = audio::queue;
	engine.audio_queued_size = audio::queued_size;
	engine.audio_free = audio::free;
	engine.audio_set_playing = audio::set_playing;

	engine.image_load = image_load;
	engine.screen_dimensions = screen_dimensions;

	engine.quit = quit;
	engine.audio_set_playing(true);

	plugin.setup(plugin_mspace, &engine, &_input);

	while (running) {
#if !MYGL
		SDL_Event sdl_event = {};
		_input.mouse_xrel = 0;
		_input.mouse_yrel = 0;
		while (SDL_PollEvent(&sdl_event) != 0) {
			switch (sdl_event.type) {
				case SDL_QUIT: {
					running = false;
				} break;
				case SDL_MOUSEMOTION: {
					mouse_motion(sdl_event.motion.x, sdl_event.motion.y, sdl_event.motion.xrel, sdl_event.motion.yrel);
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					mouse_down(sdl_event.button.button);
				} break;
				case SDL_MOUSEBUTTONUP: {
					mouse_up(sdl_event.button.button);
				} break;
				case SDL_KEYDOWN: {
					if (sdl_event.key.repeat == 0)
						key_down(sdl_event.key.keysym.sym, sdl_event.key.keysym.mod);
				} break;
				case SDL_KEYUP: {
					if (sdl_event.key.repeat == 0)
						key_up(sdl_event.key.keysym.sym, sdl_event.key.keysym.mod);
				} break;
			}
		}
#endif

		if (plugin.valid) {
			time_t timestamp = get_timestamp(*reload.reload_marker_path);
			if (timestamp > plugin.timestamp) {
				#if DEVELOPMENT
					unload_symbols(plugin);
				#endif // DEVELOPMENT

				// Free plugin
				unload_plugin_code(plugin);

				// Find most recently built dll
				find_newest_plugin(reload);

				// Load dll
				plugin = load_plugin_code(*reload.plugin_path, timestamp);

				#if DEVELOPMENT
					mspace new_plugin_mspace = create_mspace(PLUGIN_MEMORY_SIZE, 0);

					reload.reload_header.new_mspace = new_plugin_mspace;
					reload.reload_header.old_mspace = plugin_mspace;
					reload.reload_header.old_memory_size = PLUGIN_MEMORY_SIZE;

					load_symbols(plugin, reload);
				#endif // DEVELOPMENT

				if (plugin.reload) {
					plugin.reload(new_plugin_mspace, &engine, &_input);
				}
				LOG_INFO("Engine", "Plugin reloaded\n");

				#if DEVELOPMENT
					// Swap the memory
					destroy_mspace(plugin_mspace);
					plugin_mspace = new_plugin_mspace;
				#endif // DEVELOPMENT
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

		if (plugin.update(plugin_mspace, (float)dt) < 0) {
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

	destroy_mspace(plugin_mspace);
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

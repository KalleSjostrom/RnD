#include "utils/common.h"

#define RES_WIDTH 1024
#define RES_HEIGHT 768
#define ASPECT_RATIO ((float)RES_WIDTH/(float)RES_HEIGHT)

enum PixelFormat {
	PixelFormat_RGBA,
	PixelFormat_ARGB
};

struct ImageData {
	void *pixels;

	i32 bytes_per_pixel;
	i32 width, height;
	PixelFormat format;
};

struct EngineApi {
	void (*audio_load)(const char *filename, u8 **buffer, u32 *length);
	void (*audio_queue)(u8 *buffer, u32 length);
	u32 (*audio_queued_size)();
	void (*audio_free)(i16 *buffer);
	void (*audio_set_playing)(b32 playing);

	b32 (*image_load)(const char *filename, ImageData &image_data);

	void (*quit)();
};

enum InputKey {
	InputKey_A = 0,
	InputKey_B,
	InputKey_C,
	InputKey_D,
	InputKey_E,
	InputKey_F,
	InputKey_G,
	InputKey_H,
	InputKey_I,
	InputKey_J,
	InputKey_K,
	InputKey_L,
	InputKey_M,
	InputKey_N,
	InputKey_O,
	InputKey_P,
	InputKey_Q,
	InputKey_R,
	InputKey_S,
	InputKey_T,
	InputKey_U,
	InputKey_V,
	InputKey_W,
	InputKey_X,
	InputKey_Y,
	InputKey_Z,

	InputKey_Space,
	InputKey_Enter,
	InputKey_Escape,
	InputKey_Tab,
	InputKey_Command,
	InputKey_Shift,
	InputKey_Option,
	InputKey_Control,
	InputKey_RightShift,
	InputKey_RightOption,
	InputKey_RightControl,
	InputKey_Function,

	InputKey_Count
};

struct InputTime {
	float pressed;
	float released;
};
struct Input {
	b32 pressed[InputKey_Count];
	b32 released[InputKey_Count];
	InputTime times[InputKey_Count];
};
#define IS_HELD(input, key) ((input).times[(key)].pressed > (input).times[(key)].released)
#define IS_PRESSED(input, key) (input).pressed[(key)]

#define PLUGIN_UPDATE(name) i32 name(void *memory, EngineApi &engine, i32 screen_width, i32 screen_height, Input &input, float dt)
typedef PLUGIN_UPDATE(plugin_update_t);

#define PLUGIN_RELOAD(name) void name(void *memory, EngineApi &engine, i32 screen_width, i32 screen_height, Input &input)
typedef PLUGIN_RELOAD(plugin_reload_t);

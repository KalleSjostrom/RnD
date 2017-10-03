#include "utils/common.h"

#define RES_WIDTH 1024
#define RES_HEIGHT 768

struct ImageData {
	int bytes_per_pixel;
	int width, height;
	void *pixels;
};

struct EngineApi {
	void (*audio_load)(const char *filename, uint8_t **buffer, uint32_t *length);
	void (*audio_queue)(uint8_t *buffer, uint32_t length);
	uint32_t (*audio_queued_size)();
	void (*audio_free)(int16_t *buffer);
	void (*audio_set_playing)(bool playing);

	ImageData (*image_load)(const char *filename);

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
	bool pressed[InputKey_Count];
	bool released[InputKey_Count];
	InputTime times[InputKey_Count];
};
#define IS_HELD(input, key) ((input).times[(key)].pressed > (input).times[(key)].released)
#define IS_PRESSED(input, key) (input).pressed[(key)]

#define PLUGIN_UPDATE(name) int name(void *memory, EngineApi &engine, int screen_width, int screen_height, Input &input, float dt)
typedef PLUGIN_UPDATE(plugin_update_t);

#define PLUGIN_RELOAD(name) void name(void *memory, int screen_width, int screen_height)
typedef PLUGIN_RELOAD(plugin_reload_t);

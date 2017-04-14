#include "utils/common.h"

#define RES_WIDTH 1024
#define RES_HEIGHT 768

#define ASPECT_RATIO ((float)RES_WIDTH/(float)RES_HEIGHT)

#define EXPORT extern "C" __attribute__((visibility("default")))

struct EngineApi {
	void (*audio_load)(const char *filename, uint8_t **buffer, uint32_t *length);
	void (*audio_queue)(uint8_t *buffer, uint32_t length);
	uint32_t (*audio_queued_size)();
	void (*audio_free)(int16_t *buffer);
	void (*audio_set_playing)(bool playing);
};

enum InputKey {
	InputKey_Left = 0,
	InputKey_Right,
	InputKey_Up,
	InputKey_Down,

	InputKey_Action,
	InputKey_Jump,

	InputKey_Count,
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

#define GAME_UPDATE(name) int name(void *memory, EngineApi &engine, int screen_width, int screen_height, Input &input, float dt)
typedef GAME_UPDATE(game_update_t);

#define GAME_RELOAD(name) void name(void *memory, int screen_width, int screen_height)
typedef GAME_RELOAD(game_reload_t);
#pragma once

#include "core/memory/allocator.h"

#include "modules/audio_api.h"
#include "modules/image_api.h"
#include "modules/input_api.h"
#include "modules/logging_api.h"

struct EngineApi {
	AudioApi audio;
	ImageApi image;

	void (*screen_dimensions)(i32 &screen_width, i32 &screen_height);

	void (*quit)();
};

#define PLUGIN_SETUP(name) void name(void *mspace, EngineApi *engine, InputData *input)
typedef PLUGIN_SETUP(plugin_setup_t);

#define PLUGIN_UPDATE(name) i32 name(void *mspace, float dt)
typedef PLUGIN_UPDATE(plugin_update_t);

#define PLUGIN_RELOAD(name) void name(void *mspace)
typedef PLUGIN_RELOAD(plugin_reload_t);

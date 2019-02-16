#pragma once

#include "core/memory/allocator.h"

#include "modules/audio_api.h"
#include "modules/image_api.h"
#include "modules/input_api.h"
#include "modules/error_api.h"
#include "modules/logging_api.h"

struct EngineApi {
	AudioApi audio;
	ImageApi image;
	ErrorApi error;
	LoggingApi logging;

	void (*screen_dimensions)(i32 &screen_width, i32 &screen_height);

	void (*quit)();
};

#define PLUGIN_SETUP(name) void name(Allocator *allocator, EngineApi *engine, InputData *input)
typedef PLUGIN_SETUP(plugin_setup_t);

#define PLUGIN_UPDATE(name) void name(Allocator *allocator, float dt)
typedef PLUGIN_UPDATE(plugin_update_t);

#define PLUGIN_RELOAD(name) void name(Allocator *old_allocator, Allocator *new_allocator)
typedef PLUGIN_RELOAD(plugin_reload_t);

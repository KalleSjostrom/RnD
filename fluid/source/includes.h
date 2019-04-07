#define SYSTEM_OPENGL
#define SYSTEM_AUDIO
#include "systems.h"

#include "settings.h"


///// GAME SPECIFICS
namespace globals {
	static ArenaAllocator *transient_arena;
};
#define SCRATCH_ALLOCATE_STRUCT(type, count) PUSH_STRUCTS(*globals::transient_arena, count, type)
#define SCRATCH_ALLOCATE_STRING(count) PUSH_STRING(*globals::transient_arena, count)
#define SCRATCH_ALLOCATE_SIZE(type, count) PUSH_SIZE(*globals::transient_arena, count, type)

#include "engine/utils/audio_manager.cpp"
#include "engine/utils/camera.cpp"
#include "engine/utils/animation.h"

/////// ASSETS
#include "engine/generated/component_group.cpp"
#include "render_pipe.cpp"

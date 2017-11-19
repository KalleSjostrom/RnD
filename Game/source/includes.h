#include "engine/utils/platform.h"

#ifdef OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <gl/gl.h>
#include "engine/utils/win32_setup_gl.h"
#else
#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>
#endif

#include "engine/plugin.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/file_utils.h"

#define GLSL(src) "#version 410\n" #src
typedef u16 GLindex;
// #define GL_INDEX GL_UNSIGNED_SHORT
#include "engine/opengl/gl_program_builder.cpp"

#define USE_INTRINSICS 1
#include "engine/utils/math/math.h"

///// GAME SPECIFICS
namespace globals {
	static MemoryArena *transient_arena;
};
#define SCRATCH_ALLOCATE_STRUCT(type, count) PUSH_STRUCTS(*globals::transient_arena, count, type)

#include "engine/utils/audio_manager.cpp"
#include "engine/utils/camera.cpp"
#include "engine/utils/animation.h"

/////// ASSETS
#include "../generated/animations.generated.cpp"

#include "engine/generated/component_group.cpp"
#include "render_pipe.cpp"

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

#define GLSL(src) "#version 410\n" #src
typedef uint16_t GLindex;
// #define GL_INDEX GL_UNSIGNED_SHORT

#include "engine/plugin.h"
#include "engine/utils/memory/arena_allocator.cpp"
#include "engine/utils/file_utils.h"
#include "engine/opengl/gl_program_builder.cpp"

#define USE_INTRINSICS 1
#include "engine/utils/math/math.h"

///// GAME SPECIFICS
namespace globals {
	static ArenaAllocator *transient_arena;
};
#define SCRATCH_ALLOCATE_STRUCT(type, count) PUSH_STRUCTS(*globals::transient_arena, count, type)

#include "engine/utils/audio_manager.cpp"
#include "engine/utils/camera.cpp"
#include "engine/utils/animation.h"

/////// ASSETS
#include "engine/generated/component_group.cpp"
#include "render_pipe.cpp"

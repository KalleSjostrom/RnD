#include "engine/utils/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <float.h>

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
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/file_utils.h"
#include "opengl/gl_program_builder.cpp"

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
#include "shaders/default.shader.cpp"
#include "shaders/avatar.shader.cpp"
#include "shaders/fullscreen_effects.shader.cpp"

// #define CALL(owner, compname, command, ...) (((ComponentGroup*)globals::components)->compname.command(owner.compname ## _id, ## __VA_ARGS__))
// #define GET(owner, compname, member) (((ComponentGroup*)globals::components)->compname.instances[owner.compname ## _id].member)

#include "component_group.cpp"
#include "render_pipe.cpp"

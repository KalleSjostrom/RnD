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
#include "engine/utils/win32_setup_cl.h"
#else
#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>

#include "OpenCL/opencl.h"
#endif

#define GLSL(src) "#version 410\n" #src
typedef uint16_t GLindex;
// #define GL_INDEX GL_UNSIGNED_SHORT

#include "engine/plugin.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/file_utils.h"
#include "engine/opengl/gl_program_builder.cpp"

#define CL_ERROR_CHECKING 1
#define CL(src) "" #src
#define DEVICE_INDEX 1
// #define DEVICE_INDEX 1 // NOTE(kalle): 1 is Device GeForce GT 650M on my mac book.
#include "engine/opencl/cl_manager.cpp"
#include "engine/opencl/cl_program_builder.cpp"

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
#include "shaders/default.shader.cpp"
#include "shaders/fullscreen_effects.shader.cpp"

#include "component_group.cpp"
#include "render_pipe.cpp"
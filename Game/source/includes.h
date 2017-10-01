#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>

#define GLSL(src) "#version 410\n" #src
typedef uint16_t GLindex;
#define GL_INDEX GL_UNSIGNED_SHORT

#include "engine/plugin.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/file_utils.h"
#include "opengl/gl_program_builder.cpp"

#define USE_INTRINSICS 1
#include "engine/utils/math/math.h"

///// GAME SPECIFICS
namespace globals {
	static MemoryArena *transient_arena;
	static void *components;
};
#define SCRATCH_ALLOCATE_STRUCT(type, count) PUSH_STRUCTS(*globals::transient_arena, count, type)

#include "engine/utils/audio_manager.cpp"
#include "engine/utils/camera.cpp"
#include "engine/utils/animation.h"

/////// ASSETS
#include "../assets/hatch.c"
#include "../generated/animations.generated.cpp"
#include "shaders/default.shader.cpp"
#include "shaders/avatar.shader.cpp"

#define CALL(owner, compname, command, ...) (((ComponentGroup*)globals::components)->compname.command(owner.compname ## _id, ## __VA_ARGS__))
#define GET(owner, compname, member) (((ComponentGroup*)globals::components)->compname.instances[owner.compname ## _id].member)

#include "component_group.cpp"
#include "render_pipe.cpp"
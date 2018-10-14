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

#include "engine/plugin.h"
#include "engine/utils/memory/arena_allocator.cpp"
#include "engine/utils/file_utils.h"
#include "engine/opengl/gl_program_builder.cpp"

#define USE_INTRINSICS 1
#include "engine/utils/math/math.h"

#include "engine/utils/string.h"
#include "engine/utils/camera.cpp"
#include "engine/utils/animation.h"
#include "engine/utils/gui.cpp"

///// PLUGIN SPECIFICS
#include "roomba.shader.cpp"
#include "engine/generated/component_group.cpp"

#include <fcntl.h>

#include "engine/common.h"


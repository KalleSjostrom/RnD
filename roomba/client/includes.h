#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <float.h>
#include <errno.h>

#include <unistd.h>
#include <mach/mach_time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

#include "engine/utils/string.h"
#include "engine/utils/camera.cpp"
#include "engine/utils/gui.cpp"

///// PLUGIN SPECIFICS
#include "roomba.shader.cpp"

#define CALL(components, owner, compname, command, ...) (components.compname.command(owner.compname ## _id, ## __VA_ARGS__))
#define GET(components, owner, compname, member) (components.compname.instances[owner.compname ## _id].member)
#include "component_group.cpp"

#include <fcntl.h>

#include "engine/utils/common.h"
#include "../sci.h"
#include "../tcp_socket.cpp"

#include "../roomba_common.cpp"
#include "roomba.cpp"

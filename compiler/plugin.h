#pragma once

#pragma warning(disable : 4458)
#pragma warning(disable : 4244)
#pragma warning(disable : 4061)
#pragma warning(disable : 4062)
#pragma warning(disable : 4365)
#pragma warning(disable : 4464)
#pragma warning(disable : 4514)
#pragma warning(disable : 4668)
#pragma warning(disable : 4820)
#pragma warning(disable : 4625)
#pragma warning(disable : 4710)
#pragma warning(disable : 4626)
#pragma warning(disable : 4582)
#pragma warning(disable : 4623)
#pragma warning(disable : 4060)
#pragma warning(disable : 4068)
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)
#pragma warning(disable : 4191)
#pragma warning(disable : 4505)
#pragma warning(disable : 4100)
#pragma warning(disable : 4324)

#pragma warning(disable : 5026)
#pragma warning(disable : 5027)
#pragma warning(disable : 4577)
#pragma warning(disable : 4711)

#include "engine/utils/platform.h"
#include "engine/common.h"

#ifdef OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"
#endif

#include "engine/utils/threading/threading.cpp"
#include "engine/utils/threading/atomics.cpp"
#include "engine/utils/file_utils.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/string.h"
#include "engine/utils/parser.cpp"
#include "engine/utils/file_system.h"
#include "engine/utils/fibers/task_scheduler.h"

struct CompilerContext {
	MemoryArena *arena;
	FileSystem *file_system;
};

struct StringId {
	String string;
	u32 id;
};
StringId make_string_id(String string, u32 id) {
	StringId sid = {string, id};
	return sid;
}

struct StringIdArray {
	StringId *ids;
	i32 count;
};

#define PLUGIN_ON_FILE_CHANGE(name) Task *name(CompilerContext &context, FileInfo &file_info)
typedef PLUGIN_ON_FILE_CHANGE(plugin_on_file_change_t);

#define PLUGIN_GET_FILE_ENDINGS(name) void name(CompilerContext &context, StringIdArray &array)
typedef PLUGIN_GET_FILE_ENDINGS(plugin_get_file_endings_t);

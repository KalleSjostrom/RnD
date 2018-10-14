#pragma once

#include "core/utils/dynamic_string.h"
#include "core/utils/string_id.h"
#include "core/utils/jobs.h"
#include "utils/file_system.h"

struct CompilerContext {
	struct ArenaAllocator *arena;
	struct Allocator allocator;
	struct FileSystem *file_system;
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

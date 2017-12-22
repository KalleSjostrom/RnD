#include "compiler/plugin.h"
#include "glsl_compiler.cpp"

namespace glsl {
	static String filetype_glsl = MAKE_STRING("glsl");
	static u32 filetype_glsl_id = make_string_id32(filetype_glsl);

	struct CompileObjTask {
		String output_folder;
		FileInfo *file_info;
	};

	void do_task(void *arg) {
		CompileObjTask *task = (CompileObjTask*) arg;

		FileInfo *info = task->file_info;
		ASSERT(info->state == CacheState_Modified || info->state == CacheState_Added, "Invalid state");

		String filename = get_filename(info->filepath);
		String source_folder = info->filepath;
		source_folder.length -= filename.length;
		source_folder.length--; // Swallow trailing /

		compile_glsl(source_folder, filename, task->output_folder);
	}

	EXPORT PLUGIN_ON_FILE_CHANGE(on_file_change) {
		if (file_info.filetype == filetype_glsl_id) {
			if (file_info.state == CacheState_Added || file_info.state == CacheState_Modified) {
				printf("Reloader: register_file_info_change. (filepath=%s, 0x%016zx)\n", *file_info.filepath, file_info.key);

				CompileObjTask *grt = PUSH_STRUCTS(*context.arena, 1, CompileObjTask);
				grt->output_folder = context.file_system->output_folder;
				grt->file_info = &file_info;

				Task *task = PUSH_STRUCTS(*context.arena, 1, Task);
				task->function = do_task;
				task->argument = grt;
				return task;
			}
		}
		return 0;
	}

	EXPORT PLUGIN_GET_FILE_ENDINGS(get_file_endings) {
		// TODO(kalle): Make this a setup call instead
		setup_gl();

		array.count = 1;

		array.ids = PUSH_STRUCTS(*context.arena, array.count, StringId);
		array.ids[0] = make_string_id(filetype_glsl, filetype_glsl_id);
	}
}
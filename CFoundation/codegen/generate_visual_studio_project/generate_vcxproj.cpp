#include "../utils/common.cpp"
#include "../utils/exceptions.h"
#include <objbase.h>


bool ignored_project_directory(char *directory) {
	return
		are_cstrings_equal(directory, ".vs") ||
		are_cstrings_equal(directory, "build") ||
		are_cstrings_equal(directory, "v14") ||
		are_cstrings_equal(directory, "backup") ||
		are_cstrings_equal(directory, "build_experiment") ||
		are_cstrings_equal(directory, ".hg") ||
		are_cstrings_equal(directory, "plugin_environment") ||
		are_cstrings_equal(directory, "codegen");
}

struct FolderArray {
	String *entries;
	unsigned count;
	unsigned debug_max_count;
};
struct File {
	String path;
	String folder;
};
struct FileArray {
	File *entries;
	unsigned count;
	unsigned debug_max_count;
};

#include "output_vcxproj.cpp"

String make_string_copy(char *text, MemoryArena &arena) {
	return clone_string(make_string(text), arena);
}

void gather_project_files(MemoryArena &arena, char *filepath, String folder, FolderArray &folder_array, FileArray &file_array) {
	WIN32_FIND_DATA find_data = {};
	char buffer[MAX_PATH];
	sprintf(buffer, "%s*.*", filepath);

	bool has_added_folder = false;

	HANDLE handle = find_first_file(buffer, &find_data);
	if (handle != INVALID_HANDLE_VALUE) {
		do {
			if (find_data.cFileName[0] == '.') {
				if (find_data.cFileName[1] == '\0')
					continue;

				if (find_data.cFileName[1] == '.' && find_data.cFileName[2] == '\0')
					continue;
			}

			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!ignored_project_directory(find_data.cFileName)) {
					sprintf(buffer, "%s%s/", filepath, find_data.cFileName);

					char folderBuffer[512] = {};
					sprintf(folderBuffer, "%s\\%s", *folder, find_data.cFileName);

					gather_project_files(arena, buffer, make_string_copy(folderBuffer, arena), folder_array, file_array);
				}
			} else {
				String filename = make_string(find_data.cFileName);
				if (string_ends_in(filename, MAKE_STRING(".c")) ||
					string_ends_in(filename, MAKE_STRING(".cpp")) ||
					string_ends_in(filename, MAKE_STRING(".inl")) ||
					string_ends_in(filename, MAKE_STRING(".h"))) {

					// Only add folders containing source files
					if(!has_added_folder) {
						has_added_folder = true;
						ARRAY_CHECK_BOUNDS(folder_array);
						folder_array.entries[folder_array.count++] = folder;
					}

					sprintf(buffer, "%s%s", filepath, find_data.cFileName);
					char filepath_buffer[1024];
					GetFullPathName(buffer, 1024, filepath_buffer, 0);

					File file = { make_string_copy(filepath_buffer, arena), folder };
					ARRAY_CHECK_BOUNDS(file_array);
					file_array.entries[file_array.count++] = file;
				}
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}
}

// TODO(kalle): What to do with the .generated. file ending and the property system in stingray?
int main(int argc, char *argv[]) {
	exceptions_setup();
	MemoryArena arena = init_memory(16*MB);

	const unsigned num_folders = 512;
	String *folders = (String*)allocate_memory(arena, num_folders*sizeof(String));
	FolderArray folder_array = {
		folders, 0, num_folders,
	};

	const unsigned num_files = 1024;
	File *files = (File*)allocate_memory(arena, num_files*sizeof(File));
	FileArray file_array = {
		files, 0, num_files,
	};

	ASSERT(argc == 2, "Need to specify the path to the dependency folder (%%SR_SOURCE_DIR%%/runtime/sdk)");

	String dependecy_folder = make_string(argv[1]);

	folder_array.entries[folder_array.count++] = MAKE_STRING("foundation");
	folder_array.entries[folder_array.count++] = MAKE_STRING("game");

	gather_project_files(arena, "../../", MAKE_STRING("foundation"), folder_array, file_array);
	gather_project_files(arena, "../../"GAME_CODE_DIR"/", MAKE_STRING("game"), folder_array, file_array);

	output_vcxproj(arena, dependecy_folder, folder_array, file_array);
	exceptions_shutdown();
}

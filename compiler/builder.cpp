#include "run_command.cpp"

bool build(Project &project, FileSystem &file_system, char *command_line) {
	char command[4096] = {}; // Virtual alloc trick?
	char *at = command;
	u32 count = ARRAY_COUNT(command);

	u32 bytes_written = sprintf_s(at, count, "cl.exe");
	at += bytes_written;
	count -= bytes_written;

	if (project.user_commands.length) {
		bytes_written = sprintf_s(at, count, " %.*s", STR(project.user_commands));
		at += bytes_written;
		count -= bytes_written;
	}

	for (i32 i = 0; i < array_count(project.translation_units); ++i) {
		bytes_written = sprintf_s(at, count, " %.*s", STR(project.translation_units[i]));
		at += bytes_written;
		count -= bytes_written;
	}

	// printf("Building %s plugin\n", *project.name);

	// TODO(kalle): Don't actually remove the pbd at this point, since that will cause a full link.
	// 				Try to instead check if the dll is locked for writing?
	// Try and clear out all previous pdbs still in the output folder
	char search_string[MAX_PATH] = {}; // Virtual alloc trick?
	sprintf_s(search_string, MAX_PATH, "%.*s/*.dll", STR(file_system.output_folder));

	// Check if we have any locked dlls
	bool instance_running = has_locked_files(search_string, *file_system.output_folder);

	// Setup the compiler parameters and general variables
	// TODO(kalle): Read these from the project file!
	const char *defines = "-D DEVELOPMENT";
	const char *includes = "-I ../ -I ../engine/include/opencl22/ -I ../engine/";
	const char *libraries = "opengl32.lib dbghelp.lib";
	const char *flags = "-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -arch:AVX2 -MP";

	char plugin_name[MAX_PATH];
	if (instance_running) {
		// If we have instances runing, we need to append a random name to the plugin name
		u64 time = __rdtsc();
		sprintf_s(plugin_name, ARRAY_COUNT(plugin_name), "%.*s_%016zx", STR(project.name), time);
	} else {
		sprintf_s(plugin_name, ARRAY_COUNT(plugin_name), "%.*s", STR(project.name));

		int clean_old = 1;
		if (clean_old) {
			remove_files(search_string, *file_system.output_folder);

			sprintf_s(search_string, MAX_PATH, "%.*s/*.pdb", STR(file_system.output_folder));
			remove_files(search_string, *file_system.output_folder);

			sprintf_s(search_string, MAX_PATH, "%.*s/*.ilk", STR(file_system.output_folder));
			remove_files(search_string, *file_system.output_folder);

			sprintf_s(search_string, MAX_PATH, "%.*s/*.exp", STR(file_system.output_folder));
			remove_files(search_string, *file_system.output_folder);

			sprintf_s(search_string, MAX_PATH, "%.*s/*.lib", STR(file_system.output_folder));
			remove_files(search_string, *file_system.output_folder);

			sprintf_s(search_string, MAX_PATH, "%.*s/*.obj", STR(file_system.output_folder));
			remove_files(search_string, *file_system.output_folder);
		}
	}

	char plugin_output_path[MAX_PATH];
	sprintf_s(plugin_output_path, ARRAY_COUNT(plugin_output_path), "%.*s/%s", STR(project.output_path), plugin_name);

	bytes_written = sprintf_s(at, count, " %s %s %s %s %s /Fo%s.obj -LD /link /IMPLIB:%s.lib /PDB:%s.pdb /OUT:%s.dll", flags, defines, command_line ? command_line : "", includes, libraries, plugin_output_path, plugin_output_path, plugin_output_path, plugin_output_path);
	at += bytes_written;
	count -= bytes_written;

	bool success = run_command(0, command, *project.root);
	if (success) {
		char reload_marker[MAX_PATH];
		sprintf_s(reload_marker, ARRAY_COUNT(reload_marker), "%.*s/%s", STR(file_system.output_folder), "__reload_marker");
		if (instance_running) {
			printf("Found running instances, poking the __reload_marker to cause a reload.\n");
			// We need to let the engine process know there's a new plugin dll to load
			HANDLE handle = CreateFile(reload_marker, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			CloseHandle(handle);
		} else {
			printf("Found no running instances, make sure __reload_marker is removed.\n");
			DeleteFile(reload_marker);
		}
	} else {
		// printf("Failed to build, make sure __reload_marker is removed.\n");
		// DeleteFile(reload_marker);
	}
	return success;
}
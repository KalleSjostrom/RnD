#define PLUGIN_MEMORY_SIZE (MB*16)

struct Plugin {
	HMODULE module;
	mspace memory;

	plugin_setup_t *setup;
	plugin_update_t *update;
	plugin_reload_t *reload;

	DynamicString path;
	DynamicString directory;
	DynamicString reload_path_pattern;
	DynamicString reload_marker;

	FILETIME timestamp;

	bool valid;
};

void load_plugin_module(Plugin &plugin) {
	HMODULE module = LoadLibrary(*plugin.path);
	if (module) {
		void *update = GetProcAddress(module, "update");
		if (update) {
			plugin.module = module;

			plugin.setup = (plugin_setup_t*) GetProcAddress(module, "setup");
			plugin.update = (plugin_update_t*) update;
			plugin.reload = (plugin_reload_t*) GetProcAddress(module, "reload");

			plugin.valid = true;
		} else {
			FreeLibrary(module);
		}
	}

	if (!plugin.valid || plugin.update == 0) {
		plugin.valid = false;
		log_error("Plugin", "Loading plugin module failed! (path=%s, error=%ld)\n", *plugin.path, GetLastError());
	} else {
		log_info("Plugin", "Plugin module loaded! (path=%s)\n", *plugin.path);
	}
}

// Unload the plugin module (dll)
void unload_plugin_module(Plugin &plugin) {
	plugin.valid = false;

	if (!FreeLibrary(plugin.module)) {
		log_error("Plugin", "Plugin unload failed! (path=%s, GetLastError=%u)\n", *plugin.path, GetLastError());
		return;
	}

	plugin.module = 0;
	log_info("Plugin", "Plugin unloaded! (path=%s)\n", *plugin.path);
}

void set_newest_plugin_path(Plugin &plugin) {
	WIN32_FIND_DATA best_data = {};
	{ // Find the newest dll in the folder
		WIN32_FIND_DATA find_data;
		HANDLE handle = FindFirstFile(*plugin.reload_path_pattern, &find_data);
		if (handle == INVALID_HANDLE_VALUE)
			return;
		do {
			FILETIME time = find_data.ftCreationTime;
			FILETIME best_time = best_data.ftCreationTime;
			if (time.dwHighDateTime > best_time.dwHighDateTime || (time.dwHighDateTime == best_time.dwHighDateTime && time.dwLowDateTime > best_time.dwLowDateTime)) {
				best_data = find_data;
			}
		} while (FindNextFile(handle, &find_data));
		FindClose(handle);
	}

	resize(plugin.path, string_length(plugin.directory));
	plugin.path += best_data.cFileName;
	null_terminate(plugin.path);
}

void init_symbols(Plugin &plugin) {
	HANDLE process = GetCurrentProcess();
	BOOL success = SymInitialize(process, *plugin.directory, FALSE);
	if (!success) {
		DWORD last_error = GetLastError();
		log_error("Reload", "Could not init symbols for plugin dll! (error=%ld).", last_error);
		return;
	}

	MODULEINFO module_info = {};
	if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
		DWORD64 image_base = SymLoadModuleEx(process, 0, *plugin.path, 0, (DWORD64)module_info.lpBaseOfDll, module_info.SizeOfImage, 0, 0);
		if (image_base == 0) {
			DWORD last_error = GetLastError();
			log_error("Reload", "Could not load symbols for plugin dll! (error=%ld, file=%s).", last_error, *plugin.path);
		}
	}
}

void unload_symbols(Allocator *allocator, Plugin &plugin, ReloadInfo &reload_info) { // Unload symbols
	MODULEINFO module_info = {};
	HANDLE process = GetCurrentProcess();
	if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
		reloader_unload_symbols(*allocator, L"Application", module_info, reload_info.symbol_context);
		BOOL success = SymUnloadModule64(process, (DWORD64)module_info.lpBaseOfDll);
		if (!success) {
			DWORD last_error = GetLastError();
			log_error("Reload", "Could not unload symbols for plugin dll! (error=%ld).", last_error);
		}
	}
}

void load_symbols(Allocator *allocator, Plugin &plugin, ReloadInfo &reload_info) { // Load symbols
	MODULEINFO module_info = {};
	HANDLE process = GetCurrentProcess();
	if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
		DWORD64 image_base = SymLoadModuleEx(process, 0, *plugin.path, 0, (DWORD64)module_info.lpBaseOfDll, module_info.SizeOfImage, 0, 0);
		if (image_base == 0) {
			DWORD last_error = GetLastError();
			log_error("Reload", "Could not load symbols for plugin dll! (error=%ld, file=%s).", last_error, *plugin.path);
		} else {
			reloader_load_symbols(*allocator, L"Application", module_info, reload_info.symbol_context, reload_info.header);
		}
	}
}


// Make a plugin by loading the newest dll from 'directory' with the pattern 'name*.dll'
void load_plugin(Allocator *allocator, Plugin &plugin, const char *directory, const char *name) {
	DynamicString base = dynamic_stringf(allocator, "%s/", directory);

	plugin.directory = dynamic_string(allocator, base);
	plugin.path = dynamic_string(allocator, base);

	base += name;

	plugin.reload_path_pattern = dynamic_string(allocator, base);
	plugin.reload_path_pattern += "*.dll";

	plugin.reload_marker = base;
	plugin.reload_marker += ".reload_marker";

	if (plugin.valid) {
		unload_plugin_module(plugin);
	}
	set_newest_plugin_path(plugin);
	load_plugin_module(plugin);
}

// Try to find a plugin that matches the reload_path_pattern that is newer than the currently loaded, and load it
void check_for_reloads(Plugin &plugin) {
	// Check if we have a reload marker in the folder, this will indicate that a build has completed
	// Note that we can't simply check if we have a new dll since the pdb (for instance) might not have been completly written yet.
	WIN32_FIND_DATA data;
	HANDLE reload_marker_handle = FindFirstFile(*plugin.reload_marker, &data);
	if (reload_marker_handle == INVALID_HANDLE_VALUE)
		return;
	FindClose(reload_marker_handle);

	bool time_changed = data.ftLastWriteTime.dwLowDateTime != plugin.timestamp.dwLowDateTime || data.ftLastWriteTime.dwHighDateTime != plugin.timestamp.dwHighDateTime;
	if (!time_changed)
		return;

	plugin.timestamp = data.ftLastWriteTime;

	DeleteFile(*plugin.reload_marker);

	Allocator allocator = allocator_mspace(8*MB);

	ReloadInfo reload = {};
	reload.header.old_mspace = plugin.memory;
	reload.header.old_memory_size = PLUGIN_MEMORY_SIZE;
	unload_symbols(&allocator, plugin, reload);

	unload_plugin_module(plugin);
	set_newest_plugin_path(plugin);
	load_plugin_module(plugin);

	mspace new_plugin_mspace = create_mspace(PLUGIN_MEMORY_SIZE, 0);
	reload.header.new_mspace = new_plugin_mspace;
	load_symbols(&allocator, plugin, reload);

	if (plugin.reload) {
		plugin.reload(new_plugin_mspace);
	}

	destroy_mspace(plugin.memory);
	plugin.memory = new_plugin_mspace;

	log_info("Plugin", "Plugin reloaded!\n");
}

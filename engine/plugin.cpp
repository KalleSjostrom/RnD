#define PLUGIN_MEMORY_SIZE (MB*16)

#if DEVELOPMENT
struct PluginReloadContext {
	plugin_reload_t *reload;

	DynamicString marker;
	DynamicString pdb_path;

	ReloadHeader header;
	void *handle;

	FILETIME timestamp;
};
#endif

struct Plugin {
	HMODULE module;
	Allocator allocator;
	void *userdata;

	plugin_setup_t *setup;
	plugin_update_t *update;

	DynamicString path;
	DynamicString directory;
	DynamicString path_pattern;

#if DEVELOPMENT
	PluginReloadContext reload_context;
#endif

	bool valid;
};

void load_plugin_module(Plugin &plugin) {
	ASSERT(!plugin.valid, "Cannot load a valid plugin!");

	HMODULE module = LoadLibrary(*plugin.path);
	if (module) {
		void *update = GetProcAddress(module, "update");
		if (update) {
			plugin.module = module;

			plugin.setup = (plugin_setup_t*) GetProcAddress(module, "setup");
			plugin.update = (plugin_update_t*) update;
#if DEVELOPMENT
			plugin.reload_context.reload = (plugin_reload_t*) GetProcAddress(module, "reload");
#endif

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
		HANDLE handle = FindFirstFile(*plugin.path_pattern, &find_data);
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

#if DEVELOPMENT
	DynamicString &pdb_path = plugin.reload_context.pdb_path;
	clone(pdb_path, plugin.path);
	array_count(pdb_path.text) -= 4; // remove dll + null termination
	pdb_path += "pdb"; // add pdb
#endif // DEVELOPMENT
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

void unload_symbols(Plugin &plugin) { // Unload symbols
	MODULEINFO module_info = {};
	HANDLE process = GetCurrentProcess();
	if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
		BOOL success = SymUnloadModule64(process, (DWORD64)module_info.lpBaseOfDll);
		if (!success) {
			DWORD last_error = GetLastError();
			log_error("Reload", "Could not unload symbols for plugin dll! (error=%ld).", last_error);
		}
	}
}

void load_symbols(Plugin &plugin) { // Load symbols
	MODULEINFO module_info = {};
	HANDLE process = GetCurrentProcess();
	if (GetModuleInformation(process, plugin.module, &module_info, sizeof(module_info))) {
		DWORD64 image_base = SymLoadModuleEx(process, 0, *plugin.path, 0, (DWORD64)module_info.lpBaseOfDll, module_info.SizeOfImage, 0, 0);
		if (image_base == 0) {
			DWORD last_error = GetLastError();
			log_error("Reload", "Could not load symbols for plugin dll! (error=%ld, file=%s).", last_error, *plugin.path);
		}
	}
}

#define STRINGIZE(name) #name

// Make a plugin by loading the newest dll from 'directory' with the pattern 'name*.dll'
Plugin make_plugin(Allocator *allocator, const char *directory, const char *name) {
	Plugin plugin = {};

	DynamicString base = dynamic_stringf(allocator, "%s/", directory);

	plugin.directory = dynamic_string(allocator, base);
	plugin.path = dynamic_string(allocator, base);

#if DEVELOPMENT
	PluginReloadContext &reload_context = plugin.reload_context;
	reload_context.marker = dynamic_string(allocator, base);
	reload_context.marker += "__reload_marker";

	reload_context.pdb_path = dynamic_string(allocator, base);
#endif

	base += name;

	plugin.path_pattern = dynamic_string(allocator, base);
	plugin.path_pattern += "*.dll";

	plugin.allocator = allocator_mspace(PLUGIN_MEMORY_SIZE);

	set_newest_plugin_path(plugin);
	load_plugin_module(plugin);

	init_symbols(plugin);

	return plugin;
}

void destroy_plugin(Plugin &plugin) {
	if (!plugin.valid)
		return;

#if DEVELOPMENT
	unload_symbols(plugin);
#endif

	unload_plugin_module(plugin);
	destroy(&plugin.allocator);
}

#if DEVELOPMENT
// Try to find a plugin that matches the reload_path_pattern that is newer than the currently loaded, and load it
void check_for_reloads(Plugin &plugin) {
	PluginReloadContext &reload_context = plugin.reload_context;

	// Check if we have a reload marker in the folder, this will indicate that a build has completed
	// Note that we can't simply check if we have a new dll since the pdb (for instance) might not have been completly written yet.
	WIN32_FIND_DATA data;
	HANDLE reload_marker_handle = FindFirstFile(*reload_context.marker, &data);
	if (reload_marker_handle == INVALID_HANDLE_VALUE)
		return;
	FindClose(reload_marker_handle);

	bool time_changed = data.ftLastWriteTime.dwLowDateTime != reload_context.timestamp.dwLowDateTime || data.ftLastWriteTime.dwHighDateTime != reload_context.timestamp.dwHighDateTime;
	if (!time_changed)
		return;
	reload_context.timestamp = data.ftLastWriteTime;

	ArenaAllocator arena = {};
	init_arena_allocator(&arena, 8);
	Allocator allocator = allocator_arena(&arena);
	reload_context.handle = reloader_setup(&allocator, *reload_context.pdb_path, "Application");

	DeleteFile(*reload_context.marker);

	unload_symbols(plugin);

	unload_plugin_module(plugin);
	set_newest_plugin_path(plugin);

	load_plugin_module(plugin);
	load_symbols(plugin);

	if (reload_context.handle) { // Did we get a valid handle from setup? If not, we cannot reload.
		Allocator new_plugin_allocator = allocator_mspace(PLUGIN_MEMORY_SIZE);

		reload_context.header.old_allocator = &plugin.allocator;
		reload_context.header.new_allocator = &new_plugin_allocator;
		reload_context.header.memory_size = PLUGIN_MEMORY_SIZE;

		int success = reloader_reload(&allocator, *reload_context.pdb_path, &reload_context.header, reload_context.handle);
		if (success) { // Only tell the plugin & swap allocator memory if we actually succeeded with the memory swap!
			if (reload_context.reload) {
				reload_context.reload(&plugin.allocator, &new_plugin_allocator);
			}

			destroy(&plugin.allocator);
			plugin.allocator = new_plugin_allocator;
		}
	}

	log_info("Plugin", "Plugin reloaded!\n");
}
#endif

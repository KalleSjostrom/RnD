// namespace settings {
	struct SettingsData {
		String name;
		unsigned name_id;
		// This is basically a union, but we use the next pointer instead of type to know if next or value is set.
		String value; // A string containing the value for a simple type e.g. an integer (somevariable = 5)
		struct SettingsDataStore *next; // If non-zero, than this data points to a structure of data, e.g. sometable = { somevalue = 5 }
	};
	struct SettingsHashEntry {
		SettingsData *value;
		unsigned key; // equals to data->name_id
	};
	// The settings data store contain overrides of the default values. These are basically the content of the _settings.entity files (or the content of a struct)
	#define SETTINGSDATASTORE_SIZE 96
	struct SettingsDataStore {
		unsigned count;
		SettingsData entries[SETTINGSDATASTORE_SIZE];
		SettingsHashEntry map[SETTINGSDATASTORE_SIZE*2];

		inline SettingsData &operator[](int index) { return entries[index]; }
		inline SettingsData &new_entry() {
			ASSERT(count < ARRAY_COUNT(entries), "Array index out of bounds!")
			return entries[count++];
		}
		inline void push_back(SettingsData &data) {
			ASSERT(count < ARRAY_COUNT(entries), "Array index out of bounds!")
			entries[count++] = data;
		}
	};
	__forceinline SettingsData *get_new_settings_data(SettingsDataStore &settings_data_store, String &name) {
		SettingsData &settings_data = settings_data_store.new_entry();

		settings_data.name = name;
		settings_data.name_id = to_id32(name.length, *name);
		settings_data.next = 0;

		HASH_LOOKUP(entry, settings_data_store.map, ARRAY_COUNT(settings_data_store.map), settings_data.name_id);
		entry->key = settings_data.name_id;
		entry->value = &settings_data;

		return &settings_data;
	}
	__forceinline void add_settings_data(SettingsDataStore &settings_data_store, SettingsData &settings_data) {
		settings_data_store.push_back(settings_data);

		HASH_LOOKUP(entry, settings_data_store.map, ARRAY_COUNT(settings_data_store.map), settings_data.name_id);
		entry->key = settings_data.name_id;
		entry->value = &settings_data;
	}
	struct ComponentSettings {
		SettingsDataStore *settings_data_store;

		unsigned guid_id;
		bool inherit;
		unsigned component_stem_id;
		uint64_t component_path_id;
		Component *component;
	};
	typedef ComponentSettings* ComponentSettingsArray;

	struct Settings {
		String path;
		uint64_t path_id;

		String inherit_path;
		uint64_t inherit_path_id;

		ComponentSettingsArray component_settings;

		GameObject *game_object; // Shortcut, if we have a game object
	};

	void write_settings_data_store(SettingsDataStore &data, char **buffer) {
		write_unsigned(data.count, buffer);
		for (int j = 0; j < data.count; ++j) {
			SettingsData &settings_data = data[j];
			write_string(settings_data.name, buffer);
			write_unsigned(settings_data.name_id, buffer);
			bool is_table = settings_data.next != 0;
			write_bool(is_table, buffer);
			if (is_table) {
				write_settings_data_store(*settings_data.next, buffer);
			} else {
				write_string(settings_data.value, buffer);
			}
		}
	}

	void read_settings_data_store(MemoryArena &arena, SettingsDataStore **settings_data_store, char **buffer) {
		*settings_data_store = (SettingsDataStore *) allocate_memory(arena, sizeof(SettingsDataStore));
		memset(*settings_data_store, 0, sizeof(SettingsDataStore));

		SettingsDataStore &data = **settings_data_store;

		read_unsigned(data.count, buffer);
		for (int j = 0; j < data.count; ++j) {
			SettingsData &settings_data = data[j];
			read_string(arena, settings_data.name, buffer);
			read_unsigned(settings_data.name_id, buffer);

			settings_data.next = 0;

			bool is_table;
			read_bool(is_table, buffer);
			if (is_table) {
				read_settings_data_store(arena, &settings_data.next, buffer);
			} else {
				read_string(arena, settings_data.value, buffer);
			}

			HASH_LOOKUP(entry, data.map, ARRAY_COUNT(data.map), settings_data.name_id);
			entry->key = settings_data.name_id;
			entry->value = &settings_data;
		}
	}

namespace settings {
	void write(Settings &data, char **buffer) {
		// path is derived when read
		write_u64(data.path_id, buffer);

		write_string(data.inherit_path, buffer);
		write_u64(data.inherit_path_id, buffer);

		write_int(array_count(data.component_settings), buffer);
		for (int i = 0; i < array_count(data.component_settings); ++i) {
			ComponentSettings &component_settings = data.component_settings[i];

			write_settings_data_store(*component_settings.settings_data_store, buffer);

			write_unsigned(component_settings.guid_id, buffer);
			write_bool(component_settings.inherit, buffer);
			write_unsigned(component_settings.component_stem_id, buffer);
			write_u64(component_settings.component_path_id, buffer);
		}
	}

	void read(MemoryArena &arena, Settings &data, char **buffer) {
		// path is derived when read
		read_u64(data.path_id, buffer);

		read_string(arena, data.inherit_path, buffer);
		read_u64(data.inherit_path_id, buffer);

		read_serialized_array(data.component_settings, buffer);
		for (int i = 0; i < array_count(data.component_settings); ++i) {
			ComponentSettings &component_settings = data.component_settings[i];

			read_settings_data_store(arena, &component_settings.settings_data_store, buffer);

			read_unsigned(component_settings.guid_id, buffer);
			read_bool(component_settings.inherit, buffer);
			read_unsigned(component_settings.component_stem_id, buffer);
			read_u64(component_settings.component_path_id, buffer);
		}
	}

	bool serialize(MemoryArena &arena, CacheHashEntry *entry, Settings* &array) {
		// Get the entry where the parsed data was placed
		Settings &data = array_last(array);
		CachedData &value = entry->value;

		value.buffer = arena.memory + arena.offset;
		char *buffer = (char*)value.buffer;

		intptr_t start = (intptr_t)buffer;
		write(data, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		value.size = size;

		return true;
	}
	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Settings &data) {
		data.path = make_project_path(filepath, arena);
		char *buffer = (char*)entry->value.buffer;
		read(arena, data, &buffer);
	}
}
typedef Settings* SettingsArray;

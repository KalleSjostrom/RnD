struct SerializationBuffer {
	char *buffer;
	size_t offset;
};
#define SERIALIZATION_BUFFER_SIZE 16*MB
#define DEBUG 0

static String type_info_filename = MAKE_STRING("typeinfo.generated.cpp");
static String reloader_filename = MAKE_STRING("reloader.generated.cpp");
static String cache_filename = MAKE_STRING("reloader.cache");

namespace generate_reloader {

	#include "data_reloadable.cpp"
	#include "parse_hfile.cpp"
	#include "parse_type_info.cpp"
	#include "coalesce_reloadable_data.cpp"
	#include "output_reloader.cpp"

	bool ignored_directory(String &directory, u64 directory_id) {
		(void)directory;
		(void)directory_id;
		return false;
	}

	static String filetype_h = MAKE_STRING("h");
	static String filetype_cpp = MAKE_STRING("cpp");

	static u32 filetype_h_id = make_string_id32(filetype_h);
	static u32 filetype_cpp_id = make_string_id32(filetype_cpp);

	struct GenerateReloadTask {
		Generator *generator;
		FileInfo *file_info;
	};

	struct TLSContext {
		ReloadableArray reloadable_array;
		MemoryArena temp_arena;
		MemoryArena arena;
		SerializationBuffer serialization_buffer;

		b32 reloadable_array_changed;
		b32 initialized;
	};

	struct GeneratorContext {
		// Shared by all threads for this generator
		CacheHashMap *cache_hash_map;
		// Thread local for this generator
		TLSContext *tls_array;
	};

	void do_task(TaskScheduler *scheduler, void *arg) {
		GenerateReloadTask *task = (GenerateReloadTask*) arg;

		i32 thread_index = get_thread_index();

		Generator *generator = task->generator;
		GeneratorContext *rg = (GeneratorContext*) generator->user_data;

		TLSContext &tls = rg->tls_array[thread_index];

		if (!tls.initialized) {
			array_init(tls.reloadable_array, 64);
			setup_arena(tls.temp_arena, 32*MB);
			tls.serialization_buffer.buffer = (char*) PUSH_SIZE(tls.arena, SERIALIZATION_BUFFER_SIZE);
			tls.serialization_buffer.offset = 0;
			tls.reloadable_array_changed = false;
			tls.initialized = true;
		}

		FileInfo *info = task->file_info;
		ASSERT(info->state != CacheState_Removed, "Should have already handled ths case.");

		CacheHashEntry *entry = 0;
		bool valid_entry = get_cache_entry_for(*rg->cache_hash_map, info->key, &entry);
		bool needs_to_parse = !valid_entry || info->state == CacheState_Modified || info->state == CacheState_Added;

		i32 count_snapshot = array_count(tls.reloadable_array);

		if (needs_to_parse) {
			MemoryBlockHandle block_handle = begin_block(tls.temp_arena);
				printf("Parsing: %s, %d\n", *info->filepath, thread_index);

				parse_hfile(tls.temp_arena, tls.arena, *info->filepath, tls.reloadable_array);
				tls.reloadable_array_changed = reloadable::serialize(tls.serialization_buffer, entry, tls.reloadable_array, count_snapshot) || tls.reloadable_array_changed;
				ASSERT(tls.serialization_buffer.offset < SERIALIZATION_BUFFER_SIZE, "Serialization buffer ran out of memory!");
			end_block(tls.temp_arena, block_handle);
		} else {
			printf("Deserialize: 0x%016zx\n", info->key);
			reloadable::deserialize(tls.arena, entry, tls.reloadable_array);
		}

#if DEBUG
		for (i32 i = count_snapshot; i < array_count(tls.reloadable_array); ++i) {
			ReloadableStruct &data = tls.reloadable_array[i];
			printf("Found struct [0x%016zx] %s %x [%d %d] %016zx\n", info->key, *data.name, data.type_id, thread_index, i, (intptr_t)tls.reloadable_array);
		}
#endif // DEBUG
	}

	void coalesce(Generator &generator, TaskScheduler &scheduler) {
		GeneratorContext *rg = (GeneratorContext*) generator.user_data;
		TLSContext *tls_array = rg->tls_array;
		MemoryArena arena = {};

		FileSystem &file_system = generator.compiler->file_system;

		i32 count = 0;
		for (i32 i = 0; i < scheduler.thread_count; i++) {
			count += array_count(tls_array[i].reloadable_array);
		}

		u32 hash_count = HASH_SIZE_FOR((u32) (count * 1.5f));
		// size_t hash_size = sizeof(ReloadableHashEntry) * hash_count;

		ReloadableHashEntry *entries = PUSH_STRUCTS(arena, hash_count, ReloadableHashEntry, true);
		ReloadableMap reloadable_map;
		HashMap_Init(reloadable_map, entries, hash_count, 2, 0);

		ReloadableStruct **struct_collection = PUSH_STRUCTS(arena, count, ReloadableStruct*);
		i32 struct_collection_count = 0;

		for (i32 i = 0; i < scheduler.thread_count; i++) {
			ReloadableArray &reloadable_array = tls_array[i].reloadable_array;
			for (i32 j = 0; j < array_count(reloadable_array); ++j) {
				ReloadableStruct &data = reloadable_array[j];

				printf("Adding struct %s %x [%d %d] %016zx\n", *data.name, data.type_id, i, j, (intptr_t)reloadable_array);

				HashMap_Lookup(entry, reloadable_map, data.type_id, 0);
				ASSERT(entry->key == 0, "Reloadable struct of this type have already been added! (full_name=%s)", *data.full_name);

				entry->key = data.type_id;
				entry->value = struct_collection_count;
				struct_collection[struct_collection_count++] = &data;
			}
		}

		ASSERT(struct_collection_count == count, "Mismatch");

		String type_info_path = make_path(arena, file_system.output_folder, type_info_filename);
		String reloader_output_path = make_path(arena, file_system.output_folder, reloader_filename);

		MemoryBlockHandle block_handle = begin_block(arena);
			parse_type_info(arena, type_info_path, struct_collection, reloadable_map);
		end_block(arena, block_handle);

		coalesce_reloadable_data(struct_collection, struct_collection_count, reloadable_map);

		output_reloader(type_info_path, reloader_output_path, struct_collection, struct_collection_count, reloadable_map);

		String cache_path = make_path(arena, file_system.output_folder, cache_filename);
		write_cache_to_disc(arena, *cache_path, *rg->cache_hash_map);
	}

	void register_file_info_change(Compiler &compiler, Generator &generator, FileInfo &file_info) {
		if (file_info.filetype == filetype_h_id || file_info.filetype == filetype_cpp_id) {
			if (file_info.state == CacheState_Removed) {
				return;
			}

			printf("Reloader: register_file_info_change. (filepath=%s, 0x%016zx)\n", *file_info.filepath, file_info.key);

			GenerateReloadTask *grt = PUSH_STRUCTS(compiler.arena, 1, GenerateReloadTask);
			grt->generator = &generator;
			grt->file_info = &file_info;

			Task *task = PUSH_STRUCTS(compiler.arena, 1, Task);
			task->function = do_task;
			task->argument = grt;
			int job_handle = scheduler_add_tasks(compiler.scheduler, 1, task);
		}
	}

	void register_generator(Generator &generator) {
		generator.name = MAKE_STRING("reloader");

		generator.active = true;

		generator.ignored_directory = &ignored_directory;
		generator.register_file_info_change = &register_file_info_change;

		MemoryArena arena = {}; // Free at some point?

		GeneratorContext *rg = PUSH_STRUCT(arena, GeneratorContext);
		rg->cache_hash_map = PUSH_STRUCT(arena, CacheHashMap, true);
		rg->tls_array = PUSH_STRUCTS(arena, generator.compiler->scheduler.thread_count, TLSContext, true);

		// TODO(kalle): Handle growning hashmaps?
		CacheHashEntry *entries = PUSH_STRUCTS(arena, 2048, CacheHashEntry, true);
		CacheHashMap &hashmap = *rg->cache_hash_map;
		HashMap_Init(hashmap, entries, 2048, 2, 0);

		generator.user_data = rg;

		FileSystem &file_system = generator.compiler->file_system;
		String cache_path = make_path(arena, file_system.output_folder, cache_filename);
		read_cache_from_disc(*cache_path, arena, *rg->cache_hash_map);
	}
}

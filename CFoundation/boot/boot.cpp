// warning C4996: 'vsnprintf': This function or variable may be unsafe. Consider using vsnprintf_s instead.
#pragma warning(disable : 4996)

// warning C4060: switch statement contains no 'case' or 'default' labels
#pragma warning(disable : 4060)

// warning C4100: unreferenced formal parameter
#pragma warning(disable : 4100)

// warning C4189: local variable is initialized but not referenced
#pragma warning(disable : 4189)

// NOTE(kalle): If changing these values during runtime, the debug object won't get correctly reloaded (since the offset to the start of the debug memory will be wrong)
static const size_t GAME_MEMORY_SIZE = 16*MB;
static const size_t SCRATCH_SPACE_MEMORY_SIZE = 16*MB;
static const size_t MEMORY_BLOCK_ALIGNMENT = 32; // TODO(kalle): What alignment do we want? Do we care?

#include "boot.h"

namespace game {
	static void *_base_memory = 0; // Pointer to start of our game dll memory.
	static AllocatorApi* _allocator_api = 0; // The allocator api used to create our own allocator

	struct AppMemory {
		intptr_t allocator_object; // Pointer to where the allocator object is stored in the stingray engine. (created with make_plugin_allocator api call).

		void *debug_memory; // Pointer to the debug memory
		ReloaderEntryPoint *entry_point;

		// This always have to be last in the struct, since the malloc states/chunks will follow from this point.
		AhAllocator allocator; // Space for our allocator.

		AppMemory() : allocator_object(0), debug_memory(0), entry_point(0), allocator(0,0) {}
	};

#define START_OF_GAME_MEMORY(base) ((char*)(base) + sizeof(AppMemory))
#define START_OF_SCRATCH_SPACE(base) (START_OF_GAME_MEMORY(base) + GAME_MEMORY_SIZE)

#define TOTAL_MEMORY_SIZE (sizeof(AppMemory) + GAME_MEMORY_SIZE + SCRATCH_SPACE_MEMORY_SIZE)

	// Gets the the api's from the engine
	static void set_engine_api(GetApiFunction get_engine_api) {
		globals::script_api = (ScriptApi*)get_engine_api(C_API_ID);
		globals::logging_api = (LoggingApi*)get_engine_api(LOGGING_API_ID);
		globals::profiler_api = (ProfilerApi*)get_engine_api(PROFILER_API_ID);
		globals::timer_api = (TimerApi*)get_engine_api(TIMER_API_ID);
		globals::thread_api = (ThreadApi*)get_engine_api(THREAD_API_ID);

		PluginManagerApi *plugin_manager_api = (PluginManagerApi*)get_engine_api(PLUGIN_MANAGER_API_ID);
		globals::wwise_library_api = (WwiseLibraryApi*)plugin_manager_api->get_plugin_library_api(WWISE_PLUGIN_NAME);
		globals::nav_library_api = (NavLibraryApi*)plugin_manager_api->get_plugin_library_api(NAV_PLUGIN_NAME);

		_allocator_api = (AllocatorApi*) get_engine_api(ALLOCATOR_API_ID);

#if VERIFY_ALL_APIS
		verify_all_apis();
#endif
	}

#if GENERATE_RELOAD_LAYOUT
	#include "generated/reloader.generated.cpp"
#endif

	static void setup_game(GetApiFunction get_engine_api) {
		set_engine_api(get_engine_api);

#if GENERATE_RELOAD_LAYOUT
		// Take a snapshot of how the layout looks before the reload
		Reloader::generate_layout();
#endif

		///// Get memory from stingray /////
			// Get the allocator api so that we can allocate our own memory chunk
			AllocatorObject *allocator_object = _allocator_api->make_plugin_allocator("game_root"); // The allocator object is the memory of the actual allocator

			// We allocate enough room for stingrays allocator object, our global allocator and the block of game memory
			_base_memory = _allocator_api->allocate(allocator_object, TOTAL_MEMORY_SIZE, MEMORY_BLOCK_ALIGNMENT);

			// Should we fill with zeros or something else?
			memset(_base_memory, 0, TOTAL_MEMORY_SIZE);

		///// Fill our game memory header /////
			AppMemory *app_memory = (AppMemory*)_base_memory;
			app_memory->allocator_object = (intptr_t)allocator_object; // Insert the allocator object

			// New our allocator into it's slot
			new (&app_memory->allocator) AhAllocator(START_OF_GAME_MEMORY(_base_memory), GAME_MEMORY_SIZE);
			globals::allocator = &app_memory->allocator; // Write the pointer to our allocator to our globals so AH_NEW/AH_DELETE works.

			// Use our allocator to create the reload entry point
			app_memory->entry_point = AH_ALLOC_STRUCT(ReloaderEntryPoint); // This will write it into app_memory->entry_point

#if DEVELOPMENT
			// Initiate the debug before the game.
			app_memory->debug_memory = debug::init(get_engine_api);
#endif

			// Fill in the entry point
			app_memory->entry_point->global_table = AH_ALLOC_STRUCT(GlobalTable);
			app_memory->entry_point->game         = AH_NEW(Game);

			// TODO(kalle) Remove this?
			globals::game = app_memory->entry_point->game; // Copy a pointer to the game to our global storage for easy access
			globals::global_table = app_memory->entry_point->global_table; // Copy a pointer to the global_table to our global storage for easy access

		// Reserve some scratch space for temporary (per frame) usage.
		globals::scratch_space = scratch_space::init(START_OF_SCRATCH_SPACE(_base_memory), SCRATCH_SPACE_MEMORY_SIZE);

		GET_GAME()->setup();
	}

	static void *start_reload(GetApiFunction get_engine_api) {
		while (globals::active_threads > 0) {
			_Application.sleep(100);
		}

		return _base_memory; // This is passed to finish_reload, as base_memory.
	}

	static void finish_reload(GetApiFunction get_engine_api, void *base_memory) {
		set_engine_api(get_engine_api);

#if defined(GENERATE_RELOAD_LAYOUT) && defined(RELOAD_DATA)
		{
			uint64_t reload_start_time = _Timer.ticks();
			AppMemory *old_app_memory = (AppMemory*)base_memory;

			AllocatorObject *allocator_object = (AllocatorObject*) old_app_memory->allocator_object;
			void *new_base_memory = _allocator_api->allocate(allocator_object, TOTAL_MEMORY_SIZE, MEMORY_BLOCK_ALIGNMENT);

			// Should we fill with zeros or something else?
			memset(new_base_memory, 0, TOTAL_MEMORY_SIZE);

			AppMemory *app_memory = (AppMemory*)new_base_memory;
			app_memory->allocator_object = (intptr_t)allocator_object;
			app_memory->debug_memory = old_app_memory->debug_memory;

			// New our allocator into it's slot
			new (&app_memory->allocator) AhAllocator(START_OF_GAME_MEMORY(new_base_memory), GAME_MEMORY_SIZE);

			// NOTE(kalle): The malloc state is not reloaded in it's full, meaning that the internal tracking of free chunks are not kept between reloads.
			// This will caused freed 'gaps' to not get reused, i.e. when AH_NEW'ing a new thing after a reload, it will grab of the 'top' even though there might be free gaps.
			// I don't think this will matter when debugging. We will rarely get gaps if we continue with the current code-design.
			// When going between states, we will clear out all memory used by that state and not leave any holes.
			// Even if we did have a lot of gaps, the only negative effect this has is more memory-usage, which I think is fine when debugging with reloads.
			void *old_mspace = old_app_memory->allocator.mspace();
			void *new_mspace = app_memory->allocator.mspace();

			Reloader::reload(old_mspace, GAME_MEMORY_SIZE, new_mspace, START_OF_SCRATCH_SPACE(new_base_memory));
			_allocator_api->deallocate(allocator_object, base_memory);
			base_memory = new_base_memory;

			// Store the layout for the next reload
			Reloader::generate_layout();

			double reload_time_seconds = _Timer.ticks_to_seconds(_Timer.ticks() - reload_start_time);
			LOG_INFO("Boot", "Reloaded data layout in %.2f ms", (reload_time_seconds * 1000.0f));
		}
#endif

		_base_memory = base_memory;
		AppMemory *app_memory = (AppMemory*)base_memory;

		globals::allocator = &app_memory->allocator; // Write the pointer to our allocator to our globals so AH_NEW/AH_DELETE works.

		void *mspace = app_memory->allocator.mspace();
		app_memory->entry_point = (ReloaderEntryPoint*)chunk2mem(next_chunk(mem2chunk(mspace)));

		globals::game = app_memory->entry_point->game; // Copy a pointer to the game to our global storage for easy access
		globals::global_table = app_memory->entry_point->global_table; // Copy a pointer to the global_table to our global storage for easy access

		globals::scratch_space = scratch_space::init(START_OF_SCRATCH_SPACE(_base_memory), SCRATCH_SPACE_MEMORY_SIZE);

#if DEVELOPMENT
		debug::on_script_reloaded((void *)app_memory->debug_memory);
#endif

		GET_GAME()->finish_reload();
	}

#if DEVELOPMENT
	static unsigned frame_counter = 0;
	static double frame_time_acc = 0;
	static double frame_time_min = 999;
	static double frame_time_max = 0;
	static char frame_time[64] = "Frame time: (avg: N/A ms, min: N/A ms, max: N/A ms)";
#endif

	// Game - Handling game update, rendering, shutdown and so on
	static void update(float dt) {
		uint64_t start = _Timer.ticks();

		_Debug.text("%s Engine: %s", (char*)build_info::BUILD_GAME_INFORMATION, _Application.build_identifier());

		GET_GAME()->update(dt);

#if DEVELOPMENT
		double seconds = _Timer.ticks_to_seconds(_Timer.ticks() - start);

		frame_time_acc += seconds;
		frame_time_min = seconds < frame_time_min ? seconds : frame_time_min;
		frame_time_max = seconds > frame_time_max ? seconds : frame_time_max;
		frame_counter++;

		if (frame_counter == 100) {
			sprintf(frame_time, "Frame time: (avg: %.2f ms, min: %.2f ms, max: %.2f ms)", (frame_time_acc/frame_counter)*1000.0f, frame_time_min*1000.f, frame_time_max*1000.f);
			frame_time_acc = 0;
			frame_counter = 0;
			frame_time_min = 999;
			frame_time_max = 0;
		}

		_Debug.text(frame_time);

		debug::update(dt);
#endif
		// Clear the scratch space at the end of each frame
		scratch_space::clear(globals::scratch_space);
	}

	static void render() {
		GET_GAME()->render();

#if DEVELOPMENT
		debug::render();
#endif
	}

	static void shutdown() {
		GET_GAME()->shutdown();

#if DEVELOPMENT
		debug::shutdown();
#endif

		// The pointer to the allocator object is first in base memory, get it and cast to an usuable AllocatorObject.
		intptr_t allocator_object_pointer = *(intptr_t *)_base_memory;
		AllocatorObject *allocator_object = (AllocatorObject *) allocator_object_pointer;

		// Just wipe the entire memory!
		_allocator_api->deallocate(allocator_object, _base_memory);
		_allocator_api->destroy_plugin_allocator(allocator_object);

		// Zero out our static pointers
		_allocator_api = 0;
		_base_memory = 0;
	}
}

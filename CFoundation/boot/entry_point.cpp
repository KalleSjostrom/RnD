// #pragma once

// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable : 4201)

// warning C4127: conditional expression is constant
#pragma warning(disable : 4127)

#if defined(PS4)
	#include <stddef.h>
#endif

#include "sdk/plugin_api/plugin_api.h"
#include "sdk/plugin_foundation/platform.h"

using namespace capi;

#include "wwise_plugin/wwise_library_api.h"
#include "plugins/gwnav_plugin/nav_plugin_c_api.h"

// Globals that foundation uses
namespace globals {
	static ScriptApi *script_api;
	static LoggingApi *logging_api;
	static ProfilerApi *profiler_api;
	static TimerApi *timer_api;
	static ThreadApi *thread_api;

	static WwiseLibraryApi *wwise_library_api;
	static NavLibraryApi *nav_library_api;
}

static char _msg_string[512];
#define LOG_INFO(system, format, ...) { \
	sprintf_s(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
	globals::logging_api->info(system, _msg_string); \
}
#define LOG_WARNING(system, format, ...) { \
	sprintf_s(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
	globals::logging_api->warning(system, _msg_string); \
}
#define LOG_ERROR(system, format, ...) { \
	sprintf_s(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
	globals::logging_api->error(system, _msg_string); \
}

// Use this assert inside the plugin.
// It will call the engine, producing a crash-report and shutting down the game if no debugger is present.
// If there is, we crash with trying to write to 0, causing an exception in the plugin manager.
// The plugin manager will then unload our plugin and let us fix the problem. It will reload as usual when we recompile.
// LOG_ERROR("Game Assert", "Assertion failed at %s:%d\n\t%s\n", __FILE__, __LINE__, buf);
#if defined(PS4)
#define ASSERT(arg, format, ...) { \
	if (!(arg)) { \
		char buf[1024]; \
		sprintf_s(buf, sizeof(buf), format, ##__VA_ARGS__); \
		bool is_debugger_present = globals::script_api->Error->report_script_assert_failure(__LINE__, __FILE__, (#arg), buf); \
		if (is_debugger_present) { __asm volatile ("int $0x41"); } \
		globals::script_api->Error->plugin_crash(); \
	} \
}
#else
#define ASSERT(arg, format, ...) { \
	if (!(arg)) { \
		char buf[1024]; \
		sprintf_s(buf, sizeof(buf), format, ##__VA_ARGS__); \
		bool is_debugger_present = globals::script_api->Error->report_script_assert_failure(__LINE__, __FILE__, (#arg), buf); \
		if (is_debugger_present) { *(volatile int *)0 = 0; } \
	} \
}
#endif

// Some common defines
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
#define KB 1024
#define MB 1024*KB
#define GB 1024*MB

#define FLT_EPSILON 1e-5

#include <stdint.h>
#include <string.h> /* strlen, memcpy */
#include <stdio.h> /* id_string.h: sprintf */
#include <stdlib.h> /* strtoul */
#include <limits.h> /* UINT_MAX */

#ifndef PS4
	#define va_start(a_list, ...) (_crt_va_start(a_list, __VA_ARGS__))
	#define va_end(a_list) (_crt_va_end(a_list))
#endif

#include "../utils/hash_function.h"
#include "../utils/id_string.h"

typedef unsigned Id32;
typedef uint64_t Id64;

// allocation
#include "../utils/memory/dlmalloc.c"
#include "../utils/memory/ah_allocator.cpp"
#include "../utils/memory/scratch_space.cpp"

// math
#include "math.h" /* math.inl, vector3.inl: sqrtf, cos, sin */
#include "../utils/math/math.inl"
#include "../utils/math/vector2.h"
#include "../utils/math/vector3.inl"
#include "../utils/math/vector4.h"
#include "../utils/math/matrix4x4.h"
#include "../utils/math/quaternion.h"
#include "../utils/math/oobb.h"
#include "../utils/math/local_transform.h"
#include "../utils/math/random.h"
#include "../utils/string_utils.cpp"

// Globals that the game uses
namespace globals {
	static ScratchSpace scratch_space; // Temporary scratch space, either use directly or through AhTempAllocator.
	static AhAllocator *allocator; // Our one and only master allocator governing our memory chunk
	static void *game; // This is a pointer to the actual user game memory. This is _after_ the allocators and the dlmalloc headers
}

static const unsigned MAX_INSTANCES = 2048;
static const unsigned MAX_NR_COMPONENTS = 32;
static const unsigned MAX_GO_FIELDS_PER_COMPONENTS = 32;

// overload the new operator
#if defined(PS4)
	void* operator new(size_t, void *memory) { return (memory); }
#else
	inline void *__CRTDECL operator new(size_t, void *memory) { return (memory); }
#endif

// Replaces new and delete
#define AH_NEW(T, ...) (new (globals::allocator->allocate(sizeof(T), __alignof(T))) T(__VA_ARGS__))
#define AH_ALLOC_STRUCT(T) ((T*)globals::allocator->allocate(sizeof(T), __alignof(T)))
// TODO(kalle): Try to get rid of the T parameter. Can't use typeof, or typeinfo...
#define AH_DELETE(T, object) { (object)->~T(); globals::allocator->deallocate((object)); (object) = 0; }

#define AH_NEW_WITH_ALLOCATOR_API(T, allocator, allocator_object, ...) (new (allocator->allocate(allocator_object, sizeof(T), __alignof(T))) T(__VA_ARGS__))
#define AH_DELETE_WITH_ALLOCATOR_API(T, allocator, allocator_object, object) { (object)->~T(); allocator->deallocate(allocator_object, (object)); (object) = 0; }

#define ZERO_STRUCT(s) do { memset(&(s), 0, sizeof(s)); } while(0,0)
#define ZERO_STRUCTS(s, count) do { memset(&(s), 0, (count) * sizeof(s)); } while(0,0)
#define SCRATCH_ALLOCATE(type, count) (type *)scratch_space::allocate(globals::scratch_space, (count) * sizeof(type))

// Interface with the engine. This is done to mimic how it was in lua, i.e. _Keyboard.pressed.
#define _Actor           (*globals::script_api->Actor)
#define _Application     (*globals::script_api->Application)
#define _Camera          (*globals::script_api->Camera)
#define _Entity          (*globals::script_api->Entity)
#define _EntityManager   (*globals::script_api->Entity->Manager)
#define _Flow            (*globals::script_api->Flow)
#define _GameSession     (*globals::script_api->GameSession)
#define _Gui             (*globals::script_api->Gui)
#define _GwNavWorld      (*globals::nav_library_api->nav_world_api)
#define _GwNavQueries    (*globals::nav_library_api->nav_query_api)
#define _GwNavGeneration (*globals::nav_library_api->nav_generation_api)
#define _GwNavAStar      (*globals::nav_library_api->nav_astar_api)
#define _Keyboard        (*globals::script_api->Input->Keyboard)
#define _Lan             (*globals::script_api->Lan)
#define _Level           (*globals::script_api->Level)
#define _LineObject      (*globals::script_api->LineObject)
#define _Material        (*globals::script_api->Material)
#define _Mouse           (*globals::script_api->Input->Mouse)
#define _Mover           (*globals::script_api->Mover)
#define _Network         (*globals::script_api->Network)
#define _Pad             (*globals::script_api->Input->Gamepad)
#define _PhysicsWorld    (*globals::script_api->PhysicsWorld)
#define _Profiler        (*globals::profiler_api)
#define _PS4Pad          (*globals::script_api->Input->PS4Pad)
#define _Psn             (*globals::script_api->Psn)
#define _SaveSystem      (*globals::script_api->SaveSystem)
#define _ScriptData      (*globals::script_api->DynamicScriptData)
#define _Timer           (*globals::timer_api)
#define _Unit            (*globals::script_api->Unit)
#define _Utilities       (*globals::script_api->Utilities)
#define _Window          (*globals::script_api->Window)
#define _World           (*globals::script_api->World)
#define _Wwise           (*globals::wwise_library_api)
#define _Thread          (*globals::thread_api)
#define _Terrain         (*globals::script_api->Terrain)
#define _ScatterSystem   (*globals::script_api->ScatterSystem)

typedef void* WwiseWorldPtr;

#define VERIFY_ALL_APIS 0

#if VERIFY_ALL_APIS
#define VERIFY_API(_api) do {\
	unsigned count = sizeof(_api)/sizeof(void*); \
	intptr_t base = (intptr_t)(&_api); \
	for (unsigned i = 0; i < count; i++) { \
		intptr_t func_ptr = base + i * sizeof(void*); \
		ASSERT(*(intptr_t*)func_ptr, "Function pointer is null! (function index=%d, api="#_api")", i); \
	} \
} while(0,0)

static void verify_all_apis() {
	VERIFY_API(_Actor);
	VERIFY_API(_Application);
	VERIFY_API(_Camera);
	VERIFY_API(_Entity);
	VERIFY_API(_EntityManager);
	VERIFY_API(_Flow);
	VERIFY_API(_GameSession);
	VERIFY_API(_Gui);
	VERIFY_API(_GwNavWorld);
	VERIFY_API(_GwNavQueries);
	VERIFY_API(_GwNavGeneration);
	VERIFY_API(_GwNavAStar);
	VERIFY_API(_Keyboard);
	VERIFY_API(_Lan);
	VERIFY_API(_Level);
	VERIFY_API(_LineObject);
	VERIFY_API(_Material);
	VERIFY_API(_Mouse);
	VERIFY_API(_Mover);
	VERIFY_API(_Network);
	VERIFY_API(_Pad);
	VERIFY_API(_PhysicsWorld);
	VERIFY_API(_Profiler);
	VERIFY_API(_PS4Pad);
#if PS4
	VERIFY_API(_Psn);
#endif // PS4
	VERIFY_API(_SaveSystem);
	VERIFY_API(_ScriptData);
	VERIFY_API(_Timer);
	VERIFY_API(_Unit);
	VERIFY_API(_Utilities);
	VERIFY_API(_Window);
	VERIFY_API(_World);
	VERIFY_API(_Wwise);
	VERIFY_API(_Thread);
	VERIFY_API(_Terrain);
	VERIFY_API(_ScatterSystem);
}
#endif

#include "../utils/ticket_mutex.inl"
#include "../utils/utils.cpp"

namespace game {
	struct Profile {
		Profile(const char *name) {
			globals::profiler_api->profile_start(name);
		}
		~Profile() {
			globals::profiler_api->profile_stop();
		}
	};
	struct StepProfile {
		StepProfile(const char *name) {
			globals::profiler_api->profile_start(name);
		}
		void step(const char *name) {
			globals::profiler_api->profile_stop();
			globals::profiler_api->profile_start(name);
		}
		~StepProfile() {
			globals::profiler_api->profile_stop();
		}
	};

	#include "../utils/hash_map_defines.inl"
	#include "../utils/hashmap.cpp"
	#include "../utils/linked_list_defines.inl"
	#include "../utils/ah_temp_allocator.cpp" // uses _global_scratch_space
	#include "../utils/package_manager.cpp"
}
#include "plugin.cpp"

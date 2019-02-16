#define PLUGIN 1

#include "common.h"
#include "plugin.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define USE_INTRINSICS 1
#include "core/math/math.h"

#ifdef SYSTEM_OPENGL
	#include <gl/gl.h>
	#include "utils/win32_setup_gl.h"

	#include "opengl/gl_program_builder.cpp"
#endif // SYSTEM_OPENGL

#ifdef SYSTEM_GRAPHICS
	#include "utils/camera.cpp"
#endif // SYSTEM_CAMERA

#ifdef SYSTEM_OPENCL
	#include "utils/win32_setup_cl.h"

	#define CL(src) "" #src

	#include "opencl/cl_manager.cpp"
	#include "opencl/cl_program_builder.cpp"
#endif // SYSTEM_OPENGL

#ifdef SYSTEM_COMPONENTS
	// namespace globals {
	// 	static ArenaAllocator *transient_arena;
	// };
	// #define SCRATCH_ALLOCATE_STRUCT(type, count) PUSH_STRUCTS(*globals::transient_arena, count, type)
	#include "engine/generated/component_group.cpp"
#endif

#ifdef SYSTEM_AUDIO
	#include "utils/audio_manager.cpp"
#endif // SYSTEM_AUDIO

#if defined(FEATURE_RELOAD) || defined(SYSTEM_OPENGL) || defined(SYSTEM_OPENCL)
	#define SYSTEM_RELOAD
#endif

#ifdef SYSTEM_GUI
	#include "engine/utils/gui.cpp"
#endif

struct Application {
	EngineApi *engine;
	InputData *input;

	Allocator *allocator;
	ArenaAllocator transient_arena;

#ifdef SYSTEM_COMPONENTS
	ComponentGroup components;
#endif // SYSTEM_COMPONENTS

#ifdef SYSTEM_AUDIO
	AudioManager audio_manager;
#endif // SYSTEM_AUDIO

	// RenderPipe render_pipe;
#ifdef SYSTEM_GRAPHICS
	Camera camera;
#endif SYSTEM_GRAPHICS
	// Random random;

#ifdef SYSTEM_OPENCL
	ClInfo cl_info;
#endif

#ifdef SYSTEM_GUI
	GUI gui;
#endif

	PLUGIN_DATA plugin_data;
};

#define EXPORT extern "C" __declspec(dllexport)

void plugin_setup(Application *application);
void plugin_update(Application *application, float dt);
void plugin_render(Application *application);

#ifdef SYSTEM_RELOAD
	#ifdef FEATURE_RELOAD
		void plugin_reloaded(Application *application);
	#endif

	EXPORT PLUGIN_RELOAD(reload) {
		Application *application = (Application *)top_memory(new_allocator->mspace);

		global_set_error(application->engine->error);
		global_set_logging(application->engine->logging);

		#ifdef OS_WINDOWS
			#ifdef SYSTEM_OPENGL
				setup_gl();
			#endif
			#ifdef SYSTEM_OPENCL
				setup_cl();
			#endif
		#endif

		reset(&application->transient_arena);
		#ifdef SYSTEM_COMPONENTS
			application->components.allocator = application->allocator;
			application->components.input.input_data = input;

			reload_programs(application->components);
		#endif // SYSTEM_COMPONENTS

		#ifdef FEATURE_RELOAD
			plugin_reloaded(application);
		#endif
	}
#endif // FEATURE_RELOAD

EXPORT PLUGIN_SETUP(setup) {
	global_set_error(engine->error);
	global_set_logging(engine->logging);

	Application *application = (Application *)allocate(allocator, sizeof(Application), true);
	application->allocator = allocator;

	application->engine = engine;
	application->input = input;

	#ifdef OS_WINDOWS
		#ifdef SYSTEM_OPENGL
			setup_gl();
		#endif
		#ifdef SYSTEM_OPENCL
			setup_cl();
		#endif
	#endif

	#ifdef SYSTEM_COMPONENTS
		init_arena_allocator(&application->transient_arena, 8*MB);
		globals::transient_arena = &application->transient_arena;

		application->components.input.input_data = input;
		application->components.arena = &application->persistent_arena;

		setup_programs(application->components);
	#endif

	#ifdef SYSTEM_OPENCL
		application->cl_info = init_opencl(application->transient_arena);
	#endif

	#ifdef SYSTEM_GRAPHICS
		setup_camera(application->camera, V3(0, 0, 1000), 60, ASPECT_RATIO);
	#endif

	plugin_setup(application);
}

EXPORT PLUGIN_UPDATE(update) {
	Application *application = (Application*) top_memory(allocator->mspace);
	plugin_update(application, dt);

	{ // Update the application
		#ifdef SYSTEM_COMPONENTS
			// Update all the components
			update_components(application->components, dt);
			// Handle component/component communication.
			component_glue::update(application->components, dt);
		#endif

		#ifdef SYSTEM_AUDIO
			// Update sound
			application->audio_manager.update(application->transient_arena, application->engine, dt);
		#endif
	}

	plugin_render(application);

	reset(&application->transient_arena);
}

#define PLUGIN 1

#include "common.h"
#include "plugin.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define USE_INTRINSICS 1
#include "core/math/math.h"
#include "core/math/matrix4x4.h"

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

#include "core/memory/allocator.cpp"
#include "core/memory/mspace_allocator.cpp"
#include "core/memory/arena_allocator.cpp"

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

	// NOTE(kalle) PLUGIN_DATA should be defined to the users "entry struct". Cannot be void* since the reloader won't know how to follow that!
#ifdef PLUGIN_DATA
	PLUGIN_DATA plugin_data;
#endif
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

		#ifdef SYSTEM_OPENGL
			setup_gl();
		#endif
		#ifdef SYSTEM_OPENCL
			setup_cl();
		#endif

		reset(&application->transient_arena);
		#ifdef SYSTEM_COMPONENTS
			application->components.allocator = application->allocator;
			application->components.input.input_data = application->input;

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

	#ifdef SYSTEM_OPENGL
		setup_gl();
	#endif
	#ifdef SYSTEM_OPENCL
		setup_cl();
	#endif

	#ifdef SYSTEM_COMPONENTS
		init_arena_allocator(&application->transient_arena, 8);

		application->components.input.input_data = input;
		application->components.allocator = allocator;

		setup_programs(application->components);
	#endif

	#ifdef SYSTEM_OPENCL
		application->cl_info = init_opencl(application->transient_arena);
	#endif

	#ifdef SYSTEM_GRAPHICS
		int screen_width;
		int screen_height;
		engine->screen_dimensions(screen_width, screen_height);
		setup_camera(application->camera, vector3(0, 0, 1000), 60, screen_width / screen_height);
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

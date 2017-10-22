#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include <string.h>

#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>

#include "engine/utils/common.h"
#include "engine/plugin.h"
#include "settings.h"

#define USE_INTRINSICS 1
#include "engine/utils/math/math.h"
#include "generator/utils/parser.cpp"

#define CL_ERROR_CHECKING 1
#define GL_ERROR_CHECKING 1

#include "opencl/cl_program_builder.cpp"
#include "generated/fluid_kernel.generated.cpp"
#include "opencl/cl_manager.cpp" // This is dependent on fluid_kernel.generated.cpp! :o

#include "opengl/gl_program_builder.cpp"
#include "generated/setup_vertex_array.cpp"

#include "engine/utils/profiler.c"
#include "engine/utils/font_loader.cpp"

#include "../external tools/obj_compiler/obj_compiler.cpp"

enum ProfilerScopes {
	ProfilerScopes__fluid_simulate,
	ProfilerScopes__memset,
	ProfilerScopes__hashmap_insert,
	ProfilerScopes__narrowphase,
	ProfilerScopes__acceleration,
	ProfilerScopes__first_pass,
	ProfilerScopes__gauss_seidel_init,
	ProfilerScopes__gauss_seidel_iter,
	ProfilerScopes__integrate,

	ProfilerScopes__count,
};

#include "engine/utils/gui.cpp"

struct AppMemory {
	bool initialized;
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLuint eab;

	VertexData data;

	cl_manager::ClInfo info;

	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	gui::GUI gui;

	char *fps_string;
	i32 fps_job_handle;
	bool fps_update;
	int fps_frames;
	float fps_timer;
};
#include "fluid.cpp"

EXPORT PLUGIN_RELOAD(reload) {
}

// logging
EXPORT PLUGIN_UPDATE(update) {
	AppMemory am = *(AppMemory*) memory;
	if (!am.initialized) {
		am.initialized = true;
		// generate_layout();

		MemoryArena empty = {};
		am.persistent_arena = empty;
		am.transient_arena = empty;
		setup_arena(am.transient_arena, MB);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(12);

		am.shader_program = gl_program_builder::create_from_source_files(am.transient_arena, SHADER_VERTEX_SOURCE, SHADER_FRAGMENT_SOURCE, 0);
		am.info = cl_manager::init();
		cl_manager::ClInfo &info = am.info;
		cl_manager::create_program_and_kernels(am.transient_arena, info);
		cl_manager::setup_buffers(info);
		cl_manager::setup_kernels(info);

		gl_manager::setup_vertex_array(&am.vao, info.buffers);
		gl_manager::setup_uniforms(am.shader_program);

		// clear_memory(am.transient_arena);

		gui::GUISettings settings = {};
		settings.font_path = "../assets/font.gamefont";
		settings.text_vertex_shader = "../shaders/text.vert";
		settings.text_fragment_shader = "../shaders/text.frag";

		gui::init(am.gui, am.persistent_arena, am.transient_arena, settings);

		am.fps_string = (char*)PUSH_SIZE(am.persistent_arena, 32);
		sprintf(am.fps_string, "Framerate: N/A");
		String string = make_string(am.fps_string);
		am.fps_job_handle = gui::create_string(am.gui, true, string);

		if (!gl_program_builder::validate_program(am.shader_program))
			return -1;
	}

	run_fluid(&am);

	{ // RENDER
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(am.shader_program);
		glBindVertexArray(am.vao);
		glDrawArrays(GL_POINTS, 0, NR_PARTICLES);

		static v4 color = {0.5f, 0.75f, 1.0, 1.0f};

		am.fps_frames++;
		am.fps_timer += dt;
		if (am.fps_timer > 2) {
			sprintf(am.fps_string, "Framerate: %.1f", (double)am.fps_frames/(double)am.fps_timer);
			String string = make_string(am.fps_string);
			gui::update_string(am.gui, am.fps_job_handle, string);

			am.fps_frames = 0;
			am.fps_timer = 0;
		}
		gui::begin_render(am.gui);
		gui::render_dynamic(am.gui, am.fps_job_handle, 0, 40);
		gui::render_all_static(am.gui, 0, 0, 1.0f, color);
	}
	return 0;
}

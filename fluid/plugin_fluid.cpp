#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include <string.h>

#define GL_SHARING_EXTENSION "cl_apple_gl_sharing"
#include <OpenGL/gl3.h>
#include <OpenGL/opengl.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>

#pragma OPENCL EXTENSION cl_apple_gl_sharing : enable

#include "engine/plugin.h"
#include "settings.h"

#define USE_INTRINSICS 1
#include "utils/math.h"
#include "utils/parser.cpp"

#define CL_ERROR_CHECKING 1
#define GL_ERROR_CHECKING 1

#include "opencl/cl_program_builder.cpp"
#include "generated/fluid_kernel.generated.cpp"
#include "opencl/cl_manager.cpp" // This is dependent on fluid_kernel.generated.cpp! :o

#include "opengl/gl_program_builder.cpp"
#include "generated/setup_vertex_array.cpp"
#include "opengl/gl_manager.cpp"

#include "utils/profiler.c"
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

//@ reloadable_struct
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
	u16 fps_job_handle;
	bool fps_update;
	int fps_frames;
	float fps_timer;
};
#include "fluid.cpp"
#define TRANSIENT_ARENA_SIZE (MB*2)

EXPORT PLUGIN_RELOAD(reload) {
#if 0
	reload((char*)old_memory, (char*)new_memory);

	generate_layout();

	AppMemory *am = (AppMemory*) new_memory;

	am->shader_program = gl_program_builder::create_from_source_file(SHADER_VERTEX_SOURCE, SHADER_FRAGMENT_SOURCE);
	cl_manager::ClInfo &info = am->info;

	// Since the transient arena is internal memory that won't get reloaded, we need to re-malloc it here.
	am->transient_arena.memory = (char*)malloc(TRANSIENT_ARENA_SIZE);

	cl_manager::create_program_and_kernels(am->transient_arena, info);
	cl_manager::reload_buffers(am->transient_arena, info);
	cl_manager::setup_kernels(info);

	gl_manager::setup_vertex_array(&am->vao, info.buffers);
	gl_manager::setup_uniforms(am->shader_program);

	clear_memory(am->transient_arena);
#endif
}

// logging
EXPORT PLUGIN_UPDATE(update) {

#if 0 // Obj reading
	AppMemory *am = (AppMemory*) memory;
	if (!am->initialized) {
		am->initialized = true;

		am->shader_program = gl_program_builder::create_from_source_file("./shaders/test.vert", "./shaders/test.frag");
		am->data = read_pot();

		glGenBuffers(1, &am->eab);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, am->eab);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, am->data.vertex_index_count * sizeof(unsigned), am->data.vertex_indices, GL_STATIC_DRAW);

		glGenBuffers(1, &am->vbo);
		glBindBuffer(GL_ARRAY_BUFFER, am->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v3)*am->data.vertex_count, am->data.vertices, GL_STATIC_DRAW);


		glGenVertexArrays(1, &am->vao);
		glBindVertexArray(am->vao);


		glBindBuffer(GL_ARRAY_BUFFER, am->vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(12);

	}

	glUseProgram(am->shader_program);

	m4 view = look_at(V3(0.0f, 0.0f, 20.0f), V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
	m4 projection = perspective_fov(100, RES_WIDTH / RES_HEIGHT, 1.0f, 100.0f);
	GLint model_view_location = glGetUniformLocation(am->shader_program, "model_view");
	glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat*)((projection * view).m));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// glUseProgram(am->shader_program);
	glBindVertexArray(am->vao);

	// glDrawArrays(GL_POINTS, 0, 18);
	// glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, am->eab);

	// Draw the triangles!
	glDrawElements(
	    GL_TRIANGLES,      // mode
	    am->data.vertex_index_count,    // count
	    GL_UNSIGNED_INT,   // type
	    (void*)0           // element array buffer offset
	);
#else
	AppMemory *am = (AppMemory*) memory;
	if (!am->initialized) {
		am->initialized = true;
		// generate_layout();

		am->persistent_arena = init_from_existing((char*)memory + sizeof(AppMemory), MB*8);
		am->transient_arena = init_memory(TRANSIENT_ARENA_SIZE); // This internal memory of the dll, it won't get reloaded.

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(12);

		am->shader_program = gl_program_builder::create_from_source_files(am->transient_arena, SHADER_VERTEX_SOURCE, SHADER_FRAGMENT_SOURCE, 0);
		am->info = cl_manager::init();
		cl_manager::ClInfo &info = am->info;
		cl_manager::create_program_and_kernels(am->transient_arena, info);
		cl_manager::setup_buffers(info);
		cl_manager::setup_kernels(info);

		gl_manager::setup_vertex_array(&am->vao, info.buffers);
		gl_manager::setup_uniforms(am->shader_program);

		clear_memory(am->transient_arena);

		gui::GUISettings settings = {};
		settings.font_path = "../assets/font.gamefont";
		settings.text_vertex_shader = "../shaders/text.vert";
		settings.text_fragment_shader = "../shaders/text.frag";

		gui::init(am->gui, am->persistent_arena, am->transient_arena, settings);

		am->fps_string = allocate_memory(am->persistent_arena, 32);
		sprintf(am->fps_string, "Framerate: N/A");
		String string = make_string(am->fps_string);
		am->fps_job_handle = gui::create_string(am->gui, true, string);

		if (!gl_program_builder::validate_program(am->shader_program))
			return -1;
	}

	run_fluid(am);

	{ // RENDER
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(am->shader_program);
		glBindVertexArray(am->vao);
		glDrawArrays(GL_POINTS, 0, NR_PARTICLES);

		static v4 color = {0.5f, 0.75f, 1.0, 1.0f};

		am->fps_frames++;
		am->fps_timer += dt;
		if (am->fps_timer > 2) {
			sprintf(am->fps_string, "Framerate: %.1f", am->fps_frames/am->fps_timer);
			String string = make_string(am->fps_string);
			gui::update_string(am->gui, am->fps_job_handle, string);

			am->fps_frames = 0;
			am->fps_timer = 0;
		}
		gui::begin_render(am->gui);
		gui::render_dynamic(am->gui, am->fps_job_handle, 0, 40);
		gui::render_all_static(am->gui, 0, 0, 1.0f, color);
	}
#endif
	return 0;
}

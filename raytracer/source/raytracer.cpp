#include "includes.h"
#include "levels.cpp"
#include "engine/utils/math/random.h"
#include "engine/utils/profiler.c"
#include "shaders/default.cl_shader.cpp"

struct Game {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;
	Random random;

	EngineApi *engine;

	ClInfo cl_info;
	cl_mem input_entities_mem;
	cl_mem output_mem;
	cl_kernel kernel;

	b32 initialized;

	i32 entity_count;
	Entity entities[512];
};

EXPORT PLUGIN_RELOAD(reload) {
	Game &game = *(Game*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
		setup_cl();
	#endif

	MemoryArena empty = {};
	game.transient_arena = empty;
	reset_arena(game.transient_arena, MB);
	globals::transient_arena = &game.transient_arena;

	reload_programs(game.components);
	setup_render_pipe(game.engine, game.persistent_arena, game.render_pipe, game.components, screen_width, screen_height);
	game.components.input.set_input_data(&input);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];

		Entity *entity = 0;
		if (i < game.entity_count) {
			entity = game.entities + i;
		} else {
			entity = game.entities + game.entity_count++;
		}

		model__set_position(game.components, *entity, data.offset);
		model__set_rotation(game.components, *entity, data.rotation);
		model__set_scale(game.components, *entity, data.size);
	}
	game.entity_count = level.count;
}

ALIGNED_TYPE_(struct, 16) {
	cl_float3 position;
	cl_float3 data;
	cl_float3 emittance_color;
	cl_float3 reflection_color;
	cl_float roughness;
	cl_int type;
} CLEntity;

ALIGNED_TYPE_(struct, 16) {
	cl_uint width;
	cl_uint height;
	cl_float half_width;
	cl_float half_height;
	cl_float one_over_w;
	cl_float one_over_h;
	cl_float half_pix_w;
	cl_float half_pix_h;
	cl_int rays_per_pixel;
} CLRaytracerSettings;

EXPORT PLUGIN_UPDATE(update) {
	Game &game = *(Game*) memory;

	if (!game.initialized) {
		game.initialized = true;

		#ifdef OS_WINDOWS
			setup_gl();
			setup_cl();
		#endif
		MemoryArena empty = {};
		game.persistent_arena = empty;
		game.transient_arena = empty;
		setup_arena(game.transient_arena, MB);
		globals::transient_arena = &game.transient_arena;

		game.engine = &engine;
		game.random = {};
		random_init(game.random, rdtsc(), 54u);

		Level level = make_level();
		for (i32 i = 0; i < level.count; ++i) {
			EntityData &data = level.entity_data[i];
			Entity &entity = game.entities[game.entity_count++];

			spawn_entity(game.components, entity, data.type, data.context, data.offset);

			model__set_position(game.components, entity, data.offset);
			model__set_rotation(game.components, entity, data.rotation);
			model__set_scale(game.components, entity, data.size);
		}

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		ClInfo cl_info = init_opencl(game.transient_arena);
		cl_program program = cl_program_builder::create_from_strings(game.transient_arena, cl_info.context, cl_shaders::trace_ray, 1, &cl_info.device, 0);

		cl_int errcode_ret;

		cl_kernel kernel = clCreateKernel(program, "cast_rays", &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateKernel, errcode_ret);

		setup_programs(game.components);
		setup_render_pipe(game.engine, game.persistent_arena, game.render_pipe, game.components, screen_width, screen_height);

		cl_uint input_entity_count = 0;

		cl_mem input_entities_mem;
		{
			CLEntity entities[16] = {};

			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, -100, 0 };
				plane->data = { 0, 1, 0 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, -400, 0 };
				plane->data = { 1, 0, 0 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, -500, 0 };
				plane->data = { -1, 0, 0.4f };
				plane->reflection_color = { 1, 1, 1 };
				plane->roughness = 0.01f;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, -400, 0 };
				plane->data = { 0, 0, 1 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, -400, 0 };
				plane->data = { 0, -1, 0 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->emittance_color = { 1.0f, 1.0f, 1.0f };
				plane->roughness = 1;
			}

			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 1;
				sphere->position = { 0, -100, 0 };
				sphere->data = { 70, 50, 50 };
				sphere->reflection_color = { 0.7f, 0.5f, 0.3f };
				sphere->roughness = 1.0f;
			}
			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 1;
				sphere->position = { 200, 200, -100 };
				sphere->data = { 30, 50, 50 };
				sphere->reflection_color = { 0.5f, 0.7f, 0.3f };
				sphere->roughness = 0.1f;
			}
			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 1;
				sphere->position = { 200, -50, 0 };
				sphere->data = { 50, 50, 50 };
				sphere->reflection_color = { 1, 1, 1 };
				sphere->roughness = 0.01f;
			}
			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 1;
				sphere->position = { -300, -40, 0 };
				sphere->data = { 30, 50, 50 };
				// sphere->reflection_color = { 1, 1, 1 };
				sphere->emittance_color = { 0.0f, 0.0f, 40.0f };
				sphere->roughness = 1.0f;
			}
			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 2;
				sphere->position = { -260, 200, 0 };
				sphere->data = { 50, 50, 50 };
				sphere->reflection_color = { 1, 1, 1 };
				sphere->roughness = 1.0f;
			}
			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 2;
				sphere->position = { -260, -60, 100 };
				sphere->data = { 30, 50, 50 };
				sphere->reflection_color = { 1, 1, 1 };
				sphere->roughness = 1.0f;
			}
			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 1;
				sphere->position = { -60, 0, -50 };
				sphere->data = { 40, 50, 50 };
				sphere->reflection_color = { 1, 0, 0 };
				sphere->roughness = 0.75f;
			}


			input_entities_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(CLEntity) * ARRAY_COUNT(entities), entities, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);
		}
		game.input_entities_mem = input_entities_mem;

		cl_mem input_settings_mem;
		{
			CLRaytracerSettings settings = {};

			settings.width = 1024;
			settings.height = 768;
			settings.half_width = settings.width * 0.5f;
			settings.half_height = settings.height * 0.5f;
			settings.one_over_w = 1.0f / settings.width;
			settings.one_over_h = 1.0f / settings.height;
			settings.half_pix_w = 0.5f * settings.one_over_w;
			settings.half_pix_h = 0.5f * settings.one_over_h;
			settings.rays_per_pixel = 32;

			input_settings_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(CLRaytracerSettings), &settings, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);
		}

		cl_mem accumulation_mem;
		{
			void *mem = PUSH_SIZE(game.persistent_arena, sizeof(cl_float3) * 1024 * 768, true);
			accumulation_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float3) * 1024 * 768, mem, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateFromGLTexture, errcode_ret);
		}

		cl_mem output_mem;
		{
			output_mem = clCreateFromGLTexture(cl_info.context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, game.render_pipe.ray_texture, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateFromGLTexture, errcode_ret);
		}

		errcode_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &input_entities_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);


		errcode_ret = clSetKernelArg(kernel, 1, sizeof(cl_uint), (void *) &input_entity_count);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &input_settings_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *) &accumulation_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *) &output_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		game.cl_info = cl_info;
		game.output_mem = output_mem;
		game.kernel = kernel;

		game.components.input.set_input_data(&input);

		//	// Audio
		//	game.audio_manager.play(game.engine, "../../game/assets/test.wav");
		setup_camera(game.camera, V3(0, 0, 500), ASPECT_RATIO);
	}

	{ // Update the game
		// Update all the components
		update_components(game.components, dt);
		// Handle component/component communication.
		component_glue::update(game.components, game.entities, game.entity_count, dt);
		// Update sound
		game.audio_manager.update(*globals::transient_arena, game.engine, dt);
	}

	{ // Setup opencl
		u64 workGroupSize[] = { 1024, 768 };
		cl_int errcode_ret;
		cl_event event;

		// {
		// 	static float time = 0;
		// 	time += dt;

		// 	CLEntity sphere = {};
		// 	sphere.type = 1;
		// 	sphere.position = { 100 + sinf(time) * 100, 50 + cosf(time) * 100, cosf(time) * 100 };
		// 	sphere.data = { 50, 50, 50 };
		// 	sphere.reflection_color = { 0.2f, 0.8f, 0.2f };
		// 	sphere.roughness = 1.0f;

		// 	size_t offset = 2 * sizeof(CLEntity);
		// 	errcode_ret = clEnqueueWriteBuffer(game.cl_info.command_queue, game.input_entities_mem, true, offset, sizeof(CLEntity), (void*)(&sphere), 0, 0, 0);
		// 	CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);
		// }

		u64 time = rdtsc();
		errcode_ret = clSetKernelArg(game.kernel, 3, sizeof(u64), (void *) &time);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		static cl_ulong framecounter = 0;
		framecounter++;

		errcode_ret = clSetKernelArg(game.kernel, 4, sizeof(u64), (void *) &framecounter);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(game.kernel, 5, sizeof(cl_float3), (void *) &game.camera.position);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		glFinish();
		clEnqueueAcquireGLObjects(game.cl_info.command_queue, 1, &game.output_mem, 0, 0, 0);

		errcode_ret = clEnqueueNDRangeKernel(game.cl_info.command_queue, game.kernel, ARRAY_COUNT(workGroupSize), 0, workGroupSize, 0, 0, 0, &event);
		CL_CHECK_ERRORCODE(clEnqueueNDRangeKernel, errcode_ret);

		// clWaitForEvents(1, &event);

		clFinish(game.cl_info.command_queue);
		clEnqueueReleaseGLObjects(game.cl_info.command_queue, 1, &game.output_mem, 0, 0, 0);
	}

	{ // Render
		render(game.render_pipe, game.components, game.camera);
	}

	reset_transient_memory(*globals::transient_arena);
	return 0;
}

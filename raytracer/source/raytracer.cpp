#include "includes.h"

struct Application {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;
	Random random;

	EngineApi *engine;

	ClInfo cl_info;
	cl_mem input_entity_geometry_mem;
	cl_mem input_entity_material_mem;
	cl_mem output_mem;
	cl_kernel kernel;

	b32 initialized;

	i32 entity_count;
	Entity entities[512];
};

EXPORT PLUGIN_RELOAD(reload) {
	Application &application = *(Application*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
		setup_cl();
	#endif

	MemoryArena empty = {};
	application.transient_arena = empty;
	reset_arena(application.transient_arena, MB);
	globals::transient_arena = &application.transient_arena;

	reload_programs(application.components);
	setup_render_pipe(application.engine, application.persistent_arena, application.render_pipe, application.components, screen_width, screen_height);
	application.components.input.set_input_data(&input);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];

		Entity *entity = 0;
		if (i < application.entity_count) {
			entity = application.entities + i;
		} else {
			entity = application.entities + application.entity_count++;
		}

		model__set_position(application.components, *entity, data.offset);
		model__set_rotation(application.components, *entity, data.rotation);
		model__set_scale(application.components, *entity, data.size);
	}
	application.entity_count = level.count;
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
	cl_float3 position;
	cl_float3 data;
	cl_int type;
} CLEntityGeometry;

ALIGNED_TYPE_(struct, 16) {
	cl_float3 emittance_color;
	cl_float3 reflection_color;
	cl_float roughness;
} CLEntityMaterial;

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
	Application &application = *(Application*) memory;

	if (!application.initialized) {
		application.initialized = true;

		#ifdef OS_WINDOWS
			setup_gl();
			setup_cl();
		#endif
		MemoryArena empty = {};
		application.persistent_arena = empty;
		application.transient_arena = empty;
		setup_arena(application.transient_arena, MB);
		globals::transient_arena = &application.transient_arena;

		application.components.arena = &application.persistent_arena;

		application.engine = &engine;
		application.random = {};
		random_init(application.random, rdtsc(), 54u);

		Level level = make_level();
		for (i32 i = 0; i < level.count; ++i) {
			EntityData &data = level.entity_data[i];
			Entity &entity = application.entities[application.entity_count++];

			spawn_entity(application.engine, application.components, entity, data.type, data.context, data.offset);

			model__set_position(application.components, entity, data.offset);
			model__set_rotation(application.components, entity, data.rotation);
			model__set_scale(application.components, entity, data.size);
		}

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		ClInfo cl_info = init_opencl(application.transient_arena);
		// -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations -cl-finite-math-only -cl-single-precision-constant -cl-denorms-are-zero -cl-strict-aliasing
		char *build_options = "-cl-fast-relaxed-math";
		cl_program program = cl_program_builder::create_from_strings(application.transient_arena, cl_info.context, cl_shaders::trace_ray, 1, &cl_info.device, build_options);

		cl_int errcode_ret;

		cl_kernel kernel = clCreateKernel(program, "cast_rays", &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateKernel, errcode_ret);

		setup_programs(application.components);
		setup_render_pipe(application.engine, application.persistent_arena, application.render_pipe, application.components, screen_width, screen_height);

		cl_uint input_entity_count = 0;

		cl_mem input_entity_geometry_mem;
		cl_mem input_entity_material_mem;
		{
			CLEntity entities[16] = {};

			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, 100, 0 };
				plane->data = { 0, 1, 0 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, 400, 0 };
				plane->data = { 1, 0, 0 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, 500, 0 };
				plane->data = { -1, 0, 0.4f };
				plane->reflection_color = { 1, 1, 1 };
				plane->roughness = 0.01f;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, 400, 0 };
				plane->data = { 0, 0, 1 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 0;
				plane->position = { 0, 200, 0 };
				plane->data = { 0, -1, 0 };
				plane->reflection_color = { 0.5f, 0.5f, 0.5f };
				plane->roughness = 1;
			}
			{
				CLEntity *plane = entities + input_entity_count++;
				plane->type = 3;
				plane->position = { 0, 199, 0 };
				plane->data = { 0, -1, 0 };
				plane->emittance_color = { 10.0f, 10.0f, 3.0f };
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
				sphere->type = 1;
				sphere->position = { -60, 0, -50 };
				sphere->data = { 40, 50, 50 };
				sphere->reflection_color = { 1, 0, 0 };
				sphere->roughness = 0.75f;
			}

			{
				CLEntity *sphere = entities + input_entity_count++;
				sphere->type = 4;
				sphere->position = { 100, 40, 200 };
				sphere->data = { 30, 50, 50 };
				sphere->reflection_color = { 0.1f, 0.1f, 0.8f };
				sphere->roughness = 1.0f;
			}

			CLEntityGeometry *geometry = PUSH_STRUCTS(application.transient_arena, input_entity_count, CLEntityGeometry);
			for (u32 gi = 0; gi < input_entity_count; ++gi) {
				geometry[gi].position = entities[gi].position;
				geometry[gi].data = entities[gi].data;
				geometry[gi].type = entities[gi].type;
			}

			CLEntityMaterial *material = PUSH_STRUCTS(application.transient_arena, input_entity_count, CLEntityMaterial);
			for (u32 gi = 0; gi < input_entity_count; ++gi) {
				material[gi].emittance_color = entities[gi].emittance_color;
				material[gi].reflection_color = entities[gi].reflection_color;
				material[gi].roughness = entities[gi].roughness;
			}

			input_entity_geometry_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(CLEntityGeometry) * input_entity_count, geometry, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);

			input_entity_material_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(CLEntityMaterial) * input_entity_count, material, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);
		}
		application.input_entity_geometry_mem = input_entity_geometry_mem;
		application.input_entity_material_mem = input_entity_material_mem;

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
			settings.rays_per_pixel = 8;

			input_settings_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(CLRaytracerSettings), &settings, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);
		}

		cl_mem accumulation_mem;
		{
			void *mem = PUSH_SIZE(application.persistent_arena, sizeof(cl_float3) * 1024 * 768, true);
			accumulation_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float3) * 1024 * 768, mem, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateFromGLTexture, errcode_ret);
		}

		cl_mem output_mem;
		{
			output_mem = clCreateFromGLTexture(cl_info.context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, application.render_pipe.ray_texture, &errcode_ret);
			CL_CHECK_ERRORCODE(clCreateFromGLTexture, errcode_ret);
		}

		errcode_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &input_entity_geometry_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &input_entity_material_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);


		errcode_ret = clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &input_entity_count);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *) &input_settings_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *) &accumulation_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void *) &output_mem);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		application.cl_info = cl_info;
		application.output_mem = output_mem;
		application.kernel = kernel;

		application.components.input.set_input_data(&input);

		//	// Audio
		//	application.audio_manager.play(application.engine, "../../application/assets/test.wav");
		setup_camera(application.camera, V3(0, 0, 500), ASPECT_RATIO);
	}

	{ // Update the application
		// Update all the components
		update_components(application.components, dt);
		// Handle component/component communication.
		component_glue::update(application.components, application.entities, application.entity_count, dt);
		// Update sound
		application.audio_manager.update(*globals::transient_arena, application.engine, dt);
	}

	{
		v2 m = {};
		if (IS_HELD(input, InputKey_A)) {
			m.x = -1;
		}
		if (IS_HELD(input, InputKey_S)) {
			m.y = -1;
		}
		if (IS_HELD(input, InputKey_D)) {
			m.x = 1;
		}
		if (IS_HELD(input, InputKey_W)) {
			m.y = 1;
		}

		float camera_speed = 128.0f;
		float camera_rotation_speed = 0.5f;

		if (IS_HELD(input, InputKey_MouseRight)) {
			camera_speed *= 8;
		}

		m4 &pose = application.camera.pose;

		v3 &x = *(v3*)(pose.m + 0);
		v3 &y = *(v3*)(pose.m + 4);
		v3 &z = *(v3*)(pose.m + 8);
		v3 &position = translation(pose);

		position += x * (m.x * dt * camera_speed);
		position += z * (m.y * dt * camera_speed);

		if (IS_HELD(input, InputKey_MouseLeft)) {
			v3 world_up = V3(0, 1, 0);
			q4 qx = Quaternion(world_up, -input.mouse_xrel * dt * camera_rotation_speed);
			q4 qy = Quaternion(x, -input.mouse_yrel * dt * camera_rotation_speed);
			q4 q = qx * qy;

			x = ::rotate_around(q, x);
			y = ::rotate_around(q, y);
			z = ::rotate_around(q, z);
		}

		update_view(application.camera);
	}

	{ // Setup opencl
		u64 work_group_size[] = { 1024, 768 };
		u64 *local_group_size = 0; // [] = { 48, 48 };
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
		// 	errcode_ret = clEnqueueWriteBuffer(application.cl_info.command_queue, application.input_entities_mem, true, offset, sizeof(CLEntity), (void*)(&sphere), 0, 0, 0);
		// 	CL_CHECK_ERRORCODE(clCreateBuffer, errcode_ret);
		// }

		u64 time = rdtsc();
		errcode_ret = clSetKernelArg(application.kernel, 4, sizeof(u64), (void *) &time);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		static cl_ulong framecounter = 0;
		framecounter++;

		errcode_ret = clSetKernelArg(application.kernel, 5, sizeof(u64), (void *) &framecounter);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		v3 camera_position = translation(application.camera.pose);
		errcode_ret = clSetKernelArg(application.kernel, 6, sizeof(cl_float3), (void *) &camera_position);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		glFinish();
		clEnqueueAcquireGLObjects(application.cl_info.command_queue, 1, &application.output_mem, 0, 0, 0);

		errcode_ret = clEnqueueNDRangeKernel(application.cl_info.command_queue, application.kernel, ARRAY_COUNT(work_group_size), 0, work_group_size, local_group_size, 0, 0, &event);
		CL_CHECK_ERRORCODE(clEnqueueNDRangeKernel, errcode_ret);

		// clWaitForEvents(1, &event);

		clFinish(application.cl_info.command_queue);
		clEnqueueReleaseGLObjects(application.cl_info.command_queue, 1, &application.output_mem, 0, 0, 0);
	}

	{ // Render
		render(application.render_pipe, application.components, application.camera);
	}

	reset_transient_memory(*globals::transient_arena);
	return 0;
}

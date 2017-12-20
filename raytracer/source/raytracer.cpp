#define CL_ERROR_CHECKING

struct Raytracer;
#define RELOAD_ENTRY_POINT Raytracer
#define SYSTEM_OPENGL
#define SYSTEM_OPENCL
#define SYSTEM_GRAPHICS
#define SYSTEM_COMPONENTS
#include "engine/systems.h"

#include "shaders/default.cl_shader.cpp"
#include "render_pipe.cpp"
#include "levels.cpp"

struct Raytracer {
	int a;
	RenderPipe render_pipe;
	Random random;

	cl_mem input_entity_geometry_mem;
	cl_mem input_entity_material_mem;
	cl_mem accumulation_mem;
	cl_mem output_mem;
	cl_kernel kernel;
};

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

ALIGNED_TYPE_(struct, 16) {
	cl_float4 x;
	cl_float4 y;
	cl_float4 z;
	cl_float4 w;
} CLM4;

void plugin_reloaded(Application &application) {
	// Raytracer &raytracer = *(Raytracer*)application.user_data;

	// i32 screen_width, screen_height;
	// application.engine->screen_dimensions(screen_width, screen_height);
	// setup_render_pipe(application.persistent_arena, application.engine, raytracer.render_pipe, application.components, screen_width, screen_height);
}

void plugin_setup(Application &application) {
	Allocator &allocator = application.allocator;
	application.user_data = (Raytracer*)allocator.allocate(sizeof(Raytracer), 4);
	Raytracer &raytracer = *application.user_data;

	// Raytracer &raytracer = *PUSH_STRUCT(application.persistent_arena, Raytracer);
	// application.user_data = &raytracer;

	raytracer.random = {};
	random_init(raytracer.random, rdtsc(), 54u);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];
		Entity *entity = spawn_entity(application.engine, application.components, data.type, data.context, data.offset);

		model__set_position(application.components, *entity, data.offset);
		model__set_rotation(application.components, *entity, data.rotation);
		model__set_scale(application.components, *entity, data.size);
	}

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);

	ClInfo &cl_info = application.cl_info;
	// -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations -cl-finite-math-only -cl-single-precision-constant -cl-denorms-are-zero -cl-strict-aliasing
	char *build_options = "-cl-fast-relaxed-math";
	cl_program program = cl_program_builder::create_from_strings(application.transient_arena, cl_info.context, cl_shaders::trace_ray, 1, &cl_info.device, build_options);

	cl_int errcode_ret;

	cl_kernel kernel = clCreateKernel(program, "cast_rays", &errcode_ret);
	CL_CHECK_ERRORCODE(clCreateKernel, errcode_ret);

	i32 screen_width, screen_height;
	application.engine->screen_dimensions(screen_width, screen_height);
	setup_render_pipe(application.persistent_arena, application.engine, raytracer.render_pipe, application.components, screen_width, screen_height);

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
	raytracer.input_entity_geometry_mem = input_entity_geometry_mem;
	raytracer.input_entity_material_mem = input_entity_material_mem;

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
		void *mem = PUSH_SIZE(application.transient_arena, sizeof(cl_float3) * 1024 * 768, true);
		accumulation_mem = clCreateBuffer(cl_info.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float3) * 1024 * 768, mem, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateFromGLTexture, errcode_ret);
	}
	raytracer.accumulation_mem = accumulation_mem;

	cl_mem output_mem;
	{
		output_mem = clCreateFromGLTexture(cl_info.context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, raytracer.render_pipe.ray_texture, &errcode_ret);
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

	raytracer.output_mem = output_mem;
	raytracer.kernel = kernel;
}

void plugin_update(Application &application, float dt) {
	Raytracer &raytracer = *(Raytracer*)application.user_data;

	static cl_ulong framecounter = 0;
	framecounter++;

	{
		InputData &input = *application.components.input.input_data;
		float translation_speed = 128.0f;
		float rotation_speed = 0.1f;
		bool moved = move(application.camera, input, translation_speed, rotation_speed, dt);

		if (moved) {
			size_t size = sizeof(cl_float3) * 1024 * 768;
			void *mem = PUSH_SIZE(application.transient_arena, size, true);
			cl_int errcode_ret = clEnqueueWriteBuffer(application.cl_info.command_queue, raytracer.accumulation_mem, true, 0, size, mem, 0, 0, 0);
			(void) errcode_ret;
			// CL_CHECK_ERRORCODE(clEnqueueWriteBuffer, errcode_ret);

			framecounter = 0;
		}
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
		errcode_ret = clSetKernelArg(raytracer.kernel, 4, sizeof(u64), (void *) &time);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		errcode_ret = clSetKernelArg(raytracer.kernel, 5, sizeof(u64), (void *) &framecounter);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		CLM4 clm = *(CLM4*)&application.camera.pose;

		errcode_ret = clSetKernelArg(raytracer.kernel, 6, sizeof(CLM4), (void *) &clm);
		CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

		glFinish();
		clEnqueueAcquireGLObjects(application.cl_info.command_queue, 1, &raytracer.output_mem, 0, 0, 0);

		errcode_ret = clEnqueueNDRangeKernel(application.cl_info.command_queue, raytracer.kernel, ARRAY_COUNT(work_group_size), 0, work_group_size, local_group_size, 0, 0, &event);
		CL_CHECK_ERRORCODE(clEnqueueNDRangeKernel, errcode_ret);

		// clWaitForEvents(1, &event);

		clFinish(application.cl_info.command_queue);
		clEnqueueReleaseGLObjects(application.cl_info.command_queue, 1, &raytracer.output_mem, 0, 0, 0);
	}
}

void plugin_render(Application &application) {
	Raytracer &raytracer = *(Raytracer*)application.user_data;
	render(raytracer.render_pipe, application.components, application.camera);
}
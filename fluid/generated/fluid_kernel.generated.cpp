// Exported lines
#include "../source/buffer.cpp"

namespace cl_manager {
struct SimKernels {
	cl_kernel calc_density_pressure;
	cl_kernel calc_acceleration;
};
struct ClInfo {
	cl_context context;
	cl_command_queue command_queue;
	cl_device_id device;
	cl_program program;
	SimKernels kernels;
	Buffer buffers[32];
	u64 nr_particles;
};

void reload_buffer(ArenaAllocator &arena, ClInfo &info, BufferIndex index, v2 (*f)(u64 i)) {
	cl_context context = info.context;
	cl_int errcode_ret;

	Buffer &buffer = info.buffers[index];

	u64 old_size = sizeof(v2)*info.nr_particles;
	u64 new_size = sizeof(v2) * NR_PARTICLES;

	v2 *buffer_content = PUSH_STRUCTS(arena, NR_PARTICLES, v2);
	errcode_ret = clEnqueueReadBuffer(info.command_queue, buffer.mem, CL_TRUE, 0, (new_size < old_size ? new_size : old_size), buffer_content, 0, 0, 0);

	for (u64 i = info.nr_particles; i < NR_PARTICLES; ++i) {
		buffer_content[i] = f(i);
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) new_size, buffer_content, GL_DYNAMIC_DRAW);

	errcode_ret = clReleaseMemObject(buffer.mem);
	CL_CHECK_ERRORCODE(clReleaseMemObject, errcode_ret);

	buffer.mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, buffer.vbo, &errcode_ret);
	CL_CHECK_ERRORCODE(clCreateFromGLBuffer, errcode_ret);
}
void reload_buffers(ArenaAllocator &arena, ClInfo &info) {
	reload_buffer(arena, info, BufferIndex__density_pressure, zero);
	reload_buffer(arena, info, BufferIndex__accelerations, zero);
	reload_buffer(arena, info, BufferIndex__velocities, zero);
	reload_buffer(arena, info, BufferIndex__positions, gen_random_pos);

	info.nr_particles = NR_PARTICLES;

	ASSERT(false, "Need to clear memory?");

	// clear_memory(arena);
}

void setup_kernels(ClInfo &info) {
	cl_manager::SimKernels kernels = info.kernels;
	cl_int errcode_ret;

	Buffer density_pressure = info.buffers[BufferIndex__density_pressure];
	Buffer accelerations = info.buffers[BufferIndex__accelerations];
	Buffer velocities = info.buffers[BufferIndex__velocities];
	Buffer positions = info.buffers[BufferIndex__positions];

	errcode_ret = clSetKernelArg(kernels.calc_density_pressure, 0, sizeof(cl_mem), (void *) &density_pressure.mem);
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

	errcode_ret = clSetKernelArg(kernels.calc_density_pressure, 1, sizeof(cl_mem), (void *) &positions.mem);
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

	errcode_ret = clSetKernelArg(kernels.calc_acceleration, 0, sizeof(cl_mem), (void *) &density_pressure.mem);
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

	errcode_ret = clSetKernelArg(kernels.calc_acceleration, 1, sizeof(cl_mem), (void *) &accelerations.mem);
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

	errcode_ret = clSetKernelArg(kernels.calc_acceleration, 2, sizeof(cl_mem), (void *) &velocities.mem);
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

	errcode_ret = clSetKernelArg(kernels.calc_acceleration, 3, sizeof(cl_mem), (void *) &positions.mem);
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);

	info.nr_particles = NR_PARTICLES;
}

void run_kernels(ClInfo &info) {
	u64 workGroupSize[] = { NR_PARTICLES };
	cl_int errcode_ret;
	cl_event event;

	cl_manager::SimKernels kernels = info.kernels;
	errcode_ret = clEnqueueNDRangeKernel(info.command_queue, kernels.calc_density_pressure, 1, 0, workGroupSize, 0, 0, 0, &event);
	CL_CHECK_ERRORCODE(clEnqueueNDRangeKernel, errcode_ret);

	errcode_ret = clEnqueueNDRangeKernel(info.command_queue, kernels.calc_acceleration, 1, 0, workGroupSize, 0, 0, 0, &event);
	CL_CHECK_ERRORCODE(clEnqueueNDRangeKernel, errcode_ret);

	clFinish(info.command_queue);
}
void create_program_and_kernels(ArenaAllocator &arena, ClInfo &info) {
	cl_int errcode_ret;
	cl_program program = cl_program_builder::create_from_source_file(arena, info.context, COMPUTE_SHADER_SOURCE, 1, &info.device, "-I "COMPUTE_SHADER_INCLUDE);
	SimKernels kernels = { };

	cl_kernel calc_density_pressure = clCreateKernel(program, "calc_density_pressure", &errcode_ret);
	CL_CHECK_ERRORCODE(clCreateKernel, errcode_ret);
	kernels.calc_density_pressure = calc_density_pressure;

	cl_kernel calc_acceleration = clCreateKernel(program, "calc_acceleration", &errcode_ret);
	CL_CHECK_ERRORCODE(clCreateKernel, errcode_ret);
	kernels.calc_acceleration = calc_acceleration;

	info.program = program;
	info.kernels = kernels;
}
}

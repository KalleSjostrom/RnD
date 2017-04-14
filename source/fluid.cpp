#include "fluid_common.cpp"

#define SPOOK_PTHREAD 1

#if defined(MULLER)
#include "fluid_muller.cpp"
#elif defined(SPOOK)
#include "fluid_spook.cpp"
#elif defined(SPOOK_PTHREAD)
#include "fluid_spook_pthread.cpp"
#elif defined(MPM)
#include "fluid_mpm.cpp"
#endif

void run_fluid(AppMemory *am) {
	cl_manager::ClInfo &info = am->info;

#if defined(MULLER_CL)
	cl_manager::run_kernels(info);
#else
	glBindBuffer(GL_ARRAY_BUFFER, info.buffers[BufferIndex__density_pressure].vbo);
	v2 *gpu_density_pressure = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	glBindBuffer(GL_ARRAY_BUFFER, info.buffers[BufferIndex__velocities].vbo);
	v2 *gpu_velocities = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	glBindBuffer(GL_ARRAY_BUFFER, info.buffers[BufferIndex__positions].vbo);
	v2 *gpu_positions = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	v2 density_pressure[NR_PARTICLES] = {};
	v2 velocities[NR_PARTICLES];
	v2 positions[NR_PARTICLES];

	// memcpy(density_pressure, gpu_density_pressure, NR_PARTICLES * sizeof(v2));
	memcpy(velocities, gpu_velocities, NR_PARTICLES * sizeof(v2));
	memcpy(positions, gpu_positions, NR_PARTICLES * sizeof(v2));

	PROFILER_START(fluid_simulate);
	fluid::simulate(positions, velocities, density_pressure);
	PROFILER_STOP(fluid_simulate);

	memcpy(gpu_density_pressure, density_pressure, NR_PARTICLES * sizeof(v2));
	memcpy(gpu_velocities, velocities, NR_PARTICLES * sizeof(v2));
	memcpy(gpu_positions, positions, NR_PARTICLES * sizeof(v2));

	static u32 counter = 0;
	if (++counter >= 200) {
		counter = 0;
		printf("\n");

#if defined(MULLER)
		PROFILER_PRINT(fluid_simulate);
		PROFILER_PRINT(memset);
		PROFILER_PRINT(hashmap_insert);
		PROFILER_PRINT(narrowphase);
		PROFILER_PRINT(acceleration);
		PROFILER_PRINT(integrate);

		PROFILER_RESET(fluid_simulate);
		PROFILER_RESET(memset);
		PROFILER_RESET(hashmap_insert);
		PROFILER_RESET(narrowphase);
		PROFILER_RESET(acceleration);
		PROFILER_RESET(integrate);
#elif defined(SPOOK) || defined(SPOOK_PTHREAD)
		PROFILER_PRINT(fluid_simulate);
		PROFILER_PRINT(memset);
		PROFILER_PRINT(hashmap_insert);
		PROFILER_PRINT(narrowphase);
		PROFILER_PRINT(gauss_seidel_init);
		PROFILER_PRINT(gauss_seidel_iter);
		PROFILER_PRINT(integrate);

		PROFILER_RESET(fluid_simulate);
		PROFILER_RESET(memset);
		PROFILER_RESET(hashmap_insert);
		PROFILER_RESET(narrowphase);
		PROFILER_RESET(gauss_seidel_init);
		PROFILER_RESET(gauss_seidel_iter);
		PROFILER_RESET(integrate);
#endif
	}

	glBindBuffer(GL_ARRAY_BUFFER, info.buffers[BufferIndex__density_pressure].vbo);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, info.buffers[BufferIndex__velocities].vbo);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, info.buffers[BufferIndex__positions].vbo);
	glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
}

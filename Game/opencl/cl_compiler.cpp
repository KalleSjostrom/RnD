#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>

#include "cl_settings.h"

#include "../utils/file_utils.h"
#include "../utils/memory_arena.cpp"
#include "cl_errors.cpp"
#include "cl_program_builder.cpp"

/*CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ACCELERATOR, CL_DEVICE_TYPE_DEFAULT, CL_DEVICE_TYPE_ALL*/
#define NUM_DEVICE_ENTRIES 128
#define VERIFY_BINARIES 1

int main(int argc, char *argv[]) {
	cl_device_id devices[NUM_DEVICE_ENTRIES];
	cl_uint num_devices;

	cl_int errcode_ret = clGetDeviceIDs(0, CL_DEVICE_TYPE_GPU, NUM_DEVICE_ENTRIES, devices, &num_devices);
	CL_CHECK_ERRORCODE(clGetDeviceIDs, errcode_ret);

	num_devices = 1; // NOTE(kalle): I only care about the first GPU for now since I'm doing one share group with one context.
	int device_index = DEVICE_INDEX;

	cl_context context = clCreateContext(0, num_devices, devices + device_index, 0, 0, &errcode_ret);

	char *shader_filename = argv[1];
	char *binary_filename = argv[2];

	char build_options[2048] = {0};
	int counter = 0;
	for (int i = 3; i < argc; ++i) {
		char *arg = argv[i];
		while (*arg != '\0') {
			build_options[counter++] = *arg;
			arg++;
		}
		build_options[counter++] = ' ';
	}

	cl_program program = cl_program_builder::create_from_source_file(context, shader_filename, num_devices, devices + device_index, build_options);

	MemoryArena arena = init_memory(1024*1024*4);
	cl_program_builder::write_to_binary_file(arena, program, binary_filename);

#if VERIFY_BINARIES
	cl_program_builder::create_from_binary_file(arena, context, binary_filename, devices + device_index);
#endif

	free_memory(arena);
}
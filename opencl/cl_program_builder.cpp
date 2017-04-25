#include "utils/file_utils.h"
#include "cl_errors.cpp"

namespace cl_program_builder {
	static void build(cl_program program, size_t num_devices, cl_device_id *devices, const char *build_options = 0) {
		cl_int errcode_ret = clBuildProgram(program, num_devices, devices, build_options, 0, 0);
		CL_CHECK_ERRORCODE(clBuildProgram, errcode_ret);

		for (int i = 0; i < num_devices; ++i) {
			cl_build_status build_status;
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, 0);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
			switch (build_status) {
				case CL_BUILD_NONE: printf("No build.\n"); break;
				case CL_BUILD_ERROR: printf("Build error.\n"); break;
				case CL_BUILD_SUCCESS: printf("Build successful.\n"); break;
				case CL_BUILD_IN_PROGRESS: printf("Build in progress.\n"); break;
			}

			size_t ret_val_size;
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0, 0, &ret_val_size);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
			char build_log[ret_val_size+1];
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, 0);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
			build_log[ret_val_size] = '\0';
			printf("CL build log:\n%s\n", build_log);
		}
	}

	cl_program create_from_source_file(cl_context context, const char *filename, size_t num_devices, cl_device_id *devices, const char *build_options = 0) {
		size_t size;
		FILE *file = open_file(filename, &size);
		char source_buffer[size];
		fread(source_buffer, 1, size, file);
		fclose(file);
		source_buffer[size] = '\0';

		cl_int errcode_ret;
		const char *code[] = { source_buffer };
		cl_program program = clCreateProgramWithSource(context, 1, code, &size, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithSource, errcode_ret);

		build(program, num_devices, devices, build_options);

		return program;
	}

	cl_program create_from_binary_file(MemoryArena &arena, cl_context context, const char *filename, cl_device_id *devices) {
		FILE *file = fopen(filename, "r");

		cl_uint count;
		fread(&count, sizeof(cl_uint), 1, file);

		size_t *binary_sizes = (size_t *)allocate_memory(arena, count * sizeof(size_t));
		fread(binary_sizes, sizeof(size_t), count, file);

		unsigned char **binaries;
		binaries = (unsigned char **)allocate_memory(arena, count * sizeof(unsigned char *));
		for (int i = 0; i < count; ++i) {
			binaries[i] = (unsigned char *)allocate_memory(arena, binary_sizes[i] * sizeof(unsigned char));

			fread(binaries[i], sizeof(unsigned char), binary_sizes[i], file);
		}
		fclose(file);

		cl_int errcode_ret;
		cl_int binary_status[16];
		cl_program program = clCreateProgramWithBinary(context, count, devices, binary_sizes, (const unsigned char **) binaries, (cl_int*)binary_status, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithBinary, errcode_ret);

		build(program, count, devices);
		return program;
	}

	void write_to_binary_file(MemoryArena &arena, cl_program program, const char *filename) {
		cl_uint num_devices;
		cl_int errcode_ret = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		size_t *binary_sizes = (size_t *)allocate_memory(arena, num_devices*sizeof(size_t));
		errcode_ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, num_devices*sizeof(size_t), binary_sizes, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		unsigned char **binaries = (unsigned char **)allocate_memory(arena, num_devices * sizeof(unsigned char*));
		size_t total = 0;
		for (int i = 0; i < num_devices; ++i) {
			binaries[i] = (unsigned char *)allocate_memory(arena, binary_sizes[i] * sizeof(unsigned char));
			total += binary_sizes[i];
		}

		errcode_ret = clGetProgramInfo(program, CL_PROGRAM_BINARIES, total*sizeof(unsigned char *), binaries, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		{ // Write file
			FILE *file = fopen(filename, "w");
			fwrite(&num_devices, sizeof(cl_uint), 1, file);
			fwrite(binary_sizes, sizeof(size_t), num_devices, file);

			for (int i = 0; i < num_devices; ++i) {
				fwrite(binaries[i], sizeof(unsigned char), binary_sizes[i], file);
			}

			fclose(file);
		}
	}
}

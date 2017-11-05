#include "engine/utils/file_utils.h"
#include "cl_errors.cpp"

namespace cl_program_builder {
	static void build(MemoryArena &arena, cl_program program, u32 num_devices, cl_device_id *devices, const char *build_options = 0) {
		cl_int errcode_ret = clBuildProgram(program, num_devices, devices, build_options, 0, 0);
		CL_CHECK_ERRORCODE(clBuildProgram, errcode_ret);

		for (u32 i = 0; i < num_devices; ++i) {
			cl_build_status build_status;
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, 0);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
			switch (build_status) {
				case CL_BUILD_NONE: LOG_INFO("OpenCL", "No build.\n"); break;
				case CL_BUILD_ERROR: LOG_INFO("OpenCL", "Build error.\n"); break;
				case CL_BUILD_SUCCESS: LOG_INFO("OpenCL", "Build successful.\n"); break;
				case CL_BUILD_IN_PROGRESS: LOG_INFO("OpenCL", "Build in progress.\n"); break;
			}

			u64 ret_val_size;
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0, 0, &ret_val_size);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);

            if (errcode_ret == CL_SUCCESS) {
                TempAllocator ta(&arena);
                char *build_log = PUSH_STRING(arena, ret_val_size+1);
                errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, 0);
                CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
                build_log[ret_val_size] = '\0';
                LOG_INFO("OpenCL", "CL build log:\n%s\n", build_log);
            }
		}
	}

	cl_program create_from_source_file(MemoryArena &arena, cl_context context, const char *filename, u32 num_devices, cl_device_id *devices, const char *build_options = 0) {
		u64 size;
		FILE *file = open_file(filename, &size);
		TempAllocator ta(&arena);
		char *source_buffer = PUSH_STRING(arena, size);
		size_t read_bytes = fread(source_buffer, 1, size, file);
		fclose(file);
		source_buffer[read_bytes] = '\0';

		cl_int errcode_ret;
		const char *code[] = { source_buffer };
		cl_program program = clCreateProgramWithSource(context, 1, code, &size, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithSource, errcode_ret);

		build(arena, program, num_devices, devices, build_options);

		return program;
	}


	cl_program create_from_strings(MemoryArena &arena, cl_context context, const char *source, u32 num_devices, cl_device_id *devices, const char *build_options = 0) {
		cl_int errcode_ret;
		const char *code[] = { source };
		cl_program program = clCreateProgramWithSource(context, 1, code, 0, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithSource, errcode_ret);

		build(arena, program, num_devices, devices, build_options);

		return program;
	}

	cl_program create_from_binary_file(MemoryArena &arena, cl_context context, const char *filename, cl_device_id *devices) {
		FILE *file;
		fopen_s(&file, filename, "r");

		cl_uint count;
		fread(&count, sizeof(cl_uint), 1, file);

		u64 *binary_sizes = PUSH_STRUCTS(arena, count, u64);
		fread(binary_sizes, sizeof(u64), count, file);

		u8 **binaries;
		binaries = PUSH_STRUCTS(arena, count, u8 *);
		for (u32 i = 0; i < count; ++i) {
			binaries[i] = PUSH_STRUCTS(arena, binary_sizes[i], u8);

			fread(binaries[i], sizeof(u8), binary_sizes[i], file);
		}
		fclose(file);

		cl_int errcode_ret;
		cl_int binary_status[16];
		cl_program program = clCreateProgramWithBinary(context, count, devices, binary_sizes, (const u8 **) binaries, (cl_int*)binary_status, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithBinary, errcode_ret);

		build(arena, program, count, devices);
		return program;
	}

	void write_to_binary_file(MemoryArena &arena, cl_program program, const char *filename) {
		cl_uint num_devices;
		cl_int errcode_ret = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		u64 *binary_sizes = PUSH_STRUCTS(arena, num_devices, u64);
		errcode_ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, num_devices*sizeof(u64), binary_sizes, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		u8 **binaries = PUSH_STRUCTS(arena, num_devices, u8*);
		u64 total = 0;
		for (u32 i = 0; i < num_devices; ++i) {
			binaries[i] = PUSH_STRUCTS(arena, binary_sizes[i], u8);
			total += binary_sizes[i];
		}

		errcode_ret = clGetProgramInfo(program, CL_PROGRAM_BINARIES, total*sizeof(u8 *), binaries, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		{ // Write file
			FILE *file;
			fopen_s(&file, filename, "w");
			fwrite(&num_devices, sizeof(cl_uint), 1, file);
			fwrite(binary_sizes, sizeof(u64), num_devices, file);

			for (u32 i = 0; i < num_devices; ++i) {
				fwrite(binaries[i], sizeof(u8), binary_sizes[i], file);
			}

			fclose(file);
		}
	}
}

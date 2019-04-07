#include "engine/utils/file_utils.h"
#include "cl_errors.cpp"

namespace cl_program_builder {
	static void build(ArenaAllocator &arena, cl_program program, u32 num_devices, cl_device_id *devices, const char *build_options = 0) {
		cl_int errcode_ret = clBuildProgram(program, num_devices, devices, build_options, 0, 0);
		CL_CHECK_ERRORCODE(clBuildProgram, errcode_ret);

		for (u32 i = 0; i < num_devices; ++i) {
			cl_build_status build_status;
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, 0);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
			switch (build_status) {
				case CL_BUILD_NONE: log_info("OpenCL", "No build."); break;
				case CL_BUILD_ERROR: log_info("OpenCL", "Build error."); break;
				case CL_BUILD_SUCCESS: log_info("OpenCL", "Build successful."); break;
				case CL_BUILD_IN_PROGRESS: log_info("OpenCL", "Build in progress."); break;
			}

			uint64_t ret_val_size;
			errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0, 0, &ret_val_size);
			CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);

            if (errcode_ret == CL_SUCCESS) {
                TempAllocator ta(&arena);
                char *build_log = PUSH(&arena, ret_val_size+1, char);
                errcode_ret = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, 0);
                CL_CHECK_ERRORCODE(clGetProgramBuildInfo, errcode_ret);
                build_log[ret_val_size] = '\0';
                log_info("OpenCL", "CL build log:%s", build_log);
            }
		}
	}

	cl_program create_from_source_file(ArenaAllocator &arena, cl_context context, const char *filename, u32 num_devices, cl_device_id *devices, const char *build_options = 0) {
		uint64_t size;
		FILE *file = open_file(filename, &size);
		TempAllocator ta(&arena);
		char *source_buffer = PUSH(&arena, size, char);
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


	cl_program create_from_strings(ArenaAllocator &arena, cl_context context, const char *source, u32 num_devices, cl_device_id *devices, const char *build_options = 0) {
		cl_int errcode_ret;
		const char *code[] = { source };
		cl_program program = clCreateProgramWithSource(context, 1, code, 0, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithSource, errcode_ret);

		build(arena, program, num_devices, devices, build_options);

		return program;
	}

	cl_program create_from_binary_file(ArenaAllocator &arena, cl_context context, const char *filename, cl_device_id *devices) {
		FILE *file;
		fopen_s(&file, filename, "rb");

		cl_uint count;
		fread(&count, sizeof(cl_uint), 1, file);

		uint64_t *binary_sizes = PUSH(&arena, count, uint64_t);
		fread(binary_sizes, sizeof(uint64_t), count, file);

		uint8_t **binaries;
		binaries = PUSH(&arena, count, uint8_t *);
		for (u32 i = 0; i < count; ++i) {
			binaries[i] = PUSH(&arena, binary_sizes[i], uint8_t);

			fread(binaries[i], sizeof(uint8_t), binary_sizes[i], file);
		}
		fclose(file);

		cl_int errcode_ret;
		cl_int binary_status[16];
		cl_program program = clCreateProgramWithBinary(context, count, devices, binary_sizes, (const uint8_t **) binaries, (cl_int*)binary_status, &errcode_ret);
		CL_CHECK_ERRORCODE(clCreateProgramWithBinary, errcode_ret);

		build(arena, program, count, devices);
		return program;
	}

	void write_to_binary_file(ArenaAllocator &arena, cl_program program, const char *filename) {
		cl_uint num_devices;
		cl_int errcode_ret = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		uint64_t *binary_sizes = PUSH(&arena, num_devices, uint64_t);
		errcode_ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, num_devices*sizeof(uint64_t), binary_sizes, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		uint8_t **binaries = PUSH(&arena, num_devices, uint8_t*);
		uint64_t total = 0;
		for (u32 i = 0; i < num_devices; ++i) {
			binaries[i] = PUSH(&arena, binary_sizes[i], uint8_t);
			total += binary_sizes[i];
		}

		errcode_ret = clGetProgramInfo(program, CL_PROGRAM_BINARIES, total*sizeof(uint8_t *), binaries, 0);
		CL_CHECK_ERRORCODE(clGetProgramInfo, errcode_ret);

		{ // Write file
			FILE *file;
			fopen_s(&file, filename, "w");
			fwrite(&num_devices, sizeof(cl_uint), 1, file);
			fwrite(binary_sizes, sizeof(uint64_t), num_devices, file);

			for (u32 i = 0; i < num_devices; ++i) {
				fwrite(binaries[i], sizeof(uint8_t), binary_sizes[i], file);
			}

			fclose(file);
		}
	}
}

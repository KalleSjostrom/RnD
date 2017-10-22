#include "../generator/utils/parser.cpp"

struct Buffer {
	String fill_method;
	String buffer_name;
};

int main(int argc, char *argv[]) {
	parser::Function functions[32];
	int function_counter = 0;

	Buffer buffers[32];
	int buffer_counter = 0;

	String exported_lines[128];
	int exported_line_counter = 0;

	FILE *file1 = fopen("../shaders/compute_shader.cl", "r");
	size_t filesize1 = get_filesize(file1);
	char source1[filesize1];
	fread(source1, 1, filesize1, file1);
	fclose(file1);

	{
		parser::Tokenizer tok = { source1 };
		bool parsing = true;
		while (parsing) {
			parser::Token token = parser::next_token(&tok);
			switch (token.type) {
				case TokenType_CommandMarker: {
					parser::Token variable_name = parser::next_token(&tok);
					parser::Token fill_method = parser::next_token(&tok);

					Buffer buffer = { fill_method.string, variable_name.string };
					buffers[buffer_counter++] = buffer;
				} break;
				case TokenType_Identifier: {
					if (parser::is_equal(token, TOKENIZE("__kernel"))) {
						parser::Token return_type = parser::next_token(&tok);
						parser::Token function_name = parser::next_token(&tok);

						parser::Function func = {0};

						ASSERT_NEXT_TOKEN_TYPE(tok, '(');

						int count = 0;
						while (1) {
							parser::Token scope = parser::next_token(&tok);
							parser::Token type = parser::next_token(&tok);
							ASSERT_NEXT_TOKEN(tok, "*");
							parser::Token variable_name = parser::next_token(&tok);

							ASSERT(parser::is_equal(scope, TOKENIZE("__global")), "scope not __global.");
							ASSERT(parser::is_equal(type, TOKENIZE("float2")), "type not __global.");
							ASSERT(count < 32, "count exceeded.");

							parser::Parameter p = {
								scope.string, type.string, variable_name.string
							};

							func.parameters[count++] = p;

							parser::Token next_token = parser::next_token(&tok);
							if (next_token.type == ')') {
								break;
							} else {
								ASSERT_TOKEN_TYPE(next_token, ',');
							}
						}

						func.function_name = function_name.string;
						func.parameter_count = count;
						functions[function_counter++] = func;
					}
				} break;
				case '\0': {
					parsing = false;
				} break;
			}
		}
	}
	FILE *file2 = fopen("../shaders/fluid_common.cl", "r");
	size_t filesize2 = get_filesize(file2);
	char source2[filesize2];
	fread(source2, 1, filesize2, file2);
	fclose(file2);

	{
		parser::Tokenizer tok = { source2 };
		bool parsing = true;
		while (parsing) {
			parser::Token token = parser::next_token(&tok);
			switch (token.type) {
				case TokenType_CommandMarker: {
					token = parser::next_token(&tok);
					if (parser::is_equal(token, TOKENIZE("export"))) {
						token = parser::next_line(&tok);
						exported_lines[exported_line_counter++] = token.string;
					}
				} break;
				case '\0': {
					parsing = false;
				} break;
			}
		}
	}

	// TODO(kalle): THIS IS BULLSHIT! Find a better way of null terminating the strings without the parser to fuckup..
	for (int i = 0; i < function_counter; ++i) {
		parser::Function &f = functions[i];
		null_terminate(f.function_name);
		for (int j = 0; j < f.parameter_count; ++j) {
			null_terminate(f.parameters[j].scope);
			null_terminate(f.parameters[j].type);
			null_terminate(f.parameters[j].variable_name);
		}
	}
	for (int i = 0; i < buffer_counter; ++i) {
		null_terminate(buffers[i].fill_method);
		null_terminate(buffers[i].buffer_name);
	}
	for (int i = 0; i < exported_line_counter; ++i) {
		null_terminate(exported_lines[i]);
	}

	FILE *output = fopen("../generated/fluid_kernel.generated.cpp", "w");

	fprintf(output, "// Exported lines\n");
	for (int i = 0; i < exported_line_counter; ++i) {
		fprintf(output, "%s\n", *exported_lines[i]);
	}
	fprintf(output, "\n");

	fprintf(output, "#include \"cl_preamble.cpp\"\n");

	fprintf(output, "enum BufferIndex {\n");
	for (int i = 0; i < buffer_counter; ++i) {
		fprintf(output, "\tBufferIndex__%s,\n", *buffers[i].buffer_name);
	}
	fprintf(output, "};\n\n");

	enum BufferIndex {
		BufferIndex__density_pressure,
		BufferIndex__accelerations,
		BufferIndex__velocities,
		BufferIndex__position,
	};

	fprintf(output, "namespace cl_manager {\n");

	// SIM_KERNELS
	fprintf(output, "struct SimKernels {\n");
	for (int i = 0; i < function_counter; ++i) {
		fprintf(output, "\tcl_kernel %s;\n", *functions[i].function_name);
	}
	fprintf(output, "};\n");

	fprintf(output, "struct ClInfo {\n\
	cl_context context;\n\
	cl_command_queue command_queue;\n\
	cl_device_id device;\n\
	cl_program program;\n\
	SimKernels kernels;\n\
	Buffer buffers[32];\n\
	size_t nr_particles;\n};\n\n");

fprintf(output, "void reload_buffer(MemoryArena &arena, ClInfo &info, BufferIndex index, v2 (*f)(int i)) {\n");
fprintf(output, "	cl_context context = info.context;\n");
fprintf(output, "	cl_int errcode_ret;\n");
fprintf(output, "\n");
fprintf(output, "	Buffer &buffer = info.buffers[index];\n");
fprintf(output, "\n");
fprintf(output, "	size_t old_size = sizeof(v2)*info.nr_particles;\n");
fprintf(output, "	size_t new_size = sizeof(v2) * NR_PARTICLES;\n");
fprintf(output, "\n");
fprintf(output, "	v2 *buffer_content = (v2 *) allocate_memory(arena, new_size);\n");
fprintf(output, "	errcode_ret = clEnqueueReadBuffer(info.command_queue, buffer.mem, CL_TRUE, 0, (new_size < old_size ? new_size : old_size), buffer_content, 0, 0, 0);\n");
fprintf(output, "\n");
fprintf(output, "	for (size_t i = info.nr_particles; i < NR_PARTICLES; ++i) {\n");
fprintf(output, "		buffer_content[i] = f(i);\n");
fprintf(output, "	}\n");
fprintf(output, "\n");
fprintf(output, "	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);\n");
fprintf(output, "	glBufferData(GL_ARRAY_BUFFER, new_size, buffer_content, GL_DYNAMIC_DRAW);\n");
fprintf(output, "\n");
fprintf(output, "	errcode_ret = clReleaseMemObject(buffer.mem);\n");
fprintf(output, "	CL_CHECK_ERRORCODE(clReleaseMemObject, errcode_ret);\n");
fprintf(output, "\n");
fprintf(output, "	buffer.mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, buffer.vbo, &errcode_ret);\n");
fprintf(output, "	CL_CHECK_ERRORCODE(clCreateFromGLBuffer, errcode_ret);\n");
fprintf(output, "}\n");

	// SETUP_BUFFERS
	fprintf(output,
"void reload_buffers(MemoryArena &arena, ClInfo &info) {\n\
	cl_context context = info.context;\n\n");

	for (int i = 0; i < buffer_counter; ++i) {
		fprintf(output, "\treload_buffer(arena, info, BufferIndex__%s, %s);\n", *buffers[i].buffer_name, *buffers[i].fill_method);
	}

	fprintf(output, "\n\tinfo.nr_particles = NR_PARTICLES;\n");
	fprintf(output, "\n\tclear_memory(arena);\n}\n\n");

// SETUP_BUFFERS
	fprintf(output,
"void setup_buffers(ClInfo &info) {\n\
	cl_context context = info.context;\n\n");

	for (int i = 0; i < buffer_counter; ++i) {
		fprintf(output, "\tBuffer %s = gen_buffer(context, %s);\n", *buffers[i].buffer_name, *buffers[i].fill_method);
		fprintf(output, "\tinfo.buffers[BufferIndex__%s] = %s;\n", *buffers[i].buffer_name, *buffers[i].buffer_name);
		if (i < buffer_counter-1)
			fprintf(output, "\n");
	}
	fprintf(output, "}\n\n");


// SETUP_KERNELS
	fprintf(output,
"void setup_kernels(ClInfo &info) {\n\
	cl_manager::SimKernels kernels = info.kernels;\n\
	cl_context context = info.context;\n\
	cl_int errcode_ret;\n\n");

	for (int i = 0; i < buffer_counter; ++i) {
		fprintf(output, "\tBuffer %s = info.buffers[BufferIndex__%s];\n", *buffers[i].buffer_name, *buffers[i].buffer_name);
	}

	fprintf(output, "\n");

	for (int i = 0; i < function_counter; ++i) {
		parser::Function func = functions[i];

		for (int j = 0; j < func.parameter_count; ++j) {
			parser::Parameter p = func.parameters[j];
			fprintf(output, "\
	errcode_ret = clSetKernelArg(kernels.%s, %d, sizeof(cl_mem), (void *) &%s.mem);\n\
	CL_CHECK_ERRORCODE(clSetKernelArg, errcode_ret);\n\n", *func.function_name, j, *p.variable_name);
		}
	}
	fprintf(output, "\tinfo.nr_particles = NR_PARTICLES;\n");
	fprintf(output, "}\n\n");


// RUN_KERNELS
	fprintf(output,
"void run_kernels(ClInfo &info) {\n\
	size_t workGroupSize[] = { NR_PARTICLES };\n\
	cl_int errcode_ret;\n\
	cl_event event;\n\n\
	cl_manager::SimKernels kernels = info.kernels;\n");

	for (int i = 0; i < function_counter; ++i) {
		parser::Function func = functions[i];
		fprintf(output, "\
	errcode_ret = clEnqueueNDRangeKernel(info.command_queue, kernels.%s, 1, 0, workGroupSize, 0, 0, 0, &event);\n\
	CL_CHECK_ERRORCODE(clEnqueueNDRangeKernel, errcode_ret);\n\n", *func.function_name);
	}

	fprintf(output, "\
	clFinish(info.command_queue);\n\
}\n");


// CREATE_PROGRAM
	fprintf(output,
"void create_program_and_kernels(MemoryArena &arena, ClInfo &info) {\n\
	cl_int errcode_ret;\n\
	cl_program program = cl_program_builder::create_from_binary_file(arena, info.context, COMPUTE_SHADER_BINARY, &info.device);\n\
	SimKernels kernels = { 0 };\n\n");

	for (int i = 0; i < function_counter; ++i) {
		parser::Function func = functions[i];
		char *function_name = *func.function_name;
		fprintf(output, "\
	cl_kernel %s = clCreateKernel(program, \"%s\", &errcode_ret);\n\
	CL_CHECK_ERRORCODE(clCreateKernel, errcode_ret);\n\
	kernels.%s = %s;\n\n", function_name, function_name, function_name, function_name);
	}

	fprintf(output, "\
	info.program = program;\n\
	info.kernels = kernels;\n}\n}\n");

	fclose(output);
	return 0;
}


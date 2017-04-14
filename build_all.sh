#!/bin/bash
echo Building mygl
	clang -fno-common -nostdlib -Ofast -c mygl/mygl.m -o mygl/mygl.o
	ar -r mygl/libmygllib.a mygl/mygl.o

echo Building OpenCL parser and compiler
	clang -Ofast opencl/cl_parser.cpp -o opencl/cl_parser
	clang -D CL_COMPILE -Ofast opencl/cl_compiler.cpp -o opencl/cl_compiler -framework OpenCL

echo Parsing and compiling cl shaders
	opencl/cl_parser

	# -cl-single-precision-constant -- Treat double precision floating-point constant as single precision constant.
	# -cl-opt-disable -- This option disables all optimizations. The default is optimizations are enabled.
	# -cl-mad-enable -- Allow a * b + c to be replaced by a mad. The mad computes a * b + c with reduced accuracy. For example, some OpenCL devices implement mad as truncate the result of a * b before adding it to c.
	# -cl-no-signed-zeros -- Allow optimizations for floating-point arithmetic that ignore the signedness of zero. IEEE 754 arithmetic specifies the behavior of distinct +0.0 and -0.0 values, which then prohibits simplification of expressions such as x+0.0 or 0.0*x (even with -clfinite-math only). This option implies that the sign of a zero result isn't significant.
	# -cl-unsafe-math-optimizations -- Allow optimizations for floating-point arithmetic that (a) assume that arguments and results are valid, (b) may violate IEEE 754 standard and (c) may violate the OpenCL numerical compliance requirements as defined in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5. This option includes the -cl-no-signed-zeros and -cl-mad-enable options.
	# -cl-finite-math-only -- Allow optimizations for floating-point arithmetic that assume that arguments and results are not NaNs or ±∞. This option may violate the OpenCL numerical compliance requirements defined in in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5.
	# -cl-fast-relaxed-math -- Sets the optimization options -cl-finite-math-only and -cl-unsafe-math-optimizations. This allows optimizations for floating-point arithmetic that may violate the IEEE 754 standard and the OpenCL numerical compliance requirements defined in the specification in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5. This option causes the preprocessor macro __FAST_RELAXED_MATH__ to be defined in the OpenCL program.
	opencl/cl_compiler shaders/compute_shader.cl shaders/compute_shader.bin -I shaders -D SOME_MACRO -cl-single-precision-constant -cl-fast-relaxed-math -Werror


echo Building OpenGL parser and compiler
	clang -Ofast opengl/gl_parser.cpp -o opengl/gl_parser
	clang -ggdb opengl/gl_parser.cpp -o opengl/gl_parser_debug

echo Parsing and compiling gl shaders
	opengl/run_parser

echo Compiling main program
mygl="-L./mygl -lmygllib"
clang -D USE_OPENGL -Ofast source/interface.cpp -o out/app $mygl -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

if ! [ -e generated/type_info.generated.cpp ]
then
	echo Writing type info file
	echo "" > generated/type_info.generated.cpp
fi

echo Compiling memory layout parser
	clang -Ofast reload/parse_data_layout.cpp -o reload/parse_data_layout

echo Building game dynamic library
./build_dylib.sh

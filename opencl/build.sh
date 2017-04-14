#!/bin/bash
clang -Ofast cl_parser.cpp -o cl_parser
clang -D CL_COMPILE -Ofast cl_compiler.cpp -o cl_compiler -framework OpenCL
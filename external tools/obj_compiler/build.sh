#!/bin/bash
source ../../common.sh

# local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs -Wno-format-nonliteral -Wno-double-promotion"
# frameworks="-framework OpenGL -framework OpenCL"

echo Building obj_compiler
clang $debug $common -I../../ obj_compiler.cpp -o obj_compiler $flags
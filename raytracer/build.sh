#!/bin/bash
# ./run_generators.sh

source ../common.sh

local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs -Wno-format-nonliteral -Wno-double-promotion"
frameworks="-framework OpenGL -framework OpenCL"

echo Building raytracer plugin
if clang $debug $common $local_flags -I../ -dynamiclib -std=c++11 source/raytracer.cpp -o out/raytracer.dylib $flags $frameworks; then
	echo "" > out/__reload_marker
else
	echo "crap"
fi

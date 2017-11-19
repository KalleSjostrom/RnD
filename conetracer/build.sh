#!/bin/bash
# ./run_generators.sh

source ../common.sh

local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs -Wno-format-nonliteral -Wno-double-promotion -Wno-embedded-directive"
frameworks="-framework OpenGL -framework OpenCL"

echo Building conetracer plugin
if clang $debug $common $local_flags -I../ -dynamiclib -std=c++11 source/conetracer.cpp -o out/conetracer.dylib $flags $frameworks; then
	echo "" > out/__reload_marker
else
	echo "crap"
fi

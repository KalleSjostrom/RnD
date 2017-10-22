#!/bin/bash
source ../common.sh

local_flags="-ggdb -mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs -std=c++11 -Wno-vla-extension -Wno-vla -Wno-float-equal"
frameworks="-framework OpenGL -framework Cocoa -framework CoreVideo -framework OpenCL"

# echo Building fluid plugin
# clang -dynamiclib -std=c++11 plugin_fluid.cpp -I../ -o out/fluid.dylib $flags $frameworks
# echo "" > out/__lockfile

if clang $debug $common $local_flags -I../ -dynamiclib -std=c++11 plugin_fluid.cpp -o out/fluid.dylib $flags $frameworks; then
	echo "" > out/__reload_marker
else
	echo "crap"
fi

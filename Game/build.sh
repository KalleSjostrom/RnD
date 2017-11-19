#!/bin/bash
# ./run_generators.sh

source ../common.sh

local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs"
frameworks="-framework OpenGL"

echo Building game plugin
if clang $debug $common $local_flags -I../ -dynamiclib -std=c++11 source/game.cpp -o out/game.dylib $flags $frameworks; then
	echo "" > out/__reload_marker
else
	echo "crap"
fi

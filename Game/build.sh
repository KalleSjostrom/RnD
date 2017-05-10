#!/bin/bash
source ../common.sh

local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs"
frameworks="-framework OpenGL"

echo Building game plugin
clang $debug $common $local_flags -dynamiclib -std=c++11 source/game.cpp -o out/game.dylib $flags $frameworks
echo "" > out/__lockfile

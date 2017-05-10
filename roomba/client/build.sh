#!/bin/bash
source ../../common.sh

local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs"
frameworks="-framework OpenGL"

echo Building plugin
clang $debug $common $local_flags -dynamiclib -std=c++11 main.cpp -I../../ -o ../out/client.dylib $flags $frameworks
echo "" > ../out/__lockfile

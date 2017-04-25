#!/bin/bash
flags="-ggdb -mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs"
frameworks="-framework OpenGL -framework Cocoa -framework CoreVideo -framework OpenCL"

echo Building fluid plugin
clang -dynamiclib -std=c++11 plugin_fluid.cpp -I../ -o out/fluid.dylib $flags $frameworks
echo "" > out/__lockfile
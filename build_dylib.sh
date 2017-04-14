#!/bin/bash
mygl="-L./mygl -lmygllib"
clang -Ofast -mavx -dynamiclib source/app.cpp -fvisibility=hidden -nostartfiles -o out/app.dylib $mygl -framework OpenGL -framework Cocoa -framework CoreVideo -framework OpenCL
echo "" > out/__lockfile
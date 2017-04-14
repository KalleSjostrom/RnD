#!/bin/bash

echo Compiling main program
flags="-ggdb -mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs"

echo Building game dynamic library
clang -dynamiclib -std=c++11 source/game.cpp -I.. -o out/game.dylib $flags -framework OpenGL
# clang source/game.cpp -E -I../RnD -o out/preprocess.cpp $flags
echo "" > out/__lockfile

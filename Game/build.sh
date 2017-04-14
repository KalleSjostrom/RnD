#!/bin/bash

# cd mygl
# echo Building mygl
# ./build.sh
# cd ..

mygl="-L./mygl -lmygllib"

echo Compiling main program
frameworks="-framework OpenGL -framework Cocoa -framework SDL2"
framework_path="-F/Library/Frameworks"
includes="-I/Library/Frameworks/SDL2.framework/Headers -I../"
# includes="-I../RnD"
flags="-Ofast -mavx -fvisibility=hidden -nostartfiles"
# flags="-ggdb -mavx -fvisibility=hidden -nostartfiles"
all="$framework_path $frameworks $includes $flags"
echo $all
clang $all $mygl source/app.cpp -o out/app

./build_dylib.sh
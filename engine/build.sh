#!/bin/bash

mygl="-L../mygl -lmygllib"

echo Compiling engine
frameworks="-framework OpenGL -framework Cocoa -framework SDL2"
framework_path="-F/Library/Frameworks"
includes="-I/Library/Frameworks/SDL2.framework/Headers -I../"
flags="-ggdb -mavx -fvisibility=hidden -nostartfiles -Werror -Weverything"
ignored="-Wno-variadic-macros -Wno-old-style-cast -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments -Wno-c++11-long-long -Wno-c++11-compat -Wno-reserved-id-macro -Wno-documentation -Wno-documentation-unknown-command" # -fno-stack-protector"
all="$ignored $framework_path $frameworks $includes $flags"

clang -c utils/fibers/asm/jump_x86_64_sysv_macho_gas.S -o utils/fibers/out/jump_fcontext.o
clang -c utils/fibers/asm/make_x86_64_sysv_macho_gas.S -o utils/fibers/out/make_fcontext.o

clang $all $mygl engine.cpp -o engine
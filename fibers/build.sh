#!/bin/bash
clang -c asm/jump_x86_64_sysv_macho_gas.S -o out/jump_fcontext.o
clang -c asm/make_x86_64_sysv_macho_gas.S -o out/make_fcontext.o
ignored="-Wno-variadic-macros -Wno-old-style-cast -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments -Wno-c++11-long-long -Wno-c++11-compat" # -fno-stack-protector"
clang -ggdb -nostdlib /usr/lib/libpthread.dylib -Werror -Weverything $ignored -Wpadded main.cpp out/jump_fcontext.o out/make_fcontext.o -o out/fibers


#!/bin/bash
clang -c asm/jump_x86_64_sysv_macho_gas.S -o jump_fcontext.o
clang -c asm/make_x86_64_sysv_macho_gas.S -o make_fcontext.o
ignored="-Wno-variadic-macros -Wno-old-style-cast -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments"
clang -ggdb -Werror -Weverything $ignored -Wpadded main.cpp jump_fcontext.o make_fcontext.o -o fibers


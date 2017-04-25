#!/bin/bash
source ../common.sh

clang -c asm/jump_x86_64_sysv_macho_gas.S -o out/jump_fcontext.o
clang -c asm/make_x86_64_sysv_macho_gas.S -o out/make_fcontext.o
clang $debug $common main.cpp out/jump_fcontext.o out/make_fcontext.o -o out/fibers


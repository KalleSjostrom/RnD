#!/bin/bash
# -Ofast -ggdb
# -mavx -msse4.2
# clang -Ofast -mavx mandelbrot.c -S -o mandelbrot.s
# clang -Ofast -mavx mandelbrot.c -o mandelbrot
#try  32 bit mode
#-fno-builtin??
#differnet comilers? g++
clang -D IACA -I ../iaca-mac64/include -Ofast -mavx mandelbrot.c -o run_iaca
clang -Ofast -fno-pic -mavx mandelbrot.c -o mandelbrot

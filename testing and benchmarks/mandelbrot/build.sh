#!/bin/bash
source ../../common.sh
clang -D IACA -I ../iaca-mac64/include $release $common -I../../ -mavx mandelbrot.c -o run_iaca
clang -Wno-undef $release $common -I../../ -mavx -fno-pic -mavx mandelbrot.c -o mandelbrot

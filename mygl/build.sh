#!/bin/bash
echo Start
rm mygl.o
rm libmygllib.a
clang -fno-common -nostdlib -Ofast -c mygl.m
# clang -fno-common -nostdlib -ggdb -c mygl.m
ar -r libmygllib.a mygl.o

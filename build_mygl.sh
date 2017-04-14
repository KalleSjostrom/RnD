#!/bin/bash
echo Start
rm mygl/mygl.o
rm mygl/libmygllib.a
clang -fno-common -nostdlib -Ofast -c mygl/mygl.m
ar -r mygl/libmygllib.a mygl/mygl.o

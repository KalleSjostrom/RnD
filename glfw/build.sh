#!/bin/bash
echo Start
rm glfw_init.o
rm libmylib.a
clang -fno-common -nostdlib -Ofast -c glfw_init.m
ar -r libmylib.a glfw_init.o

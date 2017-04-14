#!/bin/bash
# -Ofast -ggdb
# -mavx -msse4.2
clang -ggdb -mavx -msse4.2 -I "../../RnD" animation_compiler.cpp -o animation_compiler

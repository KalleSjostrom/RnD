#!/bin/bash
flags="-Ofast -mavx -fvisibility=hidden -nostartfiles -Wno-c++11-compat-deprecated-writable-strings -Wno-switch -std=c++11 -Wno-writable-strings"
clang $flags generate_game_strings.cpp
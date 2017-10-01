#!/bin/bash
engine_dir="../engine"
source ../common.sh

clang $debug $common -lc++ -I../ generator.cpp $fiber_bin -o generator
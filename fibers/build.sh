#!/bin/bash
engine_dir="../engine"
source ../common.sh

clang $debug -I../ $common main.cpp $fiber_bin


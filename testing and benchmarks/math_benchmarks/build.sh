#!/bin/bash
source ../../common.sh
clang $release $common -I../../ -mavx math_benchmarks.cpp -o math_benchmarks
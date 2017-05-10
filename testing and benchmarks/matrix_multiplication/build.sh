#!/bin/bash
source ../../common.sh
clang $release $common -I../../ -mavx matrix_multiplication.c -o matrix_multiplication
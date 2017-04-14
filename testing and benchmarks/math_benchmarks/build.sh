#!/bin/bash
# -Ofast -ggdb
# -mavx -msse4.2
# clang -ggdb -mavx math_benchmarks.c -o math_benchmarks
clang -Ofast -mavx math_benchmarks.cpp -o math_benchmarks
# clang -Ofast -mavx math_benchmarks.c -S -o math_benchmarks.s
# clang -Ofast -mavx math_benchmarks.c -o math_benchmarks

#!/bin/bash
# gcc -Ofast -S hello.c -o hello.s -fno-asynchronous-unwind-tables -msse4.2
# gcc -g -c hello.s -o hello.o
# ld -e _main hello.o -o hello -lc /usr/lib/crt1.o
#!/bin/bash
# -Ofast -ggdb
# -mavx -msse4.2
clang -Ofast -mavx perlin.c -S -o perlin.s
clang -Ofast -mavx perlin.c -o perlin

#!/bin/bash
# gcc -Ofast -S hello.c -o hello.s -fno-asynchronous-unwind-tables -msse4.2
# gcc -g -c hello.s -o hello.o
# ld -e _main hello.o -o hello -lc /usr/lib/crt1.o
#!/bin/bash
source ../../common.sh

clang $debug $common -I../../ -Wno-cast-align -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-nested-anon-types main.cpp -o ../out/roomba


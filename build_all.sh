#!/bin/bash
echo Building mygl
./build_mygl.sh

cd opencl
echo Building OpenCL parser and compiler
./build.sh
echo Parsing and compiling cl shaders
./parse_and_compile.sh
cd ..

cd opengl
echo Building OpenGL parser and compiler
./build.sh
echo Parsing and compiling gl shaders
./run_parser.sh
cd ..

echo Compiling main program
mygl="-L./mygl -lmygllib"
clang -D USE_OPENGL -Ofast source/interface.cpp -o out/app $mygl -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

cd reload
if ! [ -e ../generated/type_info.generated.cpp ]
then
	echo Writing type info file
	echo "" > ../generated/type_info.generated.cpp
fi
echo Compiling memory layout parser
clang -Ofast parse_data_layout.cpp -o parse_data_layout
cd ..

echo Building game dynamic library
./build_dylib.sh

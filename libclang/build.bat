@echo off

if not exist build mkdir build

pushd build
	SET "CC=cl"
	SET "CFLAGS=-nologo -W4 -WX -wd 4100 -wd 4102 -I D:\work\llvm\tools\clang\include"
	@%CC% %CFLAGS% ../main.cpp

	main.exe -x c++ -D DEBUG ../test.cpp
popd

@echo off

echo Building compiler
set NAME=compiler

rem Setup the compiler parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../ -I ../engine/include/
set LIBS=
rem ../engine/bin/SDL2.lib
set ENTRY_POINT=compiler
set OUT_PATH=bin
set OBJECTS=../engine/utils/fibers/out/jump_fcontext.obj ../engine/utils/fibers/out/make_fcontext.obj

set DISABLE_WARNINGS=/wd4458 /wd4244 /wd4061 /wd4062 /wd4365 /wd4464 /wd4514 /wd4668 /wd4820 /wd4625 /wd4710 /wd4626 /wd4582 /wd4623
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd4060 /wd4068 /wd4201 /wd4127 /wd4191 /wd4505 /wd4711
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd5026 /wd5027 /wd4577

set CC=cl.exe
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Ox -GF -WL -Wall -GT

%CC% %FLAGS% %DEFINES% %INCLUDES% %LIBS% %ENTRY_POINT%.cpp %OBJECTS% %DISABLE_WARNINGS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb

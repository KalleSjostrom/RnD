@echo off

ml64.exe /nologo /c /Foutils/fibers/out/jump_fcontext.obj utils/fibers/asm/jump_x86_64_ms_pe_masm.asm
ml64.exe /nologo /c /Foutils/fibers/out/make_fcontext.obj utils/fibers/asm/make_x86_64_ms_pe_masm.asm

echo Building engine
set NAME=engine

rem Setup the compiler parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../ -I .
SET LIBS=bin/SDL2.lib bin/SDL2_Image.lib dbghelp.lib
set ENTRY_POINT=engine
set OUT_PATH=bin
set OBJECTS=
set UNITS=engine.cpp modules/logging.cpp modules/image.cpp modules/audio.cpp modules/input.cpp modules/error.cpp

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT -MP
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Ox -GF -WL -Wall -GT -MP

%CC% %FLAGS% %DEFINES% %INCLUDES% %LIBS% %UNITS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb
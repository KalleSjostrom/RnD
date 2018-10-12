@echo off

echo Building compiler
set NAME=compiler

set ENGINE=../engine

rem Setup the compiler parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I %ENGINE% -I ../engine/include/
SET LIBS=dbghelp.lib
set ENTRY_POINT=compiler
set OUT_PATH=bin
set OBJECTS=%ENGINE%/utils/fibers/out/jump_fcontext.obj %ENGINE%/utils/fibers/out/make_fcontext.obj
set UNITS=compiler.cpp %ENGINE%/modules/logging.cpp %ENGINE%/modules/error.cpp

set CC=cl.exe
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Ox -GF -WL -Wall -GT -MP

%CC% %FLAGS% %DEFINES% %INCLUDES% %LIBS% %UNITS% %OBJECTS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb

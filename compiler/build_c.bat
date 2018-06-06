@echo off

echo Building compiler
set NAME=compiler

set DEFINES=
set INCLUDES=-I../
set LIBS=
set ENTRY_POINT=compiler
set OUT_PATH=bin

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Ox -GF -WL -Wall -GT

%CC% %FLAGS% %DEFINES% %INCLUDES% %LIBS% %ENTRY_POINT%.c -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb

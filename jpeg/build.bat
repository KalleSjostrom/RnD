@echo off

echo Building jpeg
set NAME=jpeg

rem Setup the jpeg parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../ -I ../engine/include/
set LIBS=
set ENTRY_POINT=jpeg
set OUT_PATH=bin
set OBJECTS=

set DISABLE_WARNINGS=/wd4458 /wd4244 /wd4061 /wd4062 /wd4365 /wd4464 /wd4514 /wd4668 /wd4820 /wd4625 /wd4710 /wd4626 /wd4582 /wd4623
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd4060 /wd4068 /wd4201 /wd4127 /wd4191 /wd4505 /wd4711
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd5026 /wd5027 /wd4577 /wd4100

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT

%CC% %FLAGS% %DEFINES% %INCLUDES% %LIBS% %ENTRY_POINT%.cpp %OBJECTS% %DISABLE_WARNINGS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb

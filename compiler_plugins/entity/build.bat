@echo off

echo Building obj compiler plugin
set NAME=plugin

rem Setup the compiler parameters and general variables
set INCLUDES=-I ../../
set OUT_PATH=.

set DISABLE_WARNINGS=/wd4458 /wd4244 /wd4061 /wd4062 /wd4365 /wd4464 /wd4514 /wd4668 /wd4820 /wd4625 /wd4710 /wd4626 /wd4582 /wd4623
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd4060 /wd4068 /wd4201 /wd4127 /wd4191 /wd4505

set CC=cl.exe
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Ox -GF -WL -Wall -GT

%CC% %FLAGS% %DEFINES% %INCLUDES% %NAME%.cpp %DISABLE_WARNINGS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb %LIBS% -LD /link -incremental:no -opt:ref -PDB:"%OUT_PATH%\%NAME%.pdb" -OUT:"%OUT_PATH%\%NAME%.dll"
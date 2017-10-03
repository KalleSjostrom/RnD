@echo off

rem run_generators.bat

rem source ../common.sh
rem local_flags="-mavx -fvisibility=hidden -nostartfiles -Wno-trigraphs"
rem frameworks="-framework OpenGL"

echo Building game plugin
set PLUGIN_NAME=game

rem Setup the compiler parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../
set ENTRY_POINT=source/game
set PDB_NAME=%PLUGIN_NAME%.pdb
set OUT_PATH=out

set DISABLE_WARNINGS=/wd4458 /wd4244 /wd4061 /wd4062 /wd4365 /wd4464 /wd4514 /wd4668 /wd4820 /wd4625 /wd4710 /wd4626 /wd4582 /wd4623
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd4060 /wd4068 /wd4201 /wd4127 /wd4191 /wd4505

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall
set DLL_NAME=%PLUGIN_NAME%.dll

%CC% %FLAGS% %DEFINES% %INCLUDES% opengl32.lib %ENTRY_POINT%.cpp %DISABLE_WARNINGS% -Fm%NAME%.map -LD /link -incremental:no -opt:ref -PDB:"%OUT_PATH%\%PDB_NAME%" -OUT:"%OUT_PATH%\%DLL_NAME%"

rem clang $debug $common $local_flags -I../ -dynamiclib -std=c++11 source/game.cpp -o out/game.dylib $flags $frameworks
rem echo "" > out/__lockfile

@echo off

rem mygl="-L../mygl -lmygllib"
rem frameworks="-framework OpenGL -framework Cocoa -framework SDL2"
rem framework_path="-F/Library/Frameworks"
rem includes="-I/Library/Frameworks/SDL2.framework/Headers -I../"
rem flags="-ggdb -mavx -fvisibility=hidden -nostartfiles -Werror -Weverything"
rem ignored="-Wno-variadic-macros -Wno-old-style-cast -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments -Wno-c++11-long-long -Wno-c++11-compat -Wno-reserved-id-macro -Wno-documentation -Wno-documentation-unknown-command" # -fno-stack-protector"
rem all="$ignored $framework_path $frameworks $includes $flags"

ml64.exe /nologo /c /Foutils/fibers/out/jump_fcontext.obj utils/fibers/asm/jump_x86_64_ms_pe_masm.asm
ml64.exe /nologo /c /Foutils/fibers/out/make_fcontext.obj utils/fibers/asm/make_x86_64_ms_pe_masm.asm

echo Building engine
set NAME=engine

rem Setup the compiler parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../
SET LIBS=bin/SDL2.lib bin/SDL2_Image.lib dbghelp.lib
set ENTRY_POINT=engine
set OUT_PATH=bin

set DISABLE_WARNINGS=/wd4458 /wd4244 /wd4061 /wd4062 /wd4365 /wd4464 /wd4514 /wd4668 /wd4820 /wd4625 /wd4710 /wd4626 /wd4582 /wd4623
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd4060 /wd4068 /wd4201 /wd4127 /wd4191 /wd4505 /wd4711

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -O2 -GF -WL -Wall -GT

%CC% %FLAGS% %DEFINES% %INCLUDES% %ENTRY_POINT%.cpp %DISABLE_WARNINGS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb %LIBS%
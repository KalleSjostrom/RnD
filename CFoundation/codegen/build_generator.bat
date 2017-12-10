@echo off

call "%~dp0/../build_vars.bat"

rem Params:
rem   1: debug|release
rem   2: input filename
rem   3: output filename
rem   4: echo output string
rem   5: extra flags
rem   6: extra libs

set BUILD_TYPE=%~1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=release

set INPUT_FILENAME=%~2
if "%INPUT_FILENAME%"=="" set INPUT_FILENAME=main.cpp

set OUTPUT_FILENAME=%~3
if "%OUTPUT_FILENAME%"=="" set OUTPUT_FILENAME=main.exe

set OUTPUT_STRING=%~4
if "%OUTPUT_STRING%"=="" set OUTPUT_STRING="Building Generator"

set EXTRA_FLAGS=%~5
set EXTRA_LIBS=%~6

rem echo BUILD_TYPE=%BUILD_TYPE%
rem echo INPUT_FILENAME=%INPUT_FILENAME%
rem echo OUTPUT_FILENAME=%OUTPUT_FILENAME%
rem echo OUTPUT_STRING=%OUTPUT_STRING%
rem echo EXTRA_FLAGS=%EXTRA_FLAGS%
rem echo EXTRA_LIBS=%EXTRA_LIBS%

if not defined VCVARS_HAS_RUN (
	pushd "%~dp0/../plugin_environment"
		call vcvars_all.bat
		set VCVARS_HAS_RUN=1
	popd
)

if defined VCVARS_HAS_RUN (
	goto build
)

goto error

:build
echo -- %OUTPUT_STRING% [%BUILD_TYPE%]

set FLAGS=%EXTRA_FLAGS% -nologo -fp:fast /DGAME_CODE_DIR=\"%GAME_CODE_DIR%\"

if "%BUILD_TYPE%"=="debug" (
	set FLAGS=%FLAGS% -D DEBUG -GR- -EHa -FC -Zi -GF
) else (
	set FLAGS=%FLAGS% -Z7 -O2 -Ox -GL -GS- -EHa- -GR- -GA
)

cl.exe %FLAGS% /Fe%OUTPUT_FILENAME%.exe /Fd%OUTPUT_FILENAME%.pdb %EXTRA_LIBS% %INPUT_FILENAME%

IF ERRORLEVEL 1 goto error

del /Q *.ilk *.obj >NUL 2>NUL
del /Q cache.bin >NUL 2>NUL

goto eof

:error
	set ERR=%ERRORLEVEL%
	ECHO %~n0: Error: %ERR%
	rem cd %~dp0
	exit /B %ERR%

:eof
	exit /B 0

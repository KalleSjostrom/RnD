@echo off

call "%~dp0/build_vars.bat"

echo -- Running Build Game Script

set PLATFORM=%~1
set GENERATE_RELOAD=%~2
set RELOAD=%~3
set DATA_DIR=%~4
set PLUGIN_SDK_DIR=%~5
set EXTERNAL_PLUGIN_DIR=%~6 rem the folder where stingray keeps its plugins, e.g. the wwise plugin
set PLUGIN_ENV_DIR=%~7

rem Setup the required paths for the plugin interface
rem !!NOTE!! Below we're pointing to the c_api-directory here! This is intentional to avoid collision
rem between the plugin-api- and c-includes, i.e #include <math.h> would pull in the c math.h
rem instead of the plugin-api math.h file
if not defined SR_SOURCE_DIR (
	set PLUGIN_SDK_DIR=../../binaries/runtime
	set EXTERNAL_PLUGIN_DIR=../../binaries/runtime/plugins
) else (
	set PLUGIN_SDK_DIR=%SR_SOURCE_DIR%/runtime/
	set EXTERNAL_PLUGIN_DIR=%SR_SOURCE_DIR%/runtime/plugins/
)
if [[%PLATFORM%]]==[[]] (set PLATFORM=win32)
pushd ..\
if [[%DATA_DIR%]]==[[]] (
	set DATA_DIR=%CD%_data
)
popd
if [[%PLUGIN_ENV_DIR%]]==[[]] ( set PLUGIN_ENV_DIR=.\plugin_environment )

echo PLATFORM:            "%PLATFORM%"
echo DATA_DIR:            "%DATA_DIR%"
echo SR_SOURCE_DIR:       "%SR_SOURCE_DIR%"
echo PLUGIN_SDK_DIR:      "%PLUGIN_SDK_DIR%"
echo EXTERNAL_PLUGIN_DIR: "%EXTERNAL_PLUGIN_DIR%"
echo PLUGIN_ENV_DIR:      "%PLUGIN_ENV_DIR%"
echo.

if not exist "%PLUGIN_SDK_DIR%" (
	echo Missing directory "%PLUGIN_SDK_DIR%", run update_binaries.bat!
	set ERRORLEVEL=1
	goto error
)

if not exist "%EXTERNAL_PLUGIN_DIR%" (
	echo Missing directory "%EXTERNAL_PLUGIN_DIR%", run update_binaries.bat!
	set ERRORLEVEL=1
	goto error
)

if not exist "%PLUGIN_ENV_DIR%" (
	echo Missing directory "%PLUGIN_ENV_DIR%", run update_binaries.bat!
	set ERRORLEVEL=1
	goto error
)

pushd "%PLUGIN_ENV_DIR%"
	call vcvars_min.bat
	IF ERRORLEVEL 1 goto error
popd

rem Make sure we aren't currently running the generator. It might go very wrong if we try and open/write to the same files
if exist "__lockfile" (
	echo "Generators currently active... ignoring build command"
	goto error
)

echo "" >> __lockfile

rem Create plugin output path if necessary
set OUT_PATH=%DATA_DIR%\%PLATFORM%\game\
echo %OUT_PATH%
if not exist "%OUT_PATH%" mkdir "%OUT_PATH%"

rem TODO: Only generate when needed!
pushd scripts
	call generate_code.bat
	call generate_game_strings.bat
	call generate_reload.bat
popd

del /Q __lockfile >NUL 2>NUL

rem Try and clear out all previous pdbs still in the output folder
pushd "%OUT_PATH%"
	echo    Trying to remove pdbs, dlls and still in the game folder
	del /Q *.dll *.prx *.pdb *.ilk >NUL 2>NUL
popd

set INSTANCE_RUNNING=0

rem Check if we still have pdbs after the clear above. If so, it's locked by running instances.
if exist "%OUT_PATH%\*.pdb" set INSTANCE_RUNNING=1
if exist "%OUT_PATH%\*.dll" set INSTANCE_RUNNING=1
if exist "%OUT_PATH%\*.prx" set INSTANCE_RUNNING=1

if %INSTANCE_RUNNING%==1 (
	rem therefore, we need to append a random name to the plugin name
	set PLUGIN_NAME=game_%random%
) else (
	set PLUGIN_NAME=game
)

rem Setup the compiler parameters and general variables
set DEFINES=-D ARROWHEAD -D DEVELOPMENT -D AH_PLUGIN
set INCLUDES=-I "%GAME_CODE_DIR%" -I "%EXTERNAL_PLUGIN_DIR%" -I "%PLUGIN_SDK_DIR%" -I "%~dp0."
set ENTRY_POINT=entry_point
set PDB_NAME=%PLUGIN_NAME%.pdb

rem TODO(kalle): Always generate the reload layout
rem if [[%GENERATE_RELOAD%]]==[[generate_reload]] (
echo Compiling with GENERATE_RELOAD_LAYOUT on
set DEFINES=-D GENERATE_RELOAD_LAYOUT %DEFINES%
rem )

if [[%RELOAD%]]==[[reload_data]] (
	echo Compiling with RELOAD_DATA on
	set DEFINES=-D RELOAD_DATA %DEFINES%
)

rem Select build platform
if [[%PLATFORM%]]==[[ps4]] goto build_ps4 else goto build_windows


rem -------------- BUILD WINDOWS START --------------
:build_windows
	echo    Building game dll (cl.exe)...

	set CC=cl.exe
	set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -W4
	rem set FLAGS=%FLAGS% -Ox
	set DEFINES=-D WINDOWS -D WINDOWSPC %DEFINES%
	set DLL_NAME=%PLUGIN_NAME%.dll

	%CC% %FLAGS% %DEFINES% %INCLUDES% boot\%ENTRY_POINT%.cpp -LD -link -PDB:"%OUT_PATH%\%PDB_NAME%" -OUT:"%OUT_PATH%\%DLL_NAME%"

	IF ERRORLEVEL 1 goto compile_error
	goto compile_done
rem -------------- BUILD WINDOWS DONE --------------

rem -------------- BUILD PS4 START --------------
:build_ps4
	echo    Building game dll (orbis-clang++.exe)...

	set DISABLE_WARNINGS=-Wno-switch -Wno-tautological-compare -Wno-address-of-temporary -Wno-constant-conversion -Wno-format -Wno-pointer-bool-conversion -Wno-writable-strings -Wno-c++11-narrowing -Wno-macro-redefined -Wno-invalid-offsetof
	set PRX_NAME=%PLUGIN_NAME%.prx
	set DEFINES=-D PS4 %DEFINES%
	set FLAGS=-g -O0
	set LINKER_FLAGS=-linker=sn -Wl,--oformat=prx,--Map="%OUT_PATH%\%PDB_NAME%",--sn-full-map
	set DLL_NAME=%PLUGIN_NAME%.prx

	call %PS4_CLANG_PATH% %PS4_SDK_HEADER_PATHS% %FLAGS% %DEFINES% %LINKER_FLAGS% %INCLUDES% boot\\%ENTRY_POINT%.cpp -working-directory %~dp0 %DISABLE_WARNINGS% -o %OUT_PATH%\%DLL_NAME%

	IF ERRORLEVEL 1 goto compile_error
	goto compile_done
rem -------------- BUILD PS4 END --------------


:compile_done
rem ctime -end compile.ct %ERRORLEVEL%

pushd "%OUT_PATH%"
	rem Check if we instances running.
	if %INSTANCE_RUNNING%==1 (
		echo Found running instances, poking the __reload_marker to cause a reload
		rem If so, we need to let the engine process know there's a new game dll to load
		echo "" >> __reload_marker
	) else (
		echo Found no running instances, make sure __reload_marker is removed
		del /Q __reload_marker >NUL 2>NUL
	)
popd

rem Remove unnecessary entry_point_pch binaries
del /Q *.obj *.exp *.lib *.a >NUL 2>NUL

goto eof

:compile_error
	set ERR=%ERRORLEVEL%
	rem ctime -end compile.ct %ERRORLEVEL%
	cd %~dp0
	del /Q __lockfile >NUL 2>NUL
	rem ctime -end %~n0.ct %ERR%
	ECHO %~n0: Error: %ERR%
	exit /B %ERR%

:error
	set ERR=%ERRORLEVEL%
	cd %~dp0
	del /Q __lockfile >NUL 2>NUL
	rem ctime -end %~n0.ct %ERR%
	ECHO %~n0: Error: %ERR%
	exit /B %ERR%

:eof
	cd %~dp0
	del /Q __lockfile >NUL 2>NUL
	rem ctime -end %~n0.ct
	exit /B 0

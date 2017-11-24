@echo off

echo Building %PLUGIN_NAME% plugin
set OUT_PATH=out

set INSTANCE_RUNNING=0

rem Try and clear out all previous pdbs still in the output folder
pushd "%OUT_PATH%"
	rem echo    Trying to remove pdbs, dlls and still in the plugin folder
	del /Q *.dll *.prx *.pdb *.ilk >NUL 2>NUL
popd

rem Check if we still have pdbs after the clear above. If so, it's locked by running instances.
if exist "%OUT_PATH%\*.pdb" set INSTANCE_RUNNING=1
if exist "%OUT_PATH%\*.dll" set INSTANCE_RUNNING=1

rem Setup the compiler parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../ -I ../engine/include/opencl22/
set ENTRY_POINT=source/%PLUGIN_NAME%

if %INSTANCE_RUNNING%==1 (
	rem therefore, we need to append a random name to the plugin name
	set PLUGIN_NAME="%PLUGIN_NAME%_%random%"
)
set PDB_NAME=%PLUGIN_NAME%.pdb

set DISABLE_WARNINGS=/wd4458 /wd4244 /wd4061 /wd4062 /wd4365 /wd4464 /wd4514 /wd4668 /wd4820 /wd4625 /wd4710 /wd4626 /wd4582 /wd4623
set DISABLE_WARNINGS=%DISABLE_WARNINGS% /wd4060 /wd4068 /wd4201 /wd4127 /wd4191 /wd4505 /wd4100 /wd4324

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -arch:AVX2

%CC% %FLAGS% %DEFINES% %INCLUDES% opengl32.lib %ENTRY_POINT%.cpp %DISABLE_WARNINGS% -Fm%NAME%.map -LD /link -incremental:no -opt:ref -PDB:"%OUT_PATH%\%PDB_NAME%" -OUT:"%OUT_PATH%\%PLUGIN_NAME%.dll"

IF ERRORLEVEL 1 goto compile_error
goto compile_done

:compile_done
pushd "%OUT_PATH%"
	rem Check if we instances running.
	if %INSTANCE_RUNNING%==1 (
		echo Found running instances, poking the __reload_marker to cause a reload
		rem If so, we need to let the engine process know there's a new plugin dll to load
		echo "" >> __reload_marker
	) else (
		rem echo Found no running instances, make sure __reload_marker is removed
		del /Q __reload_marker >NUL 2>NUL
	)
popd

:compile_error
	set ERR=%ERRORLEVEL%
	cd %~dp0
	del /Q __reload_marker >NUL 2>NUL
	ECHO %~n0: Error: %ERR%
	exit /B %ERR%

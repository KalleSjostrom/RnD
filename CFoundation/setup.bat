@echo off

rem ------ CLEAN ------
call clean.bat
IF ERRORLEVEL 1 goto error

call "%~dp0/build_vars.bat"

if not exist "%GAME_CODE_DIR%/generated" mkdir "%GAME_CODE_DIR%/generated"

rem ------ BUILD GENERATORS ------

pushd "codegen\generate_global"
	call build.bat
	IF ERRORLEVEL 1 goto error
popd

pushd "codegen\generate_reloader"
	call build.bat
	IF ERRORLEVEL 1 goto error
popd

pushd "codegen\generate_game_strings"
	call build.bat
	IF ERRORLEVEL 1 goto error
popd

pushd "codegen\generate_asset_strings"
	call build.bat
	IF ERRORLEVEL 1 goto error
popd

pushd "codegen\generate_event_lookups"
	call build.bat
	IF ERRORLEVEL 1 goto error
popd

pushd "codegen\generate_build_info"
	call build.bat
	IF ERRORLEVEL 1 goto error
popd

run_all_generators.bat %*
IF ERRORLEVEL 1 goto error

goto eof

:error
	set ERR=%ERRORLEVEL%
	cd %~dp0
	echo %~n0: Error: %ERR%
	pause
	exit /B %ERR%

:eof
	exit /B 0

@echo off

call "%~dp0/build_vars.bat"

pushd scripts

rem ------ BUILD EVENT LOOKUPS ------
	call generate_event_lookups.bat
	IF ERRORLEVEL 1 goto error

rem ------ BUILD ASSET STRINGS ------
	call generate_asset_strings.bat
	IF ERRORLEVEL 1 goto error

rem ------ GENERATE BUILD INFO ------
	call generate_build_info.bat
	IF ERRORLEVEL 1 goto error

popd

rem ------ BUILD GAME ------
call build.bat %*
IF ERRORLEVEL 1 goto error

goto eof

:error
	set ERR=%ERRORLEVEL%
	echo %~n0: Error: %ERR%
	cd %~dp0
	pause
	exit /B %ERR%

:eof
	exit /B 0

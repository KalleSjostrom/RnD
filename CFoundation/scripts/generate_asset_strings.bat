@echo off

rem call ..\ctime -begin %~n0.ct

rem TODO: Only generate when needed!
echo -- Running Generate Asset Strings
pushd "../codegen/generate_asset_strings"
	call generate_asset_strings.exe
	IF not ERRORLEVEL 0 goto error
popd

goto eof

:error
	set ERR=%ERRORLEVEL%
	cd %~dp0
	ECHO %~n0: Error: %ERR%
	rem call ..\ctime -end %~n0.ct %ERR%
	exit /B %ERR%

:eof
	rem call ..\ctime -end %~n0.ct
	exit /B 0

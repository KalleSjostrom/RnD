@echo off

rem call ..\ctime -begin %~n0.ct

rem TODO: Only generate when needed!
echo -- Running Reload Generator
pushd "../codegen/generate_reloader"
	call generate_reloader.exe
	IF not ERRORLEVEL 0 (
		set ERR=%ERRORLEVEL%
		del /Q cache.bin >NUL 2>NUL rem If we have errors, make sure to force a full retry next time even though no files have changed.
		popd
		goto error
	)
popd

goto eof

:error
	ECHO %~n0: Error: %ERR%
	rem call ..\ctime -end %~n0.ct %ERR%
	exit /B %ERR%

:eof
	rem call ..\ctime -end %~n0.ct

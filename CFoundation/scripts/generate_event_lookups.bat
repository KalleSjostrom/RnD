@echo off

rem call ..\ctime -begin %~n0.ct

echo -- Running Event Lookup Generation
pushd "../codegen/generate_event_lookups"
	call generate_event_lookups.exe
	IF not ERRORLEVEL 0 (
		set ERR=%ERRORLEVEL%
		del /Q cache.bin >NUL 2>NUL rem If we have errors, make sure to force a full retry next time even though no files have changed.
		goto error
	)
popd

goto eof

:error
	cd %~dp0
	ECHO %~n0: Error: %ERR%
	rem call ..\ctime -end %~n0.ct %ERR%
	exit /B %ERR%

:eof
	rem call ..\ctime -end %~n0.ct

@echo off

echo -- Running Generate Build Info
pushd "../../"
	hg summary > "tmp_build_info"
	pushd "foundation/codegen/generate_build_info"
		call generate_build_info.exe
		IF not ERRORLEVEL 0 goto error
	popd
	del "tmp_build_info"
popd

goto eof

:error
	set ERR=%ERRORLEVEL%
	cd %~dp0
	ECHO %~n0: Error: %ERR%
	exit /B %ERR%

:eof
	exit /B 0

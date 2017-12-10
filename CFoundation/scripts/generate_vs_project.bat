@echo off

rem call ..\ctime -begin %~n0.ct

if not [[%1]]==[[]] set SR_SOURCE_DIR=%1

if [[%SR_SOURCE_DIR%]]==[[]] (
	echo Missing SR_SOURCE_DIR environment variable
)

echo -- Running Visual Studio Project Generator
pushd "..\codegen\generate_visual_studio_project"
	call build.bat
	call generate_vcxproj.exe "%SR_SOURCE_DIR%/runtime/plugins/plugin_foundation"
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

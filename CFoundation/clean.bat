@echo off

call "%~dp0/build_vars.bat"


for /d %%i in ("%~dp0"*) do (
	pushd %%i

	if not [[%%~nxi]]==[[.hg]] (
		rem Keep the plugin_environment folder intact
		if not [[%%~nxi]]==[[plugin_environment]] (
			del /S /Q *.bin *.dll *.lib *.pdb *.obj *.pch *.ilk *.DataIndex *.dmp *.exp *.obj *.generated.* generated.script_flow_nodes 2>nul
		)

		rem This is to avoid removing ctime.exe
		if [[%%~nxi]]==[[codegen]] (
			del /S /Q *.exe 2>nul
			echo Removing in %%i
		)
	)
	popd
)

rmdir /Q /S "../../.dll_staging_for_hotreload" 2>nul
rmdir /Q /S "../content/generated" 2>nul
rem rmdir /Q /S "codegen/generate_visual_studio_project/x64" 2>nul
rmdir /Q /S "%GAME_CODE_DIR%/generated" 2>nul
EXIT /B 0

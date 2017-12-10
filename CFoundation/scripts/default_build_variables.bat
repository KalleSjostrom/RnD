@echo off
Setlocal EnableDelayedExpansion

rem Make sure we know where the binaries folder is at
if not defined SR_BIN_DIR (
	set SR_BIN_DIR=..\..\..\binaries
)

rem Setup the required paths for the plugin interface
if not defined SR_SOURCE_DIR (
	set PLUGIN_API_DIR=..\..\..\binaries\plugin_api\plugin_foundation
) else (
	if [[%1]]==[[ignore_sr_source_dir]] (
		set PLUGIN_API_DIR=..\..\..\binaries\plugin_api\plugin_foundation
	) else (
		set PLUGIN_API_DIR=%SR_SOURCE_DIR%/runtime/plugins/plugin_foundation
	)
)
if not exist "%PLUGIN_API_DIR%" (
	echo Missing directory "%PLUGIN_API_DIR%", run update_binaries.bat!
	exit /B 1
)

rem Setup the required paths for the compiler
set PLUGIN_ENV_DIR=../plugin_environment
if not exist "%PLUGIN_ENV_DIR%" (
	echo Missing directory "%PLUGIN_ENV_DIR%", run update_binaries.bat!
	exit /B 1
)


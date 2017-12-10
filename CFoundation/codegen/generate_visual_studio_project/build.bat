@echo off

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=release

if defined VS110COMNTOOLS (
	call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall" amd64
)

call "../build_generator.bat" %BUILD_TYPE% generate_vcxproj.cpp generate_vcxproj "Building Visual Studio Project Generator" "" "ole32.lib"
@echo off

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=release

call "../build_generator.bat" %BUILD_TYPE% generate_reloader.cpp generate_reloader "Building Reload Generator"
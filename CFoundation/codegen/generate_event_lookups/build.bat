@echo off

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=release

call "../build_generator.bat" %BUILD_TYPE% generate_event_lookups.cpp generate_event_lookups "Building Event Lookup Generator"
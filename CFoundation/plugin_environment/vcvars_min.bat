@echo off

rem echo vcvars_min

set "INCLUDE=%CD%\vcvars_min\include;%INCLUDE%"
set "PATH=%CD%\vcvars_min\bin\amd64;%PATH%"
set "LIB=%CD%\vcvars_min\lib\amd64;%LIB%"

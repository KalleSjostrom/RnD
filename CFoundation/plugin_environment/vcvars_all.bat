@echo off

rem echo vcvars_all

set "INCLUDE=%CD%\vcvars_all\winsdk8\include\shared;%CD%\vcvars_all\winsdk8\include\um;%CD%\vcvars_all\vc11\include;%INCLUDE%"
set "PATH=%CD%\vcvars_all\vc11\bin\amd64;%PATH%"
set "LIB=%CD%\vcvars_all\vc11\lib\amd64;%CD%\vcvars_all\winsdk8\lib\x64;%LIB%"

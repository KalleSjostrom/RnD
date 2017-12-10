@echo off

set GAME_CODE_DIR=../code

set PS4_SDK_DIR=%SCE_ORBIS_SDK_DIR%
set PS4_SDK_HEADER_PATHS=-I "%PS4_SDK_DIR%\\target\\include" -I "%PS4_SDK_DIR%\\target\\include_common" -I "%PS4_SDK_DIR%\\host_tools\\lib\\clang\\include"
set PS4_CLANG_PATH="%PS4_SDK_DIR%\\host_tools\\bin\\orbis-clang++.exe"

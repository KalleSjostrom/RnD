@echo off

echo Building Roomba Server
set NAME=roomba_server

rem Setup the server parameters and general variables
set DEFINES=-D DEVELOPMENT
set INCLUDES=-I ../ -I ../../engine
set LIBS=Ws2_32.lib
set ENTRY_POINT=server
set OUT_PATH=bin

set CC=cl.exe
set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Z7 -GF -WL -Wall -GT -MP
rem set FLAGS=-nologo -fp:fast -Gm- -GR- -EHa- -FC -Ox -GF -WL -Wall -GT -MP

%CC% %FLAGS% %DEFINES% %INCLUDES% %LIBS% %ENTRY_POINT%.c %OBJECTS% -Fe%OUT_PATH%/%NAME%.exe -Fd%OUT_PATH%/%NAME%.pdb

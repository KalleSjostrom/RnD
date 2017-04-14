@echo off
setlocal

if "%1"=="" (
	echo Missing arguments. Correct usage is:
	echo      %0 [library]
	echo where [library] is the dll filename without the dll-extension.
	goto eof
)
if not exist %1.dll (
	echo Unable to find %1.dll in the current working directory.
	goto eof
)

where expdef 2>NUL >NUL
if %ERRORLEVEL% NEQ 0 (
	echo Missing command 'expdef', you can downloaded it from:
	echo      http://purefractalsolutions.com/show.php?a=utils/expdef
	goto eof
)

call vcvarsall.bat 2>NUL >NUL
if %ERRORLEVEL% NEQ 0 (
	call "%VS120COMNTOOLS%vsvars32.bat" 2>NUL >NUL
	if %ERRORLEVEL% NEQ 0 (
		call "%VS110COMNTOOLS%vsvars32.bat" 2>NUL >NUL
		if %ERRORLEVEL% NEQ 0 (
			call "%VS100COMNTOOLS%vsvars32.bat" 2>NUL >NUL
			if %ERRORLEVEL% NEQ 0 (
				echo Unable to find Visual Studio environment scripts
				goto next
			)
		)
	)
)

where lib 2>NUL >NUL
if %ERRORLEVEL% NEQ 0 (
	echo Missing command 'lib', is your Visual Studio fully set up?
	goto eof
)

@expdef.exe -p -o %1.dll >%1.def
@lib.exe /NOLOGO /VERBOSE /MACHINE:IX86 /DEF:%1.def

:eof

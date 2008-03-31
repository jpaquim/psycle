@REM Created By Graity_0... Released Under GPL Licence
@echo off
title Automaker QPsycle 1.17 Final - Windows
echo Welcome to Gravity_0 Automaker for QPsycle
pause

if "%1" == "release" goto RELEASE
if "%1" == "debug" goto DEBUG

goto HELP

:DEBUG
echo Running QMake
qmake
echo QMake Complete
echo Compiling Debug Version
mingw32-make debug
echo Make Complete
pause
exit

:RELEASE
echo Running QMake
qmake
echo QMake Complete
echo Compiling Release Version
mingw32-make release
echo Make Complete
pause
exit

:HELP
echo [automaker.bat] release : compiles release version.
echo [automaker.bat] debug   : compiles debug version.
echo [automaker.bat] help 	 : shows this prompt.
pause

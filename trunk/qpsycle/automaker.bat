@REM Created By Graity_0... Released Under GPL Licence
@echo off
title Automaker QT 1.15 - Windows
COLOR 0%3
echo Welcome to Gravity_0 Automaker for QT
echo Qt is a registered trademaerk of Trolltech
pause
if "%1" == "help" goto HELP
echo Running QMake
qmake
echo QMake Complete
if "%1" == "release" goto RELEASE
if "%1" == "debug" goto DEBUG

goto HELP

:DEBUG
make debug
echo Make Complete
pause
exit

:RELEASE
make release
echo Make Complete
pause
exit

:HELP
echo [automaker.bat] release : compiles release version.
echo [automaker.bat] debug   : compiles debug version.
echo [automaker.bat] help 	 : shows this prompt.
pause
@REM Created By Graity_0... Released Under GPL Licence
@echo off
title Automaker QPsycle 1.18 Final - Windows
echo Welcome to Gravity_0 Automaker for QPsycle
pause

if "%1" == "release" goto RELEASE
if "%1" == "debug" goto DEBUG
if "%1" == "clean" goto CLEAN

goto HELP

:DEBUG
echo Running QMake
qmake -recursive CONFIG+=debug CONFIG-=release
echo QMake Complete
echo Compiling Debug Version
mingw32-make debug
echo Make Complete
pause

:RELEASE
echo Running QMake
qmake -recursive CONFIG+=release CONFIG-=debug
echo QMake Complete
echo Compiling Release Version
mingw32-make release
echo Make Complete
pause


:CLEAN
echo Cleaning the hood!
mingw32-make clean
echo Cleaning Complete
pause

:HELP
echo [automaker.cmd] release : compiles release version.
echo [automaker.cmd] debug   : compiles debug version.
echo [automaker.cmd] help    : shows this help.
pause

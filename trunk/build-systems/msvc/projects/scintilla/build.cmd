@echo off

%~d0
cd %~p0

set pkgdir=..\..\..\..\external-packages\scintilla
set libdir=%1
set output=..\..\output%2\%1

if not exist %output%\debug\bin (
	mkdir %output%\debug\bin || exit /b 1
)
if not exist %output%\release\bin (
	mkdir %output%\release\bin || exit /b 1
)
copy /Y %pkgdir%\%libdir%\*.dll %output%\debug\bin || exit /b 1
copy /Y %pkgdir%\%libdir%\*.dll %output%\release\bin || exit /b 1

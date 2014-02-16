@echo off

%~d0
cd %~p0

set pkgdir=..\..\..\..\external-packages\zlib-1.2.3
set libdir=lib-mswindows-cabi\%1
set output=..\..\output\%1

if not exist %output%\zlib-stamp (
	if not exist %output%\debug\bin (
		mkdir %output%\debug\bin || exit /b 1
	)
	xcopy /f /Y %pkgdir%\%libdir%\zlibwapi.dll %output%\debug\bin\ || exit /b 1

	if not exist %output%\debug\lib (
		mkdir %output%\debug\lib || exit /b 1
	)
	xcopy /f /Y %pkgdir%\%libdir%\zlibwapi.lib %output%\debug\lib\ || exit /b 1

	if not exist %output%\release\bin (
		mkdir %output%\release\bin || exit /b 1
	)
	xcopy /f /Y %pkgdir%\%libdir%\zlibwapi.dll %output%\release\bin\ || exit /b 1

	if not exist %output%\release\lib (
		mkdir %output%\release\lib || exit /b 1
	)
	xcopy /f /Y %pkgdir%\%libdir%\zlibwapi.lib %output%\release\lib\ || exit /b 1

	echo zlib copied > %output%\zlib-stamp || exit /b 1
)

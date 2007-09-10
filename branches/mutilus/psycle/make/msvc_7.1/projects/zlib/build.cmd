@echo off

%~d0
cd %~p0

set pkgdir=..\..\..\..\..\external-packages\zlib-1.2.3
set libdir=lib-mswindows-cabi

if not exist ..\..\..\..\include\zlib.h (
	if not exist ..\..\..\..\include (
		mkdir ..\..\..\..\include || exit /b 1
	)
	xcopy/f/i %pkgdir%\include\*.h ..\..\..\..\include\ || exit /b 1
)

if not exist ..\..\output\zlib_stamp (

	if not exist ..\..\output\debug\bin (
		mkdir ..\..\output\debug\bin || exit /b 1
	)
	xcopy/f %pkgdir%\%libdir%\zlib1.dll ..\..\output\debug\bin\ || exit /b 1

	if not exist ..\..\output\debug\lib (
		mkdir ..\..\output\debug\lib || exit /b 1
	)
	xcopy/f %pkgdir%\%libdir%\zlib.lib ..\..\output\debug\lib\ || exit /b 1

	if not exist ..\..\output\release.g7\bin (
		mkdir ..\..\output\release.g7\bin || exit /b 1
	)
	xcopy/f %pkgdir%\%libdir%\zlib1.dll ..\..\output\release.g7\bin\ || exit /b 1

	if not exist ..\..\output\release.g7\lib (
		mkdir ..\..\output\release.g7\lib || exit /b 1
	)
	xcopy/f %pkgdir%\%libdir%\zlib.lib ..\..\output\release.g7\lib\ || exit /b 1

	if not exist ..\..\output\release.g6\bin (
		mkdir ..\..\output\release.g6\bin || exit /b 1
	)
	xcopy/f %pkgdir%\%libdir%\zlib1.dll ..\..\output\release.g6\bin\ || exit /b 1

	if not exist ..\..\output\release.g6\lib (
		mkdir ..\..\output\release.g6\lib || exit /b 1
	)
	xcopy/f %pkgdir%\%libdir%\zlib.lib ..\..\output\release.g6\lib\ || exit /b 1

	echo zlib copied > ..\..\output\zlib_stamp || exit /b 1
)

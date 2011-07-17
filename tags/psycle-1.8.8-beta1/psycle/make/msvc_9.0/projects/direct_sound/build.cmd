@echo off

%~d0
cd %~p0

set pkgdir=..\..\..\..\..\external-packages\dsound-9
set libdir=lib-mswindows-msvc-cxxabi\%1
set output=..\..\output\%1

if not exist %output%\direct-sound-stamp (
	if not exist %output%\debug\lib (
		mkdir %output%\debug\lib || exit /b 1
	)
	echo xcopy /f %pkgdir%\%libdir%\dsound.lib %output%\debug\lib\
	xcopy /f %pkgdir%\%libdir%\dsound.lib %output%\debug\lib\ || exit /b 1

	if not exist %output%\release\lib (
		mkdir %output%\release\lib || exit /b 1
	)
	echo xcopy /f %pkgdir%\%libdir%\dsound.lib %output%\release\lib\
	xcopy /f %pkgdir%\%libdir%\dsound.lib %output%\release\lib\ || exit /b 1

	echo direct_sound copied > %output%\direct-sound-stamp || exit /b 1
)
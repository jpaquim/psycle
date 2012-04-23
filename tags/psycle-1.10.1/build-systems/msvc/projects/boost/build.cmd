@echo off

%~d0
cd %~p0

set pkgdir=..\..\..\..\external-packages\boost-%1
set libdir=lib-mswindows-msvc-%2
set output=..\..\output\%3

if not exist %pkgdir%\include\boost (
	pushd %pkgdir% || exit /b 1
	..\7za\7za x -y include.tar.bz2 || exit /b 1
	..\7za\7za x -y include.tar || exit /b 1
	del /q include.tar || exit /b 1
	popd || exit /b 1
)

if not exist %output%\boost-%1-%2-stamp (
	pushd %pkgdir% || exit /b 1
	..\7za\7za x -y %libdir%.tar.bz2 || exit /b 1
	..\7za\7za x -y %libdir%.tar || exit /b 1
	del /q %libdir%.tar || exit /b 1
	popd || exit /b 1

	if not exist %output%\debug\bin (
		mkdir %output%\debug\bin || exit /b 1
	)
	move %pkgdir%\%libdir%\%3\*-gd-*.dll %output%\debug\bin || exit /b 1

	if not exist %output%\debug\lib (
		mkdir %output%\debug\lib || exit /b 1
	)
	move %pkgdir%\%libdir%\%3\*-gd-*.lib %output%\debug\lib || exit /b 1

	if not exist %output%\release\bin (
		mkdir %output%\release\bin || exit /b 1
	)
	move %pkgdir%\%libdir%\%3\*.dll %output%\release\bin || exit /b 1

	if not exist %output%\release\lib (
		mkdir %output%\release\lib || exit /b 1
	)
	move %pkgdir%\%libdir%\%3\*.lib %output%\release\lib || exit /b 1

	echo boost extracted > %output%\boost-%1-%2-stamp || exit /b 1
	rmdir /s/q %pkgdir%\%libdir% || exit /b 1
)

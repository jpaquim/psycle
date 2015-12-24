@echo off

%~d0
cd %~p0

set output=..\..\output%2\%1

if exist %output% (
	pushd %output% && (
		del /s/q SciLexer.dll || exit /b 1
		popd
	)
)

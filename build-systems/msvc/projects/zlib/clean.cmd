@echo off


rem ================================================================================================
rem disabled because there's no way to make it work on windows.. file and or dirs seems to be locked
goto :eof
rem ================================================================================================



%~d0
cd %~p0

set output=..\..\output\%1

pushd %output% && (
	rem [bohan] delete the stamp file in any case, because the rest sometimes fails for unknown reasons
	del /q zlib-stamp || exit /b 1
	popd
) && (
	if exist %output% (
		pushd %output% && (
			del/s/q zlibwapi.lib || exit /b 1
			del/s/q zlibwapi.dll || exit /b 1
			popd
		)
	)
)

rem ===========================================
rem cleans all IDE and compiler generated files
rem ===========================================

%~d0
cd %~p0

del/s/q "*.ncb"
del/s/q/ah "*.suo"
del/s/q "*.vcproj.*.*.user"

rmdir/s/q output

pushd projects\direct_sound && (
	call clean
	popd
)

pushd projects\boost && (
	call clean
	popd
)

pushd projects\zlib && (
	call clean
	popd
)

rem ============================================
rem safely recursively removes empty directories
rem ============================================

call :prune .
goto :eof

:prune
for /d %%i in (%1\*) do (
	call :prune "%%i"
	rmdir "%%i"
)
goto :eof

:failed
echo %~n0 failed.
pause

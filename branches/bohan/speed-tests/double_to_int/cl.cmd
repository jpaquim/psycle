@echo off

rem =====================================================
rem This script allows to temporarily bring the msvc env.
rem =====================================================

rem ============
rem calling main
rem ============

call :main %*

rem ======================================
rem end of main flow, exiting or returning
rem ======================================

goto :return

rem ===================================
rem sub routines, called via call :name
rem ===================================

rem ----------------
rem main sub routine
rem ----------------
:main
	echo %~n0: sourcing mircosoft visual studio 8.0 environement ...
		rem microsoft made a dumb script that keeps appending things to the env vars,
		rem so, when invoke many times, we ends up with "too long line", sic. they can rot in hell.
		rem so, we're saving the env to restore it after.
		call :save_env
		call "%VS80ComnTools%\VSVars32" || goto :failed

		cl.exe %* || ( call :restore_env & goto :failed)

		call :restore_env
goto :eof

rem --------------------
rem save env sub routine
rem --------------------
:save_env
	set old_path=%PATH%
	set old_include=%INCLUDE%
	set old_lib=%LIB%
goto :eof

rem -----------------------
rem restore env sub routine
rem -----------------------
:restore_env
	set PATH=%old_path%
	set INCLUDE=%old_include%
	set LIB=%old_lib%
goto :eof

rem ------
rem failed
rem ------
:failed
	set return_code=%ErrorLevel%
	echo %~n0: failed with return code: %return_code%
	set failed=true
	if "%return_code%" == "0" (
		exit /b 1
	) else (
		exit /b %return_code%
	)

rem ------
rem return
rem ------
:return
	if "%failed%" == "true" (
		echo %~n0: failed with return code: %return_code%
		if "%return_code%" == "0" (
			exit /b 1
		) else (
			exit /b %return_code%
		)
	) else (
		goto :eof
	)

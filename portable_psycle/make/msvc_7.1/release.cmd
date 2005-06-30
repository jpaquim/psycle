@echo off

rem ====================================
rem creating temporary working directory
rem ====================================

	set work=%0.work
	if exist %work% rmdir/s/q %work%
	mkdir %work%
	pushd %work%
	echo %~n0: temporary working directory is %work%

rem ============
rem calling main
rem ============

	if exist ..\..\..\CVS\Tag (
		for /f %%i in (..\..\..\CVS\Tag) do (
			call :main %%i && echo %~n0: all operations completed successfully.
		)
	) else (
		call :main && echo %~n0: all operations completed successfully.
	)

rem ====================================================
echo %~n0: removing temporary working directory %work% ...
rem ====================================================

	popd
	echo %~n0: press any key to remove temporary working directory %work%, or break to keep it.
	pause
	rmdir/s/q %work%
	echo %~n0: removed temporary working directory %work%

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

	rem ===
	rem tag
	rem ===

	set tag=%1
	if "%tag%" == "" (
		echo %~n0: tag: no tag
	) else (
		set tag=%tag:~1%
		echo %~n0: tag: %tag%
	)

	rem =======
	rem targets
	rem =======
	
		rem set targets=g7 g6
		set targets=g7
	
	rem ===============
	rem clean and build
	rem ===============
	
		echo %~n0: sourcing mircosoft visual studio 7.1 environement ...
			rem microsoft made a dumb script that keeps appending things to the env vars,
			rem so, when invoke many times, we ends up with "too long line", sic. they can rot in hell.
			rem so, we're saving the env to restore it after.
			call :save_env
			call "%VS71ComnTools%\VSVars32" || goto :failed
		
		for %%t in (%targets%) do (
			call :rebuild release.%%t || ( call :restore_env & goto :failed)
		)
		
		call :restore_env
	
	rem ===============================================
	echo %~n0: making the directory to be distributed ...
	rem ===============================================

		rem microsoft's state of the art script engine has no late evaluation by default ...  would need to use !stamp!
		set stamp=%date%.%time%
		set stamp=%stamp: =.%
		set stamp=%stamp:/=.%
		set stamp=%stamp:\=.%
		set stamp=%stamp::=.%
		set stamp=%stamp:-=.%
		set stamp=timestamp-%stamp%

		if "%tag%" == "" (
			rem microsoft's state of the art script engine has no late evalutation by default ... would need to use !tag!
		) else (
			set stamp=%tag%
		)
		
		set distribution=.\psycle.mfc.bin.%stamp%\
		
		mkdir "%distribution%"
		echo %~n0: directory %distribution% created
		
		rem ---------------------------------------
		echo %~n0: copying libraries and programs ...
		rem ---------------------------------------
		
			for %%t in (%targets%) do (
				call :copy_binaries %%t || goto :failed
			)
		
		rem ---------------------------------------
		echo %~n0: copying end-user documentation ...
		rem ---------------------------------------
		
			mkdir "%distribution%\doc\"
			xcopy/s ..\..\..\doc\for-end-users\* "%distribution%\doc\" || goto :failed
			xcopy "%distribution%\doc\cpu.txt" "%distribution%" || goto :failed
			rename "%distribution%\cpu.txt" "readme-cpu.txt" || goto :failed
		
		rem ---------------------------------------------
		echo %~n0: removing cvs files from distribution ...
		rem ---------------------------------------------
		
			for /r "%distribution%" %%i in (CVS) do rmdir/s/q "%%i"
			for /r "%distribution%" %%i in (.cvsignore) do del/q "%%i"
		
		rem --------------------------------------------------------
		echo %~n0: converting text files to microsoft end-of-lines ...
		rem unix2dos is distributed with cygwin
		rem --------------------------------------------------------
		
			for /r "%distribution%" %%i in (*.txt *.text) do (
				unix2dos "%%i"
				echo.
			)
		
	rem ===========================
	echo %~n0: making the archive ...
	rem ===========================
		
		if "%tag%" == "" (
			set archive=psycle.mfc.bin.7z
		) else (
			set archive=psycle.mfc.bin.%tag%.7z
		)

		rem rar a -s -m5 -md4096 -r0 .\%archive% "%distribution%" 1>> .\%archive%.log 2>&1 || goto :failed
		7za a -ms=on -mx=9 -m0=BCJ2 -m1=LZMA -m2=LZMA -m3=LZMA -mb0:1 -mb0s1:2 -mb0s2:3 .\%archive% "%distribution%" 1>> .\%archive%.log 2>&1 || goto :failed
	
	rem =================================================
	rem uploading the archive and updating the site
	rem scp and ssh are cygwin/unix commands, use / not \
	rem =================================================
	
		rem ------------------------------------------------------------------------
		echo %~n0: mapping local user account name to sourceforge user account name ...
		rem ------------------------------------------------------------------------
		
			if "%USERNAME%" == "JosepMa" (
				set sourceforge_user_account=jaz001
			) else if "%USERNAME%" == "Administrador" (
				set sourceforge_user_account=sampler
			) else if "%USERNAME%" == "bohan" (
				set sourceforge_user_account=johan-boule
			) else if "%USERNAME%" == "x" (
				set sourceforge_user_account=alkenstein
			) else (
				set sourceforge_user_account=%USERNAME%
			)
			
			echo %~n0: sourceforge user account: %sourceforge_user_account%
		
		rem ------------------------------
		rem sourceforge group account
		rem ------------------------------

			set sourceforge_group_account=/home/groups/p/ps/psycle/
			echo %~n0: sourceforge group account: %sourceforge_group_account%
		
		rem ---------------------------------------------
		echo %~n0: uploading the archive to sourceforge ...
		rem ---------------------------------------------
		
			scp ./%archive% "%sourceforge_user_account%@shell.sourceforge.net:%sourceforge_group_account%/htdocs/packages/microsoft/" || goto :failed
		
		rem ------------------------------------------
		echo %~n0: updating sourceforge ht pages ...
		rem ------------------------------------------
		
			ssh "%sourceforge_user_account%@shell.sourceforge.net" "%sourceforge_group_account%/htdocs/update-timestamps" || goto :failed
goto :eof

rem ---------------
rem upx sub routine
rem ---------------
:upx
	upx.exe --overlay=strip --force --strip-relocs=1 --compress-icons=1 --best --crp-ms=999999 --nrv2d --no-backup -o %~p1%~n1.upx%~x1 %1
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

rem -------------------------------------
rem rebuild (clean and build) sub routine
rem -------------------------------------
:rebuild
	set configuration=%1
	rem for %%o in (clean build) do (
	for %%o in (build) do (
		echo %~n0: %%oing %configuration% ...
		DevEnv ..\solution.sln /%%o %configuration% /out %configuration%.%%o.log || goto :failed
	)
goto :eof

rem -------------
rem copy binaries
rem -------------
:copy_binaries
	set target=%1
	echo %~n0: copying %target% ...
	set source=..\output\release.%target%\bin\
	set destination=%distribution%\%target%\
	rem <bohan> Well i think microsoft's documentation about microsoft's xcopy is wrong.
	rem <bohan> xcopy/f <-- shows what files are being copied.
	rem <bohan> It turns out it also does something else, like allowing copy of files whose name ends with ".exe".
	rem <bohan> Also, sometimes, xcopy crashes with a memory access violation... no comment.
	echo %~n0: copying built libraries and programs ...
	xcopy/f/i "%source%\*.exe" "%destination%" || goto :failed
	xcopy/f/i "%source%\*.dll" "%destination%" || goto :failed
	del/q "%destination%\boost_date_time-*.dll" || goto :failed
	del/q "%destination%\boost_program_options-*.dll" || goto :failed
	del/q "%destination%\boost_regex-*.dll" || goto :failed
	del/q "%destination%\boost_signals-*.dll" || goto :failed
	xcopy/f/i "%source%\psycle.plugins\*.dll" "%destination%\native-psycle-plugins\" || goto :failed
	xcopy/s/i "..\..\..\closed-source" "%destination%\native-psycle-plugins\!!!closed-source!!!" || goto :failed
	echo %~n0: copying microsoft c/c++/gdi+/mfc runtime libraries ...
	xcopy "%SYSTEMROOT%\system32\msvcr71.dll" "%destination%" || goto :failed
	xcopy "%SYSTEMROOT%\system32\msvcp71.dll" "%destination%" || goto :failed
	xcopy "%SYSTEMROOT%\system32\mfc71.dll" "%destination%" || goto :failed
	rem xcopy/f/i "%SYSTEMROOT%\WinSxS\x86_Microsoft.Windows.GdiPlus_*_1.0.10.0_*\GDIPlus.dll" "%destination%" || goto :failed
	rem xcopy "%SYSTEMROOT%\WinSxS\x86_Microsoft.Windows.GdiPlus_6595b64144ccf1df_1.0.10.0_x-ww_712befd8\GDIPlus.dll" "%destination%" || goto :failed
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

for %%t in (nrv2b nrv2d) do (
	rmdir/s/q .\release.bin.upx.%%t\
	mkdir .\release.bin.upx.%%t\
	for /r .\release\bin\ %%i in (*.exe *.dll) do (
		call :upx %%t "%%i" || ( echo upx failed, aborting. && goto :pause )
	)
)
goto :eof

:upx
upx.exe --overlay=strip --force --strip-relocs=1 --compress-icons=1 --best --crp-ms=999999 --%1 --no-backup -o .\release.bin.upx.%1\%~n2%~x2 %2
goto :eof

:pause
pause
goto :eof

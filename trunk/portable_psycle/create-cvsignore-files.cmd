%~d0
cd %~p0

for %%i in (7.1 8.0) do (
	echo .cvsignore> make\msvc_%%i\.cvsignore
	echo *.suo>> make\msvc_%%i\.cvsignore
	echo *.ncb>> make\msvc_%%i\.cvsignore
	echo output>> make\msvc_%%i\.cvsignore
)

echo .cvsignore> include\.cvsignore
echo boost>> include\.cvsignore
echo asio>> include\.cvsignore

echo .cvsignore> doc/.cvsignore
echo doxygen.mfc>> doc/.cvsignore

pause

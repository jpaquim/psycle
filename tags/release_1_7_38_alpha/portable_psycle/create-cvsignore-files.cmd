%~d0
cd "%~d0%~p0"

echo .cvsignore> make\msvc_7.1\.cvsignore
echo *.suo>> make\msvc_7.1\.cvsignore
echo *.ncb>> make\msvc_7.1\.cvsignore
echo output>> make\msvc_7.1\.cvsignore

echo .cvsignore> include\.cvsignore
echo boost>> include\.cvsignore

echo .cvsignore> doc/.cvsignore
echo doxygen.mfc>> doc/.cvsignore

pause

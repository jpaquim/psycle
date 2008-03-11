pushd %~dp0 || goto :failed
call "%VS80COMNTOOLS%\VSVars32" || goto :failed
cl /EHsc cpus.cpp || goto :failed
cpus || goto :failed
pause
exit /b 0

:failed
pause
exit /b1

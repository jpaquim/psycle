%~d0
cd %~p0
set configuration=%1
if "%configuration%" == "" (
	set configuration=release
)
call "%VS90ComnTools%\VSVars32" || exit /b 1
rem msbuild solution.sln -property:configuration=%configuration% || exit /b 1
rem vcbuild solution.sln "%configuration%|Win32" || exit /b 1
DevEnv solution.sln /useenv /build %configuration% || exit /b 1

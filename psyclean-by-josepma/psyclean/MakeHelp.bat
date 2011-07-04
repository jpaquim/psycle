@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by PSYCLEAN.HPJ. >"hlp\PsyClean.hm"
echo. >>"hlp\PsyClean.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\PsyClean.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\PsyClean.hm"
echo. >>"hlp\PsyClean.hm"
echo // Prompts (IDP_*) >>"hlp\PsyClean.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\PsyClean.hm"
echo. >>"hlp\PsyClean.hm"
echo // Resources (IDR_*) >>"hlp\PsyClean.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\PsyClean.hm"
echo. >>"hlp\PsyClean.hm"
echo // Dialogs (IDD_*) >>"hlp\PsyClean.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\PsyClean.hm"
echo. >>"hlp\PsyClean.hm"
echo // Frame Controls (IDW_*) >>"hlp\PsyClean.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\PsyClean.hm"
REM -- Make help for Project PSYCLEAN


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\PsyClean.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\PsyClean.hlp" goto :Error
if not exist "hlp\PsyClean.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\PsyClean.hlp" Debug
if exist Debug\nul copy "hlp\PsyClean.cnt" Debug
if exist Release\nul copy "hlp\PsyClean.hlp" Release
if exist Release\nul copy "hlp\PsyClean.cnt" Release
echo.
goto :done

:Error
echo hlp\PsyClean.hpj(1) : error: Problem encountered creating help file

:done
echo.

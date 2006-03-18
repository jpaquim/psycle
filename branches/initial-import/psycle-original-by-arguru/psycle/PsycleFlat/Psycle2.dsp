# Microsoft Developer Studio Project File - Name="Psycle2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Psycle2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Psycle2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Psycle2.mak" CFG="Psycle2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Psycle2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Psycle2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Psycle2 - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x80a /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib dsound.lib /nologo /subsystem:windows /machine:I386 /out:"release\Psycle.exe"

!ELSEIF  "$(CFG)" == "Psycle2 - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x80a /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x80a /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib dsound.lib /nologo /subsystem:windows /debug /machine:I386 /out:"debug\Psycle.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Psycle2 - Win32 Release"
# Name "Psycle2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Gear"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GearDelay.cpp
# End Source File
# Begin Source File

SOURCE=.\GearDistort.cpp
# End Source File
# Begin Source File

SOURCE=.\Gearfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\GearFlanger.cpp
# End Source File
# Begin Source File

SOURCE=.\GearGainer.cpp
# End Source File
# Begin Source File

SOURCE=.\gearPsychosc.cpp
# End Source File
# Begin Source File

SOURCE=.\GearTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\GearVst.cpp
# End Source File
# End Group
# Begin Group "Instrument"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\EnvDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\InstrumentEditor.cpp
# End Source File
# End Group
# Begin Group "Audio_Driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ADriverDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ADriverDialog.h
# End Source File
# Begin Source File

SOURCE=.\AudioDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDriver.h
# End Source File
# Begin Source File

SOURCE=.\DirectSound.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSound.h
# End Source File
# Begin Source File

SOURCE=.\DSoundConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\DSoundConfig.h
# End Source File
# Begin Source File

SOURCE=.\WaveOut.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveOut.h
# End Source File
# Begin Source File

SOURCE=.\WaveOutDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveOutDialog.h
# End Source File
# End Group
# Begin Group "Configuration"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry.h
# End Source File
# Begin Source File

SOURCE=.\SkinDlg.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChildView.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDialogEx1.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameMachine.cpp
# End Source File
# Begin Source File

SOURCE=.\GreetDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IBrowseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MacProp.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NewMachine.cpp
# End Source File
# Begin Source File

SOURCE=.\NewVal.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Psycle2.cpp
# End Source File
# Begin Source File

SOURCE=.\Psycle2.rc
# End Source File
# Begin Source File

SOURCE=.\PtnDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RecorderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SongpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SubMarco.cpp
# End Source File
# Begin Source File

SOURCE=..\Vst\Vst.cpp
# End Source File
# Begin Source File

SOURCE=.\VstGui1.cpp
# End Source File
# Begin Source File

SOURCE=.\VSTHOSTDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VstRack.cpp
# End Source File
# Begin Source File

SOURCE=.\WireDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Gear_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GearDelay.h
# End Source File
# Begin Source File

SOURCE=.\GearDistort.h
# End Source File
# Begin Source File

SOURCE=.\Gearfilter.h
# End Source File
# Begin Source File

SOURCE=.\GearFlanger.h
# End Source File
# Begin Source File

SOURCE=.\GearGainer.h
# End Source File
# Begin Source File

SOURCE=.\gearPsychosc.h
# End Source File
# Begin Source File

SOURCE=.\GearTracker.h
# End Source File
# Begin Source File

SOURCE=.\GearVst.h
# End Source File
# End Group
# Begin Group "Instrument_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\EnvDialog.h
# End Source File
# Begin Source File

SOURCE=.\InstrumentEditor.h
# End Source File
# End Group
# Begin Group "Audio_h"

# PROP Default_Filter ""
# End Group
# Begin Group "Configuration_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SkinDlg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChildView.h
# End Source File
# Begin Source File

SOURCE=.\FileDialogEx1.h
# End Source File
# Begin Source File

SOURCE=.\FrameMachine.h
# End Source File
# Begin Source File

SOURCE=.\GreetDialog.h
# End Source File
# Begin Source File

SOURCE=.\IBrowseDlg.h
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\MacProp.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MasterDlg.h
# End Source File
# Begin Source File

SOURCE=.\NewMachine.h
# End Source File
# Begin Source File

SOURCE=.\NewVal.h
# End Source File
# Begin Source File

SOURCE=.\OutputDlg.h
# End Source File
# Begin Source File

SOURCE=.\PatDlg.h
# End Source File
# Begin Source File

SOURCE=.\Psycle2.h
# End Source File
# Begin Source File

SOURCE=.\PtnDialog.h
# End Source File
# Begin Source File

SOURCE=.\RecorderDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\SongpDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SubMarco.h
# End Source File
# Begin Source File

SOURCE=..\Vst\Vst.h
# End Source File
# Begin Source File

SOURCE=.\VstGui1.h
# End Source File
# Begin Source File

SOURCE=.\VSTHOSTDlg.h
# End Source File
# Begin Source File

SOURCE=.\VstRack.h
# End Source File
# Begin Source File

SOURCE=.\WireDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00005.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00007.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00008.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00009.bmp
# End Source File
# Begin Source File

SOURCE=.\res\dalaydelay.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_vstf.ico
# End Source File
# Begin Source File

SOURCE=.\res\knob_bac.bmp
# End Source File
# Begin Source File

SOURCE=.\res\psycle.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Psycle2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Psycle2.rc2
# End Source File
# Begin Source File

SOURCE=.\res\psycle256.bmp
# End Source File
# Begin Source File

SOURCE=.\res\stuff.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TbMainKnob.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="psycle.player_plugin.winamp.2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=psycle.player_plugin.winamp.2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "psycle.player_plugin.winamp.2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "psycle.player_plugin.winamp.2.mak" CFG="psycle.player_plugin.winamp.2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "psycle.player_plugin.winamp.2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "psycle.player_plugin.winamp.2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "psycle.player_plugin.winamp.2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PSYCLEWINAMPPLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /GB /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PSYCLEWINAMPPLUGIN_EXPORTS" /D "_WINAMP_PLUGIN_" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x403 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG" /d "_WINAMP_PLUGIN_" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /pdb:none /map /machine:I386 /out:"release/in_psycle.dll"
# SUBTRACT LINK32 /profile /debug /nodefaultlib /force

!ELSEIF  "$(CFG)" == "psycle.player_plugin.winamp.2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PSYCLEWINAMPPLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PSYCLEWINAMPPLUGIN_EXPORTS" /D "_WINAMP_PLUGIN_" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x403 /d "_DEBUG"
# ADD RSC /l 0x403 /d "_DEBUG" /d "_WINAMP_PLUGIN_" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"d:/archivos de programa/winamp/plugins/in_psycle_d.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "psycle.player_plugin.winamp.2 - Win32 Release"
# Name "psycle.player_plugin.winamp.2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Configuration.cpp
# End Source File
# Begin Source File

SOURCE=..\DataCompression.cpp
# End Source File
# Begin Source File

SOURCE=..\Dsp.cpp
# End Source File
# Begin Source File

SOURCE=..\FileIO.cpp
# End Source File
# Begin Source File

SOURCE=..\Filter.cpp
# End Source File
# Begin Source File

SOURCE=..\Global.cpp
# End Source File
# Begin Source File

SOURCE=..\Helpers.cpp
# End Source File
# Begin Source File

SOURCE=.\in_psycle.cpp
# End Source File
# Begin Source File

SOURCE=..\Instrument.cpp
# End Source File
# Begin Source File

SOURCE=..\Machine.cpp
# End Source File
# Begin Source File

SOURCE=..\Player.cpp
# End Source File
# Begin Source File

SOURCE=..\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=..\Registry.cpp
# End Source File
# Begin Source File

SOURCE=..\Sampler.cpp
# End Source File
# Begin Source File

SOURCE=..\Song.cpp
# End Source File
# Begin Source File

SOURCE=..\StdAfx.cpp

!IF  "$(CFG)" == "psycle.player_plugin.winamp.2 - Win32 Release"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "psycle.player_plugin.winamp.2 - Win32 Debug"

# ADD CPP /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\VstHost.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Vst\AEffect.h
# End Source File
# Begin Source File

SOURCE=..\Vst\AEffectx.h
# End Source File
# Begin Source File

SOURCE=..\Configuration.h
# End Source File
# Begin Source File

SOURCE=..\Constants.h
# End Source File
# Begin Source File

SOURCE=..\DataCompression.h
# End Source File
# Begin Source File

SOURCE=..\Ddc.h
# End Source File
# Begin Source File

SOURCE=..\Dsp.h
# End Source File
# Begin Source File

SOURCE=..\FileIO.h
# End Source File
# Begin Source File

SOURCE=..\Filter.h
# End Source File
# Begin Source File

SOURCE=..\Global.h
# End Source File
# Begin Source File

SOURCE=..\Helpers.h
# End Source File
# Begin Source File

SOURCE=.\IN2.H
# End Source File
# Begin Source File

SOURCE=..\InputHandler.h
# End Source File
# Begin Source File

SOURCE=..\Instrument.h
# End Source File
# Begin Source File

SOURCE=..\Machine.h
# End Source File
# Begin Source File

SOURCE=..\MachineInterface.h
# End Source File
# Begin Source File

SOURCE=.\OUT.H
# End Source File
# Begin Source File

SOURCE=..\Player.h
# End Source File
# Begin Source File

SOURCE=..\Plugin.h
# End Source File
# Begin Source File

SOURCE=..\Registry.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\Sampler.h
# End Source File
# Begin Source File

SOURCE=..\Song.h
# End Source File
# Begin Source File

SOURCE=..\SongStructs.h
# End Source File
# Begin Source File

SOURCE=..\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\VstHost.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\in_psycle.rc
# End Source File
# End Group
# End Target
# End Project
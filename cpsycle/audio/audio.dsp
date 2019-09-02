# Microsoft Developer Studio Project File - Name="audio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=audio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "audio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "audio.mak" CFG="audio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "audio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "audio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "audio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\file\src" /I "..\container\src" /I "..\dsp\src" /I "..\lua53\src" /I "..\vst-2.4\vst2.x" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\file\src" /I "..\container\src" /I "..\dsp\src" /I "..\lua53\src" /I "..\vst-2.4\vst2.x" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "audio - Win32 Release"
# Name "audio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\buffer.c
# End Source File
# Begin Source File

SOURCE=.\src\event.c
# End Source File
# Begin Source File

SOURCE=.\src\luaplugin.c
# End Source File
# Begin Source File

SOURCE=.\src\machine.c
# End Source File
# Begin Source File

SOURCE=.\src\machines.c
# End Source File
# Begin Source File

SOURCE=..\dsp\src\operations.c
# End Source File
# Begin Source File

SOURCE=.\src\pattern.c
# End Source File
# Begin Source File

SOURCE=.\src\patterns.c
# End Source File
# Begin Source File

SOURCE=.\src\player.c
# End Source File
# Begin Source File

SOURCE=.\src\plugin.c
# End Source File
# Begin Source File

SOURCE=.\src\plugin_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\plugincatcher.c
# End Source File
# Begin Source File

SOURCE=.\src\sample.c
# End Source File
# Begin Source File

SOURCE=.\src\samples.c
# End Source File
# Begin Source File

SOURCE=.\src\sequence.c
# End Source File
# Begin Source File

SOURCE=.\src\sequencer.c
# End Source File
# Begin Source File

SOURCE=.\src\silentdriver.c
# End Source File
# Begin Source File

SOURCE=.\src\song.c
# End Source File
# Begin Source File

SOURCE=.\src\vstplugin.c
# End Source File
# Begin Source File

SOURCE=.\src\waveio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\buffer.h
# End Source File
# Begin Source File

SOURCE=.\src\event.h
# End Source File
# Begin Source File

SOURCE=.\src\luaplugin.h
# End Source File
# Begin Source File

SOURCE=.\src\machine.h
# End Source File
# Begin Source File

SOURCE=.\src\machines.h
# End Source File
# Begin Source File

SOURCE=.\src\pattern.h
# End Source File
# Begin Source File

SOURCE=.\src\patterns.h
# End Source File
# Begin Source File

SOURCE=.\src\player.h
# End Source File
# Begin Source File

SOURCE=.\src\plugin.h
# End Source File
# Begin Source File

SOURCE=.\src\plugin_interface.h
# End Source File
# Begin Source File

SOURCE=.\src\plugincatcher.h
# End Source File
# Begin Source File

SOURCE=.\src\sample.h
# End Source File
# Begin Source File

SOURCE=.\src\samples.h
# End Source File
# Begin Source File

SOURCE=.\src\sequence.h
# End Source File
# Begin Source File

SOURCE=.\src\sequencer.h
# End Source File
# Begin Source File

SOURCE=.\src\silentdriver.h
# End Source File
# Begin Source File

SOURCE=.\src\song.h
# End Source File
# Begin Source File

SOURCE=.\src\vstplugin.h
# End Source File
# Begin Source File

SOURCE=.\src\waveio.h
# End Source File
# End Group
# End Target
# End Project

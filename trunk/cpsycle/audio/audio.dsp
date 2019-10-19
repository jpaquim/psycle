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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\..\diversalis\src" /I "..\file\src" /I "..\container\src" /I "..\dsp\src" /I "..\lua53\src" /I "..\vst-2.4\vst2.x" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\diversalis\src" /I "..\file\src" /I "..\container\src" /I "..\dsp\src" /I "..\lua53\src" /I "..\vst-2.4\vst2.x" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\buffercontext.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\connections.c
# End Source File
# Begin Source File

SOURCE=.\src\dummy.c
# End Source File
# Begin Source File

SOURCE=.\src\duplicator.c
# End Source File
# Begin Source File

SOURCE=.\src\duplicator2.c
# End Source File
# Begin Source File

SOURCE=.\src\duplicatormap.c
# End Source File
# Begin Source File

SOURCE=.\src\event.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\exclusivelock.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\inputmap.c
# End Source File
# Begin Source File

SOURCE=.\src\instrument.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\instruments.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\kbddriver.c
# End Source File
# Begin Source File

SOURCE=.\src\library.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\luaplugin.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\machine.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\machinefactory.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\machines.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\master.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\mixer.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\dsp\src\operations.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\pattern.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\patterns.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\player.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\plugin.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\plugin_interface.cpp

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\plugincatcher.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\psy2.c
# End Source File
# Begin Source File

SOURCE=.\src\psy3.c
# End Source File
# Begin Source File

SOURCE=.\src\sample.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sampler.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\samples.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sequence.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sequencer.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\silentdriver.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\song.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\vstplugin.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\waveio.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\xminstrument.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\xminstruments.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\buffer.h
# End Source File
# Begin Source File

SOURCE=.\src\buffercontext.h
# End Source File
# Begin Source File

SOURCE=.\src\connections.h
# End Source File
# Begin Source File

SOURCE=.\src\constants.h
# End Source File
# Begin Source File

SOURCE=.\src\dummy.h
# End Source File
# Begin Source File

SOURCE=.\src\duplicator.h
# End Source File
# Begin Source File

SOURCE=.\src\duplicator2.h
# End Source File
# Begin Source File

SOURCE=.\src\duplicatormap.h
# End Source File
# Begin Source File

SOURCE=.\src\event.h
# End Source File
# Begin Source File

SOURCE=.\src\exclusivelock.h
# End Source File
# Begin Source File

SOURCE=.\src\inputmap.h
# End Source File
# Begin Source File

SOURCE=.\src\instrument.h
# End Source File
# Begin Source File

SOURCE=.\src\instruments.h
# End Source File
# Begin Source File

SOURCE=.\src\kbddriver.h
# End Source File
# Begin Source File

SOURCE=.\src\library.h
# End Source File
# Begin Source File

SOURCE=.\src\luaplugin.h
# End Source File
# Begin Source File

SOURCE=.\src\machine.h
# End Source File
# Begin Source File

SOURCE=.\src\machinedefs.h
# End Source File
# Begin Source File

SOURCE=.\src\machinefactory.h
# End Source File
# Begin Source File

SOURCE=.\src\machines.h
# End Source File
# Begin Source File

SOURCE=.\src\master.h
# End Source File
# Begin Source File

SOURCE=.\src\mixer.h
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

SOURCE=.\src\detail\project.h
# End Source File
# Begin Source File

SOURCE=.\src\psy2.h
# End Source File
# Begin Source File

SOURCE=.\src\psy3.h
# End Source File
# Begin Source File

SOURCE=.\src\psycleversion.h
# End Source File
# Begin Source File

SOURCE=.\src\sample.h
# End Source File
# Begin Source File

SOURCE=.\src\sampler.h
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
# Begin Source File

SOURCE=.\src\xminstrument.h
# End Source File
# Begin Source File

SOURCE=.\src\xminstruments.h
# End Source File
# End Group
# End Target
# End Project

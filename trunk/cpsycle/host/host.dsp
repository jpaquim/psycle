# Microsoft Developer Studio Project File - Name="host" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=host - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "host.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "host.mak" CFG="host - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "host - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "host - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "host - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\diversalis\src" /I "..\lua53\src\\" /I "..\ui\src" /I "..\file\src" /I "..\audio\src" /I "..\container\src" /I "..\dsp\src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 lua53.lib ui.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib file.lib winmm.lib container.lib dsp.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\lua53\Release" /libpath:"..\ui\Release" /libpath:"..\file\Release" /libpath:"..\audio\Release" /libpath:"..\container\Release" /libpath:"..\dsp\Release"

!ELSEIF  "$(CFG)" == "host - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\diversalis\src" /I "..\lua53\src\\" /I "..\ui\src" /I "..\file\src" /I "..\audio\src" /I "..\container\src" /I "..\dsp\src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 lua53.lib ui.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib file.lib winmm.lib container.lib dsp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\lua53\Debug" /libpath:"..\ui\Debug" /libpath:"..\file\Debug" /libpath:"..\audio\Debug" /libpath:"..\container\Debug" /libpath:"..\dsp\Debug"

!ENDIF 

# Begin Target

# Name "host - Win32 Release"
# Name "host - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\about.c
# End Source File
# Begin Source File

SOURCE=.\src\clipbox.c
# End Source File
# Begin Source File

SOURCE=.\src\clipbox.h
# End Source File
# Begin Source File

SOURCE=.\src\command.c
# End Source File
# Begin Source File

SOURCE=.\src\config.c
# End Source File
# Begin Source File

SOURCE=.\src\envelopeview.c
# End Source File
# Begin Source File

SOURCE=.\src\file.c
# End Source File
# Begin Source File

SOURCE=.\src\filebar.c
# End Source File
# Begin Source File

SOURCE=.\src\gear.c
# End Source File
# Begin Source File

SOURCE=.\src\greet.c
# End Source File
# Begin Source File

SOURCE=.\src\helpview.c
# End Source File
# Begin Source File

SOURCE=.\src\inputdefiner.c
# End Source File
# Begin Source File

SOURCE=.\src\instrumentsbox.c
# End Source File
# Begin Source File

SOURCE=.\src\instrumentsview.c
# End Source File
# Begin Source File

SOURCE=.\src\linesperbeatbar.c
# End Source File
# Begin Source File

SOURCE=.\src\machinebar.c
# End Source File
# Begin Source File

SOURCE=.\src\machineframe.c
# End Source File
# Begin Source File

SOURCE=.\src\machinesbox.c
# End Source File
# Begin Source File

SOURCE=.\src\machineview.c
# End Source File
# Begin Source File

SOURCE=.\src\mainframe.c
# End Source File
# Begin Source File

SOURCE=.\src\newmachine.c
# End Source File
# Begin Source File

SOURCE=.\src\notemapedit.c
# End Source File
# Begin Source File

SOURCE=.\src\octavebar.c
# End Source File
# Begin Source File

SOURCE=.\src\paramview.c
# End Source File
# Begin Source File

SOURCE=.\src\patternproperties.c
# End Source File
# Begin Source File

SOURCE=.\src\patternview.c
# End Source File
# Begin Source File

SOURCE=.\src\pionoroll.c
# End Source File
# Begin Source File

SOURCE=.\src\playbar.c
# End Source File
# Begin Source File

SOURCE=.\src\playposbar.c
# End Source File
# Begin Source File

SOURCE=.\src\psycle.c
# End Source File
# Begin Source File

SOURCE=.\src\resources\resource.rc
# End Source File
# Begin Source File

SOURCE=.\src\samplerinstrumentview.c
# End Source File
# Begin Source File

SOURCE=.\src\samplesbox.c
# End Source File
# Begin Source File

SOURCE=.\src\samplesview.c
# End Source File
# Begin Source File

SOURCE=.\src\sampulseinstrumentsview.c
# End Source File
# Begin Source File

SOURCE=.\src\sequenceview.c
# End Source File
# Begin Source File

SOURCE=.\src\settingsview.c
# End Source File
# Begin Source File

SOURCE=.\src\skinio.c
# End Source File
# Begin Source File

SOURCE=.\src\slidergroup.c
# End Source File
# Begin Source File

SOURCE=.\src\songbar.c
# End Source File
# Begin Source File

SOURCE=.\src\songproperties.c
# End Source File
# Begin Source File

SOURCE=.\src\songtrackbar.c
# End Source File
# Begin Source File

SOURCE=.\src\stepbox.c
# End Source File
# Begin Source File

SOURCE=.\src\tabbar.c
# End Source File
# Begin Source File

SOURCE=.\src\timebar.c
# End Source File
# Begin Source File

SOURCE=.\src\trackerview.c
# End Source File
# Begin Source File

SOURCE=.\src\undoredo.c
# End Source File
# Begin Source File

SOURCE=.\src\undoredobar.c
# End Source File
# Begin Source File

SOURCE=.\src\volslider.c
# End Source File
# Begin Source File

SOURCE=..\audio\src\vstplugin.c
# End Source File
# Begin Source File

SOURCE=.\src\vumeter.c
# End Source File
# Begin Source File

SOURCE=.\src\wavebox.c
# End Source File
# Begin Source File

SOURCE=.\src\workspace.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\about.h
# End Source File
# Begin Source File

SOURCE=.\src\cmdsnotes.h
# End Source File
# Begin Source File

SOURCE=.\src\command.h
# End Source File
# Begin Source File

SOURCE=.\src\config.h
# End Source File
# Begin Source File

SOURCE=.\src\envelopeview.h
# End Source File
# Begin Source File

SOURCE=.\src\file.h
# End Source File
# Begin Source File

SOURCE=.\src\filebar.h
# End Source File
# Begin Source File

SOURCE=.\src\gear.h
# End Source File
# Begin Source File

SOURCE=.\src\greet.h
# End Source File
# Begin Source File

SOURCE=.\src\helpview.h
# End Source File
# Begin Source File

SOURCE=.\src\inputdefiner.h
# End Source File
# Begin Source File

SOURCE=.\src\instrumentsbox.h
# End Source File
# Begin Source File

SOURCE=.\src\instrumentsview.h
# End Source File
# Begin Source File

SOURCE=.\src\linesperbeatbar.h
# End Source File
# Begin Source File

SOURCE=.\src\machinebar.h
# End Source File
# Begin Source File

SOURCE=.\src\machineframe.h
# End Source File
# Begin Source File

SOURCE=.\src\machinesbox.h
# End Source File
# Begin Source File

SOURCE=.\src\machineview.h
# End Source File
# Begin Source File

SOURCE=.\src\mainframe.h
# End Source File
# Begin Source File

SOURCE=.\src\newmachine.h
# End Source File
# Begin Source File

SOURCE=.\src\notemapedit.h
# End Source File
# Begin Source File

SOURCE=.\src\octavebar.h
# End Source File
# Begin Source File

SOURCE=.\src\paramview.h
# End Source File
# Begin Source File

SOURCE=.\src\patternproperties.h
# End Source File
# Begin Source File

SOURCE=.\src\patternview.h
# End Source File
# Begin Source File

SOURCE=.\src\pianoroll.h
# End Source File
# Begin Source File

SOURCE=.\src\playbar.h
# End Source File
# Begin Source File

SOURCE=.\src\playposbar.h
# End Source File
# Begin Source File

SOURCE=.\src\resources\resource.h
# End Source File
# Begin Source File

SOURCE=..\audio\src\sampler.h
# End Source File
# Begin Source File

SOURCE=.\src\samplerinstrumentview.h
# End Source File
# Begin Source File

SOURCE=.\src\samplesbox.h
# End Source File
# Begin Source File

SOURCE=.\src\samplesview.h
# End Source File
# Begin Source File

SOURCE=.\src\sampulseinstrumentview.h
# End Source File
# Begin Source File

SOURCE=.\src\sequenceview.h
# End Source File
# Begin Source File

SOURCE=.\src\settingsview.h
# End Source File
# Begin Source File

SOURCE=.\src\skincoord.h
# End Source File
# Begin Source File

SOURCE=.\src\skinio.h
# End Source File
# Begin Source File

SOURCE=.\src\slidergroup.h
# End Source File
# Begin Source File

SOURCE=.\src\songbar.h
# End Source File
# Begin Source File

SOURCE=.\src\songproperties.h
# End Source File
# Begin Source File

SOURCE=.\src\songtrackbar.h
# End Source File
# Begin Source File

SOURCE=.\src\stepbox.h
# End Source File
# Begin Source File

SOURCE=.\src\tabbar.h
# End Source File
# Begin Source File

SOURCE=.\src\timebar.h
# End Source File
# Begin Source File

SOURCE=.\src\trackerview.h
# End Source File
# Begin Source File

SOURCE=.\src\undoredo.h
# End Source File
# Begin Source File

SOURCE=.\src\undoredobar.h
# End Source File
# Begin Source File

SOURCE=.\src\volslider.h
# End Source File
# Begin Source File

SOURCE=.\src\vumeter.h
# End Source File
# Begin Source File

SOURCE=.\src\wavebox.h
# End Source File
# Begin Source File

SOURCE=.\src\workspace.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\resources\machine_skin.bmp
# End Source File
# Begin Source File

SOURCE=.\src\resources\pattern_header_skin.bmp
# End Source File
# Begin Source File

SOURCE=.\src\resources\splash_screen.bmp
# End Source File
# Begin Source File

SOURCE=.\src\resources\TbMainKnob.bmp
# End Source File
# End Target
# End Project

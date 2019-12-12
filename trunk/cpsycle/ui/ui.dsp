# Microsoft Developer Studio Project File - Name="ui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak" CFG="ui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ui - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ui - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ui - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\container\src" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\container\src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

# Name "ui - Win32 Release"
# Name "ui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\uiapp.c
# End Source File
# Begin Source File

SOURCE=.\src\uibitmap.c
# End Source File
# Begin Source File

SOURCE=.\src\uibutton.c
# End Source File
# Begin Source File

SOURCE=.\src\uicheckbox.c
# End Source File
# Begin Source File

SOURCE=.\src\uicombobox.c
# End Source File
# Begin Source File

SOURCE=.\src\uicomponent.c
# End Source File
# Begin Source File

SOURCE=.\src\uidef.c
# End Source File
# Begin Source File

SOURCE=.\src\uiedit.c
# End Source File
# Begin Source File

SOURCE=.\src\uieditor.c
# End Source File
# Begin Source File

SOURCE=.\src\uievents.c
# End Source File
# Begin Source File

SOURCE=.\src\uiframe.c
# End Source File
# Begin Source File

SOURCE=.\src\uigraphics.c
# End Source File
# Begin Source File

SOURCE=.\src\uigroupbox.c
# End Source File
# Begin Source File

SOURCE=.\src\uiimage.c
# End Source File
# Begin Source File

SOURCE=.\src\uilabel.c
# End Source File
# Begin Source File

SOURCE=.\src\uilistbox.c
# End Source File
# Begin Source File

SOURCE=.\src\uimenu.c
# End Source File
# Begin Source File

SOURCE=.\src\uinotebook.c
# End Source File
# Begin Source File

SOURCE=.\src\uiprogressbar.c
# End Source File
# Begin Source File

SOURCE=.\src\uislider.c
# End Source File
# Begin Source File

SOURCE=.\src\uistatusbar.c
# End Source File
# Begin Source File

SOURCE=.\src\uiswitch.c
# End Source File
# Begin Source File

SOURCE=.\src\uiterminal.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\uiapp.h
# End Source File
# Begin Source File

SOURCE=.\src\uibitmap.h
# End Source File
# Begin Source File

SOURCE=.\src\uibutton.h
# End Source File
# Begin Source File

SOURCE=.\src\uicheckbox.h
# End Source File
# Begin Source File

SOURCE=.\src\uicombobox.h
# End Source File
# Begin Source File

SOURCE=.\src\uicomponent.h
# End Source File
# Begin Source File

SOURCE=.\src\uidef.h
# End Source File
# Begin Source File

SOURCE=.\src\uiedit.h
# End Source File
# Begin Source File

SOURCE=.\src\uieditor.h
# End Source File
# Begin Source File

SOURCE=.\src\uievents.h
# End Source File
# Begin Source File

SOURCE=.\src\uiframe.h
# End Source File
# Begin Source File

SOURCE=.\src\uigraphics.h
# End Source File
# Begin Source File

SOURCE=.\src\uigroupbox.h
# End Source File
# Begin Source File

SOURCE=.\src\uiimage.h
# End Source File
# Begin Source File

SOURCE=.\src\uilabel.h
# End Source File
# Begin Source File

SOURCE=.\src\uilistbox.h
# End Source File
# Begin Source File

SOURCE=.\src\uimenu.h
# End Source File
# Begin Source File

SOURCE=.\src\uinotebook.h
# End Source File
# Begin Source File

SOURCE=.\src\uiprogressbar.h
# End Source File
# Begin Source File

SOURCE=.\src\uislider.h
# End Source File
# Begin Source File

SOURCE=.\src\uistatusbar.h
# End Source File
# Begin Source File

SOURCE=.\src\uiswitch.h
# End Source File
# Begin Source File

SOURCE=.\src\uiterminal.h
# End Source File
# End Group
# End Target
# End Project

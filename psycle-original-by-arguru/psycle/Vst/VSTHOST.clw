; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CVstRack
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "VSTHOST.h"

ClassCount=4
Class1=CVSTHOSTApp
Class2=CVSTHOSTDlg
Class3=CAboutDlg

ResourceCount=5
Resource1=IDD_VSTRACK
Resource2=IDR_MAINFRAME
Resource3=IDD_VSTHOST_DIALOG
Resource4=IDD_ABOUTBOX
Class4=CVstRack
Resource5=IDR_EDITORMENU (Neutral (Default))

[CLS:CVSTHOSTApp]
Type=0
HeaderFile=VSTHOST.h
ImplementationFile=VSTHOST.cpp
Filter=N

[CLS:CVSTHOSTDlg]
Type=0
HeaderFile=VSTHOSTDlg.h
ImplementationFile=VSTHOSTDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CVSTHOSTDlg

[CLS:CAboutDlg]
Type=0
HeaderFile=VSTHOSTDlg.h
ImplementationFile=VSTHOSTDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_VSTHOST_DIALOG]
Type=1
Class=CVSTHOSTDlg
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LIST1,listbox,1352728833
Control4=IDC_LIST2,listbox,1352728833
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352

[MNU:IDR_EDITORMENU (Neutral (Default))]
Type=1
Class=?
Command1=ID_PROG_RENAME
CommandCount=1

[DLG:IDD_VSTRACK]
Type=1
Class=CVstRack
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LIST1,listbox,1344340225
Control4=IDC_STATIC,static,1342308352
Control5=IDC_TEXT1,static,1342308864
Control6=IDC_SLIDER1,msctls_trackbar32,1342242842
Control7=IDC_STATIC,static,1342308352
Control8=IDC_COMBO2,combobox,1344339971

[CLS:CVstRack]
Type=0
HeaderFile=VstRack.h
ImplementationFile=VstRack.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_COMBO2
VirtualFilter=dWC


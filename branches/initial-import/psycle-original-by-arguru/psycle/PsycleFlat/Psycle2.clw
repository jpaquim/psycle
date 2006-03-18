; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDSoundConfig
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Psycle2.h"
LastPage=0

ClassCount=40
Class1=CPsycle2App
Class3=CMainFrame
Class4=CAboutDlg

ResourceCount=76
Resource1=IDD_DSOUNDCONFIG
Resource2=IDD_INSTRUMENT (Neutral (Default))
Class2=CChildView
Resource3=IDD_SONGPROP
Resource4=IDD_GEAR_TRACKER
Class5=CADriverDialog
Resource5=IDD_PATDLG
Class6=gearPsychosc
Resource6=IDD_WIREDIALOG (Neutral (Default))
Class7=CInfoDlg
Resource7=IDD_OUTPUT
Class8=CWireDlg
Resource8=IDD_GEAR_DELAY
Class9=CControlDesk
Resource9=IDD_ENVDIALOG
Class10=CNewMachine
Resource10=IDD_MASTERDLG
Class11=CGearDistort
Resource11=IDD_GEAR_FILTER
Class12=CMacProp
Resource12=IDR_MACHINEFRAME
Class13=CPatDlg
Resource13=IDD_PSYBAR2 (English (U.S.))
Class14=CInstrumentEditor
Resource14=IDD_VSTRACK
Resource15=IDR_MAINFRAME (English (U.S.))
Class15=CGearTracker
Resource16=IDD_GEAR_FLANGER
Class16=CGearDelay
Resource17=IDD_NEWVAL
Class17=CEnvDialog
Resource18=IDD_HELLO (English (U.S.))
Class18=CRecorderDlg
Resource19=IDD_SAVESONG
Class19=CPtnDialog
Resource20=IDD_WAVEOUTCONFIG
Class20=CSaveDlg
Resource21=IDD_PSYBAR (Neutral (Default))
Class21=CGearfilter
Resource22=IDD_IBROWSE
Class22=CIBrowseDlg
Resource23=IDD_CONFIG
Class23=CGreetDialog
Resource24=IDD_GEAR_GAINER
Class24=CMidiDlg
Class25=CSkinDlg
Resource25=IDD_NEWMACHINE
Class26=CSongpDlg
Resource26=IDD_GEAR_PSYCHOSC
Class27=CGearGainer
Resource27=IDD_GREETS
Resource28=IDD_PATNAVDIALOG
Resource29=IDD_SEQUENCER (English (U.S.))
Class28=CGearFlanger
Class29=CGearPlugin
Resource30=IDR_WAVEFRAME
Class30=CSequencerDlg
Resource31=IDD_INSTRUMENT
Class31=CMasterDlg
Resource32=IDD_MACPROP
Class32=CFileDialogEx
Class33=CFrameMachine
Resource33=IDD_PSYBAR (English (U.S.))
Resource34=IDD_ABOUTBOX (English (U.S.))
Class34=CNewVal
Resource35=IDD_GEARDISTORT
Resource36=IDD_INFO
Class35=CVSTHOSTDlg
Resource37=IDD_WIREDIALOG
Class36=CVstRack
Resource38=IDD_RECORDER
Class37=CGearVst
Resource39=IDR_VSTFRAME
Resource40=IDD_GEAR_FILTER (Neutral (Default))
Resource41=IDD_SEQUENCER
Resource42=IDD_RECORDER (Neutral (Default))
Resource43=IDD_CONFIG (Spanish (Mexican))
Resource44=IDD_GEAR_DELAY (Neutral (Default))
Resource45=IDD_GREETS (Neutral (Default))
Resource46=IDD_ABOUTBOX
Resource47=IDD_GEAR_TRACKER (Neutral (Default))
Resource48=IDD_NEWMACHINE (Neutral (Default))
Resource49=IDD_MACPROP (Neutral (Default))
Resource50=IDD_WAVEOUTCONFIG (Neutral (Default))
Resource51=IDD_GEAR_GAINER (Neutral (Default))
Resource52=IDD_PATDLG (Neutral (Default))
Resource53=IDD_VSTHOST_DIALOG (Neutral (Default))
Resource54=IDD_IBROWSE (Neutral (Default))
Resource55=IDD_PATNAVDIALOG (Neutral (Default))
Resource56=IDD_WAVEOUTCONFIG (Neutral (Sys. Default))
Resource57=IDD_SAVESONG (Neutral (Default))
Resource58=IDD_NEWVAL (Neutral (Default))
Resource59=IDD_MASTERDLG (Neutral (Default))
Resource60=IDD_VSTRACK (Neutral (Default))
Resource61=IDR_VSTFRAME (Neutral (Default))
Resource62=IDD_CONFIG (Neutral (Default))
Resource63=IDR_MACHINEFRAME (Neutral (Default))
Resource64=IDD_GEARVST (Neutral (Default))
Resource65=IDD_GEAR_DELAY (Spanish (Mexican))
Resource66=IDD_ENVDIALOG (Spanish (Mexican))
Resource67=IDD_GEAR_FLANGER (Neutral (Default))
Resource68=IDD_AUDIOCONFIG (Neutral (Default))
Resource69=IDD_PSYBAR2 (Neutral (Default))
Resource70=IDD_GEAR_PSYCHOSC (Neutral (Default))
Resource71=IDD_ENVDIALOG (Neutral (Default))
Resource72=IDR_WAVEFRAME (Neutral (Default))
Class38=DSoundConfig
Resource73=IDD_GEARDISTORT (Neutral (Default))
Resource74=IDD_INFO (Neutral (Default))
Class39=CDSoundConfig
Resource75=IDD_SONGPROP (Neutral (Default))
Class40=COutputDlg
Resource76=IDR_MAINFRAME

[CLS:CPsycle2App]
Type=0
HeaderFile=Psycle2.h
ImplementationFile=Psycle2.cpp
Filter=N
LastObject=CPsycle2App
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CChildView]
Type=0
HeaderFile=ChildView.h
ImplementationFile=ChildView.cpp
Filter=N
LastObject=ID_RECORDB
BaseClass=CWnd 
VirtualFilter=WC

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame
BaseClass=CFrameWnd
VirtualFilter=fWC




[CLS:CAboutDlg]
Type=0
HeaderFile=Psycle2.cpp
ImplementationFile=Psycle2.cpp
Filter=D
LastObject=CAboutDlg
BaseClass=CDialog
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=8
Control1=IDOK,button,1342373889
Control2=IDC_STATIC,static,1342308865
Control3=IDC_STATIC,static,1342179342
Control4=IDC_STATIC,static,1342308353
Control5=IDC_STATIC,static,1342177284
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342177284
Control8=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_LOADSONG
Command3=ID_FILE_SAVESONG
Command4=ID_FILE_SONGPROPERTIES
Command5=ID_APP_EXIT
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_VIEW_TOOLBAR
Command11=ID_VIEW_STATUS_BAR
Command12=ID_MACHINEVIEW
Command13=ID_PATTERNVIEW
Command14=ID_VIEW_INSTRUMENTEDITOR
Command15=ID_SHOWPSEQ
Command16=ID_NEWMACHINE
Command17=ID_CONFIGURATION_AUDIODRIVER
Command18=ID_CONFIGURATION_FREEAUDIO
Command19=ID_CONFIGURATION_SETTINGS_BACKGROUNDSKIN
Command20=ID_CONFIGURATION_KEYBOARDLAYOUT_STANDARD
Command21=ID_CONFIGURATION_KEYBOARDLAYOUT_FRENCH
Command22=ID_HELP_PSYCLEENVIROMENTINFO
Command23=ID_VST2MANAGER_VST2PLUGINMANAGER
Command24=ID_PSYHELP
Command25=ID_ABOUTPSYCLE
Command26=ID_HELP_SALUDOS
CommandCount=26

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_CONFIGURATION_AUDIODRIVER
Command2=ID_FILE_NEW
Command3=ID_FILE_LOADSONG
Command4=ID_FILE_SAVESONG
Command5=ID_PSYHELP
Command6=ID_MACHINEVIEW
Command7=ID_PATTERNVIEW
Command8=ID_SHOWIED
Command9=ID_SHOWPSEQ
Command10=ID_NEWMACHINE
Command11=ID_BARPLAY
Command12=ID_BARSTOP
CommandCount=12

[TB:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_LOADSONG
Command3=ID_FILE_SAVESONG
Command4=ID_BARPLAY
Command5=ID_BARSTOP
Command6=ID_RECORDB
Command7=ID_PSYHELP
Command8=ID_MACHINEVIEW
Command9=ID_PATTERNVIEW
Command10=ID_SHOWIED
Command11=ID_NEWMACHINE
Command12=ID_ABOUTPSYCLE
CommandCount=12

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_LOADSONG
Command3=ID_FILE_SAVESONG
Command4=ID_FILE_SONGPROPERTIES
Command5=ID_APP_EXIT
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_VIEW_TOOLBAR
Command11=ID_VIEW_STATUS_BAR
Command12=ID_MACHINEVIEW
Command13=ID_PATTERNVIEW
Command14=ID_VIEW_INSTRUMENTEDITOR
Command15=ID_SHOWPSEQ
Command16=ID_NEWMACHINE
Command17=ID_CONFIGURATION_AUDIODRIVER
Command18=ID_CONFIGURATION_FREEAUDIO
Command19=ID_CONFIGURATION_SETTINGS_BACKGROUNDSKIN
Command20=ID_CONFIGURATION_KEYBOARDLAYOUT_STANDARD
Command21=ID_CONFIGURATION_KEYBOARDLAYOUT_FRENCH
Command22=ID_HELP_PSYCLEENVIROMENTINFO
Command23=ID_VST2MANAGER_VST2PLUGINMANAGER
Command24=ID_PSYHELP
Command25=ID_ABOUTPSYCLE
Command26=ID_HELP_SALUDOS
CommandCount=26

[ACL:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=CMainFrame
Command1=ID_CONFIGURATION_AUDIODRIVER
Command2=ID_FILE_NEW
Command3=ID_FILE_LOADSONG
Command4=ID_FILE_SAVESONG
Command5=ID_PSYHELP
Command6=ID_MACHINEVIEW
Command7=ID_PATTERNVIEW
Command8=ID_SHOWIED
Command9=ID_SHOWPSEQ
Command10=ID_NEWMACHINE
Command11=ID_BARPLAY
Command12=ID_BARSTOP
CommandCount=12

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CAboutDlg
ControlCount=8
Control1=IDOK,button,1342373889
Control2=IDC_STATIC,static,1342308865
Control3=IDC_STATIC,static,1342179342
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342177284
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342177284
Control8=IDC_STATIC,static,1342308352

[DLG:IDD_WAVEOUTCONFIG]
Type=1
Class=CADriverDialog
ControlCount=15
Control1=IDOK,button,1342242817
Control2=IDC_CONFIG_DEVICE,combobox,1344339971
Control3=IDC_CONFIG_SAMPLERATE,combobox,1344339971
Control4=IDC_CONFIG_DITHER,button,1342242819
Control5=IDC_CONFIG_BUFNUM,edit,1350631552
Control6=IDC_CONFIG_BUFNUM_SPIN,msctls_updown32,1342177462
Control7=IDC_CONFIG_BUFSIZE,edit,1350631552
Control8=IDC_CONFIG_BUFSIZE_SPIN,msctls_updown32,1342177462
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308864
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_CONFIG_LATENCY,static,1342308352
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,static,1342177294

[CLS:CADriverDialog]
Type=0
HeaderFile=ADriverDialog.h
ImplementationFile=ADriverDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CADriverDialog

[DLG:IDD_GEAR_PSYCHOSC]
Type=1
Class=gearPsychosc
ControlCount=16
Control1=IDC_STATIC,button,1342177287
Control2=IDC_SLIDER1,msctls_trackbar32,1342242827
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SLIDER2,msctls_trackbar32,1342242827
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,button,1342177287
Control7=IDC_SLIDER3,msctls_trackbar32,1342242827
Control8=IDC_STATIC,static,1342308352
Control9=IDC_SLIDER4,msctls_trackbar32,1342242827
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342177294
Control12=IDC_STATIC,static,1342308353
Control13=IDC_STATIC1,static,1342308865
Control14=IDC_STATIC2,static,1342308865
Control15=IDC_STATIC3,static,1342308865
Control16=IDC_STATIC4,static,1342308865

[CLS:gearPsychosc]
Type=0
HeaderFile=gearPsychosc.h
ImplementationFile=gearPsychosc.cpp
BaseClass=CDialog
Filter=D
LastObject=gearPsychosc
VirtualFilter=dWC

[DLG:IDD_INFO]
Type=1
Class=CInfoDlg
ControlCount=23
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,button,1342177287
Control3=IDC_STATIC,static,1342308352
Control4=IDC_MACHINELIST,SysListView32,1350666241
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,static,1342308865
Control7=IDC_CPUIDLE_LABEL,static,1342312961
Control8=IDC_STATIC,static,1342308865
Control9=IDC_MCPULABEL,static,1342312961
Control10=IDC_STATIC,static,1342308865
Control11=IDC_CPUL,static,1342312961
Control12=IDC_STATIC,static,1342308865
Control13=IDC_MCPULABEL2,static,1342312961
Control14=IDC_STATIC,static,1342177294
Control15=IDC_STATIC,button,1342177287
Control16=IDC_STATIC,static,1342308865
Control17=IDC_MEM_RESO,static,1342312961
Control18=IDC_STATIC,static,1342308865
Control19=IDC_MEM_RESO2,static,1342312961
Control20=IDC_STATIC,static,1342308865
Control21=IDC_MEM_RESO3,static,1342312961
Control22=IDC_STATIC,static,1342308865
Control23=IDC_MEM_RESO4,static,1342312961

[CLS:CInfoDlg]
Type=0
HeaderFile=InfoDlg.h
ImplementationFile=InfoDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CInfoDlg

[DLG:IDD_WIREDIALOG]
Type=1
Class=CWireDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308353
Control2=IDC_SLIDER1,msctls_trackbar32,1342242825
Control3=IDC_BUTTON1,button,1342242816
Control4=IDC_STATIC1,static,1342308353

[CLS:CWireDlg]
Type=0
HeaderFile=WireDlg.h
ImplementationFile=WireDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CWireDlg
VirtualFilter=dWC

[CLS:CControlDesk]
Type=0
HeaderFile=ControlDesk.h
ImplementationFile=ControlDesk.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_PLAYDISPLAY2
VirtualFilter=dWC

[DLG:IDD_NEWMACHINE]
Type=1
Class=CNewMachine
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_TREE1,SysTreeView32,1350631559
Control4=IDC_STATIC,static,1342308864
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_DESCLABEL,static,1342312448
Control9=IDC_VERLABEL,static,1342312448
Control10=IDC_STATIC,static,1342308352
Control11=IDC_NAMELABEL,static,1342312448

[CLS:CNewMachine]
Type=0
HeaderFile=NewMachine.h
ImplementationFile=NewMachine.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_TREE1
VirtualFilter=dWC

[DLG:IDD_GEARDISTORT]
Type=1
Class=CGearDistort
ControlCount=20
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SLIDER1,msctls_trackbar32,1342242827
Control5=IDC_SLIDER2,msctls_trackbar32,1342242827
Control6=IDC_STATIC,static,1342308353
Control7=IDC_STATIC,static,1342308353
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308353
Control10=IDC_STATIC,static,1342308352
Control11=IDC_SLIDER3,msctls_trackbar32,1342242827
Control12=IDC_SLIDER4,msctls_trackbar32,1342242827
Control13=IDC_STATIC,static,1342308353
Control14=IDC_STATIC,static,1342308353
Control15=IDC_BUTTON1,button,1342242816
Control16=IDC_BUTTON3,button,1342242816
Control17=IDC_BUTTON2,button,1342242816
Control18=IDC_BUTTON4,button,1342242816
Control19=IDC_STATIC,static,1342177294
Control20=IDC_STATIC,static,1342308353

[CLS:CGearDistort]
Type=0
HeaderFile=GearDistort.h
ImplementationFile=GearDistort.cpp
BaseClass=CDialog
Filter=D
LastObject=CGearDistort
VirtualFilter=dWC

[DLG:IDD_MACPROP]
Type=1
Class=CMacProp
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308865
Control4=IDC_EDIT1,edit,1350631552
Control5=IDC_BUTTON1,button,1342242816
Control6=IDC_CHECK1,button,1342242819

[CLS:CMacProp]
Type=0
HeaderFile=MacProp.h
ImplementationFile=MacProp.cpp
BaseClass=CDialog
Filter=D
LastObject=CMacProp
VirtualFilter=dWC

[DLG:IDD_PATDLG]
Type=1
Class=CPatDlg
ControlCount=7
Control1=IDOK,button,1342242816
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308864
Control4=IDC_EDIT1,edit,1350631552
Control5=IDC_EDIT2,edit,1350631552
Control6=IDC_SPIN1,msctls_updown32,1342177462
Control7=IDC_STATIC,static,1342308864

[CLS:CPatDlg]
Type=0
HeaderFile=PatDlg.h
ImplementationFile=PatDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CPatDlg
VirtualFilter=dWC

[DLG:IDD_INSTRUMENT]
Type=1
Class=CInstrumentEditor
ControlCount=64
Control1=IDC_STATIC,static,1342308866
Control2=IDC_LAYER1,button,1342181385
Control3=IDC_LAYER2,button,1342181385
Control4=IDC_LAYER3,button,1342181385
Control5=IDC_LAYER4,button,1342181385
Control6=IDC_LAYER5,button,1342181385
Control7=IDC_LAYER6,button,1342181385
Control8=IDC_LAYER7,button,1342181385
Control9=IDC_LAYER8,button,1342181385
Control10=IDC_LAYER9,button,1342181385
Control11=IDC_LAYER10,button,1342181385
Control12=IDC_LAYER11,button,1342181385
Control13=IDC_LAYER12,button,1342181385
Control14=IDC_LAYER13,button,1342181385
Control15=IDC_LAYER14,button,1342181385
Control16=IDC_LAYER15,button,1342181385
Control17=IDC_LAYER16,button,1342181385
Control18=IDC_STATIC,button,1342177287
Control19=IDC_WAVENAME,edit,1350631552
Control20=IDC_LAYERN,static,1350697473
Control21=IDC_STATIC,static,1342308866
Control22=IDC_STATIC,static,1342312961
Control23=IDC_BUTTON2,button,1342242816
Control24=IDC_BUTTON4,button,1342242816
Control25=IDC_BUTTON5,button,1342242816
Control26=IDC_BUTTON6,button,1342242816
Control27=IDC_SLIDER1,msctls_trackbar32,1342242840
Control28=IDC_VOLABEL,static,1342312961
Control29=IDC_STATIC,static,1342308866
Control30=IDC_INSTNUMBER,static,1350697473
Control31=IDC_STATIC,static,1342308866
Control32=IDC_STEREO,static,1342312961
Control33=IDC_STATIC,static,1342308866
Control34=IDC_LOOPTYPE,static,1342312961
Control35=IDC_LOOPOFF,button,1342242816
Control36=IDC_LOOPFORWARD,button,1342242816
Control37=IDC_STATIC,static,1342308865
Control38=IDC_LOOPSTART,static,1350697473
Control39=IDC_LOOPEND,static,1350697473
Control40=IDC_STATIC,static,1342308865
Control41=IDC_BUTTON11,button,1476460544
Control42=IDC_STATUS,static,1342312961
Control43=IDC_STATIC,static,1342308866
Control44=IDC_WAVELENGTH,static,1350697473
Control45=IDC_INSTNAME,edit,1350631552
Control46=IDC_BUTTON7,button,1342242816
Control47=IDC_BUTTON8,button,1342242816
Control48=IDC_STATIC,button,1342178055
Control49=IDC_BUTTON12,button,1342242816
Control50=IDC_BUTTON13,button,1342242816
Control51=IDC_STATIC,static,1342308864
Control52=IDC_NNA_COMBO,combobox,1344339971
Control53=IDC_STATIC,static,1342308353
Control54=IDC_PANSLIDER,msctls_trackbar32,1342242840
Control55=IDC_RPAN,button,1342242819
Control56=IDC_RCUT,button,1342242819
Control57=IDC_RRES,button,1342242819
Control58=IDC_CHECK4,button,1342242819
Control59=IDC_LOOPEDIT,edit,1342251008
Control60=IDC_STATIC,static,1342308352
Control61=IDC_STATIC,button,1342177287
Control62=IDC_SLIDER2,msctls_trackbar32,1342242840
Control63=IDC_VOLABEL2,static,1342312961
Control64=IDC_STATIC,static,1342308866

[CLS:CInstrumentEditor]
Type=0
HeaderFile=InstrumentEditor.h
ImplementationFile=InstrumentEditor.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_SLIDER2
VirtualFilter=dWC

[DLG:IDD_GEAR_TRACKER]
Type=1
Class=CGearTracker
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_TRACKLABEL2,static,1350697473
Control4=IDC_STATIC,static,1342308865
Control5=IDC_TRACKSLIDER2,msctls_trackbar32,1342242821
Control6=IDC_STATIC,static,1342177294
Control7=IDC_STATIC,static,1342308865
Control8=IDC_STATIC,static,1342308865
Control9=IDC_COMBO1,combobox,1344339971
Control10=IDC_STATIC,static,1342308864

[CLS:CGearTracker]
Type=0
HeaderFile=GearTracker.h
ImplementationFile=GearTracker.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_COMBO1
VirtualFilter=dWC

[DLG:IDD_GEAR_DELAY]
Type=1
Class=CGearDelay
ControlCount=42
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308865
Control3=IDC_SLIDER_TIME,msctls_trackbar32,1342242821
Control4=IDC_LABEL_LTIME,static,1342312961
Control5=IDC_STATIC,static,1342308865
Control6=IDC_SLIDER2,msctls_trackbar32,1342242821
Control7=IDC_F_LABEL_L,static,1342312961
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308865
Control10=IDC_SLIDER_TIME2,msctls_trackbar32,1342242821
Control11=IDC_LABEL_LTIME2,static,1342312961
Control12=IDC_STATIC,static,1342308865
Control13=IDC_SLIDER3,msctls_trackbar32,1342242821
Control14=IDC_F_LABEL_R,static,1342312961
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_LABEL_MSL,static,1342312961
Control18=IDC_STATIC,static,1342308352
Control19=IDC_LABEL_MSR,static,1342312961
Control20=IDC_STATIC,static,1342308352
Control21=IDC_BUTTON1,button,1342242816
Control22=IDC_BUTTON3,button,1342242816
Control23=IDC_BUTTON14,button,1342242816
Control24=IDC_BUTTON15,button,1342242816
Control25=IDC_BUTTON16,button,1342242816
Control26=IDC_BUTTON17,button,1342242816
Control27=IDC_STATIC,static,1342308352
Control28=IDC_BUTTON11,button,1342242816
Control29=IDC_BUTTON18,button,1342242816
Control30=IDC_BUTTON19,button,1342242816
Control31=IDC_BUTTON20,button,1342242816
Control32=IDC_BUTTON21,button,1342242816
Control33=IDC_BUTTON22,button,1342242816
Control34=IDC_STATIC,static,1342308352
Control35=IDC_STATIC,static,1342177294
Control36=IDC_STATIC,button,1342177287
Control37=IDC_STATIC,static,1342308865
Control38=IDC_DRYSLIDER,msctls_trackbar32,1342242840
Control39=IDC_DRYLABEL,static,1342312961
Control40=IDC_STATIC,static,1342308865
Control41=IDC_WETSLIDER,msctls_trackbar32,1342242840
Control42=IDC_WETLABEL,static,1342312961

[CLS:CGearDelay]
Type=0
HeaderFile=GearDelay.h
ImplementationFile=GearDelay.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_WETSLIDER
VirtualFilter=dWC

[DLG:IDD_ENVDIALOG]
Type=1
Class=CEnvDialog
ControlCount=41
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_AMPFRAME,static,1350565894
Control4=IDC_SLIDER1,msctls_trackbar32,1342242840
Control5=IDC_STATIC,static,1342308866
Control6=IDC_SLIDER2,msctls_trackbar32,1342242840
Control7=IDC_STATIC,static,1342308866
Control8=IDC_SLIDER3,msctls_trackbar32,1342242840
Control9=IDC_STATIC,static,1342308866
Control10=IDC_SLIDER4,msctls_trackbar32,1342242840
Control11=IDC_STATIC,static,1342308866
Control12=IDC_A_A_LABEL,static,1342308864
Control13=IDC_A_D_LABEL,static,1342308864
Control14=IDC_A_R_LABEL,static,1342308864
Control15=IDC_A_S_LABEL,static,1342308864
Control16=IDC_STATIC,button,1342177287
Control17=IDC_AMPFRAME2,static,1350565894
Control18=IDC_F_SLIDER1,msctls_trackbar32,1342242840
Control19=IDC_STATIC,static,1342308866
Control20=IDC_F_SLIDER2,msctls_trackbar32,1342242840
Control21=IDC_STATIC,static,1342308866
Control22=IDC_F_SLIDER3,msctls_trackbar32,1342242840
Control23=IDC_STATIC,static,1342308866
Control24=IDC_F_SLIDER4,msctls_trackbar32,1342242840
Control25=IDC_STATIC,static,1342308866
Control26=IDC_F_A_LABEL,static,1342308864
Control27=IDC_F_D_LABEL,static,1342308864
Control28=IDC_F_R_LABEL,static,1342308864
Control29=IDC_F_S_LABEL,static,1342308864
Control30=IDC_STATIC,button,1342177287
Control31=IDC_STATIC,static,1342308866
Control32=IDC_SLIDER9,msctls_trackbar32,1342242840
Control33=IDC_CUTOFF_LBL,static,1342308864
Control34=IDC_STATIC,static,1342308866
Control35=IDC_SLIDER10,msctls_trackbar32,1342242840
Control36=IDC_LABELQ,static,1342308864
Control37=IDC_STATIC,static,1342308865
Control38=IDC_COMBO1,combobox,1344339971
Control39=IDC_STATIC,static,1342308866
Control40=IDC_ENVELOPE,msctls_trackbar32,1342242840
Control41=IDC_A_A_LABEL7,static,1342308864

[CLS:CEnvDialog]
Type=0
HeaderFile=EnvDialog.h
ImplementationFile=EnvDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_SLIDER1
VirtualFilter=dWC

[DLG:IDD_RECORDER]
Type=1
Class=CRecorderDlg
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_BUTTON1,button,1342242816
Control5=IDC_BUTTON2,button,1342242816
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308865

[CLS:CRecorderDlg]
Type=0
HeaderFile=RecorderDlg.h
ImplementationFile=RecorderDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CRecorderDlg

[CLS:CPtnDialog]
Type=0
HeaderFile=PtnDialog.h
ImplementationFile=PtnDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_LIST1
VirtualFilter=dWC

[DLG:IDD_PATNAVDIALOG]
Type=1
Class=CPtnDialog
ControlCount=5
Control1=IDOK,button,1476460545
Control2=IDC_LIST1,SysListView32,1350631429
Control3=IDC_PNLABEL,static,1342312961
Control4=IDC_CREATEB,button,1342242816
Control5=IDC_STATIC,static,1342308864

[DLG:IDD_SAVESONG]
Type=1
Class=CSaveDlg
ControlCount=3
Control1=IDC_PROGRESS1,msctls_progress32,1350565888
Control2=IDC_STATIC,static,1342308865
Control3=IDC_STATIC,static,1342312961

[CLS:CSaveDlg]
Type=0
HeaderFile=SaveDlg.h
ImplementationFile=SaveDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CSaveDlg
VirtualFilter=dWC

[DLG:IDD_GEAR_FILTER]
Type=1
Class=CGearfilter
ControlCount=14
Control1=IDC_COMBO1,combobox,1344339971
Control2=IDC_STATIC,static,1342308865
Control3=IDC_SLIDER1,msctls_trackbar32,1342242840
Control4=IDC_STATIC,static,1342308865
Control5=IDC_SLIDER2,msctls_trackbar32,1342242840
Control6=IDC_STATIC,static,1342308865
Control7=IDC_SLIDER3,msctls_trackbar32,1342242840
Control8=IDC_STATIC,static,1342308865
Control9=IDC_SLIDER4,msctls_trackbar32,1342242840
Control10=IDC_STATIC,static,1342308865
Control11=IDC_STATIC,button,1342177287
Control12=IDC_STATIC,button,1342177287
Control13=IDC_SLIDER5,msctls_trackbar32,1342242840
Control14=IDC_STATIC,static,1342308865

[CLS:CGearfilter]
Type=0
HeaderFile=Gearfilter.h
ImplementationFile=Gearfilter.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CGearfilter

[DLG:IDD_IBROWSE]
Type=1
Class=CIBrowseDlg
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_LIST1,listbox,1352728833
Control3=IDC_STATIC,static,1342308352
Control4=IDC_BUTTON1,button,1342242816
Control5=IDC_STATIC,button,1342177287

[CLS:CIBrowseDlg]
Type=0
HeaderFile=IBrowseDlg.h
ImplementationFile=IBrowseDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CIBrowseDlg
VirtualFilter=dWC

[DLG:IDD_GREETS]
Type=1
Class=CGreetDialog
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,button,1342177287
Control5=IDC_LIST1,listbox,1352728835

[CLS:CGreetDialog]
Type=0
HeaderFile=GreetDialog.h
ImplementationFile=GreetDialog.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CGreetDialog

[DLG:IDD_HELLO (English (U.S.))]
Type=1
Class=?
ControlCount=1
Control1=IDC_STATIC,static,1342308352

[DLG:IDD_PSYBAR (English (U.S.))]
Type=1
Class=?
ControlCount=11
Control1=IDC_STATIC,static,1342308865
Control2=IDC_BUTTON1,button,1342242816
Control3=IDC_BPMLABEL,static,1342312961
Control4=IDC_BUTTON3,button,1342242816
Control5=IDC_BUTTON14,button,1342242816
Control6=IDC_BUTTON15,button,1342242816
Control7=IDC_LVUM,static,1342181380
Control8=IDC_RVUM,static,1342181380
Control9=IDC_STATIC,static,1342308352
Control10=IDC_FRAMECLIP,static,1342181380
Control11=IDC_CLIPBUT,button,1342242816

[CLS:CMidiDlg]
Type=0
HeaderFile=MidiDlg.h
ImplementationFile=MidiDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CMidiDlg

[DLG:IDD_CONFIG]
Type=1
Class=CSkinDlg
ControlCount=29
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308864
Control5=IDC_EDIT1,edit,1350631552
Control6=IDC_BROWSE_MV,button,1342242816
Control7=IDC_CHECK1,button,1342242819
Control8=IDC_STATIC,static,1342308864
Control9=IDC_EDIT3,edit,1350631552
Control10=IDC_BROWSE_PV,button,1342242816
Control11=IDC_CHECK2,button,1342242819
Control12=IDC_BUTTON1,button,1342242816
Control13=IDC_BUTTON3,button,1342242816
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,static,1342308865
Control16=IDC_BUTTON2,button,1342242816
Control17=IDC_STATIC,static,1342308865
Control18=IDC_BUTTON4,button,1342242816
Control19=IDC_STATIC,static,1342308865
Control20=IDC_BUTTON5,button,1342242816
Control21=IDC_STATIC,button,1342177287
Control22=IDC_STATIC,static,1342308865
Control23=IDC_BUTTON23,button,1342242816
Control24=IDC_STATIC,static,1342308865
Control25=IDC_BUTTON24,button,1342242816
Control26=IDC_STATIC,static,1342308865
Control27=IDC_BUTTON25,button,1342242816
Control28=IDC_STATIC,static,1342308865
Control29=IDC_BUTTON26,button,1342242816

[CLS:CSkinDlg]
Type=0
HeaderFile=SkinDlg.h
ImplementationFile=SkinDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CSkinDlg
VirtualFilter=dWC

[DLG:IDD_SONGPROP]
Type=1
Class=CSongpDlg
ControlCount=7
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT1,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT3,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_EDIT4,edit,1352728644
Control7=IDC_BUTTON1,button,1342242816

[CLS:CSongpDlg]
Type=0
HeaderFile=SongpDlg.h
ImplementationFile=SongpDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_EDIT4
VirtualFilter=dWC

[DLG:IDD_GEAR_GAINER]
Type=1
Class=CGearGainer
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDC_SLIDER1,msctls_trackbar32,1342242827
Control3=IDC_VOLABEL,static,1342312961

[CLS:CGearGainer]
Type=0
HeaderFile=GearGainer.h
ImplementationFile=GearGainer.cpp
BaseClass=CDialog
Filter=D
LastObject=CGearGainer
VirtualFilter=dWC

[DLG:IDD_PSYBAR2 (English (U.S.))]
Type=1
Class=?
ControlCount=17
Control1=IDC_STATIC,static,1342308352
Control2=IDC_SSCOMBO2,combobox,1344339971
Control3=IDC_STATIC,static,1342308352
Control4=IDC_BAR_COMBOGEN,combobox,1344339971
Control5=IDC_STATIC,static,1342308352
Control6=IDC_BAR_COMBOINS,combobox,1344339971
Control7=IDC_STATIC,static,1342308354
Control8=IDC_TRACKCOMBO,combobox,1344339971
Control9=IDC_LOADWAVE,button,1342242816
Control10=IDC_EDITWAVE,button,1342242816
Control11=IDC_WAVEBUT,button,1342242816
Control12=IDC_B_INCGEN,button,1342242816
Control13=IDC_B_DECGEN,button,1342242816
Control14=IDC_B_INCWAV,button,1342242816
Control15=IDC_B_DECWAV,button,1342242816
Control16=IDC_STATIC,static,1342177284
Control17=IDC_STATIC,static,1342177284

[DLG:IDD_GEAR_FLANGER]
Type=1
Class=CGearFlanger
ControlCount=31
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_STATIC,static,1342308864
Control4=IDC_SLIDER1,msctls_trackbar32,1342242840
Control5=IDC_LABEL1,static,1342312961
Control6=IDC_STATIC,static,1342308864
Control7=IDC_SLIDER2,msctls_trackbar32,1342242840
Control8=IDC_LABEL2,static,1342312961
Control9=IDC_STATIC,static,1342308864
Control10=IDC_SLIDER3,msctls_trackbar32,1342242840
Control11=IDC_LABEL3,static,1342312961
Control12=IDC_STATIC,static,1342308864
Control13=IDC_SLIDER4,msctls_trackbar32,1342242840
Control14=IDC_LABEL4,static,1342312961
Control15=IDC_STATIC,button,1342177287
Control16=IDC_STATIC,static,1342308864
Control17=IDC_SLIDER5,msctls_trackbar32,1342242840
Control18=IDC_LABEL5,static,1342312961
Control19=IDC_STATIC,static,1342308864
Control20=IDC_SLIDER6,msctls_trackbar32,1342242840
Control21=IDC_LABEL6,static,1342312961
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,static,1342308865
Control24=IDC_DRYSLIDER,msctls_trackbar32,1342242840
Control25=IDC_DRYLABEL,static,1342312961
Control26=IDC_STATIC,static,1342308865
Control27=IDC_WETSLIDER,msctls_trackbar32,1342242840
Control28=IDC_WETLABEL,static,1342312961
Control29=IDC_STATIC,static,1342177294
Control30=IDC_STATIC,static,1342308352
Control31=IDC_PRESETCOMBO,combobox,1344339971

[CLS:CGearFlanger]
Type=0
HeaderFile=GearFlanger.h
ImplementationFile=GearFlanger.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_PRESETCOMBO
VirtualFilter=dWC

[CLS:CGearPlugin]
Type=0
HeaderFile=GearPlugin.h
ImplementationFile=GearPlugin.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_PARLIST
VirtualFilter=dWC

[CLS:CSequencerDlg]
Type=0
HeaderFile=SequencerDlg.h
ImplementationFile=SequencerDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CSequencerDlg
VirtualFilter=dWC

[DLG:IDD_MASTERDLG]
Type=1
Class=CMasterDlg
ControlCount=2
Control1=IDC_SLIDER1,msctls_trackbar32,1342242840
Control2=IDC_DBLEVEL,static,1342312961

[CLS:CMasterDlg]
Type=0
HeaderFile=MasterDlg.h
ImplementationFile=MasterDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_SLIDER1
VirtualFilter=dWC

[MNU:IDR_WAVEFRAME]
Type=1
Class=?
Command1=ID_SELECTION_FADEIN
Command2=ID_SELECTION_FADEOUT
Command3=ID_SELECCION_ZOOM
Command4=ID_SELECCION_ZOOMOUT
CommandCount=4

[CLS:CFileDialogEx]
Type=0
HeaderFile=FileDialogEx1.h
ImplementationFile=FileDialogEx1.cpp
BaseClass=CFileDialog
Filter=D

[CLS:CFrameMachine]
Type=0
HeaderFile=FrameMachine.h
ImplementationFile=FrameMachine.cpp
BaseClass=CFrameWnd
Filter=T
VirtualFilter=fWC
LastObject=CFrameMachine

[MNU:IDR_MACHINEFRAME]
Type=1
Class=?
Command1=ID_PARAMETERS_COMMAND
Command2=ID_MACHINE_ABOUTTHISMACHINE
Command3=ID_MACHINE_ABOUTPSYGEARV10
Command4=ID_PARAMETERS_RESETPARAMETERS
Command5=ID_PARAMETERS_RANDOMPARAMETERS
Command6=ID_PARAMETERS_LOADPRESET
Command7=ID_PRESET_SAVEPRESET
CommandCount=7

[DLG:IDD_NEWVAL]
Type=1
Class=CNewVal
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1342251136

[CLS:CNewVal]
Type=0
HeaderFile=NewVal.h
ImplementationFile=NewVal.cpp
BaseClass=CDialog
Filter=D
LastObject=ID_PARAMETERS_RANDOMPARAMETERS
VirtualFilter=dWC

[ACL:IDR_MACHINEFRAME (Neutral (Default))]
Type=1
Class=?
Command1=ID_PARAMETERS_RANDOMPARAMETERS
CommandCount=1

[CLS:CVSTHOSTDlg]
Type=0
HeaderFile=VSTHOSTDlg.h
ImplementationFile=VSTHOSTDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CVSTHOSTDlg
VirtualFilter=dWC

[DLG:IDD_VSTRACK]
Type=1
Class=CVstRack
ControlCount=7
Control1=IDOK3,button,1342242817
Control2=IDC_LIST1,listbox,1344340225
Control3=IDC_STATIC,static,1342308352
Control4=IDC_TEXT1,static,1342308864
Control5=IDC_SLIDER1,msctls_trackbar32,1342242842
Control6=IDC_STATIC,static,1342308352
Control7=IDC_COMBO2,combobox,1344339971

[CLS:CVstRack]
Type=0
HeaderFile=VstRack.h
ImplementationFile=VstRack.cpp
BaseClass=CDialog
Filter=D
LastObject=CVstRack
VirtualFilter=dWC

[DLG:IDD_VSTHOST_DIALOG (Neutral (Default))]
Type=1
Class=CVSTHOSTDlg
ControlCount=9
Control1=IDCANCEL2,button,1342242816
Control2=IDC_LIST1,listbox,1352728833
Control3=IDC_LIST2,listbox,1352728833
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_BUTTON1,button,1342242816
Control7=IDC_BUTTON2,button,1342242816
Control8=IDC_STATIC,static,1342177294
Control9=IDC_BUTTON3,button,1342242816

[MNU:IDR_VSTFRAME]
Type=1
Class=?
CommandCount=0

[DLG:IDD_GEARVST (Neutral (Default))]
Type=1
Class=CGearVst
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_COMBO1,combobox,1344339971
Control5=IDC_BUTTON1,button,1342242816
Control6=IDCANCEL3,button,1342242816
Control7=IDC_COMBO2,combobox,1344339971
Control8=IDC_BUTTON2,button,1342242816

[CLS:CGearVst]
Type=0
HeaderFile=GearVst.h
ImplementationFile=GearVst.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_BUTTON2
VirtualFilter=dWC

[DLG:IDD_SEQUENCER (English (U.S.))]
Type=1
Class=DSoundConfig
ControlCount=21
Control1=IDC_SEQLIST,listbox,1352728835
Control2=IDC_INCSHORT,button,1342242816
Control3=IDC_DECSHORT,button,1342242816
Control4=IDC_SEQINS,button,1342242816
Control5=IDC_SEQDEL,button,1342242816
Control6=IDC_SEQSPR,button,1342242816
Control7=IDC_SEQCLR,button,1342242816
Control8=IDC_DECPOS2,button,1342242816
Control9=IDC_SEQ1,static,1342308865
Control10=IDC_INCPOS2,button,1342242816
Control11=IDC_STATIC,static,1342308864
Control12=IDC_DECPAT2,button,1342242816
Control13=IDC_SEQ2,static,1342308865
Control14=IDC_INCPAT2,button,1342242816
Control15=IDC_STATIC,static,1342308864
Control16=IDC_DECLEN,button,1342242816
Control17=IDC_SEQ3,static,1342308865
Control18=IDC_INCLEN,button,1342242816
Control19=IDC_STATIC,static,1342308864
Control20=IDC_SEQSLEN,button,1342242816
Control21=IDC_STATIC,static,1342308352

[DLG:IDD_SEQUENCER]
Type=1
Class=DSoundConfig
ControlCount=21
Control1=IDC_SEQLIST,listbox,1352728835
Control2=IDC_INCSHORT,button,1342242816
Control3=IDC_DECSHORT,button,1342242816
Control4=IDC_SEQINS,button,1342242816
Control5=IDC_SEQDEL,button,1342242816
Control6=IDC_SEQSPR,button,1342242816
Control7=IDC_SEQCLR,button,1342242816
Control8=IDC_DECPOS2,button,1342242816
Control9=IDC_SEQ1,static,1342308865
Control10=IDC_INCPOS2,button,1342242816
Control11=IDC_STATIC,static,1342308864
Control12=IDC_DECPAT2,button,1342242816
Control13=IDC_SEQ2,static,1342308865
Control14=IDC_INCPAT2,button,1342242816
Control15=IDC_STATIC,static,1342308864
Control16=IDC_DECLEN,button,1342242816
Control17=IDC_SEQ3,static,1342308865
Control18=IDC_INCLEN,button,1342242816
Control19=IDC_STATIC,static,1342308864
Control20=IDC_SEQSLEN,button,1342242816
Control21=IDC_STATIC,static,1342308352

[DLG:IDD_GEAR_DELAY (Spanish (Mexican))]
Type=1
Class=CGearDelay
ControlCount=42
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308865
Control3=IDC_SLIDER_TIME,msctls_trackbar32,1342242821
Control4=IDC_LABEL_LTIME,static,1342312961
Control5=IDC_STATIC,static,1342308865
Control6=IDC_SLIDER2,msctls_trackbar32,1342242821
Control7=IDC_F_LABEL_L,static,1342312961
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308865
Control10=IDC_SLIDER_TIME2,msctls_trackbar32,1342242821
Control11=IDC_LABEL_LTIME2,static,1342312961
Control12=IDC_STATIC,static,1342308865
Control13=IDC_SLIDER3,msctls_trackbar32,1342242821
Control14=IDC_F_LABEL_R,static,1342312961
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_LABEL_MSL,static,1342312961
Control18=IDC_STATIC,static,1342308352
Control19=IDC_LABEL_MSR,static,1342312961
Control20=IDC_STATIC,static,1342308352
Control21=IDC_BUTTON1,button,1342242816
Control22=IDC_BUTTON3,button,1342242816
Control23=IDC_BUTTON14,button,1342242816
Control24=IDC_BUTTON15,button,1342242816
Control25=IDC_BUTTON16,button,1342242816
Control26=IDC_BUTTON17,button,1342242816
Control27=IDC_STATIC,static,1342308352
Control28=IDC_BUTTON11,button,1342242816
Control29=IDC_BUTTON18,button,1342242816
Control30=IDC_BUTTON19,button,1342242816
Control31=IDC_BUTTON20,button,1342242816
Control32=IDC_BUTTON21,button,1342242816
Control33=IDC_BUTTON22,button,1342242816
Control34=IDC_STATIC,static,1342308352
Control35=IDC_STATIC,static,1342177294
Control36=IDC_STATIC,button,1342177287
Control37=IDC_STATIC,static,1342308865
Control38=IDC_DRYSLIDER,msctls_trackbar32,1342242840
Control39=IDC_DRYLABEL,static,1342312961
Control40=IDC_STATIC,static,1342308865
Control41=IDC_WETSLIDER,msctls_trackbar32,1342242840
Control42=IDC_WETLABEL,static,1342312961

[DLG:IDD_ENVDIALOG (Spanish (Mexican))]
Type=1
Class=CEnvDialog
ControlCount=41
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_AMPFRAME,static,1350565894
Control4=IDC_SLIDER1,msctls_trackbar32,1342242840
Control5=IDC_STATIC,static,1342308866
Control6=IDC_SLIDER2,msctls_trackbar32,1342242840
Control7=IDC_STATIC,static,1342308866
Control8=IDC_SLIDER3,msctls_trackbar32,1342242840
Control9=IDC_STATIC,static,1342308866
Control10=IDC_SLIDER4,msctls_trackbar32,1342242840
Control11=IDC_STATIC,static,1342308866
Control12=IDC_A_A_LABEL,static,1342308864
Control13=IDC_A_D_LABEL,static,1342308864
Control14=IDC_A_R_LABEL,static,1342308864
Control15=IDC_A_S_LABEL,static,1342308864
Control16=IDC_STATIC,button,1342177287
Control17=IDC_AMPFRAME2,static,1350565894
Control18=IDC_F_SLIDER1,msctls_trackbar32,1342242840
Control19=IDC_STATIC,static,1342308866
Control20=IDC_F_SLIDER2,msctls_trackbar32,1342242840
Control21=IDC_STATIC,static,1342308866
Control22=IDC_F_SLIDER3,msctls_trackbar32,1342242840
Control23=IDC_STATIC,static,1342308866
Control24=IDC_F_SLIDER4,msctls_trackbar32,1342242840
Control25=IDC_STATIC,static,1342308866
Control26=IDC_F_A_LABEL,static,1342308864
Control27=IDC_F_D_LABEL,static,1342308864
Control28=IDC_F_R_LABEL,static,1342308864
Control29=IDC_F_S_LABEL,static,1342308864
Control30=IDC_STATIC,button,1342177287
Control31=IDC_STATIC,static,1342308866
Control32=IDC_SLIDER9,msctls_trackbar32,1342242840
Control33=IDC_CUTOFF_LBL,static,1342308864
Control34=IDC_STATIC,static,1342308866
Control35=IDC_SLIDER10,msctls_trackbar32,1342242840
Control36=IDC_LABELQ,static,1342308864
Control37=IDC_STATIC,static,1342308865
Control38=IDC_COMBO1,combobox,1344339971
Control39=IDC_STATIC,static,1342308866
Control40=IDC_ENVELOPE,msctls_trackbar32,1342242840
Control41=IDC_A_A_LABEL7,static,1342308864

[DLG:IDD_CONFIG (Spanish (Mexican))]
Type=1
Class=CSkinDlg
ControlCount=29
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308864
Control5=IDC_EDIT1,edit,1350631552
Control6=IDC_BROWSE_MV,button,1342242816
Control7=IDC_CHECK1,button,1342242819
Control8=IDC_STATIC,static,1342308864
Control9=IDC_EDIT3,edit,1350631552
Control10=IDC_BROWSE_PV,button,1342242816
Control11=IDC_CHECK2,button,1342242819
Control12=IDC_BUTTON1,button,1342242816
Control13=IDC_BUTTON3,button,1342242816
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,static,1342308865
Control16=IDC_BUTTON2,button,1342242816
Control17=IDC_STATIC,static,1342308865
Control18=IDC_BUTTON4,button,1342242816
Control19=IDC_STATIC,static,1342308865
Control20=IDC_BUTTON5,button,1342242816
Control21=IDC_STATIC,button,1342177287
Control22=IDC_STATIC,static,1342308865
Control23=IDC_BUTTON23,button,1342242816
Control24=IDC_STATIC,static,1342308865
Control25=IDC_BUTTON24,button,1342242816
Control26=IDC_STATIC,static,1342308865
Control27=IDC_BUTTON25,button,1342242816
Control28=IDC_STATIC,static,1342308865
Control29=IDC_BUTTON26,button,1342242816

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_LOADSONG
Command3=ID_FILE_SAVESONG
Command4=ID_BARPLAY
Command5=ID_BARSTOP
Command6=ID_RECORDB
Command7=ID_PSYHELP
Command8=ID_MACHINEVIEW
Command9=ID_PATTERNVIEW
Command10=ID_SHOWIED
Command11=ID_NEWMACHINE
Command12=ID_ABOUTPSYCLE
CommandCount=12

[DLG:IDD_WAVEOUTCONFIG (Neutral (Sys. Default))]
Type=1
Class=CADriverDialog
ControlCount=15
Control1=IDOK,button,1342242817
Control2=IDC_CONFIG_DEVICE,combobox,1344339971
Control3=IDC_CONFIG_SAMPLERATE,combobox,1344339971
Control4=IDC_CONFIG_DITHER,button,1342242819
Control5=IDC_CONFIG_BUFNUM,edit,1350631552
Control6=IDC_CONFIG_BUFNUM_SPIN,msctls_updown32,1342177462
Control7=IDC_CONFIG_BUFSIZE,edit,1350631552
Control8=IDC_CONFIG_BUFSIZE_SPIN,msctls_updown32,1342177462
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308864
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_CONFIG_LATENCY,static,1342308352
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,static,1342177294

[DLG:IDD_WAVEOUTCONFIG (Neutral (Default))]
Type=1
Class=CADriverDialog
ControlCount=15
Control1=IDOK,button,1342242817
Control2=IDC_CONFIG_DEVICE,combobox,1344339971
Control3=IDC_CONFIG_SAMPLERATE,combobox,1344339971
Control4=IDC_CONFIG_DITHER,button,1342242819
Control5=IDC_CONFIG_BUFNUM,edit,1350631552
Control6=IDC_CONFIG_BUFNUM_SPIN,msctls_updown32,1342177462
Control7=IDC_CONFIG_BUFSIZE,edit,1350631552
Control8=IDC_CONFIG_BUFSIZE_SPIN,msctls_updown32,1342177462
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308864
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_CONFIG_LATENCY,static,1342308352
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,static,1342177294

[DLG:IDD_GEAR_DELAY (Neutral (Default))]
Type=1
Class=CGearDelay
ControlCount=42
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308865
Control3=IDC_SLIDER_TIME,msctls_trackbar32,1342242821
Control4=IDC_LABEL_LTIME,static,1342312961
Control5=IDC_STATIC,static,1342308865
Control6=IDC_SLIDER2,msctls_trackbar32,1342242821
Control7=IDC_F_LABEL_L,static,1342312961
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308865
Control10=IDC_SLIDER_TIME2,msctls_trackbar32,1342242821
Control11=IDC_LABEL_LTIME2,static,1342312961
Control12=IDC_STATIC,static,1342308865
Control13=IDC_SLIDER3,msctls_trackbar32,1342242821
Control14=IDC_F_LABEL_R,static,1342312961
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_LABEL_MSL,static,1342312961
Control18=IDC_STATIC,static,1342308352
Control19=IDC_LABEL_MSR,static,1342312961
Control20=IDC_STATIC,static,1342308352
Control21=IDC_BUTTON1,button,1342242816
Control22=IDC_BUTTON3,button,1342242816
Control23=IDC_BUTTON14,button,1342242816
Control24=IDC_BUTTON15,button,1342242816
Control25=IDC_BUTTON16,button,1342242816
Control26=IDC_BUTTON17,button,1342242816
Control27=IDC_STATIC,static,1342308352
Control28=IDC_BUTTON11,button,1342242816
Control29=IDC_BUTTON18,button,1342242816
Control30=IDC_BUTTON19,button,1342242816
Control31=IDC_BUTTON20,button,1342242816
Control32=IDC_BUTTON21,button,1342242816
Control33=IDC_BUTTON22,button,1342242816
Control34=IDC_STATIC,static,1342308352
Control35=IDC_STATIC,static,1342177294
Control36=IDC_STATIC,button,1342177287
Control37=IDC_STATIC,static,1342308865
Control38=IDC_DRYSLIDER,msctls_trackbar32,1342242840
Control39=IDC_DRYLABEL,static,1342312961
Control40=IDC_STATIC,static,1342308865
Control41=IDC_WETSLIDER,msctls_trackbar32,1342242840
Control42=IDC_WETLABEL,static,1342312961

[DLG:IDD_ENVDIALOG (Neutral (Default))]
Type=1
Class=CEnvDialog
ControlCount=41
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_AMPFRAME,static,1350565894
Control4=IDC_SLIDER1,msctls_trackbar32,1342242840
Control5=IDC_STATIC,static,1342308866
Control6=IDC_SLIDER2,msctls_trackbar32,1342242840
Control7=IDC_STATIC,static,1342308866
Control8=IDC_SLIDER3,msctls_trackbar32,1342242840
Control9=IDC_STATIC,static,1342308866
Control10=IDC_SLIDER4,msctls_trackbar32,1342242840
Control11=IDC_STATIC,static,1342308866
Control12=IDC_A_A_LABEL,static,1342308864
Control13=IDC_A_D_LABEL,static,1342308864
Control14=IDC_A_R_LABEL,static,1342308864
Control15=IDC_A_S_LABEL,static,1342308864
Control16=IDC_STATIC,button,1342177287
Control17=IDC_AMPFRAME2,static,1350565894
Control18=IDC_F_SLIDER1,msctls_trackbar32,1342242840
Control19=IDC_STATIC,static,1342308866
Control20=IDC_F_SLIDER2,msctls_trackbar32,1342242840
Control21=IDC_STATIC,static,1342308866
Control22=IDC_F_SLIDER3,msctls_trackbar32,1342242840
Control23=IDC_STATIC,static,1342308866
Control24=IDC_F_SLIDER4,msctls_trackbar32,1342242840
Control25=IDC_STATIC,static,1342308866
Control26=IDC_F_A_LABEL,static,1342308864
Control27=IDC_F_D_LABEL,static,1342308864
Control28=IDC_F_R_LABEL,static,1342308864
Control29=IDC_F_S_LABEL,static,1342308864
Control30=IDC_STATIC,button,1342177287
Control31=IDC_STATIC,static,1342308866
Control32=IDC_SLIDER9,msctls_trackbar32,1342242840
Control33=IDC_CUTOFF_LBL,static,1342308864
Control34=IDC_STATIC,static,1342308866
Control35=IDC_SLIDER10,msctls_trackbar32,1342242840
Control36=IDC_LABELQ,static,1342308864
Control37=IDC_STATIC,static,1342308865
Control38=IDC_COMBO1,combobox,1344339971
Control39=IDC_STATIC,static,1342308866
Control40=IDC_ENVELOPE,msctls_trackbar32,1342242840
Control41=IDC_A_A_LABEL7,static,1342308864

[DLG:IDD_CONFIG (Neutral (Default))]
Type=1
Class=CSkinDlg
ControlCount=29
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308864
Control5=IDC_EDIT1,edit,1350631552
Control6=IDC_BROWSE_MV,button,1342242816
Control7=IDC_CHECK1,button,1342242819
Control8=IDC_STATIC,static,1342308864
Control9=IDC_EDIT3,edit,1350631552
Control10=IDC_BROWSE_PV,button,1342242816
Control11=IDC_CHECK2,button,1342242819
Control12=IDC_BUTTON1,button,1342242816
Control13=IDC_BUTTON3,button,1342242816
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,static,1342308865
Control16=IDC_BUTTON2,button,1342242816
Control17=IDC_STATIC,static,1342308865
Control18=IDC_BUTTON4,button,1342242816
Control19=IDC_STATIC,static,1342308865
Control20=IDC_BUTTON5,button,1342242816
Control21=IDC_STATIC,button,1342177287
Control22=IDC_STATIC,static,1342308865
Control23=IDC_BUTTON23,button,1342242816
Control24=IDC_STATIC,static,1342308865
Control25=IDC_BUTTON24,button,1342242816
Control26=IDC_STATIC,static,1342308865
Control27=IDC_BUTTON25,button,1342242816
Control28=IDC_STATIC,static,1342308865
Control29=IDC_BUTTON26,button,1342242816

[DLG:IDD_GEAR_FILTER (Neutral (Default))]
Type=1
Class=CGearfilter
ControlCount=14
Control1=IDC_COMBO1,combobox,1344339971
Control2=IDC_STATIC,static,1342308865
Control3=IDC_SLIDER1,msctls_trackbar32,1342242840
Control4=IDC_STATIC,static,1342308865
Control5=IDC_SLIDER2,msctls_trackbar32,1342242840
Control6=IDC_STATIC,static,1342308865
Control7=IDC_SLIDER3,msctls_trackbar32,1342242840
Control8=IDC_STATIC,static,1342308865
Control9=IDC_SLIDER4,msctls_trackbar32,1342242840
Control10=IDC_STATIC,static,1342308865
Control11=IDC_STATIC,button,1342177287
Control12=IDC_STATIC,button,1342177287
Control13=IDC_SLIDER5,msctls_trackbar32,1342242840
Control14=IDC_STATIC,static,1342308865

[DLG:IDD_GEAR_PSYCHOSC (Neutral (Default))]
Type=1
Class=gearPsychosc
ControlCount=16
Control1=IDC_STATIC,button,1342177287
Control2=IDC_SLIDER1,msctls_trackbar32,1342242827
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SLIDER2,msctls_trackbar32,1342242827
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,button,1342177287
Control7=IDC_SLIDER3,msctls_trackbar32,1342242827
Control8=IDC_STATIC,static,1342308352
Control9=IDC_SLIDER4,msctls_trackbar32,1342242827
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342177294
Control12=IDC_STATIC,static,1342308353
Control13=IDC_STATIC1,static,1342308865
Control14=IDC_STATIC2,static,1342308865
Control15=IDC_STATIC3,static,1342308865
Control16=IDC_STATIC4,static,1342308865

[DLG:IDD_GEAR_TRACKER (Neutral (Default))]
Type=1
Class=CGearTracker
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_TRACKLABEL2,static,1350697473
Control4=IDC_STATIC,static,1342308865
Control5=IDC_TRACKSLIDER2,msctls_trackbar32,1342242821
Control6=IDC_STATIC,static,1342177294
Control7=IDC_STATIC,static,1342308865
Control8=IDC_STATIC,static,1342308865
Control9=IDC_COMBO1,combobox,1344339971
Control10=IDC_STATIC,static,1342308864

[DLG:IDD_GEAR_GAINER (Neutral (Default))]
Type=1
Class=CGearGainer
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDC_SLIDER1,msctls_trackbar32,1342242827
Control3=IDC_VOLABEL,static,1342312961

[DLG:IDD_GEAR_FLANGER (Neutral (Default))]
Type=1
Class=CGearFlanger
ControlCount=31
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342177287
Control3=IDC_STATIC,static,1342308864
Control4=IDC_SLIDER1,msctls_trackbar32,1342242840
Control5=IDC_LABEL1,static,1342312961
Control6=IDC_STATIC,static,1342308864
Control7=IDC_SLIDER2,msctls_trackbar32,1342242840
Control8=IDC_LABEL2,static,1342312961
Control9=IDC_STATIC,static,1342308864
Control10=IDC_SLIDER3,msctls_trackbar32,1342242840
Control11=IDC_LABEL3,static,1342312961
Control12=IDC_STATIC,static,1342308864
Control13=IDC_SLIDER4,msctls_trackbar32,1342242840
Control14=IDC_LABEL4,static,1342312961
Control15=IDC_STATIC,button,1342177287
Control16=IDC_STATIC,static,1342308864
Control17=IDC_SLIDER5,msctls_trackbar32,1342242840
Control18=IDC_LABEL5,static,1342312961
Control19=IDC_STATIC,static,1342308864
Control20=IDC_SLIDER6,msctls_trackbar32,1342242840
Control21=IDC_LABEL6,static,1342312961
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,static,1342308865
Control24=IDC_DRYSLIDER,msctls_trackbar32,1342242840
Control25=IDC_DRYLABEL,static,1342312961
Control26=IDC_STATIC,static,1342308865
Control27=IDC_WETSLIDER,msctls_trackbar32,1342242840
Control28=IDC_WETLABEL,static,1342312961
Control29=IDC_STATIC,static,1342177294
Control30=IDC_STATIC,static,1342308352
Control31=IDC_PRESETCOMBO,combobox,1344339971

[DLG:IDD_GEARDISTORT (Neutral (Default))]
Type=1
Class=CGearDistort
ControlCount=20
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SLIDER1,msctls_trackbar32,1342242827
Control5=IDC_SLIDER2,msctls_trackbar32,1342242827
Control6=IDC_STATIC,static,1342308353
Control7=IDC_STATIC,static,1342308353
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308353
Control10=IDC_STATIC,static,1342308352
Control11=IDC_SLIDER3,msctls_trackbar32,1342242827
Control12=IDC_SLIDER4,msctls_trackbar32,1342242827
Control13=IDC_STATIC,static,1342308353
Control14=IDC_STATIC,static,1342308353
Control15=IDC_BUTTON1,button,1342242816
Control16=IDC_BUTTON3,button,1342242816
Control17=IDC_BUTTON2,button,1342242816
Control18=IDC_BUTTON4,button,1342242816
Control19=IDC_STATIC,static,1342177294
Control20=IDC_STATIC,static,1342308353

[DLG:IDD_PSYBAR (Neutral (Default))]
Type=1
Class=?
ControlCount=11
Control1=IDC_STATIC,static,1342308865
Control2=IDC_BUTTON1,button,1342242816
Control3=IDC_BPMLABEL,static,1342312961
Control4=IDC_BUTTON3,button,1342242816
Control5=IDC_BUTTON14,button,1342242816
Control6=IDC_BUTTON15,button,1342242816
Control7=IDC_LVUM,static,1342181380
Control8=IDC_RVUM,static,1342181380
Control9=IDC_STATIC,static,1342308352
Control10=IDC_FRAMECLIP,static,1342181380
Control11=IDC_CLIPBUT,button,1342242816

[DLG:IDD_PSYBAR2 (Neutral (Default))]
Type=1
Class=?
ControlCount=17
Control1=IDC_STATIC,static,1342308352
Control2=IDC_SSCOMBO2,combobox,1344339971
Control3=IDC_STATIC,static,1342308352
Control4=IDC_BAR_COMBOGEN,combobox,1344339971
Control5=IDC_STATIC,static,1342308352
Control6=IDC_BAR_COMBOINS,combobox,1344339971
Control7=IDC_STATIC,static,1342308354
Control8=IDC_TRACKCOMBO,combobox,1344339971
Control9=IDC_LOADWAVE,button,1342242816
Control10=IDC_EDITWAVE,button,1342242816
Control11=IDC_WAVEBUT,button,1342242816
Control12=IDC_B_INCGEN,button,1342242816
Control13=IDC_B_DECGEN,button,1342242816
Control14=IDC_B_INCWAV,button,1342242816
Control15=IDC_B_DECWAV,button,1342242816
Control16=IDC_STATIC,static,1342177284
Control17=IDC_STATIC,static,1342177284

[DLG:IDD_INFO (Neutral (Default))]
Type=1
Class=CInfoDlg
ControlCount=23
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,button,1342177287
Control3=IDC_STATIC,static,1342308352
Control4=IDC_MACHINELIST,SysListView32,1350666241
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,static,1342308865
Control7=IDC_CPUIDLE_LABEL,static,1342312961
Control8=IDC_STATIC,static,1342308865
Control9=IDC_MCPULABEL,static,1342312961
Control10=IDC_STATIC,static,1342308865
Control11=IDC_CPUL,static,1342312961
Control12=IDC_STATIC,static,1342308865
Control13=IDC_MCPULABEL2,static,1342312961
Control14=IDC_STATIC,static,1342177294
Control15=IDC_STATIC,button,1342177287
Control16=IDC_STATIC,static,1342308865
Control17=IDC_MEM_RESO,static,1342312961
Control18=IDC_STATIC,static,1342308865
Control19=IDC_MEM_RESO2,static,1342312961
Control20=IDC_STATIC,static,1342308865
Control21=IDC_MEM_RESO3,static,1342312961
Control22=IDC_STATIC,static,1342308865
Control23=IDC_MEM_RESO4,static,1342312961

[DLG:IDD_WIREDIALOG (Neutral (Default))]
Type=1
Class=CWireDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308353
Control2=IDC_SLIDER1,msctls_trackbar32,1342242825
Control3=IDC_BUTTON1,button,1342242816
Control4=IDC_STATIC1,static,1342308353

[DLG:IDD_NEWMACHINE (Neutral (Default))]
Type=1
Class=CNewMachine
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_TREE1,SysTreeView32,1350631559
Control4=IDC_STATIC,static,1342308864
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_DESCLABEL,static,1342312448
Control9=IDC_VERLABEL,static,1342312448
Control10=IDC_STATIC,static,1342308352
Control11=IDC_NAMELABEL,static,1342312448

[DLG:IDD_MACPROP (Neutral (Default))]
Type=1
Class=CMacProp
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308865
Control4=IDC_EDIT1,edit,1350631552
Control5=IDC_BUTTON1,button,1342242816
Control6=IDC_CHECK1,button,1342242819

[DLG:IDD_PATDLG (Neutral (Default))]
Type=1
Class=CPatDlg
ControlCount=7
Control1=IDOK,button,1342242816
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308864
Control4=IDC_EDIT1,edit,1350631552
Control5=IDC_EDIT2,edit,1350631552
Control6=IDC_SPIN1,msctls_updown32,1342177462
Control7=IDC_STATIC,static,1342308864

[DLG:IDD_INSTRUMENT (Neutral (Default))]
Type=1
Class=CInstrumentEditor
ControlCount=64
Control1=IDC_STATIC,static,1342308866
Control2=IDC_LAYER1,button,1342181385
Control3=IDC_LAYER2,button,1342181385
Control4=IDC_LAYER3,button,1342181385
Control5=IDC_LAYER4,button,1342181385
Control6=IDC_LAYER5,button,1342181385
Control7=IDC_LAYER6,button,1342181385
Control8=IDC_LAYER7,button,1342181385
Control9=IDC_LAYER8,button,1342181385
Control10=IDC_LAYER9,button,1342181385
Control11=IDC_LAYER10,button,1342181385
Control12=IDC_LAYER11,button,1342181385
Control13=IDC_LAYER12,button,1342181385
Control14=IDC_LAYER13,button,1342181385
Control15=IDC_LAYER14,button,1342181385
Control16=IDC_LAYER15,button,1342181385
Control17=IDC_LAYER16,button,1342181385
Control18=IDC_STATIC,button,1342177287
Control19=IDC_WAVENAME,edit,1350631552
Control20=IDC_LAYERN,static,1350697473
Control21=IDC_STATIC,static,1342308866
Control22=IDC_STATIC,static,1342312961
Control23=IDC_BUTTON2,button,1342242816
Control24=IDC_BUTTON4,button,1342242816
Control25=IDC_BUTTON5,button,1342242816
Control26=IDC_BUTTON6,button,1342242816
Control27=IDC_SLIDER1,msctls_trackbar32,1342242840
Control28=IDC_VOLABEL,static,1342312961
Control29=IDC_STATIC,static,1342308866
Control30=IDC_INSTNUMBER,static,1350697473
Control31=IDC_STATIC,static,1342308866
Control32=IDC_STEREO,static,1342312961
Control33=IDC_STATIC,static,1342308866
Control34=IDC_LOOPTYPE,static,1342312961
Control35=IDC_LOOPOFF,button,1342242816
Control36=IDC_LOOPFORWARD,button,1342242816
Control37=IDC_STATIC,static,1342308865
Control38=IDC_LOOPSTART,static,1350697473
Control39=IDC_LOOPEND,static,1350697473
Control40=IDC_STATIC,static,1342308865
Control41=IDC_BUTTON11,button,1476460544
Control42=IDC_STATUS,static,1342312961
Control43=IDC_STATIC,static,1342308866
Control44=IDC_WAVELENGTH,static,1350697473
Control45=IDC_INSTNAME,edit,1350631552
Control46=IDC_BUTTON7,button,1342242816
Control47=IDC_BUTTON8,button,1342242816
Control48=IDC_STATIC,button,1342178055
Control49=IDC_BUTTON12,button,1342242816
Control50=IDC_BUTTON13,button,1342242816
Control51=IDC_STATIC,static,1342308864
Control52=IDC_NNA_COMBO,combobox,1344339971
Control53=IDC_STATIC,static,1342308353
Control54=IDC_PANSLIDER,msctls_trackbar32,1342242840
Control55=IDC_RPAN,button,1342242819
Control56=IDC_RCUT,button,1342242819
Control57=IDC_RRES,button,1342242819
Control58=IDC_CHECK4,button,1342242819
Control59=IDC_LOOPEDIT,edit,1342251008
Control60=IDC_STATIC,static,1342308352
Control61=IDC_STATIC,button,1342177287
Control62=IDC_SLIDER2,msctls_trackbar32,1342242840
Control63=IDC_VOLABEL2,static,1342312961
Control64=IDC_STATIC,static,1342308866

[DLG:IDD_RECORDER (Neutral (Default))]
Type=1
Class=CRecorderDlg
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_BUTTON1,button,1342242816
Control5=IDC_BUTTON2,button,1342242816
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308865

[DLG:IDD_PATNAVDIALOG (Neutral (Default))]
Type=1
Class=CPtnDialog
ControlCount=5
Control1=IDOK,button,1476460545
Control2=IDC_LIST1,SysListView32,1350631429
Control3=IDC_PNLABEL,static,1342312961
Control4=IDC_CREATEB,button,1342242816
Control5=IDC_STATIC,static,1342308864

[DLG:IDD_SAVESONG (Neutral (Default))]
Type=1
Class=CSaveDlg
ControlCount=3
Control1=IDC_PROGRESS1,msctls_progress32,1350565888
Control2=IDC_STATIC,static,1342308865
Control3=IDC_STATIC,static,1342312961

[DLG:IDD_IBROWSE (Neutral (Default))]
Type=1
Class=CIBrowseDlg
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_LIST1,listbox,1352728833
Control3=IDC_STATIC,static,1342308352
Control4=IDC_BUTTON1,button,1342242816
Control5=IDC_STATIC,button,1342177287

[DLG:IDD_GREETS (Neutral (Default))]
Type=1
Class=CGreetDialog
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,button,1342177287
Control5=IDC_LIST1,listbox,1352728835

[DLG:IDD_SONGPROP (Neutral (Default))]
Type=1
Class=CSongpDlg
ControlCount=7
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT1,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT3,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_EDIT4,edit,1352728644
Control7=IDC_BUTTON1,button,1342242816

[DLG:IDD_MASTERDLG (Neutral (Default))]
Type=1
Class=CMasterDlg
ControlCount=2
Control1=IDC_SLIDER1,msctls_trackbar32,1342242840
Control2=IDC_DBLEVEL,static,1342312961

[DLG:IDD_NEWVAL (Neutral (Default))]
Type=1
Class=CNewVal
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1342251136

[DLG:IDD_VSTRACK (Neutral (Default))]
Type=1
Class=CVstRack
ControlCount=7
Control1=IDOK3,button,1342242817
Control2=IDC_LIST1,listbox,1344340225
Control3=IDC_STATIC,static,1342308352
Control4=IDC_TEXT1,static,1342308864
Control5=IDC_SLIDER1,msctls_trackbar32,1342242842
Control6=IDC_STATIC,static,1342308352
Control7=IDC_COMBO2,combobox,1344339971

[MNU:IDR_WAVEFRAME (Neutral (Default))]
Type=1
Class=?
Command1=ID_SELECTION_FADEIN
Command2=ID_SELECTION_FADEOUT
Command3=ID_SELECCION_ZOOM
Command4=ID_SELECCION_ZOOMOUT
CommandCount=4

[MNU:IDR_MACHINEFRAME (Neutral (Default))]
Type=1
Class=?
Command1=ID_PARAMETERS_COMMAND
Command2=ID_MACHINE_ABOUTTHISMACHINE
Command3=ID_MACHINE_ABOUTPSYGEARV10
Command4=ID_PARAMETERS_RESETPARAMETERS
Command5=ID_PARAMETERS_RANDOMPARAMETERS
Command6=ID_PARAMETERS_LOADPRESET
Command7=ID_PRESET_SAVEPRESET
CommandCount=7

[MNU:IDR_VSTFRAME (Neutral (Default))]
Type=1
Class=?
CommandCount=0

[CLS:DSoundConfig]
Type=0
HeaderFile=DSoundConfig.h
ImplementationFile=DSoundConfig.cpp
BaseClass=CDialog
Filter=D
LastObject=DSoundConfig

[DLG:IDD_DSOUNDCONFIG]
Type=1
Class=CDSoundConfig
ControlCount=16
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_DSOUND_DEVICE,combobox,1344339971
Control4=IDC_STATIC,static,1342308864
Control5=IDC_STATIC,button,1342177287
Control6=IDC_DSOUND_SAMPLERATE_COMBO,combobox,1344340227
Control7=IDC_STATIC,static,1342308866
Control8=IDC_STATIC,static,1342308866
Control9=IDC_DSOUND_BUFNUM_EDIT,edit,1350631552
Control10=IDC_DSOUND_BUFNUM_SPIN,msctls_updown32,1342177462
Control11=IDC_DSOUND_BUFSIZE_EDIT,edit,1350631552
Control12=IDC_DSOUND_BUFSIZE_SPIN,msctls_updown32,1342177458
Control13=IDC_DSOUND_LATENCY,static,1342308864
Control14=IDC_DSOUND_DITHER,button,1342242819
Control15=IDC_STATIC,static,1342177294
Control16=IDC_EXCLUSIVE,button,1342242819

[CLS:CDSoundConfig]
Type=0
HeaderFile=DSoundConfig.h
ImplementationFile=DSoundConfig.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_DSOUND_BUFNUM_EDIT
VirtualFilter=dWC

[DLG:IDD_OUTPUT]
Type=1
Class=COutputDlg
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_DRIVER,combobox,1342242819
Control4=IDC_CONFIG,button,1342242816

[CLS:COutputDlg]
Type=0
HeaderFile=OutputDlg.h
ImplementationFile=OutputDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_CONFIG


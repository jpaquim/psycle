// MainFrm.cpp : implementation of the CMainFrame class
//

//////////////////////////////////////////////////////////////////////
// Include CTrack code

#include "stdafx.h"
#include "Psycle2.h"
#include "MainFrm.h"
#include "FileDialogEx1.h"
#include "VSTHOSTDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_BN_CLICKED(IDC_BUTTON1, OnBarButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBpmAddTen)
	ON_BN_CLICKED(IDC_BUTTON14, OnBpmDecOne)
	ON_BN_CLICKED(IDC_BUTTON15, OnBpmDecTen)
	ON_CBN_SELCHANGE(IDC_SSCOMBO2, OnSelchangeSscombo2)
	ON_CBN_CLOSEUP(IDC_SSCOMBO2, OnCloseupSscombo2)
	ON_CBN_SELCHANGE(IDC_BAR_COMBOGEN, OnSelchangeBarCombogen)
	ON_CBN_CLOSEUP(IDC_BAR_COMBOGEN, OnCloseupBarCombogen)
	ON_CBN_SELCHANGE(IDC_BAR_COMBOINS, OnSelchangeBarComboins)
	ON_CBN_CLOSEUP(IDC_BAR_COMBOINS, OnCloseupBarComboins)
	ON_BN_CLICKED(IDC_CLIPBUT, OnClipbut)
	ON_CBN_SELCHANGE(IDC_TRACKCOMBO, OnSelchangeTrackcombo)
	ON_CBN_CLOSEUP(IDC_TRACKCOMBO, OnCloseupTrackcombo)
	ON_COMMAND(ID_PSYHELP, OnPsyhelp)
	ON_BN_CLICKED(IDC_LOADWAVE, OnLoadwave)
	ON_BN_CLICKED(IDC_EDITWAVE, OnEditwave)
	ON_BN_CLICKED(IDC_WAVEBUT, OnWavebut)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_B_DECGEN, OnBDecgen)
	ON_BN_CLICKED(IDC_B_INCGEN, OnBIncgen)
	ON_BN_CLICKED(IDC_B_DECWAV, OnBDecwav)
	ON_BN_CLICKED(IDC_B_INCWAV, OnBIncwav)
	ON_WM_CLOSE()
	ON_COMMAND(ID_VST2MANAGER_VST2PLUGINMANAGER, OnVst2managerVst2pluginmanager)
	ON_LBN_SELCHANGE(IDC_SEQLIST, OnSelchangeSeqlist)
	ON_LBN_DBLCLK(IDC_SEQLIST, OnDblclkSeqlist)
	ON_BN_CLICKED(IDC_DECLEN, OnDeclen)
	ON_BN_CLICKED(IDC_DECPAT2, OnDecpat2)
	ON_BN_CLICKED(IDC_DECPOS2, OnDecpos2)
	ON_BN_CLICKED(IDC_INCLEN, OnInclen)
	ON_BN_CLICKED(IDC_INCPAT2, OnIncpat2)
	ON_BN_CLICKED(IDC_INCPOS2, OnIncpos2)
	ON_BN_CLICKED(IDC_INCSHORT, OnIncshort)
	ON_BN_CLICKED(IDC_DECSHORT, OnDecshort)
	ON_BN_CLICKED(IDC_SEQINS, OnSeqins)
	ON_BN_CLICKED(IDC_SEQSLEN, OnSeqslen)
	ON_BN_CLICKED(IDC_SEQSPR, OnSeqspr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_wndView.pParentFrame=this;
	anySampler=false;

	for(int c=0;c<MAX_MACHINES;c++)
	isguiopen[c]=false;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame

	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(4,1,8,1)) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// Status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetWindowText("Psycle tool bar");
	m_wndView.ValidateParent();

	// Creating psybar1
	if(!(m_wndControl.Create(this,IDD_PSYBAR,WS_CHILD | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY,0x9000)))
	{
		return -1;
	}
	else
	{
	SetAppSongBpm(0);
	m_wndControl.ShowWindow(SW_SHOWNORMAL);
	m_wndControl.SetWindowText("Psycle control bar");
	}
		
	// Creating psybar2
	if(!(m_wndControl2.Create(this,IDD_PSYBAR2,WS_CHILD | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY,0x9001)))
	{
		return -1;
	}
	else
	{
	m_wndControl2.ShowWindow(SW_SHOWNORMAL);
	m_wndControl2.SetWindowText("Psycle control bar 2");
	Psybar2Init();
	}

	m_wndSeq.Create(this,IDD_SEQUENCER,CBRS_LEFT,0x9002);

	// Instrument editor
	m_wndInst.songRef=songRef;
	m_wndInst.Create(IDD_INSTRUMENT,this);
	m_wndInst.Validate();

	m_wndInfo.songRef=songRef;
	m_wndInfo.Create(IDD_INFO,this);

	m_pWndWed=new CSubMarco(0);
	m_pWndWed->songRef=songRef;

	m_pWndWed->LoadFrame(IDR_WAVEFRAME,WS_OVERLAPPEDWINDOW,this);
	m_pWndWed->GenerateAndShow();

	UpdatePlayOrder(false);
	
	UpdateComboGen();
	UpdateComboIns();

	// Finally initializing timer

	UpdateSequencer();
	m_wndView.InitTimer();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnBarButton1() 
{
	SetAppSongBpm(1);
}

void CMainFrame::OnBpmAddTen() 
{
	SetAppSongBpm(10);
}

void CMainFrame::OnBpmDecOne() 
{
	SetAppSongBpm(-1);
}

void CMainFrame::OnBpmDecTen() 
{
	SetAppSongBpm(-10);
}

void CMainFrame::SetAppSongBpm(int x) 
{
char buffer[16];
sprintf(buffer,"%d",m_wndView.SongIncBpm(x));
CStatic *tmplab=(CStatic *)m_wndControl.GetDlgItem(IDC_BPMLABEL);
tmplab->SetWindowText(buffer);
m_wndView.SetFocus();	
}

void CMainFrame::UpdatePlayOrder(bool mode) 
{
	CStatic *ls_l=(CStatic *)m_wndSeq.GetDlgItem(IDC_SEQ1);
	CStatic *le_l=(CStatic *)m_wndSeq.GetDlgItem(IDC_SEQ2);
	CStatic *ll_l=(CStatic *)m_wndSeq.GetDlgItem(IDC_SEQ3);
	CListBox *pls=(CListBox*)m_wndSeq.GetDlgItem(IDC_SEQLIST);

	int ls=songRef->editPosition;
	int le=songRef->playOrder[songRef->editPosition];
	int ll=songRef->playLength;
	
	char buffer[16];

	sprintf(buffer,"%.2X",ls);
	ls_l->SetWindowText(buffer);

	sprintf(buffer,"%.2X",le);
	le_l->SetWindowText(buffer);

	sprintf(buffer,"%.2X",ll);
	ll_l->SetWindowText(buffer);

	if(mode)
	{
	pls->DeleteString(ls);
	sprintf(buffer,"%.2X: %.2X",ls,le);
	pls->InsertString(ls,buffer);
	}

	pls->SetCurSel(songRef->editPosition);

}

CMainFrame::ConfigAudio()
{
	m_wndView.ConfigAudio(false);
}


CMainFrame::UpdateVumeters(int l, int r,COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip)
{
if(l<1)l=1;
if(r<1)r=1;

if(clip)
{
CStatic *lc=(CStatic *)m_wndControl.GetDlgItem(IDC_FRAMECLIP);
CClientDC clcanvasl(lc);
clcanvasl.FillSolidRect(0,0,9,16,vu3);
}
else
{
CStatic *lc=(CStatic *)m_wndControl.GetDlgItem(IDC_FRAMECLIP);
CClientDC clcanvasl(lc);
clcanvasl.FillSolidRect(0,0,9,16,0x00000000);
}


float log_l=20.0f*(float)log10((0.000030517578125f*(float)l));
float log_r=20.0f*(float)log10((0.000030517578125f*(float)r));

bool draw_l=true;
bool draw_r=true;

if(log_l<-60.0f)
draw_l=false;

if(log_r<-60.0f)
draw_r=false;

if(log_l>0)log_l=0;
if(log_r>0)log_r=0;

int cl=226+(int)log_l;
int cr=226+(int)log_r;

CStatic *lv=(CStatic *)m_wndControl.GetDlgItem(IDC_LVUM);
CStatic *rv=(CStatic *)m_wndControl.GetDlgItem(IDC_RVUM);
CClientDC canvasl(lv);
CClientDC canvasr(rv);

if(draw_l)
{
canvasl.FillSolidRect(0,0,cl,5,vu1);
canvasl.FillSolidRect(cl,0,226-cl,5,vu2);
}
else
canvasl.FillSolidRect(0,0,226,5,vu2);

if(draw_r)
{
canvasr.FillSolidRect(0,0,cr,5,vu1);
canvasr.FillSolidRect(cr,0,226-cr,5,vu2);
}
else
canvasr.FillSolidRect(0,0,226,5,vu2);

}

CMainFrame::Psybar2Init()
{
CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
cc2->AddString("0");
cc2->AddString("1");
cc2->AddString("2");
cc2->AddString("3");
cc2->AddString("4");
cc2->AddString("5");
cc2->AddString("6");
cc2->AddString("7");
cc2->AddString("8");
cc2->AddString("9");
cc2->AddString("10");
cc2->AddString("11");
cc2->AddString("12");
cc2->AddString("13");
cc2->AddString("14");
cc2->AddString("15");
cc2->AddString("16");
cc2->SetCurSel(1);

cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_TRACKCOMBO);
cc2->AddString("4");
cc2->AddString("5");
cc2->AddString("6");
cc2->AddString("7");
cc2->AddString("8");
cc2->AddString("9");
cc2->AddString("10");
cc2->AddString("11");
cc2->AddString("12");
cc2->AddString("13");
cc2->AddString("14");
cc2->AddString("15");
cc2->AddString("16");
cc2->AddString("17");
cc2->AddString("18");
cc2->AddString("19");
cc2->AddString("20");
cc2->AddString("21");
cc2->AddString("22");
cc2->AddString("23");
cc2->AddString("24");
cc2->AddString("25");
cc2->AddString("26");
cc2->AddString("27");
cc2->AddString("28");
cc2->AddString("29");
cc2->AddString("30");
cc2->AddString("31");
cc2->AddString("32");

cc2->SetCurSel(songRef->SONGTRACKS-4);

}

void CMainFrame::OnSelchangeSscombo2() 
{
CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
int sel=cc->GetCurSel();
m_wndView.SetPatStep(sel);
m_wndView.SetFocus();
}

void CMainFrame::OnCloseupSscombo2(){m_wndView.SetFocus();}

CMainFrame::UpdateComboGen()
{
CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);

cc->ResetContent();

	anySampler=false;

	for (int b=0;b<MAX_BUSES;b++)
	{
		char buffer[64];

		if(songRef->busMachine[b]!=255)
		sprintf(buffer,"%.2d: %s",b,songRef->machine[songRef->busMachine[b]]->editName);
		else
		sprintf(buffer,"%.2d:",b);
		
		cc->AddString(buffer);
	}
	anySampler=true;
	cc->EnableWindow(true);
	cc->SetCurSel(songRef->seqBus);
}

void CMainFrame::OnSelchangeBarCombogen() 
{
	if(anySampler)
	{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	int nsb=cc->GetCurSel();

		if(songRef->seqBus!=nsb)
		{
		songRef->seqBus=nsb;
		m_wndView.SetActiveWindow();
		}
	}

	m_wndView.SetFocus();
}

void CMainFrame::OnCloseupBarCombogen(){m_wndView.SetFocus();}

CMainFrame::UpdateComboIns()
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	
	cc->ResetContent();

	int n=0;

	for (int i=0;i<PREV_WAV_INS;i++)
	{
		char buffer[64];
		sprintf(buffer,"%.3d:  %s",i,songRef->instName[i]);
		cc->AddString(buffer);
	}

	cc->SetCurSel(songRef->instSelected);
}

void CMainFrame::OnSelchangeBarComboins() 
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);

	int iSel=cc->GetCurSel();
	songRef->instSelected=iSel;
	m_wndInst.WaveUpdate();
	
	m_wndView.SetFocus();
}

void CMainFrame::OnCloseupBarComboins() {m_wndView.SetFocus();}

CMainFrame::StatusBarText(char *txt)
{
m_wndStatusBar.SetWindowText(txt);
}

void CMainFrame::OnClipbut() 
{
	songRef->machine[0]->clip=false;
	m_wndView.SetFocus();
}

CMainFrame::ShowInstrumentEditor()
{
m_wndInst.ShowWindow(SW_SHOWNORMAL);
m_wndInst.SetActiveWindow();
}

CMainFrame::HideInstrumentEditor()
{
m_wndInst.ShowWindow(SW_HIDE);
}

CMainFrame::ShowPerformanceDlg()
{
m_wndInfo.UpdateInfo();
m_wndInfo.ShowWindow(SW_SHOWNORMAL);
m_wndInfo.SetActiveWindow();
}

CMainFrame::HidePerformanceDlg()
{
m_wndInfo.ShowWindow(SW_HIDE);
}

CMainFrame::UpdateEnvInfo()
{
m_wndInfo.UpdateInfo();
}

void CMainFrame::OnSelchangeTrackcombo() 
{
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_TRACKCOMBO);
	songRef->SONGTRACKS=cc2->GetCurSel()+4;
	songRef->patTrack=0;
	m_wndView.Invalidate(true);
	m_wndView.SetFocus();
}

void CMainFrame::OnCloseupTrackcombo() 
{
	m_wndView.SetFocus();
}

void CMainFrame::OnPsyhelp() 
{
	if(!::WinHelp(this->m_hWnd,"psyhelp.hlp",HELP_FORCEFILE,0))
	MessageBox("Help failed","Help",MB_OK);
}

void CMainFrame::OnLoadwave() 
{
	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|FastTracker 2 Instruments (*.xi)|*.xi|Impulse Tracker Instruments(*.iti)|*.iti|All Files (*.*)|*.*||";
	
	CFileDialogEx dlg(true,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	dlg.songRef=songRef;
	dlg.DoModal();

	// Stopping wavepreview if started
	if(songRef->PW_Stage)
	songRef->PW_Stage=0;

	int si=songRef->instSelected;
	int sw=songRef->waveSelected;
	
	if(songRef->WavAlloc(si,sw,dlg.GetFileName()))
	{
		UpdateComboIns();
		m_wndStatusBar.SetWindowText("New wave loaded");
		WaveEditorBackUpdate();
		m_wndInst.WaveUpdate();
	}

	m_wndView.SetFocus();
}

void CMainFrame::OnEditwave() 
{
	ShowInstrumentEditor();
}

void CMainFrame::OnWavebut() 
{
	int si=songRef->instSelected;
	int sw=songRef->waveSelected;

	m_pWndWed->AdquireFromSong(si,sw,true);
}

void CMainFrame::OnDestroy() 
{
	CloseAllMacGuis();
	CFrameWnd::OnDestroy();	
}

void CMainFrame::OnBDecgen() 
{
	int si=songRef->seqBus;
	if(si>0)
	{
		CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
		--songRef->seqBus;
		cc->SetCurSel(songRef->seqBus);
	}

	m_wndView.SetFocus();
}

void CMainFrame::OnBIncgen() 
{
	int si=songRef->seqBus;
	if(si<(MAX_BUSES-1))
	{
		CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
		++songRef->seqBus;
		cc->SetCurSel(songRef->seqBus);
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnBDecwav() 
{
	int si=songRef->instSelected;
	
	if(si>0)
	{
		CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
		--songRef->instSelected;
		cc->SetCurSel(songRef->instSelected);
		WaveEditorBackUpdate();
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnBIncwav() 
{
	int si=songRef->instSelected;

	if(si<(PREV_WAV_INS-1))
	{
		CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
		++songRef->instSelected;
		cc->SetCurSel(songRef->instSelected);
		WaveEditorBackUpdate();
	}
	m_wndView.SetFocus();
}

CMainFrame::ShowMachineGui(int tmac)
{
	psyGear *ma=songRef->machine[tmac];

	if(isguiopen[tmac])
	{
	m_pWndMac[tmac]->SetActiveWindow();
	}
	else
	{
	m_pWndMac[tmac]=new CFrameMachine(0);
	m_pWndMac[tmac]->songRef=songRef;
	m_pWndMac[tmac]->guimatrix=isguiopen;

	m_pWndMac[tmac]->LoadFrame(
		IDR_MACHINEFRAME, 
		WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU,
	this);

	m_pWndMac[tmac]->Generate();
	m_pWndMac[tmac]->SelectMachine(tmac);
	
	isguiopen[tmac]=true;
	}
}


CMainFrame::CloseAllMacGuis()
{
	for(int c=0;c<MAX_MACHINES;c++)
	CloseMacGui(c);
}

CMainFrame::CloseMacGui(int mac)
{
	if(isguiopen[mac])
	{
		m_pWndMac[mac]->DestroyWindow();
		isguiopen[mac]=false;
	}
}

CMainFrame::WaveEditorBackUpdate()
{
	int const si=songRef->instSelected;
	int const sw=songRef->waveSelected;

	m_pWndWed->AdquireFromSong(si,sw,false);
}

void CMainFrame::OnClose() 
{
	if(MessageBox("Are you sure?","Quit Psycle",MB_YESNO)==IDYES)
	CFrameWnd::OnClose();
}

CMainFrame::ClosePsycle()
{
	OnClose();
}

void CMainFrame::OnVst2managerVst2pluginmanager() 
{
	CVSTHOSTDlg dlg;
	dlg.m_vst=&songRef->m_Vst;
	dlg.songRef=songRef;
	dlg.DoModal();	
}

CMainFrame::UpdateSequencer()
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	char buf[16];

	cc->ResetContent();

	for(int n=0;n<MAX_SONG_POSITIONS;n++)
	{
	sprintf(buf,"%.2X: %.2X",n,songRef->playOrder[n]);
	cc->AddString(buf);
	}

	cc->SetCurSel(songRef->editPosition);
}

void CMainFrame::OnSelchangeSeqlist() 
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const ep=cc->GetCurSel();

	int const cpos=songRef->editPosition;
	int const cpid=songRef->playOrder[cpos];

	if(ep!=songRef->editPosition)
	{
		songRef->editPosition=ep;
		UpdatePlayOrder(false);

		if(cpid!=songRef->playOrder[ep])
		m_wndView.Invalidate(false);
		
		m_wndView.SetFocus();
	}
	
}

void CMainFrame::OnDblclkSeqlist() 
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const ep=cc->GetCurSel();
	int const sep=songRef->editPosition;

	if(ep!=songRef->playOrder[sep])
	{
		songRef->playOrder[sep]=ep;
		UpdatePlayOrder(true);
		m_wndView.Invalidate(false);
		m_wndView.SetFocus();
	}

}


void CMainFrame::OnIncpat2() 
{
	int pop=songRef->editPosition;
	if(songRef->playOrder[pop]<(MAX_PATTERNS-1))
	{
	++songRef->playOrder[pop];
	UpdatePlayOrder(true);
	m_wndView.Repaint();
	m_wndView.SetActiveWindow();
	}
	m_wndView.SetFocus();	
}

void CMainFrame::OnDecpat2() 
{
	int pop=songRef->editPosition;
	if(songRef->playOrder[pop]>0)
	{
	--songRef->playOrder[pop];
	UpdatePlayOrder(true);
	m_wndView.Repaint();
	m_wndView.SetActiveWindow();
	}
	m_wndView.SetFocus();	
}

void CMainFrame::OnIncpos2() 
{
	if(songRef->editPosition<(MAX_SONG_POSITIONS-1))
	{
	++songRef->editPosition;
	UpdatePlayOrder(true);
	m_wndView.Repaint();
	m_wndView.SetActiveWindow();
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnDecpos2() 
{
	if(songRef->editPosition>0)
	{
	--songRef->editPosition;
	UpdatePlayOrder(true);
	m_wndView.Repaint();
	m_wndView.SetActiveWindow();
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnInclen() 
{
	if(songRef->playLength<(MAX_SONG_POSITIONS-1))
	{
		++songRef->playLength;
		UpdatePlayOrder(false);
	}
	m_wndView.SetFocus();

}

void CMainFrame::OnDeclen() 
{
	if(songRef->playLength>1)
	{
		--songRef->playLength;
		UpdatePlayOrder(false);
	}
	m_wndView.SetFocus();	
}


void CMainFrame::OnIncshort() 
{
	OnIncpat2();
}

void CMainFrame::OnDecshort() 
{
	OnDecpat2();	
}

void CMainFrame::OnSeqins() 
{
	int const pop=songRef->editPosition;

	for(int c=(MAX_SONG_POSITIONS-1);c>pop;c--)
	{
		songRef->playOrder[c]=songRef->playOrder[c-1];
	}
	songRef->playOrder[pop]=0;
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_wndView.SetFocus();
}

void CMainFrame::OnSeqslen() 
{
	int const pop=songRef->editPosition;

	if(songRef->playLength!=(pop+1))
	{
		songRef->playLength=pop+1;
		UpdatePlayOrder(false);	
	}

	m_wndView.SetFocus();
}

void CMainFrame::OnSeqspr()
{
	int const pop=songRef->editPosition;

	for(int c=0;c<pop;c++)
	songRef->playOrder[c]=c;

	UpdateSequencer();
	
	m_wndView.SetFocus();
	
}

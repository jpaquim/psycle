// MainFrm.cpp : implementation of the CMainFrame class
//

//////////////////////////////////////////////////////////////////////
// Include CTrack code

#include "stdafx.h"
#include "Psycle.h"
#include "MainFrm.h"
#include "WavFileDlg.h"

// Included dialogs
#include "MasterDlg.h"
#include "gearTracker.h"
#ifndef CONVERT_INTERNAL_MACHINES
#include "gearPsychosc.h"
#include "gearDistort.h"
#include "gearDelay.h"
#include "gearFilter.h"
#include "gearGainer.h"
#include "gearFlanger.h"
#endif
#include "FrameMachine.h"
#include "VstEditorDlg.h"
#include "Helpers.h"
#include "WireDlg.h"
#include "GearRackDlg.h"

#include "Player.h"
#include "MidiInput.h"
#include <math.h>
#include "inputhandler.h"
#include "KeyConfigDlg.h"
#include "AddTrackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CMainFrame
#define WM_SETMESSAGESTRING 0x0362


IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
//{{AFX_MSG_MAP(CMainFrame)
ON_EN_CHANGE(IDC_PATNAME_EDIT, OnPatternNameChange )
ON_WM_CREATE()
ON_WM_SETFOCUS()
ON_BN_CLICKED(IDC_BUTTON1, OnBarButton1)
ON_BN_CLICKED(IDC_BUTTON3, OnBpmAddTen)
ON_BN_CLICKED(IDC_BUTTON14, OnBpmDecOne)
ON_BN_CLICKED(IDC_BUTTON15, OnBpmDecTen)
ON_CBN_SELCHANGE(IDC_SSCOMBO2, OnSelchangeSscombo2)
ON_CBN_SELCHANGE(IDC_SEQCOMBO, OnSelchangeSeqCombo)
ON_CBN_CLOSEUP(IDC_SSCOMBO2, OnCloseupSscombo2)
ON_CBN_SELCHANGE(IDC_BAR_COMBOGEN, OnSelchangeBarCombogen)
ON_CBN_CLOSEUP(IDC_BAR_COMBOGEN, OnCloseupBarCombogen)
ON_CBN_SELCHANGE(IDC_BAR_COMBOINS, OnSelchangeBarComboins)
ON_CBN_CLOSEUP(IDC_BAR_COMBOINS, OnCloseupBarComboins)
ON_BN_CLICKED(IDC_CLIPBUT, OnClipbut)
ON_CBN_SELCHANGE(IDC_TRACKCOMBO, OnSelchangeTrackcombo)
ON_CBN_CLOSEUP(IDC_TRACKCOMBO, OnCloseupTrackcombo)
ON_COMMAND(ID_PSYHELP, OnPsyhelp)
ON_BN_CLICKED(IDC_SAVEWAVE, OnSavewave)
ON_BN_CLICKED(IDC_EDITWAVE, OnEditwave)
ON_BN_CLICKED(IDC_GEAR_RACK, OnGearRack)
ON_BN_CLICKED(IDC_WAVEBUT, OnWavebut)
ON_BN_CLICKED(IDC_PATCOLOUR, OnPatColour)
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_B_DECGEN, OnBDecgen)
ON_BN_CLICKED(IDC_B_INCGEN, OnBIncgen)
ON_BN_CLICKED(IDC_B_DECWAV, OnBDecwav)
ON_BN_CLICKED(IDC_B_INCWAV, OnBIncwav)
ON_WM_CLOSE()
ON_LBN_SELCHANGE(IDC_SEQLIST, OnSelchangeSeqlist)
ON_LBN_DBLCLK(IDC_SEQLIST, OnDblclkSeqlist)
ON_BN_CLICKED(IDC_SEQNEW, OnSeqnew)
ON_BN_CLICKED(IDC_SEQDELETE, OnSeqdelete)
ON_BN_CLICKED(IDC_SEQCLR, OnSeqclr)
ON_BN_CLICKED(IDC_SEQDUPLICATE, OnSeqduplicate)
ON_WM_ACTIVATE()
ON_BN_CLICKED(IDC_DEC_TPB, OnDecTPB)
ON_BN_CLICKED(IDC_INC_TPB, OnIncTPB)
ON_BN_CLICKED(IDC_FOLLOW, OnFollowSong)
ON_CBN_SELCHANGE(IDC_BAR_GENFX, OnSelchangeBarGenfx)
ON_CBN_CLOSEUP(IDC_BAR_GENFX, OnCloseupBarGenfx)
ON_BN_CLICKED(IDC_MULTICHANNEL_AUDITION, OnMultichannelAudition)
ON_BN_CLICKED(IDC_RECORD_NOTEOFF, OnRecordNoteoff)
ON_BN_CLICKED(IDC_RECORD_TWEAKS, OnRecordTweaks)
ON_CBN_CLOSEUP(IDC_AUXSELECT, OnCloseupAuxselect)
ON_CBN_SELCHANGE(IDC_AUXSELECT, OnSelchangeAuxselect)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_SEQPOS, OnUpdateIndicatorSeqPos)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_PATTERN, OnUpdateIndicatorPattern)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_LINE, OnUpdateIndicatorLine)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_TIME, OnUpdateIndicatorTime)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_EDIT, OnUpdateIndicatorEdit)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_FOLLOW, OnUpdateIndicatorFollow)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_NOTEOFF, OnUpdateIndicatorNoteoff)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_TWEAKS, OnUpdateIndicatorTweaks)
ON_UPDATE_COMMAND_UI(ID_INDICATOR_OCTAVE, OnUpdateIndicatorOctave)
	ON_CBN_CLOSEUP(IDC_COMBOOCTAVE, OnCloseupCombooctave)
	ON_CBN_SELCHANGE(IDC_COMBOOCTAVE, OnSelchangeCombooctave)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_VIEW_SONGBAR, OnViewSongbar)
	ON_COMMAND(ID_VIEW_MACHINEBAR, OnViewMachinebar)
	ON_COMMAND(ID_VIEW_SEQUENCERBAR, OnViewSequencerbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SONGBAR, OnUpdateViewSongbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SEQUENCERBAR, OnUpdateViewSequencerbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MACHINEBAR, OnUpdateViewMachinebar)
	ON_BN_CLICKED(IDC_NOTESTOEFFECTS, OnNotestoeffects)
ON_BN_CLICKED(IDC_LOADWAVE, OnLoadwave)
ON_MESSAGE (WM_SETMESSAGESTRING, OnSetMessageString)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_MASTERSLIDER, OnCustomdrawMasterslider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*
ON_BN_CLICKED(IDC_SEQSRT, OnSeqsort)
ON_BN_CLICKED(IDC_SEQCUT, OnSeqcut)
ON_BN_CLICKED(IDC_SEQCOPY, OnSeqcopy)
ON_BN_CLICKED(IDC_SEQPASTE, OnSeqpaste)
ON_BN_CLICKED(IDC_SEQINS, OnSeqins)
ON_BN_CLICKED(IDC_DECLEN, OnDeclen)
ON_BN_CLICKED(IDC_INCLEN, OnInclen)
*/

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_SEQPOS,
	ID_INDICATOR_PATTERN,
	ID_INDICATOR_LINE,
	ID_INDICATOR_TIME,
    ID_INDICATOR_OCTAVE,
    ID_INDICATOR_EDIT,
    ID_INDICATOR_FOLLOW,
    ID_INDICATOR_NOTEOFF,
    ID_INDICATOR_TWEAKS,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	Global::pInputHandler->SetMainFrame(this);
	vuprevR=0;
	vuprevL=0;
	seqcopybufferlength = 0;
	_pSong=NULL;
	pGearRackDialog = NULL;
}

CMainFrame::~CMainFrame()
{
	Global::pInputHandler->SetMainFrame(NULL);
	if (pGearRackDialog)
	{
		pGearRackDialog->OnCancel();
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_wndView.pParentFrame = this;
	macComboInitialized=false;
	
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
	// Create Toolbars.
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_wndControl.Create(this, IDD_PSYBAR, CBRS_ALIGN_TOP | CBRS_SIZE_DYNAMIC, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}
	if (!m_wndControl2.Create(this, IDD_PSYBAR2, CBRS_ALIGN_TOP | CBRS_SIZE_DYNAMIC, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}
	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) ||
		!m_wndReBar.AddBar(&m_wndControl) ||
		!m_wndReBar.AddBar(&m_wndControl2))
	{
		TRACE0("Failed to create rebar\n");
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

	m_wndView.ValidateParent();

	// CPU info Window
	m_wndInfo._pSong=_pSong;
	m_wndInfo.Create(IDD_INFO,this);

	// MIDI monitor Dialog
	m_midiMonitorDlg.Create(IDD_MIDI_MONITOR,this);

	// Instrument editor
	m_wndInst._pSong=_pSong;
	m_wndInst.pParentMain=this;
	m_wndInst.Create(IDD_INSTRUMENT,this);
	m_wndInst.Validate();

	// Wave Editor Window
	m_pWndWed=new CWaveEdFrame(this->_pSong,this);
	m_pWndWed->LoadFrame(IDR_WAVEFRAME ,WS_OVERLAPPEDWINDOW,this);
	m_pWndWed->GenerateView();

	// Init Bars Content.
	m_wndToolBar.SetWindowText("Psycle tool bar");
	m_wndControl.SetWindowText("Psycle control bar");
	CButton *cb;
	CSliderCtrl *cs;
	
	HBITMAP hi;
	blessless.LoadMappedBitmap(IDB_LESSLESS,0);
	bless.LoadMappedBitmap(IDB_LESS,0);
	bmore.LoadMappedBitmap(IDB_MORE,0);
	bmoremore.LoadMappedBitmap(IDB_MOREMORE,0);
	bplus.LoadMappedBitmap(IDB_PLUS,0);
	bminus.LoadMappedBitmap(IDB_MINUS,0);
	bplusplus.LoadMappedBitmap(IDB_PLUSPLUS,0);
	bminusminus.LoadMappedBitmap(IDB_MINUSMINUS,0);
	blittleleft.LoadMappedBitmap(IDB_LLEFT,0);
	blittleright.LoadMappedBitmap(IDB_LRIGHT,0);

	cb=(CButton *)m_wndControl.GetDlgItem(IDC_BUTTON15);
	hi = (HBITMAP)blessless; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl.GetDlgItem(IDC_BUTTON14);
	hi = (HBITMAP)bless; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl.GetDlgItem(IDC_BUTTON1);
	hi = (HBITMAP)bmore; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl.GetDlgItem(IDC_BUTTON3);
	hi = (HBITMAP)bmoremore; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl.GetDlgItem(IDC_DEC_TPB);
	hi = (HBITMAP)bless; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl.GetDlgItem(IDC_INC_TPB);
	hi = (HBITMAP)bmore; cb->SetBitmap(hi);
	
	cs=(CSliderCtrl*)m_wndControl.GetDlgItem(IDC_MASTERSLIDER);
	cs->SetRange(0,1024);
	cs->SetPos(512);
	cs->SetTicFreq(64);

	cs->SetPageSize(4);


	m_wndControl2.SetWindowText("Psycle control bar 2");

	cb=(CButton*)m_wndControl2.GetDlgItem(IDC_B_DECGEN);
	hi = (HBITMAP)blittleleft; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl2.GetDlgItem(IDC_B_INCGEN);
	hi = (HBITMAP)blittleright; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl2.GetDlgItem(IDC_B_DECWAV);
	hi = (HBITMAP)blittleleft; cb->SetBitmap(hi);

	cb=(CButton*)m_wndControl2.GetDlgItem(IDC_B_INCWAV);
	hi = (HBITMAP)blittleright; cb->SetBitmap(hi);


	CComboBox *cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_TRACKCOMBO);
	for(int i=4;i<=MAX_TRACKS;i++)
	{
		char s[4];
		_snprintf(s,4,"%i",i);
		cc2->AddString(s);
	}
	cc2->SetCurSel(_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->SONGTRACKS-4);

//	SetAppSongBpm(0);
//	SetAppSongTpb(0);

//	cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);

//	for(i=0;i<=16;i++)
//	{
//		char s[4];
//		_snprintf(s,4,"%i",i);
//		cc2->AddString(s);
//	}
//	cc2->SetCurSel(1);
	
//	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_GENFX);
//   cc->SetCurSel(0);
	
//	cc =(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
//	cc->AddString("MIDI");
//	cc->AddString("Params");
//	cc->AddString("Waves");
	
//	UpdateComboGen(); // Initializes Gen and Ins combobox.

	PsybarsUpdate();
	CComboBox *cc =(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
	cc->SetCurSel(2);


	// Sequencer Bar
	m_wndSeq.Create(this,IDD_SEQUENCER,CBRS_LEFT,AFX_IDW_DIALOGBAR);

	// set multichannel audition checkbox status
	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_MULTICHANNEL_AUDITION);
	cb->SetCheck(Global::pInputHandler->bMultiKey?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_RECORD_NOTEOFF);
	cb->SetCheck(Global::pConfig->_RecordNoteoff?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_RECORD_TWEAKS);
	cb->SetCheck(Global::pConfig->_RecordTweaks?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_NOTESTOEFFECTS);
	cb->SetCheck(Global::pConfig->_notesToEffects?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_FOLLOW);
	cb->SetCheck(Global::pConfig->_followSong?1:0);

	//cb=(CButton*)m_wndSeq.GetDlgItem(IDC_INCSHORT);
	//hi = (HBITMAP)bplus; cb->SetBitmap(hi);

	//cb=(CButton*)m_wndSeq.GetDlgItem(IDC_INCLONG);
	//hi = (HBITMAP)bplusplus; cb->SetBitmap(hi);

	//cb=(CButton*)m_wndSeq.GetDlgItem(IDC_DECSHORT);
	//hi = (HBITMAP)bminus; cb->SetBitmap(hi);
	
	//cb=(CButton*)m_wndSeq.GetDlgItem(IDC_DECLONG);
	//hi = (HBITMAP)bminusminus; cb->SetBitmap(hi);

	//cb=(CButton*)m_wndSeq.GetDlgItem(IDC_DECLEN);
	//hi = (HBITMAP)bless; cb->SetBitmap(hi);

	//cb=(CButton*)m_wndSeq.GetDlgItem(IDC_INCLEN);
	//hi = (HBITMAP)bmore; cb->SetBitmap(hi);
	
	UpdatePlayOrder(true);
	
	DragAcceptFiles(TRUE);

	Global::pConfig->CreateFonts();

	// Finally initializing timer
	
	UpdateSequencer();
	CListBox *seq=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	seq->SetCurSel(0);

	m_wndView.InitTimer();
//	m_wndView.Repaint();
	m_wndView.SetFocus();
//	m_wndView.EnableSound();
	
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
	m_wndView.Repaint();
	m_wndView.SetFocus();
	m_wndView.EnableSound();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	
	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	if (((nState == WA_ACTIVE) || (nState == WA_CLICKACTIVE)) && (bMinimized == FALSE))
	{
	}
}

void CMainFrame::OnClose() 
{
	if (m_wndView.CheckUnsavedSong("Exit Psycle"))
	{
		CloseAllMacGuis();
		m_wndView._outputActive = false;
		Global::pPlayer->Stop();
		Global::pConfig->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();

		//Recent File List;
		((CPsycleApp*)AfxGetApp())->SaveRecent(this);
		
		CFrameWnd::OnClose();
	}
}

void CMainFrame::ClosePsycle()
{
	OnClose();
}

void CMainFrame::OnDestroy() 
{
	HICON _icon = GetIcon(false);
	DestroyIcon(_icon);
	CFrameWnd::OnDestroy();
}

void CMainFrame::StatusBarText(char *txt)
{
	m_wndStatusBar.SetWindowText(txt);
}

void CMainFrame::PsybarsUpdate()
{
	SetAppSongBpm(0);
	SetAppSongTpb(0);

	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
	cc2->SetCurSel(m_wndView.patStep);
	
	cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_TRACKCOMBO);
	cc2->SetCurSel(_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->SONGTRACKS-4);

	cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	cc2->SetCurSel(_pSong->currentOctave);

	cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SEQCOMBO);
	cc2->SetCurSel(6); // 1,2,4,8,16,32,64,
	
	UpdateComboGen();
	
}

/////////////////
///////////////// PSY Bar 1
/////////////////

void CMainFrame::OnSelchangeTrackcombo() 
{
	CComboBox *cc2 = (CComboBox *)m_wndControl.GetDlgItem(IDC_TRACKCOMBO);
	_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->SONGTRACKS = cc2->GetCurSel()+4;
	if ( m_wndView.editcur.track >= _pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->SONGTRACKS )
		m_wndView.editcur.track = _pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->SONGTRACKS-1;

	m_wndView.RecalculateColourGrid();
	m_wndView.Repaint();
	m_wndView.SetFocus();
}

void CMainFrame::OnCloseupTrackcombo() 
{
	m_wndView.SetFocus();
}

void CMainFrame::OnBarButton1()  // OnBpmAddOne
{
	SetAppSongBpm(1);
	m_wndView.SetFocus();	
}

void CMainFrame::OnBpmAddTen() 
{
	SetAppSongBpm(10);
	m_wndView.SetFocus();	
}

void CMainFrame::OnBpmDecOne() 
{
	SetAppSongBpm(-1);
	m_wndView.SetFocus();	
}

void CMainFrame::OnBpmDecTen() 
{
	SetAppSongBpm(-10);
	m_wndView.SetFocus();	
}

void CMainFrame::OnDecTPB()
{
	SetAppSongTpb(-1);
	m_wndView.SetFocus();
	m_wndView.Repaint();
}

void CMainFrame::OnIncTPB()
{
	SetAppSongTpb(+1);
	m_wndView.SetFocus();
	m_wndView.Repaint();
}

void CMainFrame::SetAppSongBpm(int x) 
{
	char buffer[16];
	if (Global::pPlayer->_playing )
	{
		if ( x == 0 ) sprintf(buffer,"%d",Global::pPlayer->bpm);
		else
		{
			Global::pPlayer->bpm+=x;
			int const cur= m_wndView.SongIncBpm(Global::pPlayer->bpm - Global::_pSong->BeatsPerMin);
			sprintf(buffer,"%d",cur);
		}
	}
	else
	{
		if ( x != 0 ) m_wndView.SongIncBpm(x);
		sprintf(buffer,"%d",Global::_pSong->BeatsPerMin);
	}
	CStatic *tmplab=(CStatic *)m_wndControl.GetDlgItem(IDC_BPMLABEL);
	tmplab->SetWindowText(buffer);
}

void CMainFrame::SetAppSongTpb(int x) 
{
	char buffer[16];

	if ( x != 0)
	{
		Global::_pSong->_ticksPerBeat += x;
		
		if (Global::_pSong->_ticksPerBeat < 1) 	Global::_pSong->_ticksPerBeat = 1;
		else if (Global::_pSong->_ticksPerBeat > 32) Global::_pSong->_ticksPerBeat = 32;

		Global::_pSong->SetBPM(Global::_pSong->BeatsPerMin, Global::_pSong->_ticksPerBeat, Global::pConfig->_pOutputDriver->_samplesPerSec);
		Global::pPlayer->tpb=Global::_pSong->_ticksPerBeat;
	}
	
	if (Global::pPlayer->_playing ) sprintf(buffer, "%d", Global::pPlayer->tpb);
	else sprintf(buffer, "%d", Global::_pSong->_ticksPerBeat);
	((CStatic *)m_wndControl.GetDlgItem(IDC_TPBLABEL))->SetWindowText(buffer);
}


void CMainFrame::OnCloseupCombooctave() 
{
	m_wndView.SetFocus();
}

void CMainFrame::OnSelchangeCombooctave() 
{
	CComboBox *cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	_pSong->currentOctave=cc2->GetCurSel();
	
	m_wndView.Repaint();
	m_wndView.SetFocus();
}
//////////////////////////////////////////////////////////////////////
// Function that shift the current editing octave

void CMainFrame::ShiftOctave(int x)
{
	_pSong->currentOctave += x;
	if ( _pSong->currentOctave < 0 )	 { _pSong->currentOctave = 0; }
	else if ( _pSong->currentOctave > 8 ){ _pSong->currentOctave = 8; }
	
	CComboBox *cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	cc2->SetCurSel(_pSong->currentOctave);
}
void CMainFrame::UpdateMasterValue(int newvalue)
{
	CSliderCtrl *cs;
	if ( _pSong->_pMachine[MASTER_INDEX] != NULL)
	{
		cs=(CSliderCtrl*)m_wndControl.GetDlgItem(IDC_MASTERSLIDER);
		cs->SetPos(newvalue);
	}
}

void CMainFrame::OnCustomdrawMasterslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CSliderCtrl *cs;
	if ( _pSong->_pMachine[MASTER_INDEX] != NULL)
	{
		cs=(CSliderCtrl*)m_wndControl.GetDlgItem(IDC_MASTERSLIDER);
		//((Master*)_pSong->_pMachine[MASTER_INDEX])->_outDry = cs->GetPos()*cs->GetPos()/1024;
		((Master*)_pSong->_pMachine[MASTER_INDEX])->_outDry = cs->GetPos();

		m_wndView.SetFocus();
	}
	
	*pResult = 0;
}

void CMainFrame::OnClipbut() 
{
	((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->_clip = false;
	m_wndView.SetFocus();
}

//l and r are the left and right vu meter values
void CMainFrame::UpdateVumeters(float l, float r,COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip)
{
	if (Global::pConfig->draw_vus)
	{
		if(l<1)l=1;
		if(r<1)r=1;
		
		CStatic *lc=(CStatic *)m_wndControl.GetDlgItem(IDC_FRAMECLIP);
		CClientDC clcanvasl(lc);
		
		if (clip) clcanvasl.FillSolidRect(0,0,9,20,vu3);
		else  clcanvasl.FillSolidRect(0,0,9,20,vu2);
		
	//	bool draw_l=true;
	//	bool draw_r=true;

	//	float log_l=20*(float)log10(l/baselevel);				// Standard formula
	//  float log_l=20*(float)log10(l) - 20*log10(vaselevel);	// simplified (speedwise) formula.
	//  float log_l=(226pixels/90db)*20*(float)log10(l) // Formula for 16bit precision. (15bit, in fact)

	/*	float log_l=50.0f*(float)log10((0.000030517578125f*(float)l));
		float log_r=50.0f*(float)log10((0.000030517578125f*(float)r));
		
		if(log_l<-220.0f)
			draw_l=false;
		
		if(log_r<-220.0f)
			draw_r=false;
		
		if(log_l>0)log_l=0;
		if(log_r>0)log_r=0;
		
		int cl=226+(int)log_l;
		int cr=226+(int)log_r;
	*/	
		CStatic *lv=(CStatic *)m_wndControl.GetDlgItem(IDC_LVUM);
		CStatic *rv=(CStatic *)m_wndControl.GetDlgItem(IDC_RVUM);
		CClientDC canvasl(lv);
		CClientDC canvasr(rv);

		int log_l=f2i(100*log10f(l));
		int log_r=f2i(100*log10f(r));
		log_l=log_l-225;
		if ( log_l < 0 )log_l=0;
		log_r=log_r-225;
		if ( log_r < 0 )log_r=0;
		
		if (log_l || vuprevL)
		{
//			canvasl.FillSolidRect(0,0,log_l,5,vu1);
			canvasl.FillSolidRect(0,0,log_l,4,vu1);
			if (vuprevL > log_l )
			{
				canvasl.FillSolidRect(log_l,0,vuprevL-log_l,4,vu3);
				canvasl.FillSolidRect(vuprevL,0,225-vuprevL,4,vu2);
				vuprevL-=2;
			}
			else 
			{
				canvasl.FillSolidRect(log_l,0,225-log_l,4,vu2);
				vuprevL = log_l;
			}
		}
		else
//			canvasl.FillSolidRect(0,0,226,5,vu2);
			canvasl.FillSolidRect(0,0,225,4,vu2);

		if (log_r || vuprevR)
		{
//			canvasr.FillSolidRect(0,0,log_r,5,vu1);
			canvasl.FillSolidRect(0,5,log_r,4,vu1);
			
			if (vuprevR > log_r )
			{
				canvasl.FillSolidRect(log_r,5,vuprevR-log_r,4,vu3);
				canvasl.FillSolidRect(vuprevR,5,225-vuprevR,4,vu2);
				vuprevR-=2;
			}
			else 
			{
				canvasl.FillSolidRect(log_r,5,225-log_r,4,vu2);
				vuprevR = log_r;
			}
		}
		else
			canvasl.FillSolidRect(0,5,225,4,vu2);

	/*	if(draw_l)
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
	*/	
	}
}

/////////////////
///////////////// PSY Bar 2
/////////////////

void CMainFrame::OnSelchangeSscombo2()  // OnChangePatternStep
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
	int sel=cc->GetCurSel();
	m_wndView.patStep=sel;
	m_wndView.SetFocus();
}

void CMainFrame::OnSelchangeSeqCombo()  // OnChangeSeqPatternStep
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SEQCOMBO);
	m_wndView.seqCursor.rowStep = (int)pow(2,cc->GetCurSel());
	_pSong->patStep = m_wndView.seqCursor.rowStep;
	m_wndView.RedrawPatternSeq();
	m_wndView.SetFocus();
}


void CMainFrame::OnCloseupSscombo2()	// OnCloseupPatternStep
{
	m_wndView.SetFocus();
}

void CMainFrame::EditQuantizeChange(int diff) // User Called (Hotkey)
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);	
	const int total = cc->GetCount();
	const int nextsel = (total + cc->GetCurSel() + diff) % total;
	cc->SetCurSel(nextsel);
	m_wndView.patStep=nextsel;
}

void CMainFrame::OnSelchangeBarGenfx() 
{
	CComboBox *cc1=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_GENFX);
	_pSong->seqBus &= (MAX_BUSES-1);
	if ( cc1->GetCurSel() == 1 )
	{
		_pSong->seqBus |= MAX_BUSES;
	}
	UpdateComboGen();
}

void CMainFrame::OnCloseupBarGenfx() 
{
	m_wndView.SetFocus();
}

void CMainFrame::OnBDecgen() 
{
	//	ChangeGen(_pSong->seqBus-1);
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	const int val = cc->GetCurSel();
	if ( val > 0 ) cc->SetCurSel(val-1);
	OnSelchangeBarCombogen();
	m_wndView.SetFocus();
}

void CMainFrame::OnBIncgen() 
{
	//	ChangeGen(_pSong->seqBus+1);
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	const int val = cc->GetCurSel();
	if ( val < cc->GetCount()-1 ) cc->SetCurSel(val+1);
	OnSelchangeBarCombogen();
	m_wndView.SetFocus();
}

void CMainFrame::UpdateComboGen(bool updatelist)
{
	bool filled=false;
	bool found=false;
	int selected = -1;
	int line = -1;
	char buffer[64];
	
	if (_pSong == NULL) 
	{
		return; // why should this happen?
	}
	CComboBox *cb=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	CComboBox *cb2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
	
	macComboInitialized = false;
	if (updatelist) 
	{
		cb->ResetContent();
	}
	
	for (int b=0; b<MAX_BUSES; b++) // Check Generators
	{
		if( _pSong->_pMachine[b])
		{
			if (updatelist)
			{	
				sprintf(buffer,"%.2X: %s",b,_pSong->_pMachine[b]->_editName);
				cb->AddString(buffer);
			}
			if (!found) 
			{
				selected++;
			}
			if (_pSong->seqBus == b) 
			{
				found = true;
			}
			filled = true;
		}
	}
	if ( updatelist) 
	{
		cb->AddString("----------------------------------------------------");
	}
	if (!found) 
	{
		selected++;
		line = selected;
	}
	
	for (b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
	{
		if(_pSong->_pMachine[b])
		{
			if (updatelist)
			{	
				sprintf(buffer,"%.2X: %s",b,_pSong->_pMachine[b]->_editName);
				cb->AddString(buffer);
			}
			if (!found) 
			{
				selected++;
			}
			if (_pSong->seqBus == b) 
			{
				found = true;
			}
			filled = true;
		}
	}
	if (!filled)
	{
		cb->ResetContent();
		cb->AddString("No Machines Loaded");
		selected = 0;
	}
	else if (!found) 
	{
		selected=line;
	}
	
	cb->SetCurSel(selected);

	// Select the appropiate Option in Aux Combobox.
	if (found) // If found (which also means, if it exists)
	{
		if (_pSong->_pMachine[_pSong->seqBus])
		{
			if ( _pSong->seqBus < MAX_BUSES ) // it's a Generator
			{
				if (_pSong->_pMachine[_pSong->seqBus]->_type == MACH_SAMPLER)
				{
					cb2->SetCurSel(AUX_WAVES);
					_pSong->auxcolSelected = _pSong->instSelected;
				}
				else if (_pSong->_pMachine[_pSong->seqBus]->_type == MACH_VST)
				{
					if ( cb2->GetCurSel() == AUX_WAVES)
					{
						cb2->SetCurSel(AUX_MIDI);
						_pSong->auxcolSelected = _pSong->midiSelected;
					}
				}
				else
				{
					cb2->SetCurSel(AUX_PARAMS);
					_pSong->auxcolSelected = 0;
				}
			}
			else
			{
				cb2->SetCurSel(AUX_PARAMS);
				_pSong->auxcolSelected = 0;
			}
		}
	}
	else
	{
		cb2->SetCurSel(AUX_WAVES); // WAVES
		_pSong->auxcolSelected = _pSong->instSelected;
	}
	UpdateComboIns();
	macComboInitialized = true;
}

void CMainFrame::OnSelchangeBarCombogen() 
{
	if(macComboInitialized)
	{
		CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
		int nsb = GetNumFromCombo(cc);

		if(_pSong->seqBus!=nsb)
		{
			_pSong->seqBus=nsb;
			UpdateComboGen(false);
		}
		RedrawGearRackList();
		//Added by J.Redfern, repaints main view after changing selection in combo
		m_wndView.Repaint();

	}
}

void CMainFrame::OnCloseupBarCombogen()
{
	m_wndView.SetFocus();
}


void CMainFrame::ChangeGen(int i)	// User Called (Hotkey)
{
	if(i>=0 && i <(MAX_BUSES*2))
	{
		if ( (_pSong->seqBus & MAX_BUSES) == (i & MAX_BUSES))
		{
			_pSong->seqBus=i;
			UpdateComboGen(false);
		}
		else
		{
			_pSong->seqBus=i;
			UpdateComboGen(true);
		}
	}
}

void CMainFrame::OnCloseupAuxselect() 
{
	m_wndView.SetFocus();
}

void CMainFrame::OnSelchangeAuxselect() 
{
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if ( cc2->GetCurSel() == AUX_MIDI )	// MIDI
	{
		_pSong->auxcolSelected=_pSong->midiSelected;
	}
	else if ( cc2->GetCurSel() == AUX_WAVES )	// WAVES
	{
		_pSong->auxcolSelected=_pSong->instSelected;
	}
	UpdateComboIns();
}
void CMainFrame::OnBDecwav() 
{
	ChangeIns(_pSong->auxcolSelected-1);
	m_wndView.SetFocus();
}

void CMainFrame::OnBIncwav() 
{
	ChangeIns(_pSong->auxcolSelected+1);
	m_wndView.SetFocus();
}

void CMainFrame::UpdateComboIns(bool updatelist)
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	int listlen = 0;
	
	if (updatelist) 
	{
		cc->ResetContent();
	}

	if ( cc2->GetCurSel() == AUX_MIDI )	// MIDI
	{
		char buffer[64];
		if (updatelist) 
		{
			for (int i=0;i<16;i++)
			{
				sprintf(buffer, "%.2X: MIDI Channel %.2i", i,i+1);
				cc->AddString(buffer);
			}
		}
		listlen = 16;
//		_pSong->midiSelected=_pSong->auxcolSelected;
	}
	else if ( cc2->GetCurSel() == AUX_PARAMS)	// Params
	{

		int nmac = _pSong->seqBus;
		Machine *tmac = _pSong->_pMachine[nmac];
		if (tmac) 
		{
			int i=0;
			if (updatelist) 
			{
				for (i=0;i<tmac->GetNumParams();i++)
				{
					char buffer[64],buffer2[64];
					memset(buffer2,0,64);
					tmac->GetParamName(i,buffer2);
					sprintf(buffer, "%.2X:  %s", i, buffer2);
					cc->AddString(buffer);
					listlen++;
				}
			}
			else
			{
				listlen = cc->GetCount();
			}
		}
		else
		{
			if (updatelist) 
			{
				cc->AddString("No Machine");
			}
			listlen = 1;
		}
	}
	else	// Waves
	{
		char buffer[64];
		if (updatelist) 
		{
			for (int i=0;i<PREV_WAV_INS;i++)
			{
				sprintf(buffer, "%.2X: %s", i, _pSong->_pInstrument[i]->_sName);
				cc->AddString(buffer);
				listlen++;
			}
		}
		else
		{
			listlen = cc->GetCount();
		}
//		_pSong->instSelected=_pSong->auxcolSelected;
//		WaveEditorBackUpdate();
//		m_wndInst.WaveUpdate();
//		RedrawGearRackList();
	}
	if (_pSong->auxcolSelected >= listlen)
	{
		_pSong->auxcolSelected = 0;
	}
	cc->SetCurSel(_pSong->auxcolSelected);
}

void CMainFrame::OnSelchangeBarComboins() 
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if ( cc2->GetCurSel() == AUX_MIDI ) 
	{
		_pSong->midiSelected=cc->GetCurSel();
	}
	else if ( cc2->GetCurSel() == AUX_WAVES ) 
	{
		_pSong->instSelected=cc->GetCurSel();
		WaveEditorBackUpdate();
		m_wndInst.WaveUpdate();
		RedrawGearRackList();
	}

	_pSong->auxcolSelected=cc->GetCurSel();
}

void CMainFrame::OnCloseupBarComboins()
{
	m_wndView.SetFocus();
}

void CMainFrame::ChangeIns(int i)	// User Called (Hotkey)
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if (cc2->GetCurSel() == AUX_MIDI )
	{
		if (i>=0 && i <16)
		{
			_pSong->midiSelected=i;
			_pSong->auxcolSelected=i;
		}
	}
	else if ( cc2->GetCurSel() == AUX_PARAMS )
	{
		if (i>=0 && i < cc->GetCount() )
		{
			_pSong->auxcolSelected=i;
		}
	}
	else
	{
		if(i>=0 && i <(PREV_WAV_INS))
		{
			_pSong->instSelected=i;
			_pSong->auxcolSelected=i;
			WaveEditorBackUpdate();
			m_wndInst.WaveUpdate();
			RedrawGearRackList();
		}
	}
	cc->SetCurSel(_pSong->auxcolSelected);
}

void CMainFrame::OnLoadwave() 
{
	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|IFF Samples (*.iff)|*.iff|All Files (*.*)|*.*||";
	
	CWavFileDlg dlg(true,"wav", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg._pSong = _pSong;
	dlg.m_ofn.lpstrInitialDir = Global::pConfig->GetInstrumentDir();
	if (dlg.DoModal() == IDOK)
	{
		m_wndView.AddMacViewUndo();

		int si = _pSong->instSelected;
		int sw = _pSong->waveSelected;

		CString CurrExt=dlg.GetFileExt();
		CurrExt.MakeLower();
		
		if ( CurrExt == "wav" )
		{
			if (_pSong->WavAlloc(si,sw,dlg.GetFileName()))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText("New wave loaded");
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
			}
		}
		else if ( CurrExt == "iff" )
		{
			if (_pSong->IffAlloc(si,sw,dlg.GetFileName()))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText("New wave loaded");
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
				RedrawGearRackList();
			}
		}
		CString str = dlg.m_ofn.lpstrFile;
		int index = str.ReverseFind('\\');
		if (index != -1)
		{
			Global::pConfig->SetInstrumentDir(str.Left(index));
		}
	}
	if ( _pSong->_pInstrument[PREV_WAV_INS]->waveLength[0] > 0)
	{
		// Stopping wavepreview if not stopped.
		if(_pSong->PW_Stage)
		{
			_pSong->PW_Stage=0;
			_pSong->Invalided=true;
			Sleep(LOCK_LATENCY);
		}

		//Delete it.
		_pSong->DeleteLayer(PREV_WAV_INS,0);
		_pSong->Invalided=false;
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnSavewave()
{
	unsigned int c=0;
	WaveFile output;
	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
	
	if (_pSong->_pInstrument[_pSong->instSelected]->waveLength[_pSong->waveSelected])
	{
		CFileDialog dlg(FALSE, "wav", _pSong->_pInstrument[_pSong->instSelected]->waveName[_pSong->waveSelected], OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
		if (dlg.DoModal() == IDOK)
		{
			output.OpenForWrite(dlg.GetFileName(), 44100, 16, (_pSong->_pInstrument[_pSong->instSelected]->waveStereo[_pSong->waveSelected]) ? (2) : (1) );
			if (_pSong->_pInstrument[_pSong->instSelected]->waveStereo[_pSong->waveSelected])
			{
				for (c=0; c < _pSong->_pInstrument[_pSong->instSelected]->waveLength[_pSong->waveSelected]; c++)
				{
					output.WriteStereoSample( *(_pSong->_pInstrument[_pSong->instSelected]->waveDataL[_pSong->waveSelected] + c), *(_pSong->_pInstrument[_pSong->instSelected]->waveDataR[_pSong->waveSelected] + c) );
				}
			}
			else
			{
				output.WriteData(_pSong->_pInstrument[_pSong->instSelected]->waveDataL[_pSong->waveSelected], _pSong->_pInstrument[_pSong->instSelected]->waveLength[_pSong->waveSelected]);
			}

			output.Close();
		}
	}
	else MessageBox("Nothing to save...\nSelect nonempty wave first.", "Error", MB_ICONERROR);
}

void CMainFrame::OnGearRack() 
{
	if (pGearRackDialog == NULL)
	{
		pGearRackDialog = new CGearRackDlg(&m_wndView, this);
		pGearRackDialog->Create();
		pGearRackDialog->ShowWindow(SW_SHOW);
	}
}

void CMainFrame::OnEditwave() 
{
	ShowInstrumentEditor();
}

void CMainFrame::OnWavebut() 
{
	m_pWndWed->ShowWindow(SW_SHOWNORMAL);
	m_pWndWed->SetActiveWindow();
}

void CMainFrame::OnPatColour() 
{
	CColorDialog dlg(Global::_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patColor);
	if(dlg.DoModal() == IDOK)
	{
		Global::_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patColor = dlg.GetColor();
	}
}

void CMainFrame::WaveEditorBackUpdate()
{
	m_pWndWed->Notify();
}

void CMainFrame::ShowInstrumentEditor()
{
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
	cc2->SetCurSel(AUX_WAVES);
	_pSong->auxcolSelected=_pSong->instSelected;
	UpdateComboIns();

	m_wndView.AddMacViewUndo();

	m_wndInst.WaveUpdate();
	m_wndInst.ShowWindow(SW_SHOWNORMAL);
	m_wndInst.SetActiveWindow();
}

void CMainFrame::HideInstrumentEditor()
{
	m_wndInst.ShowWindow(SW_HIDE);
}

//////////////////
////////////////// Some Menu Commands plus ShowMachineGui
//////////////////


void CMainFrame::ShowPerformanceDlg()
{
	m_wndInfo.UpdateInfo();
	m_wndInfo.ShowWindow(SW_SHOWNORMAL);
	m_wndInfo.SetActiveWindow();
}

void CMainFrame::HidePerformanceDlg()
{
	m_wndInfo.ShowWindow(SW_HIDE);
}

void CMainFrame::ShowMidiMonitorDlg()
{
	m_midiMonitorDlg.UpdateInfo();
	m_midiMonitorDlg.ShowWindow(SW_SHOWNORMAL);
	m_midiMonitorDlg.SetActiveWindow();
}

void CMainFrame::HideMidiMonitorDlg()
{
	m_midiMonitorDlg.ShowWindow(SW_HIDE);
}

void CMainFrame::UpdateEnvInfo()
{
	m_wndInfo.UpdateInfo();
}


void CMainFrame::OnPsyhelp() 
{
	MessageBox("No one has done the Help file yet, sorry","Psycle");
//	HtmlHelp(NULL, "psycle.chm", HH_DISPLAY_TOPIC, 0);
}

void CMainFrame::ShowMachineGui(int tmac, CPoint point)
{
	Machine *ma = _pSong->_pMachine[tmac];

	if (ma)
	{
		if (isguiopen[tmac])
		{
			m_pWndMac[tmac]->SetActiveWindow();
		}
		else
		{
			m_wndView.AddMacViewUndo();

			switch (ma->_type)
			{
			case MACH_MASTER:
				if (!m_wndView.MasterMachineDialog)
				{
					m_wndView.MasterMachineDialog = new CMasterDlg(&m_wndView);
					m_wndView.MasterMachineDialog->_pMachine = (Master*)ma;
					for (int i=0;i<MAX_CONNECTIONS; i++)
					{
						if ( ma->_inputCon[i])
						{
							if (_pSong->_pMachine[ma->_inputMachines[i]])
							{
								strcpy(m_wndView.MasterMachineDialog->macname[i],_pSong->_pMachine[ma->_inputMachines[i]]->_editName);
							}
						}
					}
					m_wndView.MasterMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.MasterMachineDialog, point);
					m_wndView.MasterMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_SAMPLER:
				if (m_wndView.SamplerMachineDialog)
				{
					if (m_wndView.SamplerMachineDialog->_pMachine != (Sampler*)ma)
					{
						m_wndView.SamplerMachineDialog->OnCancel();
						m_wndView.SamplerMachineDialog = new CGearTracker(&m_wndView);
						m_wndView.SamplerMachineDialog->_pMachine = (Sampler*)ma;
						m_wndView.SamplerMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.SamplerMachineDialog, point);
						m_wndView.SamplerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.SamplerMachineDialog = new CGearTracker(&m_wndView);
					m_wndView.SamplerMachineDialog->_pMachine = (Sampler*)ma;
					m_wndView.SamplerMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.SamplerMachineDialog, point);
					m_wndView.SamplerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
#ifndef CONVERT_INTERNAL_MACHINES
			case MACH_SINE:
				if (m_wndView.PsychMachineDialog)
				{
					if (m_wndView.PsychMachineDialog->_pMachine != (Sine*)ma)
					{
						m_wndView.PsychMachineDialog->OnCancel();
						m_wndView.PsychMachineDialog = new CGearPsychOsc(&m_wndView);
						m_wndView.PsychMachineDialog->_pMachine = (Sine*)ma;
						m_wndView.PsychMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.PsychMachineDialog, point);
						m_wndView.PsychMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.PsychMachineDialog = new CGearPsychOsc(&m_wndView);
					m_wndView.PsychMachineDialog->_pMachine = (Sine*)ma;
					m_wndView.PsychMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.PsychMachineDialog, point);
					m_wndView.PsychMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_DIST:
				if (m_wndView.DistortionMachineDialog)
				{
					if (m_wndView.DistortionMachineDialog->_pMachine != (Distortion*)ma)
					{
						m_wndView.DistortionMachineDialog->OnCancel();
						m_wndView.DistortionMachineDialog = new CGearDistort(&m_wndView);
						m_wndView.DistortionMachineDialog->_pMachine = (Distortion*)ma;
						m_wndView.DistortionMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.DistortionMachineDialog, point);
						m_wndView.DistortionMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.DistortionMachineDialog = new CGearDistort(&m_wndView);
					m_wndView.DistortionMachineDialog->_pMachine = (Distortion*)ma;
					m_wndView.DistortionMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.DistortionMachineDialog, point);
					m_wndView.DistortionMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_DELAY:
				if (m_wndView.DelayMachineDialog)
				{
					if (m_wndView.DelayMachineDialog->_pMachine != (Delay*)ma)
					{
						m_wndView.DelayMachineDialog->OnCancel();
						m_wndView.DelayMachineDialog = new CGearDelay(&m_wndView);
						m_wndView.DelayMachineDialog->_pMachine = (Delay*)ma;
						m_wndView.DelayMachineDialog->SPT = &Global::_pSong->SamplesPerTick;
						m_wndView.DelayMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.DelayMachineDialog, point);
						m_wndView.DelayMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.DelayMachineDialog = new CGearDelay(&m_wndView);
					m_wndView.DelayMachineDialog->_pMachine = (Delay*)ma;
					m_wndView.DelayMachineDialog->SPT = &Global::_pSong->SamplesPerTick;
					m_wndView.DelayMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.DelayMachineDialog, point);
					m_wndView.DelayMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_2PFILTER:
				if (m_wndView.FilterMachineDialog)
				{
					if (m_wndView.FilterMachineDialog->_pMachine != (Filter2p*)ma)
					{
						m_wndView.FilterMachineDialog->OnCancel();
						m_wndView.FilterMachineDialog = new CGearfilter(&m_wndView);
						m_wndView.FilterMachineDialog->_pMachine = (Filter2p*)ma;
						m_wndView.FilterMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.FilterMachineDialog, point);
						m_wndView.FilterMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.FilterMachineDialog = new CGearfilter(&m_wndView);
					m_wndView.FilterMachineDialog->_pMachine = (Filter2p*)ma;
					m_wndView.FilterMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.FilterMachineDialog, point);
					m_wndView.FilterMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_GAIN:
				if (m_wndView.GainerMachineDialog)
				{
					if (m_wndView.GainerMachineDialog->_pMachine != (Gainer*)ma)
					{
						m_wndView.GainerMachineDialog->OnCancel();
						m_wndView.GainerMachineDialog = new CGearGainer(&m_wndView);
						m_wndView.GainerMachineDialog->_pMachine = (Gainer*)ma;
						m_wndView.GainerMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.GainerMachineDialog, point);
						m_wndView.GainerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.GainerMachineDialog = new CGearGainer(&m_wndView);
					m_wndView.GainerMachineDialog->_pMachine = (Gainer*)ma;
					m_wndView.GainerMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.GainerMachineDialog, point);
					m_wndView.GainerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
			case MACH_FLANGER:
				if (m_wndView.FlangerMachineDialog)
				{
					if (m_wndView.FlangerMachineDialog->_pMachine != (Flanger*)ma)
					{
						m_wndView.FlangerMachineDialog->OnCancel();
						m_wndView.FlangerMachineDialog = new CGearFlanger(&m_wndView);
						m_wndView.FlangerMachineDialog->_pMachine = (Flanger*)ma;
						m_wndView.FlangerMachineDialog->Create();
						CenterWindowOnPoint(m_wndView.FlangerMachineDialog, point);
						m_wndView.FlangerMachineDialog->ShowWindow(SW_SHOW);
					}
				}
				else
				{
					m_wndView.FlangerMachineDialog = new CGearFlanger(&m_wndView);
					m_wndView.FlangerMachineDialog->_pMachine = (Flanger*)ma;
					m_wndView.FlangerMachineDialog->Create();
					CenterWindowOnPoint(m_wndView.FlangerMachineDialog, point);
					m_wndView.FlangerMachineDialog->ShowWindow(SW_SHOW);
				}
				break;
#endif
			case MACH_PLUGIN:
				{
					m_pWndMac[tmac] = new CFrameMachine(tmac);
					((CFrameMachine*)m_pWndMac[tmac])->_pActive = &isguiopen[tmac];
					((CFrameMachine*)m_pWndMac[tmac])->wndView = &m_wndView;
					((CFrameMachine*)m_pWndMac[tmac])->MachineIndex=_pSong->FindBusFromIndex(tmac);

					m_pWndMac[tmac]->LoadFrame(
						IDR_MACHINEFRAME, 
						WS_POPUPWINDOW | WS_CAPTION,
						this);
					((CFrameMachine*)m_pWndMac[tmac])->Generate();
					((CFrameMachine*)m_pWndMac[tmac])->SelectMachine(ma);
					char winname[32];
					sprintf(winname,"%.2X : %s",((CFrameMachine*)m_pWndMac[tmac])->MachineIndex
											 ,ma->_editName);
					((CFrameMachine*)m_pWndMac[tmac])->SetWindowText(winname);
					isguiopen[tmac] = true;
					CenterWindowOnPoint(m_pWndMac[tmac], point);
				}
				break;
			case MACH_VST:
			case MACH_VSTFX:
				{
				m_pWndMac[tmac] = new CVstEditorDlg(0);
				((CVstEditorDlg*)m_pWndMac[tmac])->_editorActive = &isguiopen[tmac];
				((CVstEditorDlg*)m_pWndMac[tmac])->wndView = &m_wndView;
				((CVstEditorDlg*)m_pWndMac[tmac])->MachineIndex=_pSong->FindBusFromIndex(tmac);
				((CVstEditorDlg*)m_pWndMac[tmac])->_pMachine = (VSTPlugin*)ma;
				((VSTPlugin*)ma)->editorWnd = NULL;
				
				((CVstEditorDlg*)m_pWndMac[tmac])->LoadFrame(IDR_VSTFRAME,
						WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU,
						this);
				((VSTPlugin*)ma)->editorWnd = m_pWndMac[tmac];
				char winname[32];
				sprintf(winname,"%.2X : %s",((CVstEditorDlg*)m_pWndMac[tmac])->MachineIndex
										,ma->_editName);

				((CVstEditorDlg*)m_pWndMac[tmac])->SetWindowText(winname);
				((CVstEditorDlg*)m_pWndMac[tmac])->ShowWindow(SW_SHOWNORMAL);
				isguiopen[tmac] = true;
				CenterWindowOnPoint(m_pWndMac[tmac], point);
			break;
				}
			}
		}
	}
}

void CMainFrame::CenterWindowOnPoint(CWnd* pWnd, POINT point)
{
	RECT r,rw;
	WINDOWPLACEMENT w1;
	pWnd->GetWindowRect(&r);
	m_wndView.GetWindowPlacement(&w1);
	/*
	WINDOWPLACEMENT w2;
	GetWindowPlacement(&w2);
	if (w2.showCmd & SW_SHOWMAXIMIZED)
	{
	*/
		rw.top = w1.rcNormalPosition.top;
		rw.left = w1.rcNormalPosition.left;
		rw.right = w1.rcNormalPosition.right;
		rw.bottom = w1.rcNormalPosition.bottom+64;
		/*
	}
	else
	{
		rw.top = w1.rcNormalPosition.top + w2.rcNormalPosition.top;
		rw.left = w1.rcNormalPosition.left + w2.rcNormalPosition.left;
		rw.bottom = w1.rcNormalPosition.bottom + w2.rcNormalPosition.top;
		rw.right = w1.rcNormalPosition.right + w2.rcNormalPosition.left;
	}
	*/

	int x = rw.left+point.x-((r.right-r.left)/2);
	int y = rw.top+point.y-((r.bottom-r.top)/2);

	if (x+(r.right-r.left) > (rw.right))
	{
		x = rw.right-(r.right-r.left);
	}
	// no else incase window is bigger than screen
	if (x < rw.left)
	{
		x = rw.left;
	}

	if (y+(r.bottom-r.top) > (rw.bottom))
	{
		y = rw.bottom-(r.bottom-r.top);
	}
	// no else incase window is bigger than screen
	if (y < rw.top)
	{
		y = rw.top;
	}

	pWnd->SetWindowPos(NULL,x,y,0,0,SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}

void CMainFrame::CloseAllMacGuis()
{
	for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
	{
		if (m_wndView.WireDialog[i])
		{
			m_wndView.WireDialog[i]->OnCancel();
		}
	}
	for (int c=0; c<MAX_MACHINES; c++)
	{
		if ( _pSong->_pMachine[c] ) CloseMacGui(c,false);
	}
}

void CMainFrame::CloseMacGui(int mac,bool closewiredialogs)
{
	if (closewiredialogs ) 
	{
		for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
		{
			if (m_wndView.WireDialog[i])
			{
				if ((m_wndView.WireDialog[i]->_pSrcMachine == _pSong->_pMachine[mac]) ||
					(m_wndView.WireDialog[i]->_pDstMachine == _pSong->_pMachine[mac]))
				{
					m_wndView.WireDialog[i]->OnCancel();
				}
			}
		}
	}
	if (_pSong->_pMachine[mac])
	{
		switch (_pSong->_pMachine[mac]->_type)
		{
			case MACH_MASTER:
				if (m_wndView.MasterMachineDialog) m_wndView.MasterMachineDialog->OnCancel();
				break;
			case MACH_SAMPLER:
				if (m_wndView.SamplerMachineDialog) m_wndView.SamplerMachineDialog->OnCancel();
				break;
#ifndef CONVERT_INTERNAL_MACHINES
			case MACH_SINE:
				if (m_wndView.PsychMachineDialog) m_wndView.PsychMachineDialog->OnCancel();
				break;
			case MACH_DIST:
				if (m_wndView.DistortionMachineDialog) m_wndView.DistortionMachineDialog->OnCancel();
				break;
			case MACH_DELAY:
				if (m_wndView.DelayMachineDialog) m_wndView.DelayMachineDialog->OnCancel();
				break;
			case MACH_2PFILTER:
				if (m_wndView.FilterMachineDialog) m_wndView.FilterMachineDialog->OnCancel();
				break;
			case MACH_GAIN:
				if (m_wndView.GainerMachineDialog) m_wndView.GainerMachineDialog->OnCancel();
				break;
			case MACH_FLANGER:
				if (m_wndView.FlangerMachineDialog) m_wndView.FlangerMachineDialog->OnCancel();
				break;
#endif
			case MACH_PLUGIN:
			case MACH_VST:
			case MACH_VSTFX:
				if (isguiopen[mac])
				{
					m_pWndMac[mac]->DestroyWindow();
					isguiopen[mac] = false;
				}
				break;
			default:break;
		}
	}
}


////////////////////
//////////////////// Sequencer Dialog
////////////////////


void CMainFrame::UpdateSequencer(int bottom)
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	char buf[64];

	int ep  = cc->GetCurSel();
	cc->ResetContent();

	if ( _pSong->pSequencer->machineList.size() == 0 ) return;
	int trackID = _pSong->pSequencer->machineList.at(_pSong->activeTrack);
	for(int n = 0; n <_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patCount; n++)
	{
		sprintf(buf,"%.2X:%.2X %s", _pSong->activeTrack , n, 
			_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[n]);
//		sprintf(buf,"%.2X:%.2X",_pSong->activeTrack,n) ;
		cc->AddString(buf);
	}
	if ( bottom >= 0 ) 
		cc->SetCurSel(bottom);
	else 
		cc->SetCurSel(ep);
}

void CMainFrame::OnSelchangeSeqlist() 
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int maxitems=cc->GetCount();
	int const ep=cc->GetCurSel();
	if(m_wndView.editPosition < 0 ) m_wndView.editPosition = 0; // DAN FIXME

	m_wndSeq.SetDlgItemText(IDC_PATNAME_EDIT, _pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[ep]);

	if((ep != m_wndView.editPosition))// && ( cc->GetSelCount() == 1))
	{
		if ((Global::pPlayer->_playing) && (Global::pConfig->_followSong))
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(ep,0);
			Global::pPlayer->_playBlock = b;
		}
		m_wndView.editPosition=ep;
		m_wndView.prevEditPosition=ep;
		m_wndView.Repaint(DMPattern);

	}
	m_wndView.SetFocus();
}

void CMainFrame::OnDblclkSeqlist() 
{
/*
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const ep=_pSong->GetBlankPatternUnsed();
	int const sep=m_wndView.editPosition;
	
	if(ep!=_pSong->playOrder[sep])
	{
		_pSong->playOrder[sep]=ep;
		UpdatePlayOrder(true);
		m_wndView.Repaint(DMPattern);
	}
	m_wndView.SetFocus();
	*/		
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const ep=cc->GetCurSel();
	m_wndView.editPosition=ep;
	m_wndView.OnPatternView();
}

void CMainFrame::OnIncshort() 
{
	int indexes[MAX_SONG_POSITIONS];
	m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const num= cc->GetSelCount();
	cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->playOrder[indexes[i]]<(MAX_PATTERNS-1))
		{
			_pSong->playOrder[indexes[i]]++;
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_wndView.Repaint(DMPattern);
	m_wndView.SetFocus();
}

void CMainFrame::OnDecshort() 
{
	int indexes[MAX_SONG_POSITIONS];
	m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const num= cc->GetSelCount();
	cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->playOrder[indexes[i]]>0)
		{
			_pSong->playOrder[indexes[i]]--;
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_wndView.Repaint(DMPattern);
	m_wndView.SetFocus();
}

void CMainFrame::OnInclong() 
{
	int indexes[MAX_SONG_POSITIONS];
	m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const num= cc->GetSelCount();
	cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->playOrder[indexes[i]]<(MAX_PATTERNS-16))
		{
			_pSong->playOrder[indexes[i]]+=16;			
		}
		else
		{
			_pSong->playOrder[indexes[i]]=(MAX_PATTERNS-1);
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_wndView.Repaint(DMPattern);
	m_wndView.SetFocus();
}

void CMainFrame::OnDeclong() 
{
	int indexes[MAX_SONG_POSITIONS];
	m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const num= cc->GetSelCount();
	cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->playOrder[indexes[i]]>=16)
		{
			_pSong->playOrder[indexes[i]]-=16;			
		}
		else
		{
			_pSong->playOrder[indexes[i]]=0;
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_wndView.Repaint(DMPattern);
	m_wndView.SetFocus();
}

void CMainFrame::OnSeqnew() 
{
	if ( _pSong->GetFreeMachine() == 0 ) return;
	if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
	{
		m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

		int pos = _pSong->GetNumPatternsUsed();
		_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->CreateNewPattern(pos);
		_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patCount++;
		m_wndView.editPosition = pos;

		UpdateSequencer(pos);
		m_wndView.Repaint(DMPattern);
		++_pSong->playLength;
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnSeqins() 
{
	if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
	{
		m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
		++_pSong->playLength;

		m_wndView.editPosition++;
		int const pop=m_wndView.editPosition;
		for(int c=(_pSong->playLength-1);c>=pop;c--)
		{
			_pSong->playOrder[c]=_pSong->playOrder[c-1];
		}

		UpdatePlayOrder(true);
		UpdateSequencer(m_wndView.editPosition);

		m_wndView.Repaint(DMPattern);
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnSeqduplicate() 
{
	if ( _pSong->GetFreeMachine() == 0 ) return;
	if ( _pSong->GetNumPatternsUsed() > 0 ) 
	{
		CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
		int item = cc->GetCurSel();
		int newpat = _pSong->GetBlankPatternUnused();

		if (newpat < MAX_PATTERNS-1)
		{
			m_wndView.editPosition = item;
			m_wndView.AddUndoSequence(_pSong->playLength, m_wndView.editcur.track, m_wndView.editcur.line,
				m_wndView.editcur.col, m_wndView.editPosition);
			int oldpat = item;
			
			// now we copy the data
			// we don't really need to be able to undo this, since it's a new pattern anyway.
	//			m_wndView.AddUndo(newpat,0,0,MAX_TRACKS,_pSong->patternLines[newpat],m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
			_pSong->CreateNewPattern(newpat);
			strcpy(_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[newpat],
				_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[oldpat]);
			_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[newpat] = 
				_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[oldpat];

			memcpy(_pSong->_ppattern(newpat), _pSong->_ppattern(oldpat), MULTIPLY2);

			++_pSong->playLength;
			_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patCount++;
			m_wndView.editPosition = newpat;

		}
		UpdateSequencer(m_wndView.editPosition);

		m_wndView.Repaint(DMPattern);
		m_wndView.SetFocus();
	}
}

void CMainFrame::OnSeqcut() 
{
	OnSeqcopy();
	OnSeqdelete();
}

void CMainFrame::OnSeqdelete() 
{
	if ( _pSong->GetFreeMachine() == 0 ) return;
	if ( _pSong->GetNumPatternsUsed() > 0 ) 
	{
		if (MessageBox("Do you really want to delete pattern?","Sequencer",MB_YESNO) == IDYES)
		{
			m_wndView.AddUndoSong(m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

			CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
			int item = cc->GetCurSel();
			m_wndView.editPosition = item;
			m_wndView.AddUndoSequence(_pSong->playLength, m_wndView.editcur.track, m_wndView.editcur.line,
				m_wndView.editcur.col, m_wndView.editPosition);
			_pSong->RemovePattern(item);
			_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->RemovePatternFromTrack(item);
			_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patCount--;
			UpdateSequencer();
			m_wndView.Repaint(DMPattern);
		}
		m_wndView.SetFocus();
	}
}

void CMainFrame::OnSeqcopy() 
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	seqcopybufferlength= cc->GetSelCount();
	cc->GetSelItems(MAX_SONG_POSITIONS,seqcopybuffer);

	// sort our table so we can paste it in a sensible manner later
	for (int i=0; i < seqcopybufferlength; i++)
	{
		for (int j=i+1; j < seqcopybufferlength; j++)
		{
			if (seqcopybuffer[j] < seqcopybuffer[i])
			{
				int k = seqcopybuffer[i];
				seqcopybuffer[i] = seqcopybuffer[j];
				seqcopybuffer[j] = k;
			}
		}
		// convert to actual index
		seqcopybuffer[i] = _pSong->playOrder[seqcopybuffer[i]];
	}
}

void CMainFrame::OnSeqpaste() 
{
	if (seqcopybufferlength > 0)
	{
		if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
		{
			m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

			// we will do this in a loop to easily handle an error if we run out of space

			// our list can be in any order so we must be careful
			int pastedcount = 0;
			for (int i=0; i < seqcopybufferlength; i++)
			{
				if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
				{
					++_pSong->playLength;

					m_wndView.editPosition++;
					pastedcount++;
					for(int c=(_pSong->playLength-1);c>=m_wndView.editPosition;c--)
					{
						_pSong->playOrder[c]=_pSong->playOrder[c-1];
					}
					_pSong->playOrder[c+1] = seqcopybuffer[i];
				}
			}

			if (pastedcount>0)
			{
				UpdatePlayOrder(true);
				for (i=m_wndView.editPosition+1-pastedcount; i<m_wndView.editPosition;i++)
				{
					_pSong->playOrderSel[i] = true;
				}
				UpdateSequencer(m_wndView.editPosition);
				m_wndView.Repaint(DMPattern);
				m_wndView.SetFocus();
			}
		}
	}
}


void CMainFrame::OnSeqclr() 
{
	if ( _pSong->GetFreeMachine() == 0 ) return;
	if ( _pSong->GetNumPatternsUsed() > 0 ) 
	{
		if (MessageBox("Do you really want to clear pattern data?","Sequencer",MB_YESNO) == IDYES)
		{
			m_wndView.AddUndoSong(m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

			CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
			int item = cc->GetCurSel();
			m_wndView.editPosition = item;
			m_wndView.AddUndoSequence(_pSong->playLength, m_wndView.editcur.track, m_wndView.editcur.line,
				m_wndView.editcur.col, m_wndView.editPosition);
			
			_pSong->ClearPattern(item);
			m_wndView.Repaint(DMPattern);
		}
		m_wndView.SetFocus();
	}		
}

void CMainFrame::OnSeqsort()
{
	m_wndView.AddUndoSong(m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
	unsigned char oldtonew[MAX_PATTERNS];
	unsigned char newtoold[MAX_PATTERNS];
	memset(oldtonew,255,MAX_PATTERNS*sizeof(char));
	memset(newtoold,255,MAX_PATTERNS*sizeof(char));

// Part one, Read patterns from sequence and assign them a new ordered number.
	unsigned char freep=0;
	for ( int i=0 ; i<_pSong->playLength ; i++ )
	{
		const char cp=_pSong->playOrder[i];
		if ( oldtonew[cp] == 255 ) // else, we have processed it already
		{
			oldtonew[cp]=freep;
			newtoold[freep]=cp;
			freep++;
		}
	}
// Part one and a half. End filling the order numbers.
	for(i=0; i<MAX_PATTERNS ; i++ )
	{
		if ( oldtonew[i] == 255 )
		{
			oldtonew[i] = freep;
			newtoold[freep] = i;
			freep++;
		}
	}
// Part two. Sort Patterns. Take first "invalid" out, and start putting patterns in their place.
//			 When we have to put the first read one back, do it and find next candidate.

	int patl; // first one is initial one, next one is temp one
	char patn[32]; // ""
	unsigned char * pData; // ""


	int idx=0;
	int idx2=0;
	for ( i=0 ; i < MAX_PATTERNS ; i++ )
	{
		if ( newtoold[i] != i ) // check if this place belongs to another pattern
		{
			pData = _pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->ppPatternData[i];
			memcpy(&patl,&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[i],sizeof(int));
			memcpy(patn,&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[i],sizeof(char)*32);

			idx = i;
			while ( newtoold[idx] != i ) // Start moving patterns while it is not the stored one.
			{
				idx2 = newtoold[idx]; // get pattern that goes here and move.

				_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->ppPatternData[idx] = 
					_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->ppPatternData[idx2];
				memcpy(&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[idx],&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[idx2],sizeof(int));
				memcpy(&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[idx],&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[idx2],sizeof(char)*32);
				
				newtoold[idx]=idx; // and indicate that this pattern has been corrected.
				idx = idx2;
			}

			// Put pattern back.
			_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->ppPatternData[idx] = pData;
			memcpy(&_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[idx],&patl,sizeof(int));
			memcpy(_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[idx],patn,sizeof(char)*32);

			newtoold[idx]=idx; // and indicate that this pattern has been corrected.
		}
	}
// Part three. Update the sequence

	for (i=0 ; i<_pSong->playLength ; i++ )
	{
		_pSong->playOrder[i]=oldtonew[_pSong->playOrder[i]];
	}

// Part four. All the needed things.

	seqcopybufferlength = 0;
	UpdateSequencer();
	m_wndView.Repaint(DMPattern);
	m_wndView.SetFocus();
}


void CMainFrame::OnInclen() 
{
	m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
	if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
	{
		++_pSong->playLength;
		UpdatePlayOrder(false);
		UpdateSequencer();
	}
	m_wndView.SetFocus();
}

void CMainFrame::OnDeclen() 
{
	m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
	if(_pSong->playLength>1)
	{
		--_pSong->playLength;
		_pSong->playOrder[_pSong->playLength]=0;
		UpdatePlayOrder(false);
		UpdateSequencer();
	}
	m_wndView.SetFocus();	
}


void CMainFrame::OnMultichannelAudition() 
{
	Global::pInputHandler->bMultiKey = !Global::pInputHandler->bMultiKey;
	m_wndView.SetFocus();
}

void CMainFrame::OnRecordNoteoff() 
{
	if ( ((CButton*)m_wndSeq.GetDlgItem(IDC_RECORD_NOTEOFF))->GetCheck() ) Global::pConfig->_RecordNoteoff=true;
	else Global::pConfig->_RecordNoteoff=false;
	m_wndView.SetFocus();
}

void CMainFrame::OnRecordTweaks() 
{
	if ( ((CButton*)m_wndSeq.GetDlgItem(IDC_RECORD_TWEAKS))->GetCheck() ) Global::pConfig->_RecordTweaks=true;
	else Global::pConfig->_RecordTweaks=false;
	m_wndView.SetFocus();
}

void CMainFrame::OnNotestoeffects() 
{
	if ( ((CButton*)m_wndSeq.GetDlgItem(IDC_NOTESTOEFFECTS))->GetCheck() ) Global::pConfig->_notesToEffects=true;
	else Global::pConfig->_notesToEffects=false;
	m_wndView.SetFocus();
	
}

void CMainFrame::OnFollowSong() 
{
	Global::pConfig->_followSong = ((CButton*)m_wndSeq.GetDlgItem(IDC_FOLLOW))->GetCheck()?true:false;
	CListBox* pSeqList = (CListBox*)m_wndSeq.GetDlgItem(IDC_SEQLIST);

	if (( Global::pConfig->_followSong ) && ( Global::pPlayer->_playing ))
	{
		m_wndView.ChordModeOffs = 0;
		m_wndView.bScrollDetatch=false;
		if (pSeqList->GetCurSel() != Global::pPlayer->_playPosition)
		{
			pSeqList->SelItemRange(false,0,pSeqList->GetCount()-1);
			pSeqList->SetSel(Global::pPlayer->_playPosition,true);
		}
		if ( m_wndView.editPosition  != Global::pPlayer->_playPosition )
		{
			m_wndView.editPosition=Global::pPlayer->_playPosition;
			m_wndView.Repaint(DMPattern);
		}
	}
	else if ( !Global::pPlayer->_playing )
	{
		pSeqList->SelItemRange(false,0,pSeqList->GetCount()-1);
		for (int i=0;i<MAX_SONG_POSITIONS;i++ )
		{
			if (_pSong->playOrderSel[i]) pSeqList->SetSel(i,true);
		}
	}

	m_wndView.SetFocus();
}

void CMainFrame::UpdatePlayOrder(bool mode)
{
	
	//CStatic *ll_l=(CStatic *)m_wndSeq.GetDlgItem(IDC_SEQ3);
	CListBox *pls=(CListBox*)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	CStatic *pLength = (CStatic*)m_wndSeq.GetDlgItem(IDC_LENGTH);
	
	int ll = _pSong->playLength;

	char buffer[16];

// Update Labels
	
	//sprintf(buffer,"%.2X",ll);
	//ll_l->SetWindowText(buffer);

	/*
	int songLength = 0;
	for (int i=0; i <ll; i++)
	{
		int pattern = _pSong->playOrder[i];
		// this should parse each line for ffxx commands if you want it to be truly accurate
		songLength += (_pSong->patternLines[pattern] * 60/(_pSong->BeatsPerMin * _pSong->_ticksPerBeat));
	}

	sprintf(buffer, "%02d:%02d", songLength / 60, songLength % 60);
	*/

	// take ff and fe commands into account

	float songLength = 0;
	int bpm = _pSong->BeatsPerMin;
	int tpb = _pSong->_ticksPerBeat;
	//for (int i=0; i <ll; i++)
	//{
	//	int pattern = i;
	//	// this should parse each line for ffxx commands if you want it to be truly accurate
	//	unsigned char* const plineOffset = _pSong->_ppattern(pattern);
	//	for (int l = 0; l < _pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternLines[pattern]*MULTIPLY; l+=MULTIPLY)
	//	{
	//		for (int t = 0; t < _pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->SONGTRACKS*5; t+=5)
	//		{
	//			PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
	//			switch (pEntry->_cmd)
	//			{
	//			case 0xFF:
	//				if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
	//				{
	//					bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
	//				}
	//				break;
	//				
	//			case 0xFE:
	//				if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
	//				{
	//					tpb=pEntry->_parameter;
	//				}
	//				break;
	//			}
	//		}
	//	}
	//}
	songLength = (_pSong->pSequencer->endLine - _pSong->pSequencer->startLine) * (60.0f/(bpm * tpb));
	
	sprintf(buffer, "%02d:%02d", f2i(songLength / 60), f2i(songLength) % 60);
	pLength->SetWindowText(buffer);
	
	// Update sequencer line
	
	//if (mode)
	//{
	//	const int ls=m_wndView.editPosition;
	//	const int le=_pSong->playOrder[ls];
	//	pls->DeleteString(ls);
	//	sprintf(buffer,"%.2X: %.2X",ls,le);
	//	pls->InsertString(ls,buffer);
	//	// Update sequencer selection	
	//	pls->SelItemRange(false,0,pls->GetCount()-1);
	//	pls->SetSel(ls,true);
	//	memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
	//	_pSong->playOrderSel[ls] = true;
	//}
	//else
	//{
	//	pls->SelItemRange(false,0,pls->GetCount()-1);
	//	for (int i=0;i<MAX_SONG_POSITIONS;i++ )
	//	{
	//		if (_pSong->playOrderSel[i]) pls->SetSel(i,true);
	//	}
	//}
	
}

void CMainFrame::OnUpdateIndicatorSeqPos(CCmdUI *pCmdUI) 
{
    pCmdUI->Enable();
    CString str;
	if (Global::pPlayer->_playing)
	{
	    str.Format("Pos %.2X", Global::pPlayer->_playPosition); 
	}
	else
	{
	    str.Format("Pos %.2X", m_wndView.editPosition); 
	}
    pCmdUI->SetText(str); 
}

void CMainFrame::OnUpdateIndicatorPattern(CCmdUI *pCmdUI) 
{
    pCmdUI->Enable(); 
    CString str;
	if (Global::pPlayer->_playing)
	{
	    str.Format("Pat %.2X", Global::pPlayer->_playPattern); 
	}
	else
	{
	    str.Format("Pat %.2X", m_wndView.editPosition); 
	}
    pCmdUI->SetText(str); 
}

void CMainFrame::OnUpdateIndicatorLine(CCmdUI *pCmdUI) 
{
    pCmdUI->Enable(); 
    CString str;
	if (Global::pPlayer->_playing)
	{
	    str.Format("Line %u", Global::pPlayer->_lineCounter); 
	}
	else
	{
	    str.Format("Line %u", m_wndView.editcur.line); 
	}
    pCmdUI->SetText(str); 
}

void CMainFrame::OnUpdateIndicatorTime(CCmdUI *pCmdUI) 
{
    pCmdUI->Enable(); 
	if (Global::pPlayer->_playing)
	{
		CString str;
	    str.Format( "%.2u:%.2u:%.2u.%.2u", Global::pPlayer->_playTimem / 60, Global::pPlayer->_playTimem % 60, f2i(Global::pPlayer->_playTime), f2i(Global::pPlayer->_playTime*100)-(f2i(Global::pPlayer->_playTime)*100)); 
		pCmdUI->SetText(str); 
	}
}

void CMainFrame::OnUpdateIndicatorEdit(CCmdUI *pCmdUI) 
{
	if (m_wndView.bEditMode)
	{
		pCmdUI->Enable(); 
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnUpdateIndicatorFollow(CCmdUI *pCmdUI) 
{
	if (Global::pConfig->_followSong)
	{
		pCmdUI->Enable(); 
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnUpdateIndicatorNoteoff(CCmdUI *pCmdUI) 
{
	if (Global::pConfig->_RecordNoteoff)
	{
		pCmdUI->Enable(); 
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnUpdateIndicatorTweaks(CCmdUI *pCmdUI) 
{
	if (Global::pConfig->_RecordTweaks)
	{
		pCmdUI->Enable(); 
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnUpdateIndicatorOctave(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(); 
    CString str;
	str.Format("Oct %u", _pSong->currentOctave); 
    pCmdUI->SetText(str); 

}


int CMainFrame::GetNumFromCombo(CComboBox *cb)
{
	CString str;
	cb->GetWindowText(str);
	return _httoi(str.Left(2).GetBuffer(2));
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	if (wParam == AFX_IDS_IDLEMESSAGE)
	{
		if (StatusBarIdleText())
		{
			return CFrameWnd::OnSetMessageString (0,(LPARAM)szStatusIdle);
		}
	}
	return CFrameWnd::OnSetMessageString (wParam, lParam);

}

void CMainFrame::StatusBarIdle()
{
	if (StatusBarIdleText())
	{
		m_wndStatusBar.SetWindowText(szStatusIdle);
	}
}

BOOL CMainFrame::StatusBarIdleText()
{
	if (_pSong)
	{
		if ((m_wndView.viewMode==VMPattern)	&& (!Global::pPlayer->_playing))
		{
			unsigned char *toffset=_pSong->_ptrackline(m_wndView.editPosition,m_wndView.editcur.track,m_wndView.editcur.line);
			int machine = toffset[2];
			if (machine<MAX_MACHINES)
			{
				if (_pSong->_pMachine[machine])
				{
					if (_pSong->_pMachine[machine]->_type == MACH_SAMPLER)
					{
						if (_pSong->_pInstrument[toffset[1]]->_sName[0])
						{
							sprintf(szStatusIdle,"%s - %s - %s - %s",_pSong->Name,_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[m_wndView.editPosition],_pSong->_pMachine[machine]->_editName,_pSong->_pInstrument[toffset[1]]->_sName);
							return TRUE;
						}
						else 
						{
							sprintf(szStatusIdle,"%s - %s - %s",_pSong->Name,_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[m_wndView.editPosition],_pSong->_pMachine[machine]->_editName);
							return TRUE;
						}
					}
					else
					{
						char buf[64];
						buf[0]=0;
						_pSong->_pMachine[machine]->GetParamName(toffset[1],buf);
						if (buf[0])
						{
							sprintf(szStatusIdle,"%s - %s - %s - %s",_pSong->Name,_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[m_wndView.editPosition],_pSong->_pMachine[machine]->_editName,buf);
							return TRUE;
						}
						else 
						{
							sprintf(szStatusIdle,"%s - %s - %s",_pSong->Name,_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[m_wndView.editPosition],_pSong->_pMachine[machine]->_editName);
							return TRUE;
						}
					}
				}
				else
				{
					sprintf(szStatusIdle,"%s - %s - Machine Out of Range",_pSong->Name,_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[m_wndView.editPosition]);
					return TRUE;
				}
			}
		}
		sprintf(szStatusIdle,"%s - %s",_pSong->Name,_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[m_wndView.editPosition]);
		return TRUE;
	}
	return FALSE;
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
    char szFileName[MAX_PATH];
	char * szExtension;

	int iNumFiles = DragQueryFile((HDROP)  hDropInfo,	// handle of structure for dropped files
		0xFFFFFFFF, // this returns number of dropped files
		NULL,
		NULL);

	for (int i = 0; i < iNumFiles; i++)
	{
		DragQueryFile((HDROP)  hDropInfo,	// handle of structure for dropped files
			i,	// index of file to query
			szFileName,	// buffer for returned filename
			MAX_PATH); 	// size of buffer for filename

		// check for .bmp files only

		if (szExtension = strrchr(szFileName, 46)) // point to everything past last "."
		{
			if (!strcmpi(szExtension, ".psy")) // compare to ".psy"
			{
				SetForegroundWindow();
				m_wndView.OnFileLoadsongNamed(szFileName, 1);
				DragFinish((HDROP)  hDropInfo);	// handle of structure for dropped files
				return;
			}
			// add psb, psv?
			// load waves and crap here
		}
	}
	DragFinish((HDROP)  hDropInfo);	// handle of structure for dropped files
	SetForegroundWindow();
}


// void CMainFrame::LoadFonts() - removed, use Configuration::CreateFonts

void CMainFrame::OnViewSongbar() 
{
	if (m_wndControl.IsWindowVisible())
	{
		ShowControlBar(&m_wndControl,FALSE,FALSE);
	}
	else {	ShowControlBar(&m_wndControl,TRUE,FALSE);	}
}

void CMainFrame::OnViewMachinebar() 
{
	if (m_wndControl2.IsWindowVisible())
	{
		ShowControlBar(&m_wndControl2,FALSE,FALSE);
	}
	else {	ShowControlBar(&m_wndControl2,TRUE,FALSE);	}
}

void CMainFrame::OnViewSequencerbar() 
{
	if (m_wndSeq.IsWindowVisible())
	{
		ShowControlBar(&m_wndSeq,FALSE,FALSE);
	}
	else {	ShowControlBar(&m_wndSeq,TRUE,FALSE);	}	
}

void CMainFrame::OnUpdateViewSongbar(CCmdUI* pCmdUI) 
{
	if ( m_wndControl.IsWindowVisible()) pCmdUI->SetCheck(TRUE);
	else pCmdUI->SetCheck(FALSE);
	
}

void CMainFrame::OnUpdateViewMachinebar(CCmdUI* pCmdUI) 
{
	if ( m_wndControl2.IsWindowVisible()) pCmdUI->SetCheck(TRUE);
	else pCmdUI->SetCheck(FALSE);
	
}

void CMainFrame::OnUpdateViewSequencerbar(CCmdUI* pCmdUI) 
{
	if ( m_wndSeq.IsWindowVisible()) pCmdUI->SetCheck(TRUE);
	else pCmdUI->SetCheck(FALSE);
}


void CMainFrame::RedrawGearRackList()
{
	if (pGearRackDialog)
	{
		pGearRackDialog->RedrawList();
	}
}


void CMainFrame::OnPatternNameChange()
{
	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
	int const ep=cc->GetCurSel();

	char patName[32];
	CEdit *patE=(CEdit *)m_wndSeq.GetDlgItem(IDC_PATNAME_EDIT);
	patE->GetWindowText(patName,32);
	strcpy(_pSong->pSequencer->pSequenceData[_pSong->pSequencer->machineList.at(_pSong->activeTrack)]->patternName[ep], patName);
	UpdateSequencer();
}


///\file
///\brief implementation file for psycle::host::CMainFrame.

#include <psycle/host/detail/project.private.hpp>
#include "MainFrm.hpp"

#include "Project.hpp"
#include "PatternView.hpp"
#include "Configuration.hpp"
#include "InputHandler.hpp"
#include "MidiInput.hpp"
#include "WavFileDlg.hpp"
#include "KeyConfigDlg.hpp"
#include "GearRackDlg.hpp"
#include "MasterDlg.hpp"
#include "GearTracker.hpp"
#include "XMSamplerUI.hpp"
#include "FrameMachine.hpp"
#include "VstEffectWnd.hpp"
#include "FrameMixerMachine.hpp"
#include "WaveInMacDlg.hpp"
#include "WireDlg.hpp"

#include "SeqNewCommand.hpp"
#include "SeqDeleteCommand.hpp"
#include "SeqPasteCommand.hpp"
#include "SeqCloneCommand.hpp"
#include "SeqInsCommand.hpp"
#include "SeqSortCommand.hpp"
#include "SeqIncLenCommand.hpp"
#include "SeqDecLenCommand.hpp"
#include "SeqIncShortCommand.hpp"
#include "SeqDecShortCommand.hpp"
#include "SeqIncLongCommand.hpp"
#include "SeqDecLongCommand.hpp"

#include <psycle/core/player.h>
#include <psycle/core/plugin.h>
#include <psycle/core/sampler.h>
#include <psycle/core/xmsampler.h>
#include <psycle/core/song.h>
#include <psycle/core/internal_machines.h>
#include <psycle/core/fileio.h>
#include <psycle/helpers/math.hpp>
#include <psycle/helpers/hexstring_to_integer.hpp>

#include <cmath>
#include <sstream>
#include <iomanip>

#include <HtmlHelp.h>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

#define WM_SETMESSAGESTRING 0x0362

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
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
	ON_BN_CLICKED(IDC_SAVEWAVE, OnSavewave)
	ON_BN_CLICKED(IDC_EDITWAVE, OnEditwave)
	ON_BN_CLICKED(IDC_GEAR_RACK, OnGearRack)
	ON_BN_CLICKED(IDC_WAVEBUT, OnWavebut)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_B_DECGEN, OnBDecgen)
	ON_BN_CLICKED(IDC_B_INCGEN, OnBIncgen)
	ON_BN_CLICKED(IDC_B_DECWAV, OnBDecwav)
	ON_BN_CLICKED(IDC_B_INCWAV, OnBIncwav)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_INCLEN, OnInclen)
	ON_BN_CLICKED(IDC_DECLEN, OnDeclen)
	ON_LBN_SELCHANGE(IDC_SEQLIST, OnSelchangeSeqlist)
	ON_LBN_DBLCLK(IDC_SEQLIST, OnDblclkSeqlist)
	ON_BN_CLICKED(IDC_INCSHORT, OnIncshort)
	ON_BN_CLICKED(IDC_DECSHORT, OnDecshort)
	ON_BN_CLICKED(IDC_SEQINS, OnSeqins)
	ON_BN_CLICKED(IDC_SEQNEW, OnSeqnew)
	ON_BN_CLICKED(IDC_SEQCUT, OnSeqcut)
	ON_BN_CLICKED(IDC_SEQCOPY, OnSeqcopy)
	ON_BN_CLICKED(IDC_SEQPASTE, OnSeqpaste)
	ON_BN_CLICKED(IDC_SEQDELETE, OnSeqdelete)
	ON_BN_CLICKED(IDC_SEQDUPLICATE, OnSeqduplicate)
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDC_DEC_TPB, OnDecTPB)
	ON_BN_CLICKED(IDC_INC_TPB, OnIncTPB)
	ON_BN_CLICKED(IDC_FOLLOW, OnFollowSong)
	ON_BN_CLICKED(IDC_SEQCLR, OnSeqclr)
	ON_BN_CLICKED(IDC_SEQSRT, OnSeqsort)
	ON_BN_CLICKED(IDC_MULTICHANNEL_AUDITION, OnMultichannelAudition)
	ON_BN_CLICKED(IDC_RECORD_NOTEOFF, OnRecordNoteoff)
	ON_BN_CLICKED(IDC_RECORD_TWEAKS, OnRecordTweaks)
	ON_CBN_CLOSEUP(IDC_AUXSELECT, OnCloseupAuxselect)
	ON_CBN_SELCHANGE(IDC_AUXSELECT, OnSelchangeAuxselect)
	ON_BN_CLICKED(IDC_DECLONG, OnDeclong)
	ON_BN_CLICKED(IDC_INCLONG, OnInclong)
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
	ON_BN_CLICKED(IDC_MOVECURSORPASTE, OnMoveCursorPaste)
	ON_BN_CLICKED(IDC_LOADWAVE, OnLoadwave)
	ON_MESSAGE (WM_SETMESSAGESTRING, OnSetMessageString)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_MASTERSLIDER, OnCustomdrawMasterslider)
	ON_BN_CLICKED(IDC_SHOWPATTERNAME, OnSeqShowpattername)
END_MESSAGE_MAP()

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

CMainFrame::CMainFrame()
	: m_wndView(this, &projects_),
	  m_wndSeq(this),
	  m_wndInfo(&projects_, this),
	  vuprevR(0),
	  vuprevL(0),
	  pGearRackDialog(0),
	  m_wndInst(&projects_, this)
{
	Global::pInputHandler->SetMainFrame(this);						
	SetUpStartProject();
	m_wndView.AddModules(projects_.active_project());
	wave_ed_ = new CWaveEdFrame(&projects_,this);
}

CMainFrame::~CMainFrame()
{
//			Gdiplus::GdiplusShutdown(gdiplusToken); // GDI+ stuff
	Global::pInputHandler->SetMainFrame(NULL);
	if(pGearRackDialog) pGearRackDialog->OnCancel();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	macComboInitialized = false;
		
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

	// CPU info Window
	m_wndInfo.Create(IDD_INFO,this);

	// MIDI monitor Dialog
	m_midiMonitorDlg.Create(IDD_MIDI_MONITOR,this);			
	// Wave Editor Window
	wave_ed_->LoadFrame(IDR_WAVEFRAME ,WS_OVERLAPPEDWINDOW,this);
	wave_ed_->GenerateView();

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
	cs->SetPos(256);
	cs->SetTicFreq(64);

	/*
	cs->SetRange(0,256);
	cs->SetPos(128);
	cs->SetTicFreq(16);
	*/

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
	cc2->SetCurSel(projects_.active_project()->song().tracks()-4);

//	SetAppSongBpm(0);
//	SetAppSongTpb(0);

//	cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);

//	for(int i=0;i<=16;i++)
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
	m_seqListbox.SubclassDlgItem(IDC_SEQLIST,&m_wndSeq );

	// set multichannel audition checkbox status
	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_MULTICHANNEL_AUDITION);
	cb->SetCheck(Global::pInputHandler->bMultiKey?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_MOVECURSORPASTE);
	cb->SetCheck(Global::pInputHandler->bMoveCursorPaste?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_SHOWPATTERNAME);
	cb->SetCheck(Global::pConfig->_bShowPatternNames?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_RECORD_NOTEOFF);
	cb->SetCheck(Global::pConfig->_RecordNoteoff?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_RECORD_TWEAKS);
	cb->SetCheck(Global::pConfig->_RecordTweaks?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_NOTESTOEFFECTS);
	cb->SetCheck(Global::pConfig->_notesToEffects?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_FOLLOW);
	cb->SetCheck(Global::pConfig->_followSong?1:0);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_INCSHORT);
	hi = (HBITMAP)bplus; cb->SetBitmap(hi);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_INCLONG);
	hi = (HBITMAP)bplusplus; cb->SetBitmap(hi);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_DECSHORT);
	hi = (HBITMAP)bminus; cb->SetBitmap(hi);
	
	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_DECLONG);
	hi = (HBITMAP)bminusminus; cb->SetBitmap(hi);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_DECLEN);
	hi = (HBITMAP)bless; cb->SetBitmap(hi);

	cb=(CButton*)m_wndSeq.GetDlgItem(IDC_INCLEN);
	hi = (HBITMAP)bmore; cb->SetBitmap(hi);
	
	m_wndSeq.UpdatePlayOrder(true);
	
	DragAcceptFiles(true);

	Global::pConfig->CreateFonts();

	// Finally initializing timer
	
	m_wndSeq.UpdateSequencer();
	// Show Machine view and init MIDI
	m_wndView.OnMachineview();
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
	
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

void CMainFrame::SetUpStartProject()
{
	Project* prj = new Project(&projects_);
	projects_.Add(prj);
	m_wndSeq.SetProject(prj);
}

#if !defined NDEBUG
	void CMainFrame::AssertValid() const
	{
		CFrameWnd::AssertValid();
	}

	void CMainFrame::Dump(CDumpContext& dc) const
	{
		CFrameWnd::Dump(dc);
	}
#endif

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
	if (projects_.active_project()->CheckUnsavedSong("Exit Psycle"))
	{
		m_wndView._outputActive = false;
		Global::pPlayer->stop();
		#if PSYCLE__CONFIGURATION__USE_PSYCORE
			Global::pConfig->_pOutputDriver->set_started(false);
		#else
			Global::pConfig->_pOutputDriver->Enable(false);
		#endif
		// MIDI IMPLEMENTATION
		Global::pConfig->_pMidiInput->Close();

		//Recent File List;
		((CPsycleApp*)AfxGetApp())->SaveRecent(this);

		CString filepath = Global::pConfig->GetSongDir().c_str();
		filepath += "\\autosave.psy";
		DeleteFile(filepath);
		
		CFrameWnd::OnClose();
	}

}

void CMainFrame::CheckForAutosave()
{
	CString filepath = Global::pConfig->GetSongDir().c_str();
	filepath += "\\autosave.psy";

	RiffFile file;
	if(file.Open(filepath.GetBuffer(1)))
	{
		file.Close();
		int val = MessageBox("An autosave.psy file has been found in the root song dir. Do you want to reload it? (Press \"No\" to delete it)","Song Recovery",MB_YESNOCANCEL);

		if (val == IDYES ) projects_.active_project()->FileLoadsongNamed(filepath.GetBuffer(1));
		else if (val == IDNO ) DeleteFile(filepath);
	}
}

void CMainFrame::ClosePsycle()
{
	OnClose();
}

void CMainFrame::OnDestroy() 
{
	m_wndInfo.DestroyWindow();
	m_midiMonitorDlg.DestroyWindow();
	m_wndInst.DestroyWindow();
	m_wndToolBar.DestroyWindow();
	m_wndControl.DestroyWindow();
	m_wndControl2.DestroyWindow();
	// wave_ed_->DestroyWindow();// is called by the default CFrameWnd::OnClose() function, and the memory freed (no idea how...)
	m_wndView.DestroyWindow();
	HICON _icon = GetIcon(false);
	DestroyIcon(_icon);
	DragAcceptFiles(false);
	CFrameWnd::OnDestroy();
}

void CMainFrame::StatusBarText(const std::string& txt)
{
	m_wndStatusBar.SetWindowText(txt.c_str());
}

void CMainFrame::PsybarsUpdate()
{
	SetAppSongBpm(0);
	SetAppSongTpb(0);
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
	cc2->SetCurSel(m_wndView.pattern_view()->patStep);		
	cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_TRACKCOMBO);
	cc2->SetCurSel(projects_.active_project()->song().tracks()-4);
	cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	cc2->SetCurSel(projects_.active_project()->song().currentOctave);		
	UpdateComboGen();
	UpdateMasterValue(((Master*)projects_.active_project()->song().machine(MASTER_INDEX))->_outDry);			
}

/////////////////
///////////////// PSY Bar 1
/////////////////

void CMainFrame::OnSelchangeTrackcombo() {
	CComboBox *cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_TRACKCOMBO);
	projects_.active_project()->song().tracks(cc2->GetCurSel()+4);
	if (m_wndView.pattern_view()->editcur.track >= projects_.active_project()->song().tracks())
		m_wndView.pattern_view()->editcur.track = projects_.active_project()->song().tracks()-1;
	m_wndView.pattern_view()->RecalculateColourGrid();
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
	if ( x != 0 )
	{
		if (Global::pPlayer->playing() ) 
		{
			projects_.active_project()->song().BeatsPerMin(Global::pPlayer->bpm()+x);
		}
		else projects_.active_project()->song().BeatsPerMin(projects_.active_project()->song().BeatsPerMin()+x);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		Global::player().setBpm(projects_.active_project()->song().bpm());
#else
		Global::pPlayer->SetBPM(projects_.active_project()->song().BeatsPerMin(),projects_.active_project()->song().LinesPerBeat());
#endif
		sprintf(buffer,"%d",projects_.active_project()->song().BeatsPerMin());
	}
	else sprintf(buffer,"%d",static_cast<int>(Global::pPlayer->bpm()));
	
	((CStatic *)m_wndControl.GetDlgItem(IDC_BPMLABEL))->SetWindowText(buffer);
}

void CMainFrame::SetAppSongTpb(int x) 
{
	char buffer[16];
#if PSYCLE__CONFIGURATION__USE_PSYCORE
	if ( x != 0)
	{
		if (projects_.active_project()) {
			projects_.active_project()->set_beat_zoom(x + projects_.active_project()->beat_zoom());
		}			
	}
	sprintf(buffer, "%d", projects_.active_project()->beat_zoom());
#else
	if ( x != 0)
	{
		if (Global::pPlayer->playing() ) 
		{
			projects_.active_project()->song().LinesPerBeat(Global::pPlayer->tpb()+x);
		}
		else projects_.active_project()->song().LinesPerBeat(projects_.active_project()->song().LinesPerBeat()+x);
		Global::pPlayer->SetBPM(projects_.active_project()->song().BeatsPerMin(), projects_.active_project()->song().LinesPerBeat());
		sprintf(buffer,"%d",projects_.active_project()->song().LinesPerBeat());
	}
	else sprintf(buffer, "%d", Global::pPlayer->tpb());
#endif
	
	((CStatic *)m_wndControl.GetDlgItem(IDC_TPBLABEL))->SetWindowText(buffer);
}

void CMainFrame::OnCloseupCombooctave() 
{
	m_wndView.SetFocus();
}

void CMainFrame::OnSelchangeCombooctave() {
	CComboBox *cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	projects_.active_project()->song().currentOctave=cc2->GetCurSel();			
	m_wndView.Repaint();
	m_wndView.SetFocus();
}

//////////////////////////////////////////////////////////////////////
// Function that shift the current editing octave

void CMainFrame::ShiftOctave(int x)
{			
	projects_.active_project()->song().currentOctave += x;
	if (projects_.active_project()->song().currentOctave < 0) { 
		projects_.active_project()->song().currentOctave = 0;
	}
	else if (projects_.active_project()->song().currentOctave > 8) { 
		projects_.active_project()->song().currentOctave = 8;
	}
	CComboBox *cc2=(CComboBox *)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	cc2->SetCurSel(projects_.active_project()->song().currentOctave);
}
void CMainFrame::UpdateMasterValue(int newvalue)
{
	CSliderCtrl *cs;
	if (projects_.active_project()->song().machine(MASTER_INDEX)) {
		cs=(CSliderCtrl*)m_wndControl.GetDlgItem(IDC_MASTERSLIDER);
		if (cs->GetPos() != newvalue) {
			cs->SetPos(newvalue);
		}
	}
}

void CMainFrame::OnCustomdrawMasterslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CSliderCtrl *cs;
	if ( projects_.active_project()->song().machine(MASTER_INDEX)) {
		cs=(CSliderCtrl*)m_wndControl.GetDlgItem(IDC_MASTERSLIDER);
		//((Master*)projects_.active_project()->song().machine(MASTER_INDEX))->_outDry = cs->GetPos()*cs->GetPos()/1024;
		///\todo: this causes problems sometimes when loading songs.
		// customdraw happening before updatemastervalue, so invalid value get set.
		// Added call to UpdateMasterValue() in PsybarsUpdate() in order to fix this.
		((Master*)projects_.active_project()->song().machine(MASTER_INDEX))->_outDry = cs->GetPos();
		m_wndView.SetFocus();
	}			
	*pResult = 0;
}

void CMainFrame::OnClipbut() 
{
	((Master*)(projects_.active_project()->song().machine(MASTER_INDEX)))->_clip = false;
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
	//  float log_l=20*(float)log10(l) - 20*log10(baselevel);	// simplified (speedwise) formula.
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
//				CStatic *rv=(CStatic *)m_wndControl.GetDlgItem(IDC_RVUM);
		CClientDC canvasl(lv);
//				CClientDC canvasr(rv);

		int log_l = static_cast<int>(100 * std::log10(l));
		int log_r = static_cast<int>(100 * std::log10(r));
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
	m_wndView.pattern_view()->patStep=sel;
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
	m_wndView.pattern_view()->patStep=nextsel;
}

void CMainFrame::OnBDecgen() // called by Button and Hotkey.
{
	//	ChangeGen(projects_.active_project()->song().seqBus-1);
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	const int val = cc->GetCurSel();
	if ( val > 0 ) cc->SetCurSel(val-1);
	else cc->SetCurSel(cc->GetCount()-1);
	if ( cc->GetItemData(cc->GetCurSel()) == 65535 )
	{
		if ( val >1) cc->SetCurSel(val-2);
		else cc->SetCurSel(val);
	}
	OnSelchangeBarCombogen();
	m_wndView.SetFocus();
}

void CMainFrame::OnBIncgen() // called by Button and Hotkey.
{
	//	ChangeGen(projects_.active_project()->song().seqBus+1);
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	const int val = cc->GetCurSel();
	if ( val < cc->GetCount()-1 ) cc->SetCurSel(val+1);
	else cc->SetCurSel(0);
	if ( cc->GetItemData(cc->GetCurSel()) == 65535 )
	{
		if ( val < cc->GetCount()-2) cc->SetCurSel(val+2);
		else cc->SetCurSel(val);
	}
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
	CComboBox *cb=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	CComboBox *cb2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
	
	macComboInitialized = false;
	if (updatelist) 
	{
		cb->ResetContent();
	}
	
#if PSYCLE__CONFIGURATION__USE_PSYCORE
	Song* song = &projects_.active_project()->song();
	for (int b=0; b<psycle::core::MAX_BUSES; b++) // Check Generators
	{
		if( song->machine(b))
		{
			if (updatelist)
			{	
				sprintf(buffer,"%.2X: %s",b,song->machine(b)->GetEditName().c_str());
				cb->AddString(buffer);
				cb->SetItemData(cb->GetCount()-1,b);
			}
			if (!found) 
			{
				selected++;
			}
			if (song->seqBus == b) 
			{
				found = true;
			}
			filled = true;
		}
	}
#else
	for (int b=0; b<MAX_BUSES; b++) // Check Generators
	{
		if( projects_.active_project()->song().machine(b))
		{
			if (updatelist)
			{	
				sprintf(buffer,"%.2X: %s",b,projects_.active_project()->song().machine(b)->_editName);
				cb->AddString(buffer);
				cb->SetItemData(cb->GetCount()-1,b);
			}
			if (!found) 
			{
				selected++;
			}
			if (projects_.active_project()->song().seqBus == b) 
			{
				found = true;
			}
			filled = true;
		}
	}

#endif
	if ( updatelist) 
	{
		cb->AddString("----------------------------------------------------");
		cb->SetItemData(cb->GetCount()-1,65535);
	}
	if (!found) 
	{
		selected++;
		line = selected;
	}
	
#if PSYCLE__CONFIGURATION__USE_PSYCORE
	for (int b=psycle::core::MAX_BUSES; b<psycle::core::MAX_BUSES*2; b++) // Write Effects Names.
	{
		if(song->machine(b))
		{
			if (updatelist)
			{	
				sprintf(buffer,"%.2X: %s",b,song->machine(b)->GetEditName().c_str());
				cb->AddString(buffer);
				cb->SetItemData(cb->GetCount()-1,b);
			}
			if (!found) 
			{
				selected++;
			}
			if (song->seqBus == b) 
			{
				found = true;
			}
			filled = true;
		}
	}

#else
	for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
	{
		if(projects_.active_project()->song().machine(b))
		{
			if (updatelist)
			{	
				sprintf(buffer,"%.2X: %s",b,projects_.active_project()->song().machine(b)->_editName);
				cb->AddString(buffer);
				cb->SetItemData(cb->GetCount()-1,b);
			}
			if (!found) 
			{
				selected++;
			}
			if (projects_.active_project()->song().seqBus == b) 
			{
				found = true;
			}
			filled = true;
		}
	}
#endif
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

#if PSYCLE__CONFIGURATION__USE_PSYCORE
	// Select the appropiate Option in Aux Combobox.
	if (found) // If found (which also means, if it exists)
	{
		if (song->machine(song->seqBus))
		{
			if ( song->seqBus < psycle::core::MAX_BUSES ) // it's a Generator
			{
				if (song->machine(song->seqBus)->getMachineKey() == InternalKeys::sampler
					|| song->machine(song->seqBus)->getMachineKey() == InternalKeys::sampulse )
				{
					cb2->SetCurSel(AUX_WAVES);
					song->auxcolSelected = song->instSelected();
				}
				else if (song->machine(song->seqBus)->getMachineKey().host() == Hosts::VST)
				{
					if ( cb2->GetCurSel() == AUX_WAVES)
					{
						cb2->SetCurSel(AUX_MIDI);
						song->auxcolSelected = song->midiSelected;
					}
				}
				else
				{
					cb2->SetCurSel(AUX_PARAMS);
					song->auxcolSelected = 0;
				}
			}
			else
			{
				cb2->SetCurSel(AUX_PARAMS);
				song->auxcolSelected = 0;
			}
		}
	}
	else
	{
		cb2->SetCurSel(AUX_WAVES); // WAVES
		song->auxcolSelected = song->instSelected();
	}

#else
	// Select the appropiate Option in Aux Combobox.
	if (found) // If found (which also means, if it exists)
	{
		if (projects_.active_project()->song().machine(projects_.active_project()->song().seqBus))
		{
			if ( projects_.active_project()->song().seqBus < MAX_BUSES ) // it's a Generator
			{
				if (projects_.active_project()->song().machine(projects_.active_project()->song().seqBus)->_type == MACH_SAMPLER ||projects_.active_project()->song().machine(projects_.active_project()->song().seqBus)->_type == MACH_XMSAMPLER  )
				{
					cb2->SetCurSel(AUX_WAVES);
					projects_.active_project()->song().auxcolSelected = projects_.active_project()->song().instSelected();
				}
				else if (projects_.active_project()->song().machine(projects_.active_project()->song().seqBus)->_type == MACH_VST)
				{
					if ( cb2->GetCurSel() == AUX_WAVES)
					{
						cb2->SetCurSel(AUX_MIDI);
						projects_.active_project()->song().auxcolSelected = projects_.active_project()->song().midiSelected;
					}
				}
				else
				{
					cb2->SetCurSel(AUX_PARAMS);
					projects_.active_project()->song().auxcolSelected = 0;
				}
			}
			else
			{
				cb2->SetCurSel(AUX_PARAMS);
				projects_.active_project()->song().auxcolSelected = 0;
			}
		}
	}
	else
	{
		cb2->SetCurSel(AUX_WAVES); // WAVES
		projects_.active_project()->song().auxcolSelected = projects_.active_project()->song().instSelected();
	}
#endif
	UpdateComboIns();
	macComboInitialized = true;
}

void CMainFrame::OnSelchangeBarCombogen() 
{
	if(macComboInitialized)
	{
		CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
		int nsb = GetNumFromCombo(cc);

		if(projects_.active_project()->song().seqBus!=nsb)
		{
			projects_.active_project()->song().seqBus=nsb;
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


void CMainFrame::ChangeGen(int i)	// Used to set an specific seqBus (used in "CChildView::SelectMachineUnderCursor")
{
	if(i>=0 && i <(MAX_BUSES*2))
	{
		if ( (projects_.active_project()->song().seqBus & MAX_BUSES) == (i & MAX_BUSES))
		{
			projects_.active_project()->song().seqBus=i;
			UpdateComboGen(false);
		}
		else
		{
			projects_.active_project()->song().seqBus=i;
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
		projects_.active_project()->song().auxcolSelected=projects_.active_project()->song().midiSelected;
	}
	else if ( cc2->GetCurSel() == AUX_WAVES )	// WAVES
	{
		projects_.active_project()->song().auxcolSelected=projects_.active_project()->song().instSelected();
	}
	UpdateComboIns();
}
void CMainFrame::OnBDecwav() 
{
	ChangeIns(projects_.active_project()->song().auxcolSelected-1);
	m_wndView.SetFocus();
}

void CMainFrame::OnBIncwav() 
{
	ChangeIns(projects_.active_project()->song().auxcolSelected+1);
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
//		projects_.active_project()->song().midiSelected=projects_.active_project()->song().auxcolSelected;
	}
	else if ( cc2->GetCurSel() == AUX_PARAMS)	// Params
	{
		int nmac = projects_.active_project()->song().seqBus;
		Machine *tmac = projects_.active_project()->song().machine(nmac);
		if (tmac) 
		{
			int i=0;
			if (updatelist) 
			{
				for (i=0;i<tmac->GetNumParams();i++)
				{
					char buffer[64],buffer2[64];
					std::memset(buffer2,0,64);
					tmac->GetParamName(i,buffer2);
					bool label(false);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					if(tmac->getMachineKey().host() == Hosts::NATIVE )
					{
						if(!(static_cast<Plugin*>(tmac)->GetInfo().Parameters[i]->Flags & psycle::plugin_interface::MPF_STATE))
							label = true;
					}
#else
					if(tmac->_type == MACH_PLUGIN)
					{
						if(!(static_cast<Plugin*>(tmac)->GetInfo()->Parameters[i]->Flags & MPF_STATE))
							label = true;
					}
#endif
					if(label)
						// just a label
						sprintf(buffer, "------ %s ------", buffer2);
					else
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
			int nmac = projects_.active_project()->song().seqBus;
			Machine *tmac = projects_.active_project()->song().machine(nmac);
			if (tmac) 
			{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				if ( tmac->getMachineKey() == InternalKeys::sampulse)
#else
				if ( tmac->_type == MACH_XMSAMPLER)
#endif
				{
					for (int i(0); i<XMSampler::MAX_INSTRUMENT; i++)
					{
						sprintf(buffer, "%.2X: %s", i, projects_.active_project()->song().rInstrument(i).Name().c_str());
						cc->AddString(buffer);
						listlen++;

					}
				}

				else for (int i=0;i<PREV_WAV_INS;i++)
				{
					sprintf(buffer, "%.2X: %s", i, projects_.active_project()->song()._pInstrument[i]->_sName);
					cc->AddString(buffer);
					listlen++;
				}
			}
		}
		else
		{
			listlen = cc->GetCount();
		}
//		projects_.active_project()->song().instSelected(projects_.active_project()->song().auxcolSelected);
//		WaveEditorBackUpdate();
//		m_wndInst.WaveUpdate();
//		RedrawGearRackList();
	}
	if (projects_.active_project()->song().auxcolSelected >= listlen)
	{
		projects_.active_project()->song().auxcolSelected = 0;
	}
	cc->SetCurSel(projects_.active_project()->song().auxcolSelected);
}

void CMainFrame::OnSelchangeBarComboins() 
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if ( cc2->GetCurSel() == AUX_MIDI ) 
	{
		projects_.active_project()->song().midiSelected=cc->GetCurSel();
	}
	else if ( cc2->GetCurSel() == AUX_WAVES ) 
	{
		projects_.active_project()->song().instSelected(cc->GetCurSel());
		WaveEditorBackUpdate();
		m_wndInst.WaveUpdate();
		RedrawGearRackList();
	}

	projects_.active_project()->song().auxcolSelected=cc->GetCurSel();
}

void CMainFrame::OnCloseupBarComboins()
{
	m_wndView.SetFocus();
}

void CMainFrame::ChangeIns(int i)	// User Called (Hotkey)
{
	CComboBox *cc=(CComboBox *)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if ( cc->GetCurSel() == i) return;

	if (cc2->GetCurSel() == AUX_MIDI )
	{
		if (i>=0 && i <16)
		{
			projects_.active_project()->song().midiSelected=i;
			projects_.active_project()->song().auxcolSelected=i;
		}
	}
	else if ( cc2->GetCurSel() == AUX_PARAMS )
	{
		if (i>=0 && i < cc->GetCount() )
		{
			projects_.active_project()->song().auxcolSelected=i;
		}
	}
	else
	{
		if(i>=0 && i <(PREV_WAV_INS))
		{
			projects_.active_project()->song().instSelected(i);
			projects_.active_project()->song().auxcolSelected=i;
			WaveEditorBackUpdate();
			m_wndInst.WaveUpdate();
			RedrawGearRackList();
		}
	}
	cc->SetCurSel(projects_.active_project()->song().auxcolSelected);
}

void CMainFrame::OnLoadwave() 
{
	int nmac = projects_.active_project()->song().seqBus;
	Machine *tmac = projects_.active_project()->song().machine(nmac);
	if (tmac) 
	{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		if ( tmac->getMachineKey() == InternalKeys::sampulse)
#else
		if ( tmac->_type == MACH_XMSAMPLER)
#endif
		{
			CPoint point(-1,-1);
			// ShowMachineGui(nmac,point); maybe a todo
			return;
		}
	}

	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|IFF Samples (*.iff)|*.iff|All Files (*.*)|*.*||";
	
	CWavFileDlg dlg(true,"wav", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg._pSong = &projects_.active_project()->song();
	std::string tmpstr = Global::pConfig->GetCurrentInstrumentDir();
	dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
	if (dlg.DoModal() == IDOK)
	{

		int si = projects_.active_project()->song().instSelected();
		
		//added by sampler
		if ( projects_.active_project()->song()._pInstrument[si]->waveLength != 0)
		{
			if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;					
		}
		//end of added by sampler

		CString CurrExt=dlg.GetFileExt();
		CurrExt.MakeLower();
		
		if ( CurrExt == "wav" )
		{
			if (projects_.active_project()->song().WavAlloc(si,dlg.GetFileName()))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText("New wave loaded");
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
			}
		}
		else if ( CurrExt == "iff" )
		{
			if (projects_.active_project()->song().IffAlloc(si,dlg.GetFileName()))
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
			Global::pConfig->SetCurrentInstrumentDir(static_cast<char const *>(str.Left(index)));
		}
	}
	if ( projects_.active_project()->song()._pInstrument[PREV_WAV_INS]->waveLength > 0)
	{
		// Stopping wavepreview if not stopped.
		Sampler::wavprev.Stop();
		//Delete it.
		projects_.active_project()->song().DeleteInstrument(PREV_WAV_INS);
	}


	m_wndView.SetFocus();
}

void CMainFrame::OnSavewave()
{
	WaveFile output;
	static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
	
	if (projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveLength)
	{
		CFileDialog dlg(FALSE, "wav", projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
		if (dlg.DoModal() == IDOK)
		{
			output.OpenForWrite(dlg.GetFileName(), 44100, 16, (projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveStereo) ? (2) : (1) );
			if (projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveStereo)
			{
				for ( unsigned int c=0; c < projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveLength; c++)
				{
					output.WriteStereoSample( *(projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveDataL + c), *(projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveDataR + c) );
				}
			}
			else
			{
				output.WriteData(projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveDataL, projects_.active_project()->song()._pInstrument[projects_.active_project()->song().instSelected()]->waveLength);
			}

			output.Close();
		}
	}
	else MessageBox("Nothing to save...\nSelect nonempty wave first.", "Error", MB_ICONERROR);
	m_wndView.SetFocus();
}

void CMainFrame::OnGearRack() 
{
	if (pGearRackDialog == NULL)
	{
		pGearRackDialog = new CGearRackDlg(m_wndView.machine_view());				
		pGearRackDialog->Create();
		pGearRackDialog->ShowWindow(SW_SHOW);
	}
}

void CMainFrame::OnEditwave() 
{
	int nmac = projects_.active_project()->song().seqBus;
	Machine *tmac = projects_.active_project()->song().machine(nmac);
	if (tmac) 
	{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		if ( tmac->getMachineKey() == InternalKeys::sampulse)
#else
		if ( tmac->_type == MACH_XMSAMPLER)
#endif
		{
			CPoint point(-1,-1);
//					ShowMachineGui(nmac,point); maybe a todo
			return;
		}
	}
	ShowInstrumentEditor();
}

void CMainFrame::OnWavebut() 
{
	wave_ed_->ShowWindow(SW_SHOWNORMAL);
	wave_ed_->SetActiveWindow();
}

void CMainFrame::WaveEditorBackUpdate()
{
	wave_ed_->Notify();
}

void CMainFrame::ShowInstrumentEditor()
{
	CComboBox *cc2=(CComboBox *)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
	cc2->SetCurSel(AUX_WAVES);
	projects_.active_project()->song().auxcolSelected=projects_.active_project()->song().instSelected();
	UpdateComboIns();
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

void CMainFrame::UpdateEnvInfo() {
	m_wndInfo.UpdateInfo();
}

void CMainFrame::OnPsyhelp() 
{
	CString helppath(Global::pConfig->appPath().c_str());
	helppath +=  "Docs\\psycle.chm";
	::HtmlHelp(::GetDesktopWindow(),helppath, HH_DISPLAY_TOPIC, 0);
}

//////////////////// Sequencer Dialog 
void CMainFrame::OnInclen() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqIncLenCommand(&m_wndSeq));
}
void CMainFrame::OnDeclen() { 
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqDecLenCommand(&m_wndSeq));
}
void CMainFrame::OnSelchangeSeqlist() { m_wndSeq.OnSelchangeSeqlist(); }
void CMainFrame::OnDblclkSeqlist() { m_wndSeq.OnDblclkSeqlist(); }
void CMainFrame::OnIncshort() {	
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqIncShortCommand(&m_wndSeq));
}
void CMainFrame::OnDecshort() { 
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqDecShortCommand(&m_wndSeq));
}
void CMainFrame::OnInclong() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqIncLongCommand(&m_wndSeq));
}
void CMainFrame::OnDeclong() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqDecLongCommand(&m_wndSeq));
}
void CMainFrame::OnSeqnew() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqNewCommand(&m_wndSeq));
}
void CMainFrame::OnSeqins() { 
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqInsCommand(&m_wndSeq));
}
void CMainFrame::OnSeqduplicate() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqCloneCommand(&m_wndSeq));
}
void CMainFrame::OnSeqcut() { m_wndSeq.OnSeqcut(); }
void CMainFrame::OnSeqdelete() { 
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqDeleteCommand(&m_wndSeq));
}
void CMainFrame::OnSeqcopy() { m_wndSeq.OnSeqcopy(); }
void CMainFrame::OnSeqpaste() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqPasteCommand(&m_wndSeq));
}
void CMainFrame::OnSeqclr() { 
	m_wndSeq.OnSeqclr();
}
void CMainFrame::OnSeqsort() {
	m_wndSeq.project()->cmd_manager()->ExecuteCommand(new SeqSortCommand(&m_wndSeq));
}
void CMainFrame::OnSeqShowpattername() { m_wndSeq.OnSeqShowpattername(); }

void CMainFrame::OnMultichannelAudition() 
{
	Global::pInputHandler->bMultiKey = !Global::pInputHandler->bMultiKey;
	m_wndView.SetFocus();
}

void CMainFrame::OnMoveCursorPaste()
{
	Global::pInputHandler->bMoveCursorPaste = !Global::pInputHandler->bMoveCursorPaste;
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

	if ( Global::pConfig->_followSong )
	{
		if  ( Global::pPlayer->playing() )
		{
			m_wndView.pattern_view()->ChordModeOffs = 0;
			m_wndView.pattern_view()->bScrollDetatch=false;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			//left out in psycore, since it's assumed the timer will update it.
#else
			if (pSeqList->GetCurSel() != Global::pPlayer->_sequencePosition)
			{
				pSeqList->SelItemRange(false,0,pSeqList->GetCount()-1);
				pSeqList->SetSel(Global::pPlayer->_sequencePosition,true);
			}
			if ( m_wndView.pattern_view()->editPosition  != Global::pPlayer->_sequencePosition )
			{
				m_wndView.pattern_view()->editPosition=Global::pPlayer->_sequencePosition;
				m_wndView.Repaint(draw_modes::pattern);
			}
			int top = Global::pPlayer->_sequencePosition - 0xC;
			if (top < 0) top = 0;
			pSeqList->SetTopIndex(top);
#endif
		}
		else
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
// TODO
#else
			pSeqList->SelItemRange(false,0,pSeqList->GetCount()-1);
			for (int i=0;i<MAX_SONG_POSITIONS;i++ )
			{
				if (projects_.active_project()->song().playOrderSel[i]) pSeqList->SetSel(i,true);
			}
			int top = m_wndView.pattern_view()->editPosition - 0xC;
			if (top < 0) top = 0;
			pSeqList->SetTopIndex(top);
#endif
		}
	}
	m_wndView.SetFocus();
}


void CMainFrame::OnUpdateIndicatorSeqPos(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable();
	CString str;
	if (Global::pPlayer->playing())
	{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
//				SequenceLine *pSLine = *projects_.active_project()->song().sequence().begin();
//				SequenceLine::reverse_iterator sLineIt( pSLine->lower_bound(Player::singleton().playPos()) );
//				str.Format("B %.2f", );
#else
		str.Format("Pos %.2X", Global::pPlayer->_sequencePosition); 
#endif
	}
	else
	{
		str.Format("Pos %.2X", m_wndView.pattern_view()->editPosition); 
	}
	pCmdUI->SetText(str); 
}

void CMainFrame::OnUpdateIndicatorPattern(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(); 
	CString str;
	if (Global::pPlayer->playing())
	{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		SequenceLine *pSLine = *projects_.active_project()->song().sequence().begin();
		SequenceLine::reverse_iterator sLineIt( pSLine->lower_bound(Player::singleton().playPos()) );
		if (sLineIt != pSLine->rend()) {
			Pattern & pat = sLineIt->second->pattern();
			str.Format("Pat %.2X", pat.id());
		}
		else {
			str.Format("Pat ??");
		}
#else
		str.Format("Pat %.2X", Global::pPlayer->_playPattern); 
#endif
	}
	else
	{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
// TODO
#else
		str.Format("Pat %.2X", projects_.active_project()->song().playOrder[m_wndView.pattern_view()->editPosition]); 
#endif
	}
	pCmdUI->SetText(str); 
}

void CMainFrame::OnUpdateIndicatorLine(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(); 
	CString str;
	if (Global::pPlayer->playing())
	{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		str.Format("B %.2f", Player::singleton().playPos());
#else
		str.Format("Line %u", Global::pPlayer->_lineCounter); 
#endif
	}
	else
	{
		str.Format("Line %u", m_wndView.pattern_view()->editcur.line); 
	}
	pCmdUI->SetText(str); 
}

void CMainFrame::OnUpdateIndicatorTime(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(); 
	if (Global::pPlayer->playing())
	{
		CString str;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		const float playTime = Player::singleton().timeInfo().samplePos() /  (float) Player::singleton().timeInfo().sampleRate();
		const int timeInt = static_cast<int>(playTime);
		const int cents = (playTime - timeInt) * 100;
		const int secs = timeInt % 60;
		const int mins = ((timeInt/60) % 60);
		const int hour = timeInt / 3600;
		str.Format( "%.2u:%.2u:%.2u.%.2u", hour, mins, secs, cents); 
#else
		str.Format( "%.2u:%.2u:%.2u.%.2u",
			Global::pPlayer->_playTimem / 60,
			Global::pPlayer->_playTimem % 60,
			static_cast<unsigned int>(Global::pPlayer->_playTime),
			static_cast<unsigned int>(Global::pPlayer->_playTime * 100) -
			static_cast<unsigned int>(Global::pPlayer->_playTime) * 100);
#endif
		pCmdUI->SetText(str); 
	}
}

void CMainFrame::OnUpdateIndicatorEdit(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(m_wndView.pattern_view()->bEditMode);
}

void CMainFrame::OnUpdateIndicatorFollow(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(Global::pConfig->_followSong);
}

void CMainFrame::OnUpdateIndicatorNoteoff(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(Global::pConfig->_RecordNoteoff);
}

void CMainFrame::OnUpdateIndicatorTweaks(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(Global::pConfig->_RecordTweaks);
}

void CMainFrame::OnUpdateIndicatorOctave(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(); 
	CString str;
	str.Format("Oct %u", projects_.active_project()->song().currentOctave); 
	pCmdUI->SetText(str); 

}

int CMainFrame::GetNumFromCombo(CComboBox *cb)
{
	CString str;
	cb->GetWindowText(str);
	int result;
	hexstring_to_integer(str.Left(2).GetBuffer(2), result);
	return result;
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	if (wParam == AFX_IDS_IDLEMESSAGE)
	{
		if (StatusBarIdleText())
		{
			return CFrameWnd::OnSetMessageString (0,(LPARAM)szStatusIdle.c_str());
		}
	}
	return CFrameWnd::OnSetMessageString (wParam, lParam);

}

void CMainFrame::StatusBarIdle()
{
	if (StatusBarIdleText())
	{
		m_wndStatusBar.SetWindowText(szStatusIdle.c_str());
	}
}

BOOL CMainFrame::StatusBarIdleText()
{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
		// todo
#else
		std::ostringstream oss;
		oss << projects_.active_project()->song().name()
			<< " - " << projects_.active_project()->song().patternName[projects_.active_project()->song().playOrder[m_wndView.pattern_view()->editPosition]];

		if ((m_wndView.viewMode==view_modes::pattern)	&& (!Global::pPlayer->playing()))
		{
			int machine = toffset[2];
			if (machine<MAX_MACHINES)
			{
				if (projects_.active_project()->song().machine(machine))
				{
					oss << " - " << projects_.active_project()->song().machine(machine)->GetEditName();
					if (projects_.active_project()->song().machine(machine)->getMachineKey() == MachineKey::sampler)
					{
						if (projects_.active_project()->song()._pInstrument[toffset[1]]->_sName[0])
							oss <<  " - " << projects_.active_project()->song()._pInstrument[toffset[1]]->_sName;
					}
					else if (projects_.active_project()->song().machine(machine)->getMachineKey() == MachineKey::sampulse)
					{
						if (projects_.active_project()->song().rInstrument(toffset[1]).IsEnabled())
							oss <<  " - " << projects_.active_project()->song().rInstrument(toffset[1]).Name();
					}
					else
					{
						char buf[64];
						buf[0]=0;
						projects_.active_project()->song().machine(machine)->GetParamName(toffset[1],buf);
						if(buf[0])
							oss <<  " - " << buf;
					}
				}
				else
				{
					oss << " - Machine Out of Range";
				}
			}
		}

		szStatusIdle=oss.str();
		return TRUE;
	}
#endif
	return true;
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
				projects_.active_project()->OnFileLoadsongNamed(szFileName, 1);
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

void CMainFrame::OnViewSongbar() {
	ShowControlBar(&m_wndControl,!m_wndControl.IsWindowVisible(),FALSE);
}

void CMainFrame::OnViewMachinebar() {
	ShowControlBar(&m_wndControl2,!m_wndControl2.IsWindowVisible(),FALSE);
}

void CMainFrame::OnViewSequencerbar()  {
	ShowControlBar(&m_wndSeq,!m_wndSeq.IsWindowVisible(),FALSE);
}

void CMainFrame::OnUpdateViewSongbar(CCmdUI* pCmdUI)  {
	pCmdUI->SetCheck(m_wndControl.IsWindowVisible());			
}

void CMainFrame::OnUpdateViewMachinebar(CCmdUI* pCmdUI)  {
	pCmdUI->SetCheck(m_wndControl2.IsWindowVisible());			
}

void CMainFrame::OnUpdateViewSequencerbar(CCmdUI* pCmdUI)  {
	pCmdUI->SetCheck(m_wndSeq.IsWindowVisible());			
}

void CMainFrame::RedrawGearRackList() {
	if (pGearRackDialog) {
		pGearRackDialog->RedrawList();
	}
}

}}

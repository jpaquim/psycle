/** @file MainFrm.cpp
 *  @brief implmentation of the CMainFrame class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.8 $
 **/

#include "stdafx.h"
#include "TabbingFramework\atlgdix.h"
#include "TabbingFramework\CustomTabCtrl.h"
#include "TabbingFramework\DotNetTabCtrl.h"
#include "TabbingFramework\TabbedFrame.h"
#include "D3D.h"
#include <memory>
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"

#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "UndoRedo.h"
#include "sfhelper.h"
#include "SimpleDialog.h"
#include "resource.h"
#include "aboutdlg.h"
//#include "PsycleWTLView.h"
#include "configuration.h"
#include "ConfigDlg.h"
#include "DSoundConfig.h"
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
#include "MainFrm.h"
#include "XMInstrument.h"
#include "XMSampler.h"
#include "volume.h"
#include "XMSamplerUI.h"
#include "SaveWavDlg.h"
#include "SongpDlg.h"
#include "GreetDialog.h"


////////////////////////////////////////////////////////////////////////////
// CMainFrame
#define WM_SETMESSAGESTRING 0x0362

extern void testmain();

static int indicators[] =
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

boost::shared_ptr<CMainFrame> CMainFrame::m_pMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() : 
	m_wndControl(IDD_PSYBAR),m_wndControl2(IDD_PSYBAR2),m_wndSeq(IDD_SEQUENCER)
{
	Global::pInputHandler->SetMainFrame(this);
	vuprevR = 0;
	vuprevL = 0;
	seqcopybufferlength = 0;
	_pSong = NULL;
	pGearRackDialog = NULL;
    m_pWndWed = NULL;
	for(int i = 0;i < MAX_MACHINES;i++){
		m_pWndMac[i] = NULL;
	}

	_outputActive = false;

}

CMainFrame::~CMainFrame()
{
	ATLTRACE2("~CMainFrame()");
//	CDialogBar dfg;
	if(m_pWndWed){
		delete m_pWndWed;
		 m_pWndWed = NULL;

	}

	Global::pInputHandler->SetMainFrame(NULL);

	if (pGearRackDialog)
	{
		pGearRackDialog->OnCancel();
	}

	SF::D3D::Release();
}

/*
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
*/
/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
/*
BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_view.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	
	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
*/
/*
void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	if (((nState == WA_ACTIVE) || (nState == WA_CLICKACTIVE)) && (bMinimized == FALSE))
	{

	}
}
*/
void CMainFrame::ClosePsycle()
{
	BOOL bDum;
	OnClose(0,0,NULL,bDum);
}

void CMainFrame::StatusBarText(const TCHAR *txt)
{
	m_wndStatusBar.SetWindowText(txt);
}

void CMainFrame::PsybarsUpdate()
{
	m_pController->AddBpm(0);
	m_pController->AddTpb(0);


	CComboBox cc2 = (HWND)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
	cc2.SetCurSel(m_view.patStep);cc2.Detach();
	
	cc2.Attach((HWND)m_wndControl.GetDlgItem(IDC_TRACKCOMBO));
	cc2.SetCurSel(_pSong->SongTracks() - 4);cc2.Detach();

	cc2.Attach((HWND)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE));
	cc2.SetCurSel(_pSong->CurrentOctave());cc2.Detach();
	
	UpdateComboGen();
	
}

/////////////////
///////////////// PSY Bar 2
/////////////////



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




void CMainFrame::CenterWindowOnPoint(HWND hWnd, POINT point)
{
	RECT r,rw;
	WINDOWPLACEMENT w1;
	::GetWindowRect(hWnd,&r);
	m_view.GetWindowPlacement(&w1);
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

	::SetWindowPos(hWnd,NULL,x,y,0,0,SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}




////////////////////
//////////////////// Sequencer Dialog
////////////////////


int CMainFrame::GetNumFromCombo(CComboBox *cb)
{
	CString str;
	cb->GetWindowText(str);
	return _httoi(str.Left(2).GetBuffer(2));
}
/*
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
*/
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
		if ((m_view.viewMode == VMPattern)	&& (!m_pController->Player()._playing))
		{
			unsigned char *toffset = _pSong->_ptrackline(m_view.editPosition,m_view.editcur.track,m_view.editcur.line);
			int machine = toffset[2];
			if (machine < MAX_MACHINES)
			{
				if (_pSong->pMachine(machine))
				{
					if (_pSong->pMachine(machine)->_type == MACH_SAMPLER)
					{
						if (_pSong->pInstrument(toffset[1])->_sName[0])
						{
							_stprintf(szStatusIdle,_T("%s - %s - %s - %s"),
								_pSong->Name().c_str(),
								_pSong->PatternName(_pSong->PlayOrder(m_view.editPosition)),
								_pSong->pMachine(machine)->_editName,
								_pSong->pInstrument(toffset[1])->_sName);
							return TRUE;
						}
						else 
						{
							_stprintf(szStatusIdle,_T("%s - %s - %s"),
								_pSong->Name().c_str(),
								_pSong->PatternName(_pSong->PlayOrder(m_view.editPosition)),
								_pSong->pMachine(machine)->_editName);
							return TRUE;
						}
					}
					else
					{
						TCHAR buf[64];
						buf[0]=0;
						_pSong->pMachine(machine)->GetParamName(toffset[1],buf);
						if (buf[0])
						{
							_stprintf(szStatusIdle,_T("%s - %s - %s - %s"),
								_pSong->Name().c_str(),
								_pSong->PatternName(_pSong->PlayOrder(m_view.editPosition)),
								_pSong->pMachine(machine)->_editName,buf);
							return TRUE;
						}
						else 
						{
							_stprintf(szStatusIdle,_T("%s - %s - %s"),
								_pSong->Name().c_str(),
								_pSong->PatternName(_pSong->PlayOrder(m_view.editPosition)),
								_pSong->pMachine(machine)->_editName);
							return TRUE;
						}
					}
				}
				else
				{
					_stprintf(szStatusIdle,_T("%s - %s - Machine 範囲外"),
						_pSong->Name().c_str(),
						_pSong->PatternName(_pSong->PlayOrder(m_view.editPosition)));
					return TRUE;
				}
			}
		}
		_stprintf(szStatusIdle,_T("%s - %s"),
			_pSong->Name().c_str(),
			_pSong->PatternName(_pSong->PlayOrder(m_view.editPosition)));
		return TRUE;
	}
	return FALSE;
}

void CMainFrame::RedrawGearRackList()
{
	if (pGearRackDialog)
	{
		pGearRackDialog->RedrawList();
	}
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

LRESULT CMainFrame::OnCreate(LPCREATESTRUCT pCreateStruct)
{


	m_view.pMainFrame = this;
	m_view.Initialize();

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);

	// remove old menu
	SetMenu(NULL);
	
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | RBS_VERTICALGRIPPER );
	//CreateSimpleReBar();
	m_wndReBar = m_hWndToolBar;
	m_wndReBar.GetBarHeight();


	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	
	//CreateSimpleStatusBar();
	m_hWndStatusBar = m_wndStatusBar.Create(*this);
	m_wndStatusBar.SetPanes(::indicators,10);

	
//	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS , WS_EX_CLIENTEDGE);
/*	const DWORD dwSplitStyle = WS_CHILD | WS_VISIBLE |
                           WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            dwSplitExStyle = WS_EX_CLIENTEDGE;
    m_wndVertSplit.Create( *this, rcDefault, NULL,
                            dwSplitStyle, dwSplitExStyle );
    m_hWndClient = m_wndVertSplit;
*/
		// Sequencer Bar
	m_wndSeq.Create(m_hWnd,m_wndSeq.rcDefault,WS_CLIPSIBLINGS | WS_VISIBLE | WS_CLIPCHILDREN);
//	AddSimpleReBarBand(m_wndSeq,_T(""),TRUE);
//	m_view.Create(m_wndVertSplit, NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS ,WS_EX_CLIENTEDGE);
//	m_wndVertSplit.SetSplitterPanes(m_wndSeq,m_view);

	UIAddToolBar(hWndToolBar);
	UIAddStatusBar(m_hWndStatusBar);
	
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

////
////
////

	macComboInitialized = false;
	for(int c = 0;c < MAX_MACHINES ; c++){
		isguiopen[c] = false;
	}
	
	m_wndControl.Create(m_hWnd,0);
	m_wndControl.ShowWindow(SW_SHOW);
	m_wndControl2.Create(m_hWnd,0);
	m_wndControl2.ShowWindow(SW_SHOW);
	AddSimpleReBarBand(m_wndControl,0,TRUE);
	AddSimpleReBarBand(m_wndControl2,0,TRUE);
	m_view.ValidateParent();
	//m_view.ValidateParent();

	// CPU info Window
	m_wndInfo._pSong=_pSong;
	m_wndInfo.Create(m_hWnd,0);

	// MIDI monitor Dialog
	m_midiMonitorDlg.Create(m_hWnd,0);

	// Instrument editor
	m_wndInst._pSong = _pSong;
	m_wndInst.pParentMain = this;
	m_wndInst.Create(m_hWnd,0);
	m_wndInst.Validate();


	// Wave Editor Window
	m_pWndWed = new CWaveEdFrame(this->_pSong,this);
	m_pWndWed->CreateEx(m_hWnd);
	

	//m_pWndWed->LoadFrame(IDR_WAVEFRAME ,WS_OVERLAPPEDWINDOW,this);
	
	m_pWndWed->GenerateView();

	// Init Bars Content.
	//m_wndToolBar.SetWindowText(_T("Psycle ﾂｰﾙ ﾊﾞｰ"));
	m_wndControl.SetWindowText(_T("Psycle ｺﾝﾄﾛｰﾙ ﾊﾞｰ"));
	


	m_iLessLess.Create(ATL::_U_STRINGorID(IDB_LESSLESS),15,1,RGB(192,192,192));
	m_iLess.Create(IDB_LESS,15,1,RGB(192,192,192));
	m_iMore.Create(IDB_MORE,15,1,RGB(192,192,192));
	m_iMoreMore.Create(IDB_MOREMORE,15,1,RGB(192,192,192));
	m_iPlus.Create(IDB_PLUS,15,1,RGB(192,192,192));
	m_iMinus.Create(IDB_MINUS,15,1,RGB(192,192,192));
	m_iPlusPlus.Create(IDB_PLUSPLUS,15,1,RGB(192,192,192));
	m_iMinusMinus.Create(IDB_MINUSMINUS,15,1,RGB(192,192,192));
	m_iLittleLeft.Create(IDB_LLEFT,10,1,RGB(192,192,192));
	m_iLittleRight.Create(IDB_LRIGHT,10,1,RGB(192,192,192));
	

	WTL::CButton cb;
	//WTL::CBitmapButton cb1;
//	m_ImageList.SetBkColor(RGB(192,192,192));
  //  m_ImageList.CreateFromImage ( IDB_LESSLESS, 15, 1, CLR_NONE,ILC_COLOR4, 
  //                       LR_DEFAULTCOLOR );
//	COLORREF cr = 0;
//	m_ImageList.Add(m_iLessLess,RGB(192,192,192));

	cb.Attach((HWND)(m_wndControl.GetDlgItem(IDC_BTN_DECTEN)));
	//	cb1.SetBitmapButtonExtendedStyle();
	BUTTON_IMAGELIST img;
	
	img.himl = m_iLessLess;

	img.margin.bottom = 0;
	img.margin.top = 0;
	img.margin.right = 0;
	img.margin.left = 0;
	img.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
	cb.SetImageList(&img);
//	cb.SetIcon(LoadIcon(_Module.GetResourceInstance(),_T("IDR_MAINFRAME")));
//	cb.SetBitmap(hi);
	cb.Detach();
/*
	DWORD dw = BMPBTN_SHAREIMAGELISTS | BMPBTN_AUTO3D_SINGLE | BMPBTN_AUTOSIZE;
	m_cb1.SetBitmapButtonExtendedStyle(dw);
	m_cb1.SubclassWindow((HWND)(m_wndControl.GetDlgItem(IDC_BTN_DECTEN)));
	m_cb1.SetImageList(m_ImageList);
	m_cb1.SetImages(0,0,0,0);
*/
	//m_cb1.Detach();


	cb.Attach((HWND)(m_wndControl.GetDlgItem(IDC_BTN_DECONE)));
	img.himl = m_iLess; cb.SetImageList(&img);; cb.Detach();

	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_BTN_INCONE));
	img.himl = m_iMore; cb.SetImageList(&img);; cb.Detach();

	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_BTN_INCTEN));
	img.himl = m_iMoreMore; cb.SetImageList(&img);; cb.Detach();

	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_DEC_TPB));
	img.himl = m_iLess; cb.SetImageList(&img);; cb.Detach();

	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_INC_TPB));
	img.himl = m_iMore; cb.SetImageList(&img);; cb.Detach();


	m_wndControl2.SetWindowText(_T("Psycle ｺﾝﾄﾛｰﾙ ﾊﾞｰ 2"));

	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_DECGEN));
	img.himl = m_iLittleLeft; cb.SetImageList(&img);; cb.Detach();

	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_INCGEN));
	img.himl = m_iLittleRight; cb.SetImageList(&img);;cb.Detach();

	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_DECWAV));
	img.himl = m_iLittleLeft; cb.SetImageList(&img);;cb.Detach();

	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_INCWAV));
	img.himl = m_iLittleRight; cb.SetImageList(&img);;cb.Detach();
	


	CComboBox cc2;
	cc2.Attach((HWND)m_wndControl.GetDlgItem(IDC_TRACKCOMBO));
	for(int i = 4;i <= MAX_TRACKS ; i++)
	{
		TCHAR s[4];
		_stprintf(s,4,_T("%i"),i);
		cc2.AddString(s);
	}
	
	cc2.SetCurSel(_pSong->SongTracks() - 4);

	PsybarsUpdate();
	CComboBox cc;
	cc.Attach((HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT));
	cc.SetCurSel(2);


	InitSequencer();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_INCSHORT));
	img.himl = m_iPlus; cb.SetImageList(&img);;cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_INCLONG));
	img.himl = m_iPlusPlus; cb.SetImageList(&img);;cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_DECSHORT));
	img.himl = m_iMinus; cb.SetImageList(&img);;cb.Detach();
	
	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_DECLONG));
	img.himl = m_iMinusMinus; cb.SetImageList(&img);;cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_DECLEN));
	img.himl = m_iLess; cb.SetImageList(&img);;cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_INCLEN));
	img.himl = m_iMore; cb.SetImageList(&img);;cb.Detach();
	
	UpdatePlayOrder(true);
	
	::DragAcceptFiles(this->m_hWnd,TRUE);

	m_pController->configuration()->CreateFonts();

	// Finally initializing timer
	
	UpdateSequencer();
	m_wndSeq.ShowWindow(SW_SHOW);

	//AddSimpleReBarBand(m_wndSeq,0,TRUE);
	
#ifdef _TEST
	// Unit Test Menu
	WTL::CMenuHandle _menu;
	_menu.Attach(m_CmdBar.GetMenu());
	CMenuHandle _h = _menu.GetSubMenu(4);
	_h.AppendMenu(MF_STRING,ID_TEST,_T("DoTest"));
#endif

	LoadRecent();
	
	//m_wndSeq.SetFocus();
	UpdateWindow();
	OnIdle();
	m_hWndClient = m_view.Create(m_hWnd);
	InitTimer();
	m_view.SetFocus();
	
	// D3Dオブジェクトの初期化
	// 最初の1回だけはこの形式で呼び出し、初期化
	// 次からは D3DGetInstance()で取り出せる
	SF::D3D::GetInstance(this).Initialize();

	return 0;
}

void CMainFrame::InitSequencer(){
	WTL::CButton cb;
	// set multichannel audition checkbox status
	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_MULTICHANNEL_AUDITION));
	cb.SetCheck(Global::pInputHandler->bMultiKey?1:0);cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_RECORD_NOTEOFF));
	cb.SetCheck(m_pController->configuration()->_RecordNoteoff?1:0);cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_RECORD_TWEAKS));
	cb.SetCheck(m_pController->configuration()->_RecordTweaks?1:0);cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_NOTESTOEFFECTS));
	cb.SetCheck(m_pController->configuration()->_notesToEffects?1:0);cb.Detach();

	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_FOLLOW));
	cb.SetCheck(m_pController->configuration()->_followSong?1:0);cb.Detach();
}

// アイドル時の処理 //
BOOL CMainFrame::OnIdle()
{
	CString str;
	if (m_pController->Player()._playing)
	{
	    str.Format(_T("Pos %.2X"), m_pController->Player()._playPosition); 
	}
	else
	{
	    str.Format(_T("Pos %.2X"), m_view.editPosition); 
	}
	UISetText(INDICATOR_SEQPOS,str,FALSE);

	if (m_pController->Player()._playing)
	{
	    str.Format(_T("Pat %.2X"), m_pController->Player()._playPattern); 
	}
	else
	{
	    str.Format(_T("Pat %.2X"), m_pController->Song()->PlayOrder(m_view.editPosition)); 
	}
	UISetText(INDICATOR_PATTERN,str,FALSE);

	if (m_pController->Player()._playing)
	{
	    str.Format(_T("Line %u"), m_pController->Player()._lineCounter); 
	}
	else
	{
	    str.Format(_T("Line %u"), m_view.editcur.line); 
	}
	UISetText(INDICATOR_LINE,str,FALSE);

	if (m_pController->Player()._playing)
	{
		CString str;
	    str.Format( _T("%.2u:%.2u:%.2u.%.2u"), m_pController->Player()._playTimem / 60, m_pController->Player()._playTimem % 60, f2i(m_pController->Player()._playTime), f2i(m_pController->Player()._playTime*100)-(f2i(m_pController->Player()._playTime)*100)); 
		UISetText(INDICATOR_TIME,str,FALSE);
	}

	UIEnable(INDICATOR_EDIT,m_view.bEditMode); 
	UIEnable(INDICATOR_FOLLOW,m_pController->configuration()->_followSong);
	UIEnable(INDICATOR_NOTEOFF,m_pController->configuration()->_RecordNoteoff);
	UIEnable(INDICATOR_TWEAKS,m_pController->configuration()->_RecordTweaks);
	str.Format(_T("Oct %u"), _pSong->CurrentOctave());
	UISetText(INDICATOR_OCTAVE,str,FALSE);
	
	if (m_pController->configuration()->autoStopMachines == true ) 
		UISetCheck(ID_AUTOSTOP,1);
	else 
		UISetCheck(ID_AUTOSTOP,0);

	UISetCheck(ID_VIEW_SONGBAR,m_wndControl.IsWindowVisible());
	UISetCheck(ID_VIEW_MACHINEBAR,m_wndControl2.IsWindowVisible());
	UISetCheck(ID_VIEW_SEQUENCERBAR,m_wndSeq.IsWindowVisible());

	m_view.UpdateUI();

	UIUpdateMenuBar();
	UIUpdateToolBar();
	UIUpdateStatusBar();


	
	return FALSE;
}


LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	
	WTL::CReBarCtrlT<CWindow> rebar;
	
	rebar = (HWND)m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}


LRESULT CMainFrame::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// forward focus to the view window
	m_view.Repaint();
	m_view.SetFocus();
	m_view.EnableSound();
	return 0;
}

LRESULT CMainFrame::OnViewSequencerbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//WTL::CReBarCtrl rebar = m_hWndToolBar;

	if (!m_wndSeq.IsWindowVisible())
	{
		m_wndSeq.ShowWindow(SW_SHOW);//rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1),TRUE);
		UISetCheck(ID_VIEW_SEQUENCERBAR,TRUE);
	}
	else {	
		//rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1),FALSE);
		m_wndSeq.ShowWindow(SW_HIDE);
		UISetCheck(ID_VIEW_SEQUENCERBAR,FALSE);
	}
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewSongbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WTL::CReBarCtrl rebar = m_hWndToolBar;
	if (!m_wndControl.IsWindowVisible())
	{
		rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 2),TRUE);
		UISetCheck(ID_VIEW_SONGBAR,TRUE);
	} else {
		rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 2),FALSE);
		UISetCheck(ID_VIEW_SONGBAR,FALSE);
	}
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewMachinebar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WTL::CReBarCtrl rebar = m_hWndToolBar;
	if (!m_wndControl2.IsWindowVisible())
	{		
		rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 3),TRUE);
		UISetCheck(ID_VIEW_MACHINEBAR,TRUE);
	} else 	{	
		rebar.ShowBand(rebar.IdToIndex(ATL_IDW_BAND_FIRST + 3),FALSE);
		UISetCheck(ID_VIEW_MACHINEBAR,FALSE);
	}
	UpdateLayout();
	return 0;
}

//
// Pybar-1
//

LRESULT CMainFrame::OnCbnSelchangeTrackcombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	WTL::CComboBox cc2 = (HWND)m_wndControl.GetDlgItem(IDC_TRACKCOMBO);
	_pSong->SongTracks(cc2.GetCurSel() + 4);
	
	if (m_view.editcur.track >= _pSong->SongTracks() )
		m_view.editcur.track= _pSong->SongTracks()-1;

	m_view.RecalculateColourGrid();
	m_view.Repaint();
	m_view.SetFocus();

	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnCbnCloseupTrackcombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	m_view.SetFocus();
	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnBpmIncOne(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->AddBpm(1);
	m_view.SetFocus();	
	return 0;
}

LRESULT CMainFrame::OnBpmIncTen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->AddBpm(10);
	m_view.SetFocus();	
	return 0;
}

LRESULT CMainFrame::OnBpmDecOne(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->AddBpm(-1);
	m_view.SetFocus();	
	return 0;
}

LRESULT CMainFrame::OnBpmDecTen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->AddBpm(-10);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedDecTpb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->AddTpb(-1);
	// TODO: View に応じて変更
	m_view.SetFocus();
	m_view.Repaint();
	return 0;
}

LRESULT CMainFrame::OnBnClickedIncTpb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->AddTpb(1);

	// TODO: View に応じて変更
	m_view.SetFocus();
	m_view.Repaint();
	return 0;
}

LRESULT CMainFrame::OnCbnSelchangeCombooctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComboBox cc2 = (HWND)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	_pSong->CurrentOctave(cc2.GetCurSel());
	
	m_view.Repaint();
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnCbnCloseupCombooctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedClipbut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	((Master*)(m_pController->Song()->pMachine(MASTER_INDEX)))->_clip = false;
	m_view.SetFocus();
	return 0;
}

void CMainFrame::ShiftOctave(int x)
{
	_pSong->CurrentOctave( _pSong->CurrentOctave() + x);


	CComboBox cc2 = (HWND)m_wndControl.GetDlgItem(IDC_COMBOOCTAVE);
	cc2.SetCurSel(_pSong->CurrentOctave());
}


void CMainFrame::UpdateVumeters(float l, float r,COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip)
{
	if (m_pController->configuration()->draw_vus)
	{
		if(l<1)l=1;
		if(r<1)r=1;
		
		//CStatic lc = (HWND)m_wndControl.GetDlgItem(IDC_FRAMECLIP);
		WTL::CClientDC  clcanvasl((HWND)m_wndControl.GetDlgItem(IDC_FRAMECLIP));
		
		if (clip) clcanvasl.FillSolidRect(0,0,9,16,vu3);
		else  clcanvasl.FillSolidRect(0,0,9,16,vu2);

		//CStatic lv = (HWND)m_wndControl.GetDlgItem(IDC_LVUM);
		//CStatic rv = (HWND)m_wndControl.GetDlgItem(IDC_RVUM);
		
		CClientDC canvasl((HWND)m_wndControl.GetDlgItem(IDC_LVUM));
		CClientDC canvasr((HWND)m_wndControl.GetDlgItem(IDC_RVUM));

		int log_l = f2i(100 * log10f(l));
		int log_r = f2i(100 * log10f(r));
		log_l = log_l - 226;
		
		if( log_l < 0 ) log_l = 0;
		
		log_r = log_r-226;
		
		if ( log_r < 0 ) log_r = 0;
		
		if (log_l || vuprevL)
		{
			canvasl.FillSolidRect(0,0,log_l,5,vu1);
			if (vuprevL > log_l )
			{
				canvasl.FillSolidRect(log_l,0,vuprevL - log_l,5,vu3);
				canvasl.FillSolidRect(vuprevL,0,226 - vuprevL,5,vu2);
				vuprevL -= 2;
			}
			else 
			{
				canvasl.FillSolidRect(log_l,0,226 - log_l,5,vu2);
				vuprevL = log_l;
			}
		}
		else
			canvasl.FillSolidRect(0,0,226,5,vu2);

		if (log_r || vuprevR)
		{
			canvasr.FillSolidRect(0,0,log_r,5,vu1);
			if (vuprevR > log_r )
			{
				canvasr.FillSolidRect(log_r,0,vuprevR - log_r,5,vu3);
				canvasr.FillSolidRect(vuprevR,0,226 - vuprevR,5,vu2);
				vuprevR-=2;
			}
			else 
			{
				canvasr.FillSolidRect(log_r,0,226 - log_r,5,vu2);
				vuprevR = log_r;
			}
		}
		else
			canvasr.FillSolidRect(0,0,226,5,vu2);
	}
}

LRESULT CMainFrame::OnCbnSelchangeSscombo2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComboBox cc = (HWND)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);
	int sel = cc.GetCurSel();
	m_view.patStep = sel;
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnCbnCloseupSscombo2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.SetFocus();
	return 0;
}
void CMainFrame::EditQuantizeChange(int diff) // User Called (Hotkey)
{
	CComboBox cc = (HWND)m_wndControl2.GetDlgItem(IDC_SSCOMBO2);	
	const int total = cc.GetCount();
	const int nextsel = (total + cc.GetCurSel() + diff) % total;
	cc.SetCurSel(nextsel);
	m_view.patStep = nextsel;
}

LRESULT CMainFrame::OnCbnSelchangeBarCombogen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(macComboInitialized)
	{
		CComboBox cc((HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN));
		int nsb = GetNumFromCombo(&cc);

		if(_pSong->SeqBus()!=nsb)
		{
			_pSong->SeqBus(nsb);
			UpdateComboGen(false);
		}
		RedrawGearRackList();
	}
	/*
	CComboBox cc1 = (HWND)m_wndControl2.GetDlgItem(IDC_BAR_GENFX);
	_pSong->seqBus &= (MAX_BUSES-1);
	if ( cc1.GetCurSel() == 1 )
	{
		_pSong->seqBus |= MAX_BUSES;
	}
	UpdateComboGen();
	*/
	return 0;
}

LRESULT CMainFrame::OnCbnCloseupBarCombogen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedBDecgen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//	ChangeGen(_pSong->seqBus-1);
	CComboBox cc((HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN));
	const int val = cc.GetCurSel();
	if ( val > 0 ) cc.SetCurSel(val-1);
	BOOL bDum;
	OnCbnSelchangeBarCombogen(0,0,NULL,bDum);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedBIncgen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//	ChangeGen(_pSong->seqBus+1);
	CComboBox cc((HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN));
	const int val = cc.GetCurSel();
	if ( val < cc.GetCount()-1 ) cc.SetCurSel(val+1);
	BOOL bDum;
	OnCbnSelchangeBarCombogen(0,0,NULL,bDum);
	m_view.SetFocus();
	return 0;
}
void CMainFrame::UpdateComboGen(bool updatelist)
{
	bool filled = false;
	bool found = false;
	int selected = -1;
	int line = -1;
	TCHAR buffer[64];
	
	if (_pSong == NULL) 
	{
		return; // why should this happen?
	}
	CComboBox  cb = (HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOGEN);
	CComboBox cb2 = (HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	macComboInitialized = false;
	if (updatelist) 
	{
		cb.ResetContent();
	}
	
	for (int b = 0; b < MAX_BUSES; b++) // Check Generators
	{
		if( _pSong->pMachine(b))
		{
			if (updatelist)
			{	
				_stprintf(buffer,_T("%.2X: %s"),b,_pSong->pMachine(b)->_editName);
				cb.AddString(buffer);
				
			}
			if (!found) 
			{
				selected++;
			}
			if (_pSong->SeqBus() == b) 
			{
				found = true;
			}
			filled = true;
		}
	}
	if ( updatelist) 
	{
		cb.AddString(_T("----------------------------------------------------"));
	}
	if (!found) 
	{
		selected++;
		line = selected;
	}
	
	for (b = MAX_BUSES; b < MAX_BUSES * 2; b++) // Write Effects Names.
	{
		if(_pSong->pMachine(b))
		{
			if (updatelist)
			{	
				_stprintf(buffer,_T("%.2X: %s"),b,_pSong->pMachine(b)->_editName);
				cb.AddString(buffer);
			}
			if (!found) 
			{
				selected++;
			}
			if (_pSong->SeqBus() == b) 
			{
				found = true;
			}
			filled = true;
		}
	}
	if (!filled)
	{
		cb.ResetContent();
		cb.AddString(RES_STR(IDS_NOTMAC));
		selected = 0;
	}
	else if (!found) 
	{	
		selected = line;
	}
	
	cb.SetCurSel(selected);

	// Select the appropiate Option in Aux Combobox.
	if (found) // If found (which also means, if it exists)
	{
		if (_pSong->pMachine(_pSong->SeqBus()))
		{
			if ( _pSong->SeqBus() < MAX_BUSES ) // it's a Generator
			{
				if (_pSong->pMachine(_pSong->SeqBus())->_type == MACH_SAMPLER)
				{
					cb2.SetCurSel(AUX_WAVES);
					_pSong->AuxcolSelected(_pSong->InstSelected());
				}
				else if (_pSong->pMachine(_pSong->SeqBus())->_type == MACH_VST)
				{
					if ( cb2.GetCurSel() == AUX_WAVES)
					{
						cb2.SetCurSel(AUX_MIDI);
						_pSong->AuxcolSelected(_pSong->MidiSelected());
					}
				}
				else
				{
					cb2.SetCurSel(AUX_PARAMS);
					_pSong->AuxcolSelected(0);
				}
			}
			else
			{
				cb2.SetCurSel(AUX_PARAMS);
				_pSong->AuxcolSelected(0);
			}
		}
	}
	else
	{
		cb2.SetCurSel(AUX_WAVES); // WAVES
		_pSong->AuxcolSelected(_pSong->InstSelected());
	}
	UpdateComboIns();
	macComboInitialized = true;
}

void CMainFrame::ChangeGen(int i)	// User Called (Hotkey)
{
	if(i >= 0 && i < (MAX_BUSES * 2))
	{
		if ( (_pSong->SeqBus() & MAX_BUSES) == (i & MAX_BUSES))
		{
			_pSong->SeqBus(i);
			UpdateComboGen(false);
		}
		else
		{
			_pSong->SeqBus(i);
			UpdateComboGen(true);
		}
	}
}


LRESULT CMainFrame::OnCbnSelchangeAuxselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComboBox cc2((HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT));

	if ( cc2.GetCurSel() == AUX_MIDI )	// MIDI
	{
		_pSong->AuxcolSelected(_pSong->MidiSelected());
	}
	else if ( cc2.GetCurSel() == AUX_WAVES )	// WAVES
	{
		_pSong->AuxcolSelected(_pSong->InstSelected());
	}
	UpdateComboIns();
	return 0;
}

LRESULT CMainFrame::OnCbnCloseupAuxselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedBIncwav(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ChangeIns(_pSong->AuxcolSelected()+1);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedBDecwav(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ChangeIns(_pSong->AuxcolSelected()-1);
	m_view.SetFocus();
	return 0;
}
void CMainFrame::UpdateComboIns(bool updatelist)
{
	CComboBox cc = (HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox cc2 = (HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	int listlen = 0;
	
	if (updatelist) 
	{
		cc.ResetContent();
	}

	if ( cc2.GetCurSel() == AUX_MIDI )	// MIDI
	{
		TCHAR buffer[64];
		if (updatelist) 
		{
			for (int i=0;i<16;i++)
			{
				_stprintf(buffer, RES_STR(IDS_MIDICH_FMT), i,i+1);
				cc.AddString(buffer);
			}
		}
		listlen = 16;
//		_pSong->midiSelected=_pSong->AuxcolSelected();
	}
	else if ( cc2.GetCurSel() == AUX_PARAMS)	// Params
	{

		int nmac = _pSong->SeqBus();
		Machine *tmac = _pSong->pMachine(nmac);
		if (tmac) 
		{
			int i = 0;
			if (updatelist) 
			{
				for (i = 0;i < tmac->GetNumParams() ; i++)
				{
					TCHAR buffer[64],buffer2[64];
					::ZeroMemory((void *)buffer2,64);
					tmac->GetParamName(i,buffer2);
					_stprintf(buffer, _T("%.2X:  %s"), i, buffer2);
					cc.AddString(buffer);
					listlen++;
				}
			}
			else
			{
				listlen = cc.GetCount();
			}
		}
		else
		{
			if (updatelist) 
			{
				cc.AddString(RES_STR(IDS_NOTMAC));
			}
			listlen = 1;
		}
	}
	else	// Waves
	{
		TCHAR buffer[64];
		if (updatelist) 
		{
			for (int i = 0 ; i < PREV_WAV_INS ; i++)
			{
				_stprintf(buffer, _T("%.2X: %s"), i, _pSong->pInstrument(i)->_sName);
				cc.AddString(buffer);
				listlen++;
			}
		}
		else
		{
			listlen = cc.GetCount();
		}
	}
	if (_pSong->AuxcolSelected() >= listlen)
	{
		_pSong->AuxcolSelected(0);
	}
	cc.SetCurSel(_pSong->AuxcolSelected());
}

LRESULT CMainFrame::OnCbnSelchangeBarComboins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComboBox cc = (HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox cc2 = (HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if ( cc2.GetCurSel() == AUX_MIDI ) 
	{
		_pSong->MidiSelected(cc.GetCurSel());
	}
	else if ( cc2.GetCurSel() == AUX_WAVES ) 
	{
		_pSong->InstSelected(cc.GetCurSel());
		WaveEditorBackUpdate();
		m_wndInst.WaveUpdate();
		RedrawGearRackList();
	}

	_pSong->AuxcolSelected(cc.GetCurSel());
	return 0;
}

LRESULT CMainFrame::OnCbnCloseupBarComboins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.SetFocus();
	return 0;
}

void CMainFrame::ChangeIns(int i)	// User Called (Hotkey)
{
	CComboBox cc=(HWND)m_wndControl2.GetDlgItem(IDC_BAR_COMBOINS);
	CComboBox cc2=(HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT);

	if (cc2.GetCurSel() == AUX_MIDI )
	{
		if (i >= 0 && i < 16)
		{
			_pSong->MidiSelected(i);
			_pSong->AuxcolSelected(i);
		}
	}
	else if ( cc2.GetCurSel() == AUX_PARAMS )
	{
		if (i >= 0 && i < cc.GetCount() )
		{
			_pSong->AuxcolSelected(i);
		}
	}
	else
	{
		if(i >= 0 && i < (PREV_WAV_INS))
		{
			_pSong->InstSelected(i);
			_pSong->AuxcolSelected(i);
			WaveEditorBackUpdate();
			m_wndInst.WaveUpdate();
			RedrawGearRackList();
		}
	}
	cc.SetCurSel(_pSong->AuxcolSelected());
}

LRESULT CMainFrame::OnBnClickedLoadwave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString _ps(RES_STR(IDS_FILTER_WAVE));
	_ps.Replace(_T('|'),_T('\0'));
	
	CWavFileDlg dlg(true,_T("wav"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_ps.GetBuffer(),m_hWnd);
	dlg._pSong = _pSong;
	
	dlg.m_ofn.lpstrInitialDir = m_pController->configuration()->GetInstrumentDir();
	if (dlg.DoModal() == IDOK)
	{
		AddMacViewUndo();

		int si = _pSong->InstSelected();
		int sw = _pSong->WaveSelected();

		CString CurrExt(dlg.m_ofn.lpstrDefExt);
		CurrExt.MakeLower();
		
		if ( CurrExt == _T("wav") )
		{
			if (_pSong->WavAlloc(si,sw,dlg.m_ofn.lpstrFile))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText(RES_STR(IDS_LOAD_WAVE));
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
			}
		}
		else if ( CurrExt == _T("iff") )
		{
			if (_pSong->IffAlloc(si,sw,dlg.m_ofn.lpstrFile))
			{
				UpdateComboIns();
				m_wndStatusBar.SetWindowText(RES_STR(IDS_LOAD_WAVE));
				WaveEditorBackUpdate();
				m_wndInst.WaveUpdate();
				RedrawGearRackList();
			}
		}
		CString str = dlg.m_ofn.lpstrFile;
		int index = str.ReverseFind(_T('\\'));
		if (index != -1)
		{
			m_pController->configuration()->SetInstrumentDir(str.Left(index));
		}
	}
	if ( _pSong->pInstrument(PREV_WAV_INS)->waveLength[0] > 0)
	{
		// Stopping wavepreview if not stopped.
		if(_pSong->PW_Stage())
		{
			_pSong->PW_Stage(0);
			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);
		}

		//Delete it.
		_pSong->DeleteLayer(PREV_WAV_INS,0);
		_pSong->IsInvalided(false);
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedSavewave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
int c = 0;
	WaveFile output;
	
	if (_pSong->pInstrument(_pSong->InstSelected())->waveLength[_pSong->WaveSelected()])
	{
		CFileDialog dlg(FALSE,
			_T("wav"), 
			_pSong->pInstrument(_pSong->InstSelected())->waveName[_pSong->WaveSelected()],
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, RES_STR(IDS_FILTER_WAVE));
		if (dlg.DoModal() == IDOK)
		{
			output.OpenForWrite(dlg.m_ofn.lpstrFile, 
				44100, 16,
				(_pSong->pInstrument(_pSong->InstSelected())->waveStereo[_pSong->WaveSelected()]) ? (2) : (1) );
			if (_pSong->pInstrument(_pSong->InstSelected())->waveStereo[_pSong->WaveSelected()])
			{
				for ( unsigned int c=0; c < _pSong->pInstrument(_pSong->InstSelected())->waveLength[_pSong->WaveSelected()]; c++)
				{
					output.WriteStereoSample( *(_pSong->pInstrument(_pSong->InstSelected())->waveDataL[_pSong->WaveSelected()] + c),
						*(_pSong->pInstrument(_pSong->InstSelected())->waveDataR[_pSong->WaveSelected()] + c) );
				}
			}
			else
			{
				output.WriteData(
					_pSong->pInstrument(_pSong->InstSelected())->waveDataL[_pSong->WaveSelected()],
					_pSong->pInstrument(_pSong->InstSelected())->waveLength[_pSong->WaveSelected()]);
			}

			output.Close();
		}
	}
	else MessageBox(RES_STR(IDS_SAVE_NOTFOUND), RES_STR(IDS_ERROR), MB_ICONERROR);
	return 0;
}

LRESULT CMainFrame::OnBnClickedGearRack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (pGearRackDialog == NULL)
	{
		pGearRackDialog = new CGearRackDlg(&m_view, this);
		pGearRackDialog->Create(m_hWnd);
		pGearRackDialog->ShowWindow(SW_SHOW);
	}
	return 0;
}

LRESULT CMainFrame::OnBnClickedWavebut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pWndWed->ShowWindow(SW_SHOWNORMAL);
	m_pWndWed->SetActiveWindow();
	return 0;
}

LRESULT CMainFrame::OnBnClickedEditwave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowInstrumentEditor();
	return 0;
}
void CMainFrame::WaveEditorBackUpdate()
{
	m_pWndWed->Notify();
}

void CMainFrame::ShowInstrumentEditor()
{
	CComboBox cc2 = (HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT);
	cc2.SetCurSel(AUX_WAVES);
	_pSong->AuxcolSelected(_pSong->InstSelected());
	UpdateComboIns();

	AddMacViewUndo();

	m_wndInst.WaveUpdate();
	m_wndInst.ShowWindow(SW_SHOWNORMAL);
	m_wndInst.SetActiveWindow();
}

void CMainFrame::HideInstrumentEditor()
{
	m_wndInst.ShowWindow(SW_HIDE);
}

LRESULT CMainFrame::OnPsyhelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MessageBox(RES_STR(IDS_NOT_MAKEHELP),_T("Psycle"));
//	HtmlHelp(NULL, _T("psycle.chm"), HH_DISPLAY_TOPIC, 0);
	return 0;
}

//
// Sequencer
//
void CMainFrame::UpdateSequencer(int bottom)
{
	WTL::CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	
	TCHAR buf[16];
	int top = cc.GetTopIndex();
	cc.ResetContent();
	
	for(int n = 0;n < _pSong->PlayLength(); n++)
	{
		_stprintf(buf,_T("%.2X: %.2X"),n,_pSong->PlayOrder(n));
		cc.AddString(buf);
	}
	
	cc.SelItemRange(false,0,cc.GetCount()-1);

	for (int i = 0; i < MAX_SONG_POSITIONS;i++)
	{
		if ( _pSong->PlayOrderSel(i)) cc.SetSel(i,true);
	}
	if (bottom >= 0)
	{
		if (top < bottom-0x15)
		{
			top = bottom-0x15;
		}
		if (top < 0)
		{
			top = 0;
		}
	}
	cc.SetTopIndex(top);
}

LRESULT CMainFrame::OnLbnSelchangeSeqlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int maxitems = cc.GetCount();
	int const ep = cc.GetCaretIndex();
	if(m_view.editPosition < 0) m_view.editPosition = 0; // DAN FIXME
	int const cpid = _pSong->PlayOrder(m_view.editPosition);
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		_pSong->PlayOrderSel(i,false);
	}
	//memset(_pSong->playOrderSel , 0 , MAX_SONG_POSITIONS * sizeof(bool));
	for (int c = 0; c < maxitems; c++) 
	{
		if ( cc.GetSel(c) != 0) _pSong->PlayOrderSel(c,true);
	}
	
	if((ep != m_view.editPosition))// && ( cc.GetSelCount() == 1))
	{
		if ((m_pController->Player()._playing) && (m_pController->configuration()->_followSong))
		{
			bool b = m_pController->Player()._playBlock;
			m_pController->Player().Start(ep,0);
			m_pController->Player()._playBlock = b;
		}
		m_view.editPosition = ep;
		m_view.prevEditPosition = ep;
		UpdatePlayOrder(false);
		
		if(cpid!=_pSong->PlayOrder(ep))
		{
			m_view.Repaint(DMPattern);
		}		
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnLbnDblclkSeqlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int const ep = cc.GetCaretIndex();
	if (m_pController->Player()._playing)
	{
		bool b = m_pController->Player()._playBlock;
		m_pController->Player().Start(ep,0);
		m_pController->Player()._playBlock = b;
	}
	else
	{
		m_pController->Player().Start(ep,0);
	}
	m_view.editPosition = ep;
	OnPatternView();
	return 0;
}

LRESULT CMainFrame::OnBnClickedIncshort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
int indexes[MAX_SONG_POSITIONS];
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);

	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int const num= cc.GetSelCount();
	cc.GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->PlayOrder(indexes[i]) < (MAX_PATTERNS - 1))
		{
			_pSong->PlayOrder(indexes[i],_pSong->PlayOrder(indexes[i]) + 1);
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_view.Repaint(DMPattern);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedDecshort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int indexes[MAX_SONG_POSITIONS];
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);

	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int const num= cc.GetSelCount();
	cc.GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->PlayOrder(indexes[i])>0)
		{
			_pSong->PlayOrder(indexes[i],_pSong->PlayOrder(indexes[i]) - 1);
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_view.Repaint(DMPattern);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedInclong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
int indexes[MAX_SONG_POSITIONS];
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);

	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int const num = cc.GetSelCount();
	cc.GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->PlayOrder(indexes[i]) < (MAX_PATTERNS - 16))
		{
			_pSong->PlayOrder(indexes[i],_pSong->PlayOrder(indexes[i]) + 16);			
		}
		else
		{
			_pSong->PlayOrder(indexes[i],(MAX_PATTERNS - 1));
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_view.Repaint(DMPattern);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedDeclong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int indexes[MAX_SONG_POSITIONS];
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);

	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int const num= cc.GetSelCount();
	cc.GetSelItems(MAX_SONG_POSITIONS,indexes);

	for (int i = 0; i < num; i++)
	{
		if(_pSong->PlayOrder(indexes[i])>=16)
		{
			_pSong->PlayOrder(indexes[i], _pSong->PlayOrder(indexes[i]) - 16);			
		}
		else
		{
			_pSong->PlayOrder(indexes[i],0);
		}
	}
	UpdatePlayOrder(false);
	UpdateSequencer();
	m_view.Repaint(DMPattern);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqnew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
if(_pSong->PlayLength() < (MAX_SONG_POSITIONS - 1))
	{
		m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
		_pSong->PlayLength(_pSong->PlayLength() + 1);

		m_view.editPosition++;
		int const pop=m_view.editPosition;
		for(int c=(_pSong->PlayLength()-1);c>=pop;c--)
		{
			_pSong->PlayOrder(c,_pSong->PlayOrder(c - 1));
		}
		_pSong->PlayOrder(m_view.editPosition,_pSong->GetBlankPatternUnused());
		
		if ( _pSong->PlayOrder(m_view.editPosition)>= MAX_PATTERNS )
		{
			_pSong->PlayOrder(m_view.editPosition,MAX_PATTERNS - 1);
		}

		_pSong->AllocNewPattern(_pSong->PlayOrder(m_view.editPosition),_T(""),m_pController->configuration()->defaultPatLines,FALSE);

		UpdatePlayOrder(true);
		UpdateSequencer(m_view.editPosition);

		m_view.Repaint(DMPattern);
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(_pSong->PlayLength()<(MAX_SONG_POSITIONS-1))
	{
		m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
		_pSong->PlayLength(_pSong->PlayLength() + 1);

		m_view.editPosition++;
		int const pop=m_view.editPosition;
		for(int c=(_pSong->PlayLength()-1);c>=pop;c--)
		{
			_pSong->PlayOrder(c,_pSong->PlayOrder(c - 1));
		}

		UpdatePlayOrder(true);
		UpdateSequencer(m_view.editPosition);

		m_view.Repaint(DMPattern);
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqduplicate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int newpat = _pSong->GetBlankPatternUnused();
	if ((_pSong->PlayLength()<(MAX_SONG_POSITIONS-1)) && (newpat < MAX_PATTERNS-1))
	{
		m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
		int oldpat = _pSong->PlayOrder(m_view.editPosition);
		_pSong->PlayLength(_pSong->PlayLength() + 1);

		m_view.editPosition++;
		int const pop=m_view.editPosition;
		for(int c=(_pSong->PlayLength()-1);c>=pop;c--)
		{
			_pSong->PlayOrder(c,_pSong->PlayOrder(c - 1));
		}

		_pSong->PlayOrder(m_view.editPosition,newpat);
		
		// now we copy the data
		// we don't really need to be able to undo this, since it's a new pattern anyway.
//		m_view.AddUndo(newpat,0,0,MAX_TRACKS,_pSong->PatternLines(newpat),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
		_pSong->AllocNewPattern(newpat,
			_pSong->PatternName(oldpat),
			_pSong->PatternLines(oldpat),FALSE);

		memcpy(_pSong->_ppattern(newpat),_pSong->_ppattern(oldpat),MULTIPLY2);

		UpdatePlayOrder(true);
		UpdateSequencer(m_view.editPosition);

		m_view.Repaint(DMPattern);
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqcut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bDum;
	OnBnClickedSeqcopy(NULL,NULL,NULL,bDum);
	OnBnClickedSeqdelete(NULL,NULL,NULL,bDum);
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqdelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int indexes[MAX_SONG_POSITIONS];
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);

	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	int const num= cc.GetSelCount();
	cc.GetSelItems(MAX_SONG_POSITIONS,indexes);

	// our list can be in any order so we must be careful
	int smallest = indexes[0]; // we need a good place to put the cursor when we are done, above the topmost selection seems most intuitive
	for (int i=0; i < num; i++)
	{
		for(int c=indexes[i];c<_pSong->PlayLength()-1;c++)
		{
			_pSong->PlayOrder(c,_pSong->PlayOrder(c + 1));
		}
		_pSong->PlayOrder(c,0);
		_pSong->PlayLength(_pSong->PlayLength() - 1);
		if (_pSong->PlayLength() <= 0)
		{
			_pSong->PlayLength(1);
		}
		for (int j=i+1;j<num;j++)
		{
			if (indexes[j] > indexes[i])
			{
				indexes[j]--;
			}
		}
		if (indexes[i] < smallest)
		{
			smallest = indexes[i];
		}
	}
	m_view.editPosition = smallest-1;

	if (m_view.editPosition<0)
	{
		m_view.editPosition = 0;
	}
	else if (m_view.editPosition>=_pSong->PlayLength())
	{
		m_view.editPosition=_pSong->PlayLength()-1;
	}

	UpdatePlayOrder(true);
	UpdateSequencer(m_view.editPosition);
	m_view.Repaint(DMPattern);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqcopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CListBox cc((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	seqcopybufferlength= cc.GetSelCount();
	cc.GetSelItems(MAX_SONG_POSITIONS,seqcopybuffer);

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
		seqcopybuffer[i] = _pSong->PlayOrder(seqcopybuffer[i]);
	}
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqpaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (seqcopybufferlength > 0)
	{
		if(_pSong->PlayLength()<(MAX_SONG_POSITIONS-1))
		{
			m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);

			// we will do this in a loop to easily handle an error if we run out of space

			// our list can be in any order so we must be careful
			int pastedcount = 0;
			for (int i=0; i < seqcopybufferlength; i++)
			{
				if(_pSong->PlayLength() < (MAX_SONG_POSITIONS - 1))
				{
					_pSong->PlayLength(_pSong->PlayLength() + 1);

					m_view.editPosition++;
					pastedcount++;
					for(int c=(_pSong->PlayLength()-1);c>=m_view.editPosition;c--)
					{
						_pSong->PlayOrder(c,_pSong->PlayOrder(c - 1));
					}
					_pSong->PlayOrder(c + 1,seqcopybuffer[i]);
				}
			}

			if (pastedcount>0)
			{
				UpdatePlayOrder(true);
				for (i=m_view.editPosition+1-pastedcount; i<m_view.editPosition;i++)
				{
					_pSong->PlayOrderSel(i,true);
				}
				UpdateSequencer(m_view.editPosition);
				m_view.Repaint(DMPattern);
				m_view.SetFocus();
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqclr(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (MessageBox(RES_STR(IDS_CLEAR_CONFIRM),_T("Sequencer"),MB_YESNO) == IDYES)
	{
		m_view.AddUndoSong(m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
		// clear sequence
		for(int c = 0; c < MAX_SONG_POSITIONS; c++)
		{
			_pSong->PlayOrder(c,0);
		}
		// clear pattern data
		_pSong->DeleteAllPatterns();
		// init a pattern for #0
		_pSong->_ppattern(0);

		m_view.editPosition = 0;
		_pSong->PlayLength(1);
		UpdatePlayOrder(true);
		UpdateSequencer();
		m_view.Repaint(DMPattern);
	}
	m_view.SetFocus();
	
	return 0;
}

LRESULT CMainFrame::OnBnClickedSeqsrt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.AddUndoSong(m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
	unsigned char oldtonew[MAX_PATTERNS];
	unsigned char newtoold[MAX_PATTERNS];
	memset(oldtonew,255,MAX_PATTERNS * sizeof(unsigned char));
	memset(newtoold,255,MAX_PATTERNS * sizeof(unsigned char));

// Part one, Read patterns from sequence and assign them a new ordered number.
	unsigned char freep=0;
	for ( int i=0 ; i<_pSong->PlayLength() ; i++ )
	{
		const char cp=_pSong->PlayOrder(i);
		if ( oldtonew[cp] == 255 ) // else, we have processed it already
		{
			oldtonew[cp] = freep;
			newtoold[freep] = cp;
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
	TCHAR patn[32]; // ""
	unsigned char * pData; // ""


	int idx = 0;
	int idx2 = 0;
	for ( i=0 ; i < MAX_PATTERNS ; i++ )
	{
		if ( newtoold[i] != i ) // check if this place belongs to another pattern
		{
			pData = _pSong->pPatternData(i);
			patl = _pSong->PatternLines(i);
			memcpy(patn,_pSong->PatternName(i),sizeof(TCHAR) * 32);

			idx = i;
			while ( newtoold[idx] != i ) // Start moving patterns while it is not the stored one.
			{
				idx2 = newtoold[idx]; // get pattern that goes here and move.

				_pSong->pPatternData(idx,_pSong->pPatternData(idx2));
				_pSong->PatternLines(idx,_pSong->PatternLines(idx2));
				memcpy(_pSong->PatternName(idx),_pSong->PatternName(idx2),sizeof(TCHAR) * 32);
				
				newtoold[idx]=idx; // and indicate that this pattern has been corrected.
				idx = idx2;
			}

			// Put pattern back.
			_pSong->pPatternData(idx,pData);
			_pSong->PatternLines(idx,patl);
			memcpy(_pSong->PatternName(idx),patn,sizeof(TCHAR)*32);
			newtoold[idx] = idx; // and indicate that this pattern has been corrected.
		}
	}
// Part three. Update the sequence

	for (i=0 ; i<_pSong->PlayLength() ; i++ )
	{
		_pSong->PlayOrder(i,oldtonew[_pSong->PlayOrder(i)]);
	}

// Part four. All the needed things.

	seqcopybufferlength = 0;
	UpdateSequencer();
	m_view.Repaint(DMPattern);
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedInclen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
	if(_pSong->PlayLength()<(MAX_SONG_POSITIONS-1))
	{
		_pSong->PlayLength(_pSong->PlayLength() + 1);
		UpdatePlayOrder(false);
		UpdateSequencer();
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedDeclen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.AddUndoSequence(_pSong->PlayLength(),m_view.editcur.track,m_view.editcur.line,m_view.editcur.col,m_view.editPosition);
	if(_pSong->PlayLength()>1)
	{
		_pSong->PlayLength(_pSong->PlayLength() - 1);
		_pSong->PlayOrder(_pSong->PlayLength(),0);
		UpdatePlayOrder(false);
		UpdateSequencer();
	}
	m_view.SetFocus();	
	return 0;
}

LRESULT CMainFrame::OnBnClickedMultichannelAudition(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Global::pInputHandler->bMultiKey = !Global::pInputHandler->bMultiKey;
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedRecordNoteoff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CButton cb((HWND)m_wndSeq.GetDlgItem(IDC_RECORD_NOTEOFF));
	if ( cb.GetCheck()){
		m_pController->configuration()->_RecordNoteoff = true;
	}
	else m_pController->configuration()->_RecordNoteoff = false;
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedRecordTweaks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CButton cb((HWND)m_wndSeq.GetDlgItem(IDC_RECORD_TWEAKS));
	if ( cb.GetCheck() ){
		m_pController->configuration()->_RecordTweaks = true;
	} else {
		m_pController->configuration()->_RecordTweaks = false;
	}
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedNotestoeffects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CButton cb((HWND)m_wndSeq.GetDlgItem(IDC_NOTESTOEFFECTS));
	
	if ( cb.GetCheck() ) 
		m_pController->configuration()->_notesToEffects=true;
	else 
		m_pController->configuration()->_notesToEffects=false;
	
	m_view.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnBnClickedFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pController->configuration()->_followSong = (BOOL)::SendMessage((HWND)m_wndSeq.GetDlgItem(IDC_FOLLOW), BM_GETCHECK, 0, 0L)?true:false;
	CListBox SeqList((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));

	if (( m_pController->configuration()->_followSong ) && ( m_pController->Player()._playing ))
	{
		m_view.ChordModeOffs = 0;
		m_view.bScrollDetatch = false;

		if (SeqList.GetAnchorIndex() != m_pController->Player()._playPosition)
		{
			SeqList.SelItemRange(false,0,SeqList.GetCount() - 1);
			SeqList.SetSel(m_pController->Player()._playPosition,true);
		}
		if ( m_view.editPosition != m_pController->Player()._playPosition )
		{
			m_view.editPosition = m_pController->Player()._playPosition;
			m_view.Repaint(DMPattern);
		}
	}
	else if ( !m_pController->Player()._playing )
	{
		SeqList.SelItemRange(false,0,SeqList.GetCount() - 1);
		for (int i = 0; i < MAX_SONG_POSITIONS; i++ )
		{
			if (_pSong->PlayOrderSel(i)){
				SeqList.SetSel(i,true);
			}
		}
	}

	m_view.SetFocus();
	return 0;
}

void CMainFrame::UpdatePlayOrder(bool mode)
{
	
	CStatic ll_l((HWND)m_wndSeq.GetDlgItem(IDC_SEQ3));
	CListBox _listbox((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
	CStatic _length((HWND)m_wndSeq.GetDlgItem(IDC_LENGTH));
	
	int ll = _pSong->PlayLength();

	TCHAR buffer[16];

// Update Labels
	
	_stprintf(buffer,_T("%.2X"),ll);
	ll_l.SetWindowText(buffer);

	/*
	int songLength = 0;
	for (int i=0; i <ll; i++)
	{
		int pattern = _pSong->PlayOrder(i);
		// this should parse each line for ffxx commands if you want it to be truly accurate
		songLength += (_pSong->PatternLines(pattern) * 60/(_pSong->BeatsPerMin * _pSong->_ticksPerBeat));
	}

	_stprintf(buffer, _T("%02d:%02d"), songLength / 60, songLength % 60);
	*/

	// take ff and fe commands into account

	float songLength = 0;
	int bpm = _pSong->BeatsPerMin();
	int tpb = _pSong->TicksPerBeat();
	for (int i=0; i <ll; i++)
	{
		int pattern = _pSong->PlayOrder(i);
		// this should parse each line for ffxx commands if you want it to be truly accurate
		unsigned char* const plineOffset = _pSong->_ppattern(pattern);
		for (int l = 0; l < _pSong->PatternLines(pattern)*MULTIPLY; l+=MULTIPLY)
		{
			for (int t = 0; t < _pSong->SongTracks()*EVENT_SIZE; t+=EVENT_SIZE)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
				switch (pEntry->_cmd)
				{
				case 0xFF:
					if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
					{
						bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
					}
					break;
					
				case 0xFE:
					if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
					{
						tpb=pEntry->_parameter;
					}
					break;
				}
			}
			songLength += (60.0f/(bpm * tpb));
		}
	}
	
	_stprintf(buffer, _T("%02d:%02d"), f2i(songLength / 60), f2i(songLength) % 60);
	_length.SetWindowText(buffer);
	
	// Update sequencer line
	
	if (mode)
	{
		const int ls=m_view.editPosition;
		const int le=_pSong->PlayOrder(ls);
		_listbox.DeleteString(ls);
		_stprintf(buffer,_T("%.2X: %.2X"),ls,le);
		_listbox.InsertString(ls,buffer);
		// Update sequencer selection	
		_listbox.SelItemRange(false,0,_listbox.GetCount()-1);
		_listbox.SetSel(ls,true);
		for(int i = 0; i < MAX_SONG_POSITIONS;i++){
			_pSong->PlayOrderSel(i,false);
		}
		//memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
		_pSong->PlayOrderSel(ls,true);
	}
	else
	{
		_listbox.SelItemRange(false,0,_listbox.GetCount()-1);
		for (int i=0;i<MAX_SONG_POSITIONS;i++ )
		{
			if (_pSong->PlayOrderSel(i)) _listbox.SetSel(i,true);
		}
	}
	
}



LRESULT CMainFrame::OnDropFiles(UINT /*uMsg*/, WPARAM hDropInfo, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	TCHAR szFileName[MAX_PATH];
	TCHAR *szExtension;

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

		if (szExtension = _tcsrchr(szFileName, 46)) // point to everything past last "."
		{
			if (!_tcsicmp(szExtension, RES_STR(IDS_EXTENTION))) // compare to ".psy"
			{
				SetForegroundWindow(m_hWnd);
				OnFileLoadsongNamed(szFileName, 1);
				DragFinish((HDROP)  hDropInfo);	// handle of structure for dropped files
				return 0;
			}
			// add psb, psv?
			// load waves and crap here
		}
	}
	DragFinish((HDROP)  hDropInfo);	// handle of structure for dropped files
	SetForegroundWindow(m_hWnd);
	return 0;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (CheckUnsavedSong(_T("Psycle 終了")))
	{
		CloseAllMacGuis();
		_outputActive = false;
		m_pController->Player().Stop();
		m_pController->configuration()->_pOutputDriver->Enable(false);
		// MIDI IMPLEMENTATION
		m_pController->configuration()->_pMidiInput->Close();

		//Recent File List;
		SaveRecent();
		
	}
	// Release D3D Interface
	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	HICON _icon = GetIcon(false);
	DestroyIcon(_icon);
	bHandled = FALSE;
	return 0;
}


void CMainFrame::ProcessCmdLine(LPCSTR pszcmdline)
{
	if(*(pszcmdline) != 0)
	{
		TCHAR * tmpName =new TCHAR[257];
		CA2T _cmdline(pszcmdline + 1);
		_tcsncpy(tmpName, _cmdline, _tcslen(_cmdline) -1 );
		tmpName[strlen(pszcmdline+1) -1 ] = 0;
		OnFileLoadsongNamed(tmpName, 1);
		delete tmpName;
	}
}


//////////////////////////////////////////////////////////////////////
// Show the CPU Performance dialog

void CMainFrame::OnHelpPsycleenviromentinfo() 
{
	ShowPerformanceDlg();
}

//////////////////////////////////////////////////////////////////////
// Show the MIDI monitor dialog

void CMainFrame::OnMidiMonitorDlg() 
{
	ShowMidiMonitorDlg();
}

void CMainFrame::OnAppExit() 
{
	SetMsgHandled(FALSE);
	
}

//////////////////////////////////////////////////////////////////////
// Tool bar buttons and View Commands
void CMainFrame::OnMachineview() 
{
	if (m_view.viewMode != VMMachine)
	{
		m_view.viewMode = VMMachine;
		m_view.ShowScrollBar(SB_BOTH,FALSE);

		// set midi input mode to real-time or Step
		if(m_pController->configuration()->_midiMachineViewSeqMode)
			CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
		else
			CMidiInput::Instance()->m_midiMode = MODE_STEP;

		m_view.Repaint();
		StatusBarIdle();
	}
}


void CMainFrame::OnPatternView() 
{
	if (m_view.viewMode != VMPattern)
	{
		m_view.RecalcMetrics();

		m_view.viewMode = VMPattern;
//		ShowScrollBar(SB_BOTH,FALSE);
		
		// set midi input mode to step insert
		CMidiInput::Instance()->m_midiMode = MODE_STEP;
		
		SetActiveWindow();

		if (( m_pController->configuration()->_followSong ) && ( m_view.editPosition  != m_pController->Player()._playPosition )
			&& (m_pController->Player()._playing) )
		{
			m_view.editPosition = m_pController->Player()._playPosition;
		}
		m_view.SetFocus();
		m_view.Repaint();
		StatusBarIdle();
	}
	
	m_view.SetFocus();
	
}

void CMainFrame::UpdateLayout(BOOL bResizeBars)
{
#ifdef _DEBUG_PATVIEW
	ATLTRACE(_T("UpdateLayout\n"));
#endif
	RECT rectseq = {0,0,0,0};

	RECT rect;
	GetClientRect(&rect);

	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	// seqbarの調節
	if(m_wndSeq.m_hWnd != NULL && m_wndSeq.IsWindowVisible()){
		RECT rc;
		this->GetClientRect(&rc);	
		RECT rctoolbar;
		::GetClientRect(this->m_hWndToolBar,&rctoolbar);
		int height =  (int)::SendMessage(this->m_hWndToolBar,RB_GETBARHEIGHT,0,0);
		m_wndSeq.GetClientRect(&rectseq);
		RECT rcstatus;
		::GetClientRect(this->m_hWndStatusBar,&rcstatus);
		rectseq.top = rc.top  + (height);
		rectseq.bottom = rc.bottom + (height) - (rcstatus.bottom - rcstatus.top);
//		RECT rcview;
//		m_view.GetClientRect(&rcview);
		//::SetWindowPos(m_wndSeq,NULL,rectseq.top,rectseq.bottom,rectseq.right - rectseq.left,rectseq.bottom - rectseq.top,SWP_NOACTIVATE | SWP_NOZORDER);
		m_wndSeq.MoveWindow(0,rectseq.top,rectseq.right - rectseq.left,rectseq.bottom - rectseq.top  ,TRUE);
//		m_view.MoveWindow(rectseq.right - rectseq.left,rctoolbar.bottom - rctoolbar.top , rc.right - rc.left - (rectseq.right - rectseq.left),rcview.bottom - rcview.top,FALSE);

	}


	// resize client window
/*	if(m_hWndClient != NULL){
		::SetWindowPos(m_hWndClient, NULL, rect.left + (rectseq.right - rectseq.left), rect.top,
			rect.right - rect.left - (rectseq.right - rectseq.left), rect.bottom - rect.top,
			 SWP_NOACTIVATE | SWP_NOZORDER);
	}*/
//	CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits>::UpdateLayout(bResizeBars);
	if(m_hWndClient != NULL){
		::SetWindowPos(m_hWndClient, NULL, rect.left + (rectseq.right - rectseq.left), rect.top,
			rect.right - rect.left - (rectseq.right - rectseq.left), rect.bottom - rect.top,
			 SWP_NOACTIVATE | SWP_NOZORDER);
	}
//	CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits>::UpdateLayout(bResizeBars);

}

void CMainFrame::LoadRecent()
{
	//This one should be into configuration class. It isn't, coz I'm not much
	//into Psycle internal configuration loading routines.
	//If YOU are, go and put it where it sould be put.
	//
	//I know there's a class "Registry" in psycle, but... I don't like using it.
	//I think it's a little bit nonsense to use class that does not nuch more
	//than API itself. The only one for plus is variable encapsulation.
	//
	//Fideloop.
	//
	HKEY RegKey;
	HMENU hRootMenuBar, hFileMenu;
	MENUITEMINFO hNewItemInfo;
	UINT ids[] = {ID_FILE_RECENT_01,
				ID_FILE_RECENT_02,
				ID_FILE_RECENT_03,
				ID_FILE_RECENT_04};

	int iCount =0;
	TCHAR nameBuff[256];
	TCHAR cntBuff[3];
	DWORD cntSize = sizeof(cntBuff);
	DWORD nameSize = sizeof(nameBuff) * sizeof(TCHAR);
	DWORD nValues = 0;

	hRootMenuBar = m_CmdBar.GetMenu();
	hFileMenu = GetSubMenu(hRootMenuBar, 0);
	hRecentMenu = GetSubMenu(hFileMenu, 9);

	TCHAR key[72] = CONFIG_ROOT_KEY;
	_tcscat(key,_T("\\RecentFiles"));
	if (RegOpenKeyEx(HKEY_CURRENT_USER , key, 0, KEY_READ, &RegKey) == ERROR_SUCCESS)
	{
		RegQueryInfoKey(RegKey, 0, 0, 0, 0, 0, 0, &nValues, 0, 0, 0, 0);
		if (nValues)
		{
			DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
			while (RegEnumValue(RegKey,
							iCount,
							cntBuff,
							&cntSize,
							NULL,
							NULL,
							(unsigned char*)nameBuff,
							&nameSize) == ERROR_SUCCESS)
			{
						
				hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
				hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
				hNewItemInfo.fType		= MFT_STRING;
				hNewItemInfo.wID		= ids[iCount];
				hNewItemInfo.cch		= _tcslen(nameBuff);
				hNewItemInfo.dwTypeData = nameBuff;

				InsertMenuItem(hRecentMenu, iCount, TRUE, &hNewItemInfo);
				cntSize = sizeof(cntBuff);
				nameSize = sizeof(nameBuff);
				iCount++;
			}
			RegCloseKey(RegKey);
		}
	}
}

void CMainFrame::SaveRecent()
{
	HKEY RegKey;
	HMENU hRootMenuBar, hFileMenu,hRecentMenu;
	DWORD Effect;
	
	int iCount;
	TCHAR nameBuff[256];
	TCHAR cntBuff[3];
	UINT nameSize;
	
	hRootMenuBar = m_CmdBar.GetMenu();
	hFileMenu = GetSubMenu(hRootMenuBar, 0);
	hRecentMenu = GetSubMenu(hFileMenu, 9);
	
	if (RegOpenKeyEx(HKEY_CURRENT_USER, CONFIG_ROOT_KEY, 0, KEY_WRITE, &RegKey) == ERROR_SUCCESS)
	{
		RegDeleteKey(RegKey, _T("RecentFiles"));
	}
	RegCloseKey(RegKey);
	
	TCHAR key[72] = CONFIG_ROOT_KEY;
	_tcscat(key,_T("\\RecentFiles"));	
	if (RegCreateKeyEx(HKEY_CURRENT_USER,
						key,
						0,
						0,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&RegKey,
						&Effect) == ERROR_SUCCESS)
	{
		for (iCount = 0; iCount<GetMenuItemCount(hRecentMenu);iCount++)
		{
			nameSize = GetMenuString(hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
			GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
			if (_tcscmp(nameBuff, _T("No recent files")))
			{
				_itot(iCount, cntBuff, 10);
				RegSetValueEx(RegKey, cntBuff, 0, REG_SZ, (const unsigned char*)nameBuff, nameSize * sizeof(TCHAR));
			}

		}
		RegCloseKey(RegKey);
	}
}

void CMainFrame::OnFileLoadsong()
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0]=_T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	SF::CResourceString _filter(IDS_MSG0022);
	boost::scoped_array<TCHAR> _pfilter(new TCHAR[ _filter.Length() + 2 ]);
	memcpy(_pfilter.get(),_filter,_filter.Length() * sizeof(TCHAR));
	
	_pfilter[_filter.Length() - 1] = 0;
	_pfilter[_filter.Length()] = 0;

	ofn.lpstrFilter = _pfilter.get();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = m_pController->configuration()->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
	}
	
	StatusBarIdle();
}

//////////////////////////////////////////////////////////////////////
// "Save Song" Function

BOOL CMainFrame::OnFileSave() 
{
//	MessageBox("Saving Disabled");
//	return false;
	BOOL bResult = TRUE;
	if ( m_pController->Song()->IsSaved() )
	{
		if (MessageBox(SF::CResourceString(IDS_MSG0016),SF::CResourceString(IDS_MSG0017),MB_YESNO) == IDYES)
		{
			string filepath = m_pController->configuration()->GetSongDir();
			filepath.append(_T("\\"));
			filepath.append(m_pController->Song()->FileName());
			
			OldPsyFile file;
			
			if (!file.Create(filepath.data(), true))
			{
				MessageBox(
					SF::CResourceString(IDS_ERR_MSG0062),
					SF::CResourceString(IDS_ERR_MSG0043),
					MB_OK);
				return FALSE;
			}
			std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
			try {
				_saver->Save(file,*_pSong);
			} catch (std::exception e) {
				MessageBox(CA2T(e.what()), SF::CResourceString(IDS_ERR_MSG0043) /* Error*/, MB_OK);
				bResult = FALSE;
				return FALSE;
			}
	
			_pSong->IsSaved(true);
			
			UndoSaved = m_UndoController.UndoCount();
			UndoMacSaved = UndoMacCounter;

			SetTitleBarText();
//			file.Close();  <- save handles this 
		}
		else 
		{
			return FALSE;
		}
	}
	else 
	{
		return OnFileSaveAs();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////
// "Save Song As" Function
void CMainFrame::OnFileSaveAs() 
{
	m_pController->FileSaveAs();
}

const bool CMainFrame::SelectFile(TCHAR* fileName);
{
	OPENFILENAME ofn;       // common dialog box structure

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent().m_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	SF::CResourceString _filterstr(IDS_MSG0021);
	boost::scoped_array<TCHAR> _pfilter(new TCHAR[_filterstr.Length() + 2 ]);
	memcpy(_pfilter.get(),_filterstr,_filterstr.Length() * sizeof(TCHAR));
	_pfilter[_filterstr.Length() - 1] = 0;
	_pfilter[_filterstr.Length()] = 0;
	ofn.lpstrFilter = _pfilter.get();
	CString _filter(MAKEINTRESOURCE(IDS_MSG0021));
	_filter.Append(_T("\0\0\0\0"));
	ofn.lpstrFilter = _filter;

	//_T("Songs (*.psf)\0*.psf\0Psycle Pattern (*.psb)\0*.psb\0All (*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = m_pController->configuration().SongDir().data();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	bool bResult = true;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		string str = ofn.lpstrFile;
		string str2 = str.substr(str.length() - 4,4);// Right(4);

		_tcslwr(const_cast<TCHAR *>(str2.c_str()));

		if ( ofn.nFilterIndex == 2) 
		{

			if (str2.compare(_T(".psb")) != 0 )
			{
				str += _T(".psb");
			}
			
		} else {
			if ( str2.compare(_T(".psf")) != 0 ){
				str += _T(".psf");
			}
		}
		_tcscpy(fileName,str.data());
		return true;
	}

	return false;
}

void CMainFrame::OnFileNew() 
{
	m_pController->NewFile();
	StatusBarIdle();
}

/** */
void CMainFrame::NewSong()
{
	m_pMainFrame->SetTitleBarText();
	m_view.editPosition = 0;

	PsybarsUpdate(); // Updates all values of the bars
	WaveEditorBackUpdate();
	m_wndInst.WaveUpdate();
	RedrawGearRackList();
	UpdateSequencer();
	UpdatePlayOrder(false); // should be done always after updatesequencer
//		UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
	m_view.RecalculateColourGrid();
	m_view.Repaint();

}

void CMainFrame::OnFileSaveaudio() 
{
	OnBarstop();
	KillTimer(31);
	KillTimer(159);
	OnTimer(159); // Autosave

	CSaveWavDlg dlg;
	dlg.DoModal();

	InitTimer();
}

const bool  CMainFrame::CheckUnsavedSong(const TCHAR * szTitle)
{
	// that method does not take machine changes into account
	//BOOL bChecked = TRUE;
	//if (UndoSaved != m_UndoController.UndoCount())
	//{
	//		bChecked = FALSE;
	//}
	//else if (UndoMacSaved != UndoMacCounter)
	//{
	//	bChecked = FALSE;
	//}
	//else
	//{
	//	if (UndoSaved != 0)
	//	{
	//		bChecked = FALSE;
	//	}
	//}
	
	//if (!bChecked)
	//{
	if (m_pController->configuration()->IsFileSaveRemind())
	{
		//std::stringstream _message;
		string filepath = m_pController->configuration()->SongDir();
		
		filepath += _T("\\");
		filepath += m_pController->Song()->FileName();
		
		OldPsyFile file;
		
		//_message << SF::tformat(_T("\'%1%\' への変更を保存しますか?")) % m_pController->Song()->FileName();

		//_stprintf(szText,_T("\'%s\' への変更を保存しますか?"),m_pController->Song()->FileName().c_str());
		
		int result = MessageBox(
			(SF::tformat(SF::CResourceString(IDS_MSG0018)) % m_pController->Song()->FileName()).str().data(),szTitle,MB_YESNOCANCEL | MB_ICONEXCLAMATION);
		
		switch (result)
		{
		case IDYES:
			{
			if (!file.Create(filepath.c_str(), true))
			{
				MessageBox(
					(SF::tformat(SF::CResourceString(IDS_ERR_MSG0051)) % filepath.c_str()).str().c_str(),
					szTitle,MB_ICONEXCLAMATION);
				return FALSE;
			}

				std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
				_saver->Save(file,*_pSong);
			}
			return TRUE;
			break;
		case IDNO:
			return TRUE;
			break;
		case IDCANCEL:
			return FALSE;
			break;
		}
	}
	//}
	return TRUE;
}

void CMainFrame::OnFileRevert()
{
	if (MessageBox(SF::CResourceString(IDS_MSG0019),SF::CResourceString(IDS_MSG0020),MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
	{
		if (m_pController->Song()->IsSaved())
		{
			FileLoadsongNamed(
				(SF::tformat(_T("%s\\%s")) % m_pController->configuration()->GetSongDir() % m_pController->Song()->FileName()).str().data()
			);
		}
	}
	StatusBarIdle();
}


void CMainFrame::OnBarplay() 
{
	if (m_pController->configuration()->_followSong)
	{
		m_view.bScrollDetatch = false;
	}
	m_view.prevEditPosition = m_view.editPosition;
	m_pController->Player().Start(m_view.editPosition,0);
	StatusBarIdle();
}

void CMainFrame::OnBarplayFromStart() 
{
	if (m_pController->configuration()->_followSong)
	{
		m_view.bScrollDetatch = false;
	}
	m_view.prevEditPosition = m_view.editPosition;
	m_pController->Player().Start(0,0);
	StatusBarIdle();
}

void CMainFrame::OnBarrec() 
{
	if (m_pController->configuration()->_followSong && m_view.bEditMode)
	{
		m_view.bEditMode = FALSE;
	}
	else
	{
		m_pController->configuration()->_followSong = TRUE;
		m_view.bEditMode = TRUE;
		CButton cb(m_wndSeq.GetDlgItem(IDC_FOLLOW));
		cb.SetCheck(1);
	}
	StatusBarIdle();
}

void CMainFrame::OnButtonplayseqblock() 
{
	if (m_pController->configuration()->_followSong)
	{
		m_view.bScrollDetatch = false;
	}

	m_view.prevEditPosition = m_view.editPosition;
	int i = 0;
	while ( m_pController->Song()->PlayOrderSel(i) == false ) i++;
	
	if(!m_pController->Player()._playing)
		m_pController->Player().Start(i,0);

	m_pController->Player()._playBlock = !m_pController->Player()._playBlock;

	StatusBarIdle();

	if ( m_view.viewMode == VMPattern ) m_view.Repaint(DMPattern);
}

void CMainFrame::OnBarstop()
{
	const bool pl = m_pController->Player()._playing;
	const bool blk = m_pController->Player()._playBlock;
	m_pController->Player().Stop();
	m_pController->AddBpm(0);
	m_pController->AddTpb(0);

	if (pl)
	{
		if ( m_pController->configuration()->_followSong && blk)
		{
			m_view.editPosition = m_view.prevEditPosition;
			UpdatePlayOrder(false); // <- This restores the selected block
			m_view.Repaint(DMPattern);
		}
		else
		{
			for(int i = 0; i < MAX_SONG_POSITIONS;i++){
				m_pController->Song()->PlayOrderSel(i,false);
			}
			
			m_pController->Song()->PlayOrderSel(m_view.editPosition,true);
			m_view.Repaint(DMCursor); 
		}
	}
}

void CMainFrame::OnRecordWav() 
{
	if (!m_pController->Player()._recording)
	{
		CFileDialog dlg(false,_T("wav"),NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,RES_STR(IDS_FILTER_WAV));
		if ( dlg.DoModal() == IDOK ) 
		{
			m_pController->Player().StartRecording(dlg.m_szFileName);
		}
		if ( m_pController->configuration()->autoStopMachines ) 
		{
			OnAutostop();
		}
	}
	else
	{
		m_pController->Player().StopRecording();
	}
}

void CMainFrame::OnAutostop() 
{
	if ( m_pController->configuration()->autoStopMachines )
	{
		m_pController->configuration()->autoStopMachines = false;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if (m_pController->Song()->pMachine(c))
			{
				m_pController->Song()->pMachine(c)->_stopped=false;
			}
		}
	}
	else m_pController->configuration()->autoStopMachines = true;
}
/////////////////////
//

void CMainFrame::OnFileSongproperties() 
{	
	CSongpDlg dlg;
	dlg._pSong=m_pController->Song();
	dlg.DoModal();
	StatusBarIdle();
//	Repaint();
}

void CMainFrame::OnViewInstrumenteditor()
{
	ShowInstrumentEditor();
}
void CMainFrame::OnNewmachine() 
{
	m_view.NewMachine();
}
void CMainFrame::OnConfigurationSettings() 
{

	SF::CResourceString _title(IDS_MSG0024);

	CConfigDlg dlg(ATL::_U_STRINGorID((TCHAR*)_title));

	_outputActive = false;
	
	dlg.Init(m_pController->configuration());

	if (dlg.DoModal() == IDOK)
	{
		KillTimer(159);
		if ( m_pController->configuration()->autosaveSong )
		{
			SetTimer(159,m_pController->configuration()->autosaveSongTime*60000,NULL);
		}
		_outputActive = true;
		m_view.EnableSound();
	}
//	Repaint();
}

void CMainFrame::OnHelpSaludos() 
{
	CGreetDialog dlg;
	dlg.DoModal();
//	Repaint();
}

void CMainFrame::AppendToRecent(const TCHAR* fName)
{
	int iCount;
	TCHAR* nameBuff;
	UINT nameSize;

	HMENU hFileMenu, hRootMenuBar;
		
	UINT ids[] = {ID_FILE_RECENT_01,
				ID_FILE_RECENT_02,
				ID_FILE_RECENT_03,
				ID_FILE_RECENT_04};

	MENUITEMINFO hNewItemInfo, hTempItemInfo;

	hRootMenuBar = m_CmdBar.GetMenu();
//	::GetMenu(GetParent());
//	pRootMenuBar = this->GetParent()->GetMenu();
//	hRootMenuBar = HMENU (*pRootMenuBar);
	hFileMenu = GetSubMenu(hRootMenuBar, 0);
	hRecentMenu = GetSubMenu(hFileMenu, 9);

	//Remove initial empty element, if present.
	if (GetMenuItemID(hRecentMenu, 0) == ID_FILE_RECENT_NONE)
	{
		DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
	}

	//Check for duplicates and eventually remove.
	for (iCount = 0; iCount<GetMenuItemCount(hRecentMenu);iCount++)
	{
		nameSize = GetMenuString(hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
		nameBuff = new TCHAR[nameSize];
		GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
		if ( !_tcscmp(nameBuff, fName) )
		{
			DeleteMenu(hRecentMenu, iCount, MF_BYPOSITION);
		}
		delete nameBuff;
	}
	
	//Ensure menu size doesn't exceed 4 positions.
	if (GetMenuItemCount(hRecentMenu) == 4)
	{
		DeleteMenu(hRecentMenu, 4-1, MF_BYPOSITION);
	}

	hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
	hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
	hNewItemInfo.fType		= MFT_STRING;
	hNewItemInfo.wID		= ids[0];
	hNewItemInfo.cch		= _tcslen(fName);
	hNewItemInfo.dwTypeData = const_cast<TCHAR *>(fName);
	
	
	BOOL _result = InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);
	
	ATLASSERT(_result == TRUE);

	//Update identifiers.
	for (iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
	{
		hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
		hTempItemInfo.fMask		= MIIM_ID;
		hTempItemInfo.wID		= ids[iCount];

		SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
	}

}

void CMainFrame::OnFileRecent_01()
{
	CallOpenRecent(0);
}
void CMainFrame::OnFileRecent_02()
{
	CallOpenRecent(1);
}
void CMainFrame::OnFileRecent_03()
{
	CallOpenRecent(2);
}
void CMainFrame::OnFileRecent_04()
{
	CallOpenRecent(3);
}

void CMainFrame::OnFileLoadsongNamed(TCHAR* fName, int fType)
{
	if(CheckUnsavedSong(SF::CResourceString(IDS_MSG0025)))
	{
		FileLoadsongNamed(fName);
	}
}

// ファイルロード前の初期化処理
void CMainFrame::InitializeBeforeLoading()
{
	//KillUndo();
	//KillRedo();
	m_UndoController.Clear();

	CloseAllMacGuis();
	m_pController->Player().Stop();
	Sleep(LOCK_LATENCY);
	_outputActive = false;
	m_pController->configuration()->_pOutputDriver->Enable(false);
	// MIDI IMPLEMENTATION
	m_pController->configuration()->_pMidiInput->Close();
	Sleep(LOCK_LATENCY);
}

void CMainFrame::ProcessAfterLoading()
{

	m_pController->Song()->SetBPM(m_pController->Song()->BeatsPerMin(),
		m_pController->Song()->TicksPerBeat(),
		m_pController->configuration()->_pOutputDriver->_samplesPerSec);

	_outputActive = true;
	
	if (!m_pController->configuration()->_pOutputDriver->Enable(true))
	{
		_outputActive = false;
	}
	else
	{
		// MIDI IMPLEMENTATION
		m_pController->configuration()->_pMidiInput->Open();
	}

	m_view.editPosition = 0;
	
	PsybarsUpdate();
	WaveEditorBackUpdate();
	m_wndInst.WaveUpdate();
	RedrawGearRackList();
	UpdateSequencer();
	UpdatePlayOrder(false);
//			UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
	m_view.RecalculateColourGrid();
	m_view.Repaint();
	SetTitleBarText();

}

void CMainFrame::FileLoadsongNamed(const TCHAR* fName)
{
	
	WTL::CFindFile file;
	if(!file.FindFile(fName)){
		MessageBox(SF::CResourceString(IDS_ERR_MSG0063),
			SF::CResourceString(IDS_ERR_MSG0064), MB_OK);
	}

	string f = fName;
	string ext = f.substr(f.find_last_of(_T(".")) + 1,f.length() - (f.find_last_of(_T(".")) + 1));
	_tcslwr(const_cast<TCHAR *>(ext.c_str()));
	SongLoaderMap::iterator it = m_SongLoaderMap.find(ext);
	if(it == m_SongLoaderMap.end())
	{
		MessageBox(
			(SF::tformat(SF::CResourceString(IDS_ERR_MSG0065)) % fName).str().c_str());
		return;
	}
	
	InitializeBeforeLoading();

	try {
		it->second->Load(string(fName),*_pSong);
	} catch(std::exception e) {
		MessageBox((SF::tformat(SF::CResourceString(IDS_ERR_MSG0066)) % CA2T(e.what())).str().c_str());
		m_pController->Song()->New();
		return;
	}

	//!Fidelooop!!//
	AppendToRecent(fName);
	
	if(ext == _T("psf")){
		ext = _T("");
	} else {
		ext = _T(".psf");
	}

	SetSongDirAndFileName(fName,ext.c_str());

	ProcessAfterLoading();

	if (m_pController->configuration()->bShowSongInfoOnLoad)
	{
		TCHAR buffer[512];
		_stprintf(buffer,_T("'%s'\n\n%s\n\n%s")
			,_pSong->Name().c_str()
			,_pSong->Author().c_str()
			,_pSong->Comment().c_str());
		
		MessageBox(buffer,SF::CResourceString(IDS_ERR_MSG0067),MB_OK);
	}

}

void CMainFrame::CallOpenRecent(int pos)
{
	TCHAR* nameBuff;
	UINT nameSize;

	nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
	nameBuff = new TCHAR[nameSize];
	GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
	
	OnFileLoadsongNamed(nameBuff, 1);
	delete nameBuff;
}

void CMainFrame::SetTitleBarText()
{
	string titlename(_T("["));
	titlename += m_pController->Song()->FileName();

/*	if (!(m_pController->Song()->_saved))
	{
		titlename+=_T(" *");
	}
	else */ 

	if (UndoSaved != m_UndoController.UndoCount())
	{
		titlename.append(_T(" *"));
	}
	else if (UndoMacSaved != UndoMacCounter)
	{
		titlename.append(_T(" *"));
	}
	else
	{
		if (UndoSaved != 0)
		{
			titlename.append(_T(" *"));
		}
	}
	// I don't know how to access to the
	titlename.append(SF::CResourceString(IDS_MSG0026));	
	SetWindowText(titlename.c_str());				// IDR_MAINFRAME String Title.
}


void CMainFrame::OnHelpKeybtxt() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPKEYB),m_pController->configuration()->appPath);
	::ShellExecute(m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CMainFrame::OnHelpReadme() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPREADME),m_pController->configuration()->appPath);
	::ShellExecute(m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CMainFrame::OnHelpTweaking() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPTWEAK),m_pController->configuration()->appPath);
	::ShellExecute(m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CMainFrame::OnHelpWhatsnew() 
{
	TCHAR path[MAX_PATH];
	_stprintf(path,RES_STR(IDS_FMT_HELPWHATSNEW),m_pController->configuration()->appPath);
	::ShellExecute(m_hWnd,_T("open"),path,NULL,_T(""),SW_SHOW);
}

void CMainFrame::OnConfigurationLoopplayback() 
{
	m_pController->Player()._loopSong = !m_pController->Player()._loopSong;
}


//////////////////////////////////////////////////////////////////////
// Timer handler

void CMainFrame::OnTimer( UINT nIDEvent,TIMERPROC pTimerProc )
{
	if (nIDEvent == 31)
	{
		
		CComCritSecLock<CComAutoCriticalSection> lock(_pSong->Door(),TRUE);

		if (m_pController->Song()->pMachine(MASTER_INDEX))
		{

			UpdateVumeters(
	//			((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_LMAX,
	//			((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_RMAX,
				((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_lMax,
				((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_rMax,
				m_pController->configuration()->vu1,
				m_pController->configuration()->vu2,
				m_pController->configuration()->vu3,
				((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_clip);

			if ( m_view.MasterMachineDialog )
			{
				if (!--((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->peaktime) 
				{
					TCHAR peak[10];
					float _current_peak = ((Master *)m_pController->Song()->pMachine(MASTER_INDEX))->currentpeak == 0.0f?0.0f:log10f(((Master *)m_pController->Song()->pMachine(MASTER_INDEX))->currentpeak);
					_stprintf(peak,_T("%.2fdB"),20.0f * _current_peak - 90.0f);
					m_view.MasterMachineDialog->Masterpeak().SetWindowText(peak);
	//				MasterMachineDialog->m_slidermaster.SetPos(256-((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_outDry);

					float val = sqrtf(((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->_outDry * 64.0f);
					m_view.MasterMachineDialog->m_slidermaster.SetPos(256 - f2i(val));
					
					((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->peaktime = 25;
					((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->currentpeak = 0.0f;
				}
			}
			((Master*)m_pController->Song()->pMachine(MASTER_INDEX))->vuupdated = true;
		}
		
		if (m_view.viewMode == VMMachine)
		{
			CClientDC dc(*this);
			m_view.DrawAllMachineVumeters(&dc);
		}

		if (m_pController->Song()->IsTweaking())
		{
			for(int c = 0; c < MAX_MACHINES; c++)
			{
				if (_pSong->pMachine(c))
				{
					if (isguiopen[c])
					{
						if ( _pSong->pMachine(c)->_type == MACH_PLUGIN )
						{
							m_pWndMac[c]->Invalidate(false);
						}
						else if ( _pSong->pMachine(c)->_type == MACH_VST ||
								_pSong->pMachine(c)->_type == MACH_VSTFX )
						{
							((CVstEditorDlg*)m_pWndMac[c])->Refresh(-1,0);
						}
	/*					else
						{
						}
	*/
					}
				}
			}
			m_pController->Song()->IsTweaking(false);
		}

		if (m_pController->Player()._playing)
		{
			if (m_pController->Player()._lineChanged)
			{
				m_pController->Player()._lineChanged = false;
				m_pController->AddBpm(0);
				SetAppSongTpb(0);

				if (m_pController->configuration()->_followSong)
				{
					CListBox _seqlist((HWND)m_wndSeq.GetDlgItem(IDC_SEQLIST));
					m_view.editcur.line = m_pController->Player()._lineCounter;
					
					if (m_view.editPosition != m_pController->Player()._playPosition)
//					if (_seqlist->GetCurSel() != m_pController->Player()._playPosition)
					{
						_seqlist.SelItemRange(false,0,_seqlist.GetCount());
						_seqlist.SetSel(m_pController->Player()._playPosition,true);
						m_view.editPosition = m_pController->Player()._playPosition;
						if ( m_view.viewMode == VMPattern ) m_view.Repaint(DMPattern);//DMPlaybackChange);  // Until this mode is coded there is no point in calling it since it just makes patterns not refresh correctly currently
					}
					else if( m_view.viewMode == VMPattern ) m_view.Repaint(DMPlayback);
				}
				else if ( m_view.viewMode == VMPattern ) m_view.Repaint(DMPlayback);
			}
		}
	}
	if (nIDEvent == 159 && !m_pController->Player()._recording)
	{
//		MessageBox("Saving Disabled");
//		return;
		CString filepath = m_pController->configuration()->GetInitialSongDir();
		filepath += _T("\\autosave.psy");
		OldPsyFile file;
		if (!file.Create(filepath.GetBuffer(1), true))
		{
			return;
		}
		std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
		_saver->Save(file,*_pSong);
//		_pSong->Save(&file);
//		file.Close(); <- save now handles this
	}
}

//////////////////////////////////////////////////////////////////////
// Timer initialization

void CMainFrame::InitTimer()
{
	KillTimer(31);
	KillTimer(159);
	
	if (!SetTimer(31,20,NULL)) // GUI update. 
	{
		MessageBox(RES_STR(IDS_COULDNT_INITIALIZE_TIMER),_T(""), MB_ICONERROR);
	}
	if ( m_pController->configuration()->autosaveSong )
	{
		if (!SetTimer(159,m_pController->configuration()->autosaveSongTime * 60000,NULL)) // Autosave Song
		{
			MessageBox(RES_STR(IDS_COULDNT_INITIALIZE_TIMER),_T(""), MB_ICONERROR);
		}
	}
}

void CMainFrame::AddMacViewUndo()
{
	// i have not written the undo code yet for machine and instruments
	// however, for now it at least tracks changes for save/new/open/close warnings
	UndoMacCounter++;
	SetTitleBarText();
}

void CMainFrame::OnEditUndo() 
{
	if(
	// TODO: Add your command handler code here
	//if (pUndoList)
	//{
	//	switch (pUndoList->type)
	//	{
	//	case UNDO_PATTERN:
	//		if(viewMode == VMPattern)// && bEditMode)
	//		{
	//			AddRedo(pUndoList->pattern,pUndoList->x,pUndoList->y,pUndoList->tracks,pUndoList->lines,editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
	//			// do undo
	//			unsigned char* pData = pUndoList->pData;

	//			for (int t=pUndoList->x;t<pUndoList->x+pUndoList->tracks;t++)
	//			{
	//				for (int l=pUndoList->y;l<pUndoList->y+pUndoList->lines;l++)
	//				{
	//					unsigned char *offset_source=_ptrackline(pUndoList->pattern,t,l);
	//					
	//					memcpy(offset_source,pData,EVENT_SIZE*sizeof(char));
	//					pData+=EVENT_SIZE*sizeof(char);
	//				}
	//			}
	//			// set up cursor
	//			editcur.track = pUndoList->edittrack;
	//			editcur.line = pUndoList->editline;
	//			editcur.col = pUndoList->editcol;
	//			if (pUndoList->seqpos == editPosition)
	//			{
	//				// display changes
	//				NewPatternDraw(pUndoList->x,pUndoList->x+pUndoList->tracks,pUndoList->y,pUndoList->y+pUndoList->lines);
	//				Repaint(DMData);
	//			}
	//			else
	//			{
	//				editPosition = pUndoList->seqpos;
	//				pMainFrame->UpdatePlayOrder(true);
	//				Repaint(DMPattern);
	//				
	//			}
	//			// delete undo from list
	//			SPatternUndo* pTemp = pUndoList->pPrev;
	//			delete (pUndoList->pData);
	//			delete (pUndoList);
	//			pUndoList = pTemp;
	//		}
	//		break;
	//	case UNDO_LENGTH:
	//		if(viewMode == VMPattern)// && bEditMode)
	//		{
	//			AddRedoLength(pUndoList->pattern,_pSong->PatternLines(pUndoList->pattern),editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
	//			// do undo
	//			_pSong->PatternLines(pUndoList->pattern,pUndoList->lines);
	//			// set up cursor
	//			editcur.track = pUndoList->edittrack;
	//			editcur.line = pUndoList->editline;
	//			editcur.col = pUndoList->editcol;
	//			if (pUndoList->seqpos != editPosition)
	//			{
	//				editPosition = pUndoList->seqpos;
	//				pMainFrame->UpdatePlayOrder(true);
	//			}
	//			// display changes
	//			Repaint(DMPattern);
	//			
	//			// delete undo from list
	//			SPatternUndo* pTemp = pUndoList->pPrev;
	//			delete (pUndoList->pData);
	//			delete (pUndoList);
	//			pUndoList = pTemp;
	//			break;
	//		}
	//	case UNDO_SEQUENCE:
	//		{
	//			AddRedoSequence(_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
	//			// do undo
	//			for(int i = 0;i <  MAX_SONG_POSITIONS;i++){
	//				_pSong->PlayOrder(i,*(pUndoList->pData + i));
	//			}
	//			//memcpy(_pSong->PlayOrder(), pUndoList->pData, MAX_SONG_POSITIONS * sizeof(char));
	//			_pSong->PlayLength(pUndoList->lines);
	//			// set up cursor
	//			editcur.track = pUndoList->edittrack;
	//			editcur.line = pUndoList->editline;
	//			editcur.col = pUndoList->editcol;
	//			editPosition = pUndoList->seqpos;
	//			pMainFrame->UpdatePlayOrder(true);
	//			pMainFrame->UpdateSequencer();
	//			// display changes
	//			Repaint(DMPattern);
	//			
	//			// delete undo from list
	//			{
	//				SPatternUndo* pTemp = pUndoList->pPrev;
	//				delete (pUndoList->pData);
	//				delete (pUndoList);
	//				pUndoList = pTemp;
	//			}
	//			break;
	//		}
	//	case UNDO_SONG:
	//		{
	//			AddRedoSong(editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
	//			// do undo
	//			unsigned char * pData = pUndoList->pData;
	//			for(int i = 0;i < MAX_SONG_POSITIONS;i++){
	//				_pSong->PlayOrder(i,*(pData + 1));
	//			}
	//			//memcpy(_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
	//			pData += MAX_SONG_POSITIONS;
	//			unsigned char count = *pData;
	//			pData += sizeof(count);
	//			for (int i = 0; i < count; i++)
	//			{
	//				unsigned char index = *pData;
	//				pData += sizeof(index);
	//				unsigned char* pWrite = _ppattern(index);

	//				memcpy(pWrite,pData,MULTIPLY2);
	//				pData+= MULTIPLY2;
	//			}
	//			_pSong->PlayLength(pUndoList->lines);
	//			// set up cursor
	//			editcur.track = pUndoList->edittrack;
	//			editcur.line = pUndoList->editline;
	//			editcur.col = pUndoList->editcol;
	//			editPosition = pUndoList->seqpos;
	//			pMainFrame->UpdatePlayOrder(true);
	//			pMainFrame->UpdateSequencer();
	//			// display changes
	//			Repaint(DMPattern);
	//			
	//			// delete undo from list
	//			{
	//				SPatternUndo* pTemp = pUndoList->pPrev;
	//				delete (pUndoList->pData);
	//				delete (pUndoList);
	//				pUndoList = pTemp;
	//			}
	//			break;
	//		}

	//	}
	//	SetTitleBarText();
	//}
}

void CMainView::OnEditRedo() 
{
	// TODO: Add your command handler code here
	if (pRedoList)
	{
		switch (pRedoList->type)
		{
		case UNDO_PATTERN:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddUndo(pRedoList->pattern,pRedoList->x,pRedoList->y,pRedoList->tracks,pRedoList->lines,editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do redo
				unsigned char* pData = pRedoList->pData;

				for (int t=pRedoList->x;t<pRedoList->x+pRedoList->tracks;t++)
				{
					for (int l=pRedoList->y;l<pRedoList->y+pRedoList->lines;l++)
					{
						unsigned char *offset_source=_ptrackline(pRedoList->pattern,t,l);

						
						memcpy(offset_source,pData,5*sizeof(char));
						pData+=5*sizeof(char);
					}
				}
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos == editPosition)
				{
					// display changes
					NewPatternDraw(pRedoList->x,pRedoList->x+pRedoList->tracks,pRedoList->y,pRedoList->y+pRedoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pRedoList->seqpos;
					pMainFrame->UpdatePlayOrder(true);
					Repaint(DMPattern);
					
				}
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
			}
			break;
		case UNDO_LENGTH:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddUndoLength(pRedoList->pattern,_pSong->PatternLines(pUndoList->pattern),editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do undo
				_pSong->PatternLines(pRedoList->pattern,pRedoList->lines);
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos != editPosition)
				{
					editPosition = pRedoList->seqpos;
					pMainFrame->UpdatePlayOrder(true);
				}
				// display changes
				Repaint(DMPattern);
				
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
				break;
			}
		case UNDO_SEQUENCE:
			{
				AddUndoSequence(_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
				// do undo
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					_pSong->PlayOrder(i,*(pRedoList->pData + i));
				}
				//memcpy(_pSong->m_PlayOrder, pRedoList->pData, MAX_SONG_POSITIONS*sizeof(char));
				_pSong->PlayLength(pRedoList->lines);
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				editPosition = pRedoList->seqpos;
				pMainFrame->UpdatePlayOrder(true);
				pMainFrame->UpdateSequencer();
				// display changes
				Repaint(DMPattern);
				
				{
					// delete redo from list
					SPatternUndo* pTemp = pRedoList->pPrev;
					delete (pRedoList->pData);
					delete (pRedoList);
					pRedoList = pTemp;
				}
				break;
			}
		case UNDO_SONG:
			{
				AddUndoSong(editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
				// do undo
				unsigned char * pData = pRedoList->pData;
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					_pSong->PlayOrder(i,*(pData + i));
				}

				//memcpy(_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
				pData += MAX_SONG_POSITIONS;
				unsigned char count = *pData;
				pData += sizeof(count);
				for (int i = 0; i < count; i++)
				{
					unsigned char index = *pData;
					pData += sizeof(index);
					unsigned char* pWrite = _ppattern(index);

					memcpy(pWrite,pData,MULTIPLY2);
					pData+= MULTIPLY2;
				}

				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				editPosition = pRedoList->seqpos;
				pMainFrame->UpdatePlayOrder(true);
				pMainFrame->UpdateSequencer();
				// display changes
				Repaint(DMPattern);
				
				{
					// delete redo from list
					SPatternUndo* pTemp = pRedoList->pPrev;
					delete (pRedoList->pData);
					delete (pRedoList);
					pRedoList = pTemp;
				}
				break;
			}
		}
		SetTitleBarText();
	}
}

#ifdef _TEST
void CMainFrame::OnTest(){
	testmain();
}
#endif



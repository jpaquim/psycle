/** @file MainFrm.cpp
 *  @brief implmentation of the CMainFrame class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 **/

#include "stdafx.h"
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

#include "TestMainFrm.h"



////////////////////////////////////////////////////////////////////////////
// CMainFrame
#define WM_SETMESSAGESTRING 0x0362

extern void testmain();

namespace view {
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

struct MainFrameHandler : event_handler<MainFrameHandler, view_frame, MainFrame> {

};


MainFrame::MainFrame(const ::model::iController& controller,const ::string & title ) 
: m_Controller(const_cast< ::model::iController&>(controller)) , extend_base(title)
{

	//wnd<menu<shared>>
	//wnd<> a = create_wnd<>(;

	create_wnd<status_bar>(this);
	create_wnd<rebar>(this);
	
	wnd<toolbar> _tool(create_wnd<toolbar>(IDR_MAINFRAME,this));
	
	sub_wnd<rebar>()->add_band
	(
		rebar_band_info()
		.style(RBBS_BREAK | RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS)
		.band_min_sizes(230, 26)
		.text(_T(""))
		.child(_tool)
	);

//	m_pController->OutputDriver()->Configuration()->Configure();
};
/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

//CMainFrame::CMainFrame(const iController *pController) : 
//	m_wndControl(IDD_PSYBAR),m_wndControl2(IDD_PSYBAR2),m_wndSeq(IDD_SEQUENCER)
//{
//	m_pController = const_cast<iController*>(pController);
//}
//
//CMainFrame::~CMainFrame()
//{
//	ATLTRACE2("~CMainFrame()");
//}
//
//
//void CMainFrame::StatusBarText(const TCHAR *txt)
//{
//	m_wndStatusBar.SetWindowText(txt);
//}
//
//
//BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
//{
//	if(CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits>::PreTranslateMessage(pMsg))
//		return TRUE;
//	return TRUE;
//}
//
//LRESULT CMainFrame::OnCreate(LPCREATESTRUCT pCreateStruct)
//{
//
//	// create command bar window
//	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
//	
//	// attach menu
//	m_CmdBar.AttachMenu(GetMenu());
//	
//	// load command bar images
//	m_CmdBar.LoadImages(IDR_MAINFRAME);
//
//	// remove old menu
//	SetMenu(NULL);
//	
//	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
//	
//	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE | RBS_VERTICALGRIPPER );
//	//CreateSimpleReBar();
//	m_wndReBar = m_hWndToolBar;
//	m_wndReBar.GetBarHeight();
//
//
//	AddSimpleReBarBand(hWndCmdBar);
//	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
//	
//	//CreateSimpleStatusBar();
//	m_hWndStatusBar = m_wndStatusBar.Create(*this);
//	m_wndStatusBar.SetPanes(::indicators,10);
//
//	
////	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS , WS_EX_CLIENTEDGE);
///*	const DWORD dwSplitStyle = WS_CHILD | WS_VISIBLE |
//                           WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
//            dwSplitExStyle = WS_EX_CLIENTEDGE;
//    m_wndVertSplit.Create( *this, rcDefault, NULL,
//                            dwSplitStyle, dwSplitExStyle );
//    m_hWndClient = m_wndVertSplit;
//*/
//		// Sequencer Bar
//	m_wndSeq.Create(m_hWnd,m_wndSeq.rcDefault,WS_CLIPSIBLINGS | WS_VISIBLE | WS_CLIPCHILDREN);
////	AddSimpleReBarBand(m_wndSeq,_T(""),TRUE);
////	m_view.Create(m_wndVertSplit, NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS ,WS_EX_CLIENTEDGE);
////	m_wndVertSplit.SetSplitterPanes(m_wndSeq,m_view);
//
//	UIAddToolBar(hWndToolBar);
//	UIAddStatusBar(m_hWndStatusBar);
//	
//	UISetCheck(ID_VIEW_TOOLBAR, 1);
//	UISetCheck(ID_VIEW_STATUS_BAR, 1);
//
//	// register object for message filtering and idle updates
//	CMessageLoop* pLoop = SF::CMessageLoopContainer::Instance().GetMessageLoop();
//
//	ATLASSERT(pLoop != NULL);
//	pLoop->AddMessageFilter(this);
//	pLoop->AddIdleHandler(this);
//
//////
//////
//////
//
//	m_wndControl.Create(m_hWnd,0);
//	m_wndControl.ShowWindow(SW_SHOW);
//	m_wndControl2.Create(m_hWnd,0);
//	m_wndControl2.ShowWindow(SW_SHOW);
//	AddSimpleReBarBand(m_wndControl,0,TRUE);
//	AddSimpleReBarBand(m_wndControl2,0,TRUE);
//
//	//m_view.ValidateParent();
//
//	//TODO  CPU info Window
//
//	// TODO MIDI monitor Dialog
//
//	// TODO Instrument editor
//
//	// TODO Wave Editor Window
//
//
//	// TODO Init Bars Content.
//
//	m_wndControl.SetWindowText(_T("Psycle ｺﾝﾄﾛｰﾙ ﾊﾞｰ"));
//
//	m_iLessLess.Create(ATL::_U_STRINGorID(IDB_LESSLESS),15,1,RGB(192,192,192));
//	m_iLess.Create(IDB_LESS,15,1,RGB(192,192,192));
//	m_iMore.Create(IDB_MORE,15,1,RGB(192,192,192));
//	m_iMoreMore.Create(IDB_MOREMORE,15,1,RGB(192,192,192));
//	m_iPlus.Create(IDB_PLUS,15,1,RGB(192,192,192));
//	m_iMinus.Create(IDB_MINUS,15,1,RGB(192,192,192));
//	m_iPlusPlus.Create(IDB_PLUSPLUS,15,1,RGB(192,192,192));
//	m_iMinusMinus.Create(IDB_MINUSMINUS,15,1,RGB(192,192,192));
//	m_iLittleLeft.Create(IDB_LLEFT,10,1,RGB(192,192,192));
//	m_iLittleRight.Create(IDB_LRIGHT,10,1,RGB(192,192,192));
//	
//
//	WTL::CButton cb;
//
//	cb.Attach((HWND)(m_wndControl.GetDlgItem(IDC_BTN_DECTEN)));
//
//	BUTTON_IMAGELIST img;
//	
//	img.himl = m_iLessLess;
//
//	img.margin.bottom = 0;
//	img.margin.top = 0;
//	img.margin.right = 0;
//	img.margin.left = 0;
//	img.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
//	cb.SetImageList(&img);
//	cb.Detach();
//
//	cb.Attach((HWND)(m_wndControl.GetDlgItem(IDC_BTN_DECONE)));
//	img.himl = m_iLess; cb.SetImageList(&img);; cb.Detach();
//
//	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_BTN_INCONE));
//	img.himl = m_iMore; cb.SetImageList(&img);; cb.Detach();
//
//	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_BTN_INCTEN));
//	img.himl = m_iMoreMore; cb.SetImageList(&img);; cb.Detach();
//
//	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_DEC_TPB));
//	img.himl = m_iLess; cb.SetImageList(&img);; cb.Detach();
//
//	cb.Attach((HWND)m_wndControl.GetDlgItem(IDC_INC_TPB));
//	img.himl = m_iMore; cb.SetImageList(&img);; cb.Detach();
//
//	m_wndControl2.SetWindowText(_T("Psycle ｺﾝﾄﾛｰﾙ ﾊﾞｰ 2"));
//
//	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_DECGEN));
//	img.himl = m_iLittleLeft; cb.SetImageList(&img);; cb.Detach();
//
//	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_INCGEN));
//	img.himl = m_iLittleRight; cb.SetImageList(&img);;cb.Detach();
//
//	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_DECWAV));
//	img.himl = m_iLittleLeft; cb.SetImageList(&img);;cb.Detach();
//
//	cb.Attach((HWND)m_wndControl2.GetDlgItem(IDC_B_INCWAV));
//	img.himl = m_iLittleRight; cb.SetImageList(&img);;cb.Detach();
//
//	CComboBox cc2;
//	cc2.Attach((HWND)m_wndControl.GetDlgItem(IDC_TRACKCOMBO));
//	
//	for(int i = 4;i <= MAX_TRACKS ; i++)
//	{
//		TCHAR s[4];
//		_stprintf(s,4,_T("%i"),i);
//		cc2.AddString(s);
//	}
//	
//	//cc2.SetCurSel(_pSong->SongTracks() - 4);
//
//	CComboBox cc;
//	cc.Attach((HWND)m_wndControl2.GetDlgItem(IDC_AUXSELECT));
//	cc.SetCurSel(2);
//
//
//	// TODO InitSequencer();
//
//	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_INCSHORT));
//	img.himl = m_iPlus; cb.SetImageList(&img);;cb.Detach();
//
//	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_INCLONG));
//	img.himl = m_iPlusPlus; cb.SetImageList(&img);;cb.Detach();
//
//	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_DECSHORT));
//	img.himl = m_iMinus; cb.SetImageList(&img);;cb.Detach();
//	
//	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_DECLONG));
//	img.himl = m_iMinusMinus; cb.SetImageList(&img);;cb.Detach();
//
//	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_DECLEN));
//	img.himl = m_iLess; cb.SetImageList(&img);;cb.Detach();
//
//	cb.Attach((HWND)m_wndSeq.GetDlgItem(IDC_INCLEN));
//	img.himl = m_iMore; cb.SetImageList(&img);;cb.Detach();
//	
//	//UpdatePlayOrder(true);
//	
//	::DragAcceptFiles(this->m_hWnd,TRUE);
//
//	//m_pController->configuration()->CreateFonts();
//
//	// Finally initializing timer
//	
//	//UpdateSequencer();
//	m_wndSeq.ShowWindow(SW_SHOW);
//
//	//AddSimpleReBarBand(m_wndSeq,0,TRUE);
//	
//#ifdef _TEST
//	// Unit Test Menu
//	WTL::CMenuHandle _menu;
//	_menu.Attach(m_CmdBar.GetMenu());
//	CMenuHandle _h = _menu.GetSubMenu(4);
//	_h.AppendMenu(MF_STRING,ID_TEST,_T("DoTest"));
//#endif
//
//	//LoadRecent();
//	
//	//m_wndSeq.SetFocus();
//	UpdateWindow();
//	OnIdle();
//	//m_hWndClient = m_view.Create(m_hWnd);
//	//InitTimer();
//	//m_view.SetFocus();
//	
//	// D3Dオブジェクトの初期化
//	// 最初の1回だけはこの形式で呼び出し、初期化
//	// 次からは D3DGetInstance()で取り出せる
//
//	return 0;
//}
//
//
//// アイドル時の処理 //
//BOOL CMainFrame::OnIdle()
//{
//	//CString str;
//	//if (m_pController->Player()._playing)
//	//{
//	//    str.Format(_T("Pos %.2X"), m_pController->Player()._playPosition); 
//	//}
//	//else
//	//{
//	//    str.Format(_T("Pos %.2X"), m_view.editPosition); 
//	//}
//	//UISetText(INDICATOR_SEQPOS,str,FALSE);
//
//	//if (m_pController->Player()._playing)
//	//{
//	//    str.Format(_T("Pat %.2X"), m_pController->Player()._playPattern); 
//	//}
//	//else
//	//{
//	//    str.Format(_T("Pat %.2X"), m_pController->Song()->PlayOrder(m_view.editPosition)); 
//	//}
//	//UISetText(INDICATOR_PATTERN,str,FALSE);
//
//	//if (m_pController->Player()._playing)
//	//{
//	//    str.Format(_T("Line %u"), m_pController->Player()._lineCounter); 
//	//}
//	//else
//	//{
//	//    str.Format(_T("Line %u"), m_view.editcur.line); 
//	//}
//	//UISetText(INDICATOR_LINE,str,FALSE);
//
//	//if (m_pController->Player()._playing)
//	//{
//	//	CString str;
//	//    str.Format( _T("%.2u:%.2u:%.2u.%.2u"), m_pController->Player()._playTimem / 60, m_pController->Player()._playTimem % 60, f2i(m_pController->Player()._playTime), f2i(m_pController->Player()._playTime*100)-(f2i(m_pController->Player()._playTime)*100)); 
//	//	UISetText(INDICATOR_TIME,str,FALSE);
//	//}
//
//	//UIEnable(INDICATOR_EDIT,m_view.bEditMode); 
//	//UIEnable(INDICATOR_FOLLOW,m_pController->configuration()->_followSong);
//	//UIEnable(INDICATOR_NOTEOFF,m_pController->configuration()->_RecordNoteoff);
//	//UIEnable(INDICATOR_TWEAKS,m_pController->configuration()->_RecordTweaks);
//	//str.Format(_T("Oct %u"), _pSong->CurrentOctave());
//	//UISetText(INDICATOR_OCTAVE,str,FALSE);
//	//
//	//if (m_pController->configuration()->autoStopMachines == true ) 
//	//	UISetCheck(ID_AUTOSTOP,1);
//	//else 
//	//	UISetCheck(ID_AUTOSTOP,0);
//
//	//UISetCheck(ID_VIEW_SONGBAR,m_wndControl.IsWindowVisible());
//	//UISetCheck(ID_VIEW_MACHINEBAR,m_wndControl2.IsWindowVisible());
//	//UISetCheck(ID_VIEW_SEQUENCERBAR,m_wndSeq.IsWindowVisible());
//
//	//m_view.UpdateUI();
//
//	//UIUpdateMenuBar();
//	//UIUpdateToolBar();
//	//UIUpdateStatusBar();
//
//
//	
//	return FALSE;
//}
//
//
//LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//	PostMessage(WM_CLOSE);
//	return 0;
//}
//
//
//
//#ifdef _TEST
//void CMainFrame::OnTest(){
//	testmain();
//}
//#endif
}
#if !defined(MAINFRM_H) 
#define MAINFRM_H
/** @file  MainFrm.h 
 *  @brief interface of the CMainFrame class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */

#include "iMainFrame.h"
#include "iController.h"

class Song;

enum
{
	AUX_MIDI = 0,
	AUX_PARAMS,
	AUX_WAVES
};
namespace view {
	namespace configuration {
		class MainFrame : public ::model::iCompornentConfiguration {
		public:

		private:

		};
	};
}

//typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> CMainFrameWinTraits;

// forward declarations

//TODO:メインフレームをドッキング可能なフレームワークに変更する
//TODO:Controllerクラスで実装すべき処理はすべてControllerクラスに持っていく
//TODO:各Modelクラスで行うべき処理もModelクラスで行うように変更する
using namespace win32::gui;
namespace view {
	namespace detail {
		struct MainFrameBase : public ::view::iMainFrame,public view_frame
		{
			MainFrameBase(const string& title) : view_frame(title){};
		};
	}

	class MainFrame : public wnd_extend<detail::MainFrameBase,MainFrame> 
{
    // note: you can add functions + data here, and keep the application logic right here.
    // Then, by extending a frame window and this, you can always use something like:
    //
    // top_wnd<app_wnd>() to get acess to this instance!
public:
	static const int ID_TEST = 43000;///< 
	static const int MAX_TABS = 10;
	typedef wnd_extend<view_frame,view::MainFrame > AppBase;
	
	MainFrame(const ::model::iController& controller,const ::string & title = ::string());

	model::iCompornentConfiguration *const Configuration(void){return NULL;};
	
	void ChangeBpm(const int bpm ){
		child(IDC_BPMLABEL)->text((SF::tformat(_T("%d")) % bpm).str());
	};
	void ChangeTpb(const int tpb){ 
		child(IDC_TPBLABEL)->text((SF::tformat(_T("%d")) % tpb).str());
	};

	void SongSaved(){};///< SongがSaveされたとあとに呼ばれる
	void SongLoaded(){};///< Songがロードされたあとに呼ばれる
	void InitializeBeforeLoading(){};///< Songがロードされる前に呼ばれる
	// TODO: このメソッドは必要？
	const bool CheckUnsavedSong(){return false;};
	const bool CheckUnsavedSong(const TCHAR *){return true;};
	void NewSong(void){};
	const UINT MessageDialog(const string &,const string &,const UINT){return 0;};
	const bool SelectFile(TCHAR *){return true;};

	const HWND GetHwnd(){return m_hwnd;};

private:
	
	//wnd<rebar> m_Rebar;
	//wnd<status_bar> m_StatusBar;
	//wnd<toolbar> m_ToolBar;
	::model::iController& m_Controller;
};

//class CMainFrame : public CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits>, public CUpdateUI<CMainFrame>,
//	public CMessageFilter, public CIdleHandler,public iMainFrame
//{
//	typedef CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits> thisClass;
//	static const int ID_TEST = 43000;
//	static const int MAX_TABS = 10;
//public:
//	explicit CMainFrame(const iController * pController);///< Constructor 
//
//	DECLARE_FRAME_WND_CLASS_EX(NULL, IDR_MAINFRAME,NULL,NULL);
//	configuration::iCompornentConfiguration *const configuration(void){return NULL;};
//	// TODO:ビュークラスをプロパティベースのものに変更
//
//	//CTabbedChildWindow< CDotNetTabCtrl<CTabViewTabItem> > m_tabbedChildWindow;
//	//boost::array<CWindow*,MAX_TABS> m_pViews;
//	/** CommandBar Control **/
//	CCommandBarCtrlXP m_CmdBar;
//	/** ToolBar Control */
//	WTL::CToolBarCtrl m_ToolBar;
//	/** Status Bar */
//	WTL::CMultiPaneStatusBarCtrl  m_wndStatusBar;
//	//	CToolBar    m_wndToolBar;
//	SF::CSimpleDialog	m_wndControl;
//	SF::CSimpleDialog	m_wndControl2;
//	SF::CSimpleDialog	m_wndSeq;
//
//	
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	virtual BOOL OnIdle();
//
//	void ProcessCmdLine(LPCSTR pszcmdline);
//
//	void ChangeBpm(const int bpm )
//	{
//		CStatic tmplab = (HWND)m_wndControl.GetDlgItem(IDC_BPMLABEL);
//		tmplab.SetWindowText((SF::tformat(_T("%d")) % bpm).str().data());
//	};
//
//	void ChangeTpb(const int tpb)
//	{
//		((CStatic)m_wndControl.GetDlgItem(IDC_TPBLABEL)).SetWindowText(
//			(SF::tformat(_T("%d")) % tpb).str().data());
//	};
//
//	void SongSaved()
//	{
//
//	};
//
//	void SongLoaded()
//	{
//
//	};
//
//	void InitializeBeforeLoading()
//	{
//
//	};
//
//	const bool CheckUnsavedSong(void){return false;};
//	void NewSong(void){};
//	const bool iMainFrame::CheckUnsavedSong(const TCHAR *){return true;};
//	const UINT MessageDialog(const string &,const string &,const UINT){return 0;};
//	const bool SelectFile(TCHAR *){return true;};
//
//	virtual ~CMainFrame();
//	
//	WTL::CImageList m_iLessLess;
//	WTL::CImageList m_iLess;
//	WTL::CImageList m_iMore;
//	WTL::CImageList m_iMoreMore;
//	WTL::CImageList m_iPlus;
//	WTL::CImageList m_iMinus;
//	WTL::CImageList m_iPlusPlus;
//	WTL::CImageList m_iMinusMinus;
//	WTL::CImageList m_iLittleLeft;
//	WTL::CImageList m_iLittleRight;
//	
//	LRESULT OnCreate(LPCREATESTRUCT pCreateStruct);
//
//	// Generated message map functions
//
//	enum {
//		INDICATOR_SEQPOS = 1,
//		INDICATOR_PATTERN,
//		INDICATOR_LINE,
//		INDICATOR_TIME,
//		INDICATOR_EDIT,
//		INDICATOR_FOLLOW,
//		INDICATOR_NOTEOFF,
//		INDICATOR_TWEAKS,
//		INDICATOR_OCTAVE
//	};
//
//
//#ifdef _TEST
//	void OnTest();
//#endif
//
//	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//
//	BEGIN_UPDATE_UI_MAP(CMainFrame)
//		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_VIEW_MACHINEBAR, UPDUI_MENUPOPUP | UPDUI_CHECKED )
//		UPDATE_ELEMENT(ID_VIEW_SEQUENCERBAR, UPDUI_MENUPOPUP | UPDUI_CHECKED )
//		UPDATE_ELEMENT(ID_MACHINEVIEW,UPDUI_MENUPOPUP | UPDUI_CHECKED | UPDUI_TOOLBAR)
//		UPDATE_ELEMENT(ID_PATTERNVIEW,UPDUI_MENUPOPUP | UPDUI_CHECKED | UPDUI_TOOLBAR)
//
//		UPDATE_ELEMENT(ID_VIEW_SONGBAR, UPDUI_MENUPOPUP | UPDUI_CHECKED)
//		UPDATE_ELEMENT(ID_AUTOSTOP,UPDUI_MENUPOPUP | UPDUI_TOOLBAR | UPDUI_CHECKED)
//
//	
//		UPDATE_ELEMENT(INDICATOR_SEQPOS, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_PATTERN, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_LINE, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_TIME, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_EDIT, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_FOLLOW, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_NOTEOFF, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_TWEAKS, UPDUI_STATUSBAR)
//		UPDATE_ELEMENT(INDICATOR_OCTAVE, UPDUI_STATUSBAR)
//		
//		UPDATE_ELEMENT(ID_RECORDB, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_BARPLAY, UPDUI_TOOLBAR | UPDUI_CHECKED)
//		UPDATE_ELEMENT(ID_BARPLAYFROMSTART, UPDUI_TOOLBAR | UPDUI_CHECKED)
//		UPDATE_ELEMENT(ID_BARREC, UPDUI_TOOLBAR | UPDUI_CHECKED)
//		UPDATE_ELEMENT(ID_BUTTONPLAYSEQBLOCK, UPDUI_TOOLBAR | UPDUI_CHECKED)
//
//		UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_EDIT_MIXPASTE, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_EDIT_DELETE, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_CONFIGURATION_LOOPPLAYBACK, UPDUI_MENUPOPUP)
//
//		UPDATE_ELEMENT(ID_POP_COPY, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_MIXPASTE, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_DELETE, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_INTERPOLATE, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_CHANGEGENERATOR, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_CHANGEINSTRUMENT, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_TRANSPOSE1, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_TRANSPOSE12, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_TRANSPOSE_1, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_TRANSPOSE_12, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_BLOCK_SWINGFILL, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_CUT, UPDUI_MENUPOPUP)
//		UPDATE_ELEMENT(ID_POP_PASTE, UPDUI_MENUPOPUP)
//
//	END_UPDATE_UI_MAP()
//
//	BEGIN_MSG_MAP_EX(CMainFrame)
//		MSG_WM_CREATE(OnCreate)
//		MSG_WM_CLOSE(OnClose)
////		MSG_WM_DESTROY(OnDestroy)
//		COMMAND_ID_HANDLER_EX2(ID_APP_EXIT, OnFileExit)
//		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
//		CHAIN_MSG_MAP(thisClass)
//		REFLECT_NOTIFICATIONS()
//	END_MSG_MAP()
//
//	void OnClose()
//	{
//
//	};
//
//
//	void OnFileExit()
//	{
//		PostMessage(WM_CLOSE);
//	};
//
//private:
//
//	void StatusBarText(const TCHAR *txt);
//
//	WTL::CReBarCtrl     m_wndReBar;
//	WTL::CSplitterWindow  m_wndVertSplit;
//
//	HMENU hRecentMenu;
//	iController *m_pController;
//
//};

}
#endif
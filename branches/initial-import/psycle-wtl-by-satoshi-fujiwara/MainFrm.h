#if !defined(MAINFRM_H) 
#define MAINFRM_H
/** @file  MainFrm.h 
 *  @brief interface of the CMainFrame class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.7 $
 */
#pragma once

#include "InstrumentEditor.h"
#include "InfoDlg.h"
#include "WaveEdFrame.h"
#include "MidiMonitorDlg.h"
#include "GearRackDlg.h"
#include "iMainFrame.h"
#include "iController.h"

class Song;

enum
{
	AUX_MIDI = 0,
	AUX_PARAMS,
	AUX_WAVES
};

namespace configuration {
	class MainFrame : iCompornentConfiguration {
	public:

	private:

	};
};

typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>		CMainFrameWinTraits;

// forward declarations

//TODO:メインフレームをドッキング可能なフレームワークに変更する
//TODO:Controllerクラスで実装すべき処理はすべてControllerクラスに持っていく
//TODO:各Modelクラスで行うべき処理もModelクラスで行うように変更する
class CMainFrame : public CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits>, public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler,public iMainFrame
{
	typedef CFrameWindowImpl<CMainFrame,CWindow,CMainFrameWinTraits> thisClass;
	static const int ID_TEST = 43000;
	static const int MAX_TABS = 10;
public:
	explicit CMainFrame(const iController * m_pController);///< Constructor 

	DECLARE_FRAME_WND_CLASS_EX(NULL, IDR_MAINFRAME,NULL,NULL);

	// TODO:ビュークラスをプロパティベースのものに変更

	CTabbedChildWindow< CDotNetTabCtrl<CTabViewTabItem> > m_tabbedChildWindow;
	boost::array<CWindow*,MAX_TABS> m_pViews;
	/** CommandBar Control **/
	CCommandBarCtrlXP m_CmdBar;
	/** ToolBar Control */
	WTL::CToolBarCtrl m_ToolBar;
	/** Status Bar */
	WTL::CMultiPaneStatusBarCtrl  m_wndStatusBar;
	//	CToolBar    m_wndToolBar;
	SF::CSimpleDialog	m_wndControl;
	SF::CSimpleDialog	m_wndControl2;
	SF::CSimpleDialog	m_wndSeq;

	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	void ProcessCmdLine(LPCSTR pszcmdline);
//	void LoadRecentConfig();
//	void SaveRecentConfig();	//Called from CMainFrm::OnClose()!
									//That's why it's public :(
/// Attributes
	::Song* _pSong;
	int vuprevL;
	int vuprevR;

	CGearRackDlg* pGearRackDialog;

	void ChangeBpm(const int bpm )
	{
		CStatic tmplab = (HWND)m_wndControl.GetDlgItem(IDC_BPMLABEL);
		tmplab.SetWindowText((SF::tformat(_T("%d")) % bpm).str().data());
	};

	void ChangeTpb(const int tpb)
	{
		((CStatic)m_wndControl.GetDlgItem(IDC_TPBLABEL)).SetWindowText(
			(SF::tformat(_T("%d")) % tpb).str().data());
	};

	void SongSaved()
	{
		AppendToRecent(m_pController->Song()->FileName().data());
		//UndoSaved = m_UndoController.UndoCount();
		//UndoMacSaved = UndoMacCounter;
		SetTitleBarText();
	};

	void SongLoaded()
	{
		AppendToRecent(m_pController->Song()->FileName().data());
		//UndoSaved = m_UndoController.UndoCount();
		//UndoMacSaved = UndoMacCounter;
		editPosition = 0;
	
		PsybarsUpdate();
		WaveEditorBackUpdate();
		m_wndInst.WaveUpdate();
		RedrawGearRackList();
		UpdateSequencer();
		UpdatePlayOrder(false);
//			pMainFrame->UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
		RecalculateColourGrid();
		Repaint();
		SetTitleBarText();
	};

	void InitializeBeforeLoading()
	{
		CloseAllMachineGuis();
	};

	void UpdatePlayOrder(bool mode);
	void CenterWindowOnPoint(HWND hWnd, POINT point);

	friend class InputHandler;
	
	void UpdateSequencer(int bottom = -1);

	int seqcopybuffer[MAX_SONG_POSITIONS];
	int seqcopybufferlength;

	void ClosePsycle();
	void WaveEditorBackUpdate();

	void UpdateEnvInfo();
	void HidePerformanceDlg();
	void ShowPerformanceDlg();
	void HideMidiMonitorDlg();
	void ShowMidiMonitorDlg();
	void HideInstrumentEditor();
	void ShowInstrumentEditor();
	void StatusBarText(const TCHAR *txt);
	void UpdateComboIns(bool updatelist=true);
	void UpdateComboGen(bool updatelist=true);
	void PsybarsUpdate();
	void UpdateVumeters(float l, float r, COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip);

	
	BOOL StatusBarIdleText();
	void StatusBarIdle();

	void RedrawGearRackList();

	virtual ~CMainFrame();
	
	int GetNumFromCombo(CComboBox* cb);

	void ChangeIns(int i);
	void ChangeGen(int i);

	TCHAR	szStatusIdle[192];
	//void OnLoadwave();
	
	void EditQuantizeChange(int diff);
	void ShiftOctave(int x);
	
	CInstrumentEditor	m_wndInst;
	CInfoDlg	m_wndInfo;
	CMidiMonitorDlg	m_midiMonitorDlg;	// MIDI_21st
	CWaveEdFrame	*m_pWndWed;

	WTL::CImageList m_iLessLess;
	WTL::CImageList m_iLess;
	WTL::CImageList m_iMore;
	WTL::CImageList m_iMoreMore;
	WTL::CImageList m_iPlus;
	WTL::CImageList m_iMinus;
	WTL::CImageList m_iPlusPlus;
	WTL::CImageList m_iMinusMinus;
	WTL::CImageList m_iLittleLeft;
	WTL::CImageList m_iLittleRight;
	
	LRESULT OnCreate(LPCREATESTRUCT pCreateStruct);


	// Generated message map functions

	enum {
		INDICATOR_SEQPOS = 1,
		INDICATOR_PATTERN,
		INDICATOR_LINE,
		INDICATOR_TIME,
		INDICATOR_EDIT,
		INDICATOR_FOLLOW,
		INDICATOR_NOTEOFF,
		INDICATOR_TWEAKS,
		INDICATOR_OCTAVE
	};


	void OnHelpPsycleenviromentinfo();
	void OnMidiMonitorDlg();
	void OnAppExit();
	void OnMachineview();
	void OnPatternView();
#ifdef _TEST
	void OnTest();
#endif

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
//	LRESULT OnConfigurationSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetMessageString (WPARAM wParam, LPARAM lParam);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnViewSequencerbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewSongbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewMachinebar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeTrackcombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupTrackcombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBpmIncOne(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBpmIncTen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBpmDecOne(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBpmDecTen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDecTpb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedIncTpb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeCombooctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupCombooctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedClipbut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeSscombo2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupSscombo2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeBarCombogen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupBarCombogen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBDecgen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBIncgen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeAuxselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupAuxselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBIncwav(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBDecwav(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeBarComboins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnCloseupBarComboins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLoadwave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSavewave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedGearRack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedWavebut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedEditwave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPsyhelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLbnSelchangeSeqlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLbnDblclkSeqlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedIncshort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDecshort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInclong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDeclong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqnew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqins(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqduplicate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqcut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqdelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqcopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqpaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqclr(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSeqsrt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInclen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDeclen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMultichannelAudition(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRecordNoteoff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRecordTweaks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedNotestoeffects(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void OnConfigurationSettings();

	void UpdateLayout(BOOL bResizeBars = TRUE);
	void SaveRecent();
	void LoadRecent();
	void InitSequencer();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_MACHINEBAR, UPDUI_MENUPOPUP | UPDUI_CHECKED )
		UPDATE_ELEMENT(ID_VIEW_SEQUENCERBAR, UPDUI_MENUPOPUP | UPDUI_CHECKED )
		UPDATE_ELEMENT(ID_MACHINEVIEW,UPDUI_MENUPOPUP | UPDUI_CHECKED | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PATTERNVIEW,UPDUI_MENUPOPUP | UPDUI_CHECKED | UPDUI_TOOLBAR)

		UPDATE_ELEMENT(ID_VIEW_SONGBAR, UPDUI_MENUPOPUP | UPDUI_CHECKED)
		UPDATE_ELEMENT(ID_AUTOSTOP,UPDUI_MENUPOPUP | UPDUI_TOOLBAR | UPDUI_CHECKED)

	
		UPDATE_ELEMENT(INDICATOR_SEQPOS, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_PATTERN, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_LINE, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_TIME, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_EDIT, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_FOLLOW, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_NOTEOFF, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_TWEAKS, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(INDICATOR_OCTAVE, UPDUI_STATUSBAR)
		
		UPDATE_ELEMENT(ID_RECORDB, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_BARPLAY, UPDUI_TOOLBAR | UPDUI_CHECKED)
		UPDATE_ELEMENT(ID_BARPLAYFROMSTART, UPDUI_TOOLBAR | UPDUI_CHECKED)
		UPDATE_ELEMENT(ID_BARREC, UPDUI_TOOLBAR | UPDUI_CHECKED)
		UPDATE_ELEMENT(ID_BUTTONPLAYSEQBLOCK, UPDUI_TOOLBAR | UPDUI_CHECKED)

		UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_MIXPASTE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CONFIGURATION_LOOPPLAYBACK, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_POP_COPY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_MIXPASTE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_DELETE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_INTERPOLATE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_CHANGEGENERATOR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_CHANGEINSTRUMENT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_TRANSPOSE1, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_TRANSPOSE12, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_TRANSPOSE_1, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_TRANSPOSE_12, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_BLOCK_SWINGFILL, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_CUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_POP_PASTE, UPDUI_MENUPOPUP)

	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_TIMER(OnTimer)

		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)


		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_ABOUTPSYCLE, OnAppAbout)
		COMMAND_ID_HANDLER_EX2(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
		COMMAND_ID_HANDLER(ID_VIEW_SEQUENCERBAR, OnViewSequencerbar)
		COMMAND_ID_HANDLER(ID_VIEW_SONGBAR, OnViewSongbar)
		COMMAND_ID_HANDLER(ID_VIEW_MACHINEBAR, OnViewMachinebar)
		COMMAND_HANDLER(IDC_TRACKCOMBO, CBN_SELCHANGE, OnCbnSelchangeTrackcombo)
		COMMAND_HANDLER(IDC_TRACKCOMBO, CBN_CLOSEUP, OnCbnCloseupTrackcombo)
		COMMAND_HANDLER(IDC_BTN_INCONE, BN_CLICKED, OnBpmIncOne)
		COMMAND_HANDLER(IDC_BTN_INCTEN, BN_CLICKED, OnBpmIncTen)
		COMMAND_HANDLER(IDC_BTN_DECONE, BN_CLICKED, OnBpmDecOne)
		COMMAND_HANDLER(IDC_BTN_DECTEN, BN_CLICKED, OnBpmDecTen)
		COMMAND_HANDLER(IDC_DEC_TPB, BN_CLICKED, OnBnClickedDecTpb)
		COMMAND_HANDLER(IDC_INC_TPB, BN_CLICKED, OnBnClickedIncTpb)
		COMMAND_HANDLER(IDC_COMBOOCTAVE, CBN_SELCHANGE, OnCbnSelchangeCombooctave)
		COMMAND_HANDLER(IDC_COMBOOCTAVE, CBN_CLOSEUP, OnCbnCloseupCombooctave)
		COMMAND_HANDLER(IDC_CLIPBUT, BN_CLICKED, OnBnClickedClipbut)
		COMMAND_HANDLER(IDC_SSCOMBO2, CBN_SELCHANGE, OnCbnSelchangeSscombo2)
		COMMAND_HANDLER(IDC_SSCOMBO2, CBN_CLOSEUP, OnCbnCloseupSscombo2)
		COMMAND_HANDLER(IDC_BAR_COMBOGEN, CBN_SELCHANGE, OnCbnSelchangeBarCombogen)
		COMMAND_HANDLER(IDC_BAR_COMBOGEN, CBN_CLOSEUP, OnCbnCloseupBarCombogen)
		COMMAND_HANDLER(IDC_B_DECGEN, BN_CLICKED, OnBnClickedBDecgen)
		COMMAND_HANDLER(IDC_B_INCGEN, BN_CLICKED, OnBnClickedBIncgen)
		COMMAND_HANDLER(IDC_AUXSELECT, CBN_SELCHANGE, OnCbnSelchangeAuxselect)
		COMMAND_HANDLER(IDC_AUXSELECT, CBN_CLOSEUP, OnCbnCloseupAuxselect)
		COMMAND_HANDLER(IDC_B_INCWAV, BN_CLICKED, OnBnClickedBIncwav)
		COMMAND_HANDLER(IDC_B_DECWAV, BN_CLICKED, OnBnClickedBDecwav)
		COMMAND_HANDLER(IDC_BAR_COMBOINS, CBN_SELCHANGE, OnCbnSelchangeBarComboins)
		COMMAND_HANDLER(IDC_BAR_COMBOINS, CBN_CLOSEUP, OnCbnCloseupBarComboins)
		COMMAND_HANDLER(IDC_LOADWAVE, BN_CLICKED, OnBnClickedLoadwave)
		COMMAND_HANDLER(IDC_SAVEWAVE, BN_CLICKED, OnBnClickedSavewave)
		COMMAND_HANDLER(IDC_GEAR_RACK, BN_CLICKED, OnBnClickedGearRack)
		COMMAND_HANDLER(IDC_WAVEBUT, BN_CLICKED, OnBnClickedWavebut)
		COMMAND_HANDLER(IDC_EDITWAVE, BN_CLICKED, OnBnClickedEditwave)
		COMMAND_ID_HANDLER(ID_PSYHELP, OnPsyhelp)
		COMMAND_HANDLER(IDC_SEQLIST, LBN_SELCHANGE, OnLbnSelchangeSeqlist)
		COMMAND_HANDLER(IDC_SEQLIST, LBN_DBLCLK, OnLbnDblclkSeqlist)
		COMMAND_HANDLER(IDC_INCSHORT, BN_CLICKED, OnBnClickedIncshort)
		COMMAND_HANDLER(IDC_DECSHORT, BN_CLICKED, OnBnClickedDecshort)
		COMMAND_HANDLER(IDC_INCLONG, BN_CLICKED, OnBnClickedInclong)
		COMMAND_HANDLER(IDC_DECLONG, BN_CLICKED, OnBnClickedDeclong)
		COMMAND_HANDLER(IDC_SEQNEW, BN_CLICKED, OnBnClickedSeqnew)
		COMMAND_HANDLER(IDC_SEQINS, BN_CLICKED, OnBnClickedSeqins)
		COMMAND_HANDLER(IDC_SEQDUPLICATE, BN_CLICKED, OnBnClickedSeqduplicate)
		COMMAND_HANDLER(IDC_SEQCUT, BN_CLICKED, OnBnClickedSeqcut)
		COMMAND_HANDLER(IDC_SEQDELETE, BN_CLICKED, OnBnClickedSeqdelete)
		COMMAND_HANDLER(IDC_SEQCOPY, BN_CLICKED, OnBnClickedSeqcopy)
		COMMAND_HANDLER(IDC_SEQPASTE, BN_CLICKED, OnBnClickedSeqpaste)
		COMMAND_HANDLER(IDC_SEQCLR, BN_CLICKED, OnBnClickedSeqclr)
		COMMAND_HANDLER(IDC_SEQSRT, BN_CLICKED, OnBnClickedSeqsrt)
		COMMAND_HANDLER(IDC_INCLEN, BN_CLICKED, OnBnClickedInclen)
		COMMAND_HANDLER(IDC_DECLEN, BN_CLICKED, OnBnClickedDeclen)
		COMMAND_HANDLER(IDC_MULTICHANNEL_AUDITION, BN_CLICKED, OnBnClickedMultichannelAudition)
		COMMAND_HANDLER(IDC_RECORD_NOTEOFF, BN_CLICKED, OnBnClickedRecordNoteoff)
		COMMAND_HANDLER(IDC_RECORD_TWEAKS, BN_CLICKED, OnBnClickedRecordTweaks)
		COMMAND_HANDLER(IDC_NOTESTOEFFECTS, BN_CLICKED, OnBnClickedNotestoeffects)
		COMMAND_HANDLER(IDC_FOLLOW, BN_CLICKED, OnBnClickedFollow)

		COMMAND_ID_HANDLER_EX2(ID_BARPLAY, OnBarplay)
		COMMAND_ID_HANDLER_EX2(ID_BARPLAYFROMSTART, OnBarplayFromStart)
		COMMAND_ID_HANDLER_EX2(ID_BARREC, OnBarrec)
		COMMAND_ID_HANDLER_EX2(ID_BARSTOP, OnBarstop)
		COMMAND_ID_HANDLER_EX2(ID_RECORDB, OnRecordWav)
		COMMAND_ID_HANDLER_EX2(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER_EX2(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER_EX2(ID_FILE_SAVE_AS, OnFileSaveAs)
		COMMAND_ID_HANDLER_EX2(ID_FILE_LOADSONG, OnFileLoadsong)
		COMMAND_ID_HANDLER_EX2(ID_FILE_REVERT, OnFileRevert)
		COMMAND_ID_HANDLER_EX2(ID_HELP_SALUDOS, OnHelpSaludos)
		COMMAND_ID_HANDLER_EX2(ID_FILE_SONGPROPERTIES, OnFileSongproperties)
		COMMAND_ID_HANDLER_EX2(ID_VIEW_INSTRUMENTEDITOR, OnViewInstrumenteditor)
		COMMAND_ID_HANDLER_EX2(ID_NEWMACHINE, OnNewmachine)
		COMMAND_ID_HANDLER_EX2(ID_BUTTONPLAYSEQBLOCK, OnButtonplayseqblock)
		COMMAND_ID_HANDLER_EX2(ID_POP_CUT, m_view.OnPopCut)
		COMMAND_ID_HANDLER_EX2(ID_POP_COPY, m_view.OnPopCopy)
		COMMAND_ID_HANDLER_EX2(ID_POP_PASTE, m_view.OnPopPaste)
		COMMAND_ID_HANDLER_EX2(ID_POP_MIXPASTE, m_view.OnPopMixpaste)
		COMMAND_ID_HANDLER_EX2(ID_POP_DELETE, m_view.OnPopDelete)
		COMMAND_ID_HANDLER_EX2(ID_POP_INTERPOLATE, m_view.OnPopInterpolate)
		COMMAND_ID_HANDLER_EX2(ID_POP_CHANGEGENERATOR, m_view.OnPopChangegenerator)
		COMMAND_ID_HANDLER_EX2(ID_POP_CHANGEINSTRUMENT, m_view.OnPopChangeinstrument)
		COMMAND_ID_HANDLER_EX2(ID_POP_TRANSPOSE1, m_view.OnPopTranspose1)
		COMMAND_ID_HANDLER_EX2(ID_POP_TRANSPOSE12, m_view.OnPopTranspose12)
		COMMAND_ID_HANDLER_EX2(ID_POP_TRANSPOSE_1, m_view.OnPopTranspose_1)
		COMMAND_ID_HANDLER_EX2(ID_POP_TRANSPOSE_12, m_view.OnPopTranspose_12)
										
		COMMAND_ID_HANDLER_EX2(ID_AUTOSTOP, OnAutostop)
		COMMAND_ID_HANDLER_EX2(ID_POP_PATTENPROPERTIES, m_view.OnPopPattenproperties)
		COMMAND_ID_HANDLER_EX2(ID_POP_BLOCK_SWINGFILL, m_view.OnPopBlockSwingfill)
		COMMAND_ID_HANDLER_EX2(ID_POP_TRACK_SWINGFILL, m_view.OnPopTrackSwingfill)
		COMMAND_ID_HANDLER_EX2(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
		COMMAND_ID_HANDLER_EX2(ID_FILE_RECENT_01, OnFileRecent_01)
		COMMAND_ID_HANDLER_EX2(ID_FILE_RECENT_02, OnFileRecent_02)
		COMMAND_ID_HANDLER_EX2(ID_FILE_RECENT_03, OnFileRecent_03)
		COMMAND_ID_HANDLER_EX2(ID_FILE_RECENT_04, OnFileRecent_04)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_UNDO, OnEditUndo)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_REDO, OnEditRedo)
		COMMAND_ID_HANDLER_EX2(ID_FILE_SAVEAUDIO, OnFileSaveaudio)
		COMMAND_ID_HANDLER_EX2(ID_HELP_KEYBTXT, OnHelpKeybtxt)
		COMMAND_ID_HANDLER_EX2(ID_HELP_README, OnHelpReadme)
		COMMAND_ID_HANDLER_EX2(ID_HELP_TWEAKING, OnHelpTweaking)
		COMMAND_ID_HANDLER_EX2(ID_HELP_WHATSNEW, OnHelpWhatsnew)
	
		// カットコマンド 
		COMMAND_ID_HANDLER_EX2(ID_EDIT_CUT, m_view.patCut)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_COPY, m_view.patCopy)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_PASTE, m_view.patPaste)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_MIXPASTE, m_view.patMixPaste)
		COMMAND_ID_HANDLER_EX2(ID_EDIT_DELETE, m_view.patDelete)
#ifdef _TEST
		COMMAND_ID_HANDLER_EX2(ID_TEST, OnTest)
#endif
		COMMAND_ID_HANDLER_EX2(ID_CONFIGURATION_LOOPPLAYBACK, OnConfigurationLoopplayback)
		COMMAND_ID_HANDLER_EX2(ID_CPUPERFORMANCE, OnHelpPsycleenviromentinfo)
		COMMAND_ID_HANDLER_EX2(ID_MIDI_MONITOR, OnMidiMonitorDlg)
		COMMAND_ID_HANDLER_EX2(ID_APP_EXIT, OnAppExit)
										
//		COMMAND_ID_HANDLER_EX2(ID_MACHINEVIEW, OnMachineview)
//		COMMAND_ID_HANDLER_EX2(ID_PATTERNVIEW, OnPatternView)	

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(thisClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
    // TODO OnEditUndoを実装する
	void OnEditUndo();
    // TODO OnEditRedoを実装する
	void OnEditRedo();
										
	void OnBarplay();
	void OnBarplayFromStart();
	void OnBarrec();
	void OnBarstop();
	void OnRecordWav();
	void OnFileNew();

	BOOL OnFileSave();///< ファイルの上書き保存
	BOOL OnFileSaveAs();///< 別名で保存

	void OnFileLoadsong();///< ファイルを開く 
	void OnFileRevert();///< 編集前に戻す
	void OnHelpSaludos();
	void OnFileSongproperties();
	void OnViewInstrumenteditor();
	void OnNewmachine();
	void OnButtonplayseqblock();
	void OnAutostop();///< 自動停止するかしないか
	void OnTimer( UINT nIDEvent,TIMERPROC pTimerProc = NULL );
	void InitTimer();
	
	void AddMacViewUndo(); // place holder
	
	void OnEditUndo(); //
	void OnEditRedo();

	const UINT MessageDialog(const string& text,const string& caption,const UINT type)
	{
		return ::MessageBox(text.data(),caption.data(),type);
	}

	const bool CheckUnsavedSong();
	const bool CheckUnsavedSong(const TCHAR * szTitle);

	void NewSong();

	void InitializeBeforeLoading();
	void ProcessAfterLoading();

	void SetTitleBarText();


private:
	//RECENT!!!//
	void AppendToRecent(const TCHAR* fName);
	void CallOpenRecent(int pos);
	void OnFileLoadsongNamed(TCHAR* fName, int fType);
	void OnFileRecent_01();
	void OnFileRecent_02();
	void OnFileRecent_03();
	void OnFileRecent_04();
//	void OnFileLoadsong();
	void OnFileSaveaudio();
	void OnHelpKeybtxt();
	void OnHelpReadme();
	void OnHelpTweaking();
	void OnHelpWhatsnew();
	void OnConfigurationLoopplayback();
	
	WTL::CReBarCtrl     m_wndReBar;
	WTL::CSplitterWindow  m_wndVertSplit;
	HMENU hRecentMenu;
	//
	typedef std::map<string,SF::ISongLoader *> SongLoaderMap;

//	SPatternUndo * pUndoList;
//	SPatternUndo * pRedoList;

	int UndoCounter;
	int UndoSaved;

	int UndoMacCounter;
	int UndoMacSaved;

	bool _outputActive;	// This variable indicates if the output (audio or midi) is active or not.
						// Its function is to prevent audio (and midi) operations while it is not
						// initialized, or while song is being modified (New(),Load()..).
						// 

	iController *m_pController;
//	SF::D3D m_D3D;	
//	WTL::CImageList m_ImageList;
//	WTL::CBitmapButton m_cb1;

};
#endif
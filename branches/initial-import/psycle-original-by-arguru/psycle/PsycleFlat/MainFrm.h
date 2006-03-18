// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__FE0D36E8_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_MAINFRM_H__FE0D36E8_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildView.h"
#include "InstrumentEditor.h"
#include "InfoDlg.h"
#include "SequencerDlg.h"
#include "Submarco.h"
#include "FrameMachine.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	psySong *songRef;

// Operations
public:
	void SetAppSongBpm(int x);
	void UpdatePlayOrder(bool mode);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	UpdateSequencer();
	bool isguiopen[MAX_MACHINES];
	CFrameMachine	*m_pWndMac[MAX_MACHINES];

	ClosePsycle();
	WaveEditorBackUpdate();
	CloseMacGui(int mac);
	CloseAllMacGuis();
	ShowMachineGui(int tmac);
	ShowMaster();
	UpdateEnvInfo();
	HidePerformanceDlg();
	ShowPerformanceDlg();
	HideInstrumentEditor();
	ShowInstrumentEditor();
	StatusBarText(char *txt);
	UpdateComboIns();
	bool anySampler;

	UpdateComboGen();
	Psybar2Init();
	UpdateVumeters(int l, int r, COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip);
	ConfigAudio();
	virtual ~CMainFrame();

private:
	// Privates

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CChildView  m_wndView;
	CDialogBar	m_wndControl;
	CDialogBar	m_wndControl2;
	CDialogBar	m_wndSeq;

	CInstrumentEditor	m_wndInst;
	CInfoDlg	m_wndInfo;
	CSubMarco	*m_pWndWed;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnBarButton1();
	afx_msg void OnBpmAddTen();
	afx_msg void OnBpmDecOne();
	afx_msg void OnBpmDecTen();
	afx_msg void OnSelchangeSscombo2();
	afx_msg void OnCloseupSscombo2();
	afx_msg void OnSelchangeBarCombogen();
	afx_msg void OnCloseupBarCombogen();
	afx_msg void OnSelchangeBarComboins();
	afx_msg void OnCloseupBarComboins();
	afx_msg void OnClipbut();
	afx_msg void OnSelchangeTrackcombo();
	afx_msg void OnCloseupTrackcombo();
	afx_msg void OnPsyhelp();
	afx_msg void OnLoadwave();
	afx_msg void OnEditwave();
	afx_msg void OnWavebut();
	afx_msg void OnDestroy();
	afx_msg void OnBDecgen();
	afx_msg void OnBIncgen();
	afx_msg void OnBDecwav();
	afx_msg void OnBIncwav();
	afx_msg void OnClose();
	afx_msg void OnVst2managerVst2pluginmanager();
	afx_msg void OnSelchangeSeqlist();
	afx_msg void OnDblclkSeqlist();
	afx_msg void OnDeclen();
	afx_msg void OnDecpat2();
	afx_msg void OnDecpos2();
	afx_msg void OnInclen();
	afx_msg void OnIncpat2();
	afx_msg void OnIncpos2();
	afx_msg void OnIncshort();
	afx_msg void OnDecshort();
	afx_msg void OnSeqins();
	afx_msg void OnSeqslen();
	afx_msg void OnSeqspr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__FE0D36E8_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)

// ChildView.h : interface of the CChildView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__FE0D36EA_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_CHILDVIEW_H__FE0D36EA_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "psy.h"

/////////////////////////////////////////////////////////////////////////////
// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();
// Attributes
public:
	psySong* childSong;
	CFrameWnd* pParentFrame;
	PSYCONFIG appConfig;
	
	CBitmap stuffbmp;

	int FLATSIZES[256];
	
	char m_appdir[_MAX_PATH];

	int seqOffset;
	int seqStep;
	int seqRow;
	
	int patOffset;
	int patRow;
	int patLine;

	int updateMode;
	int updatePar;
	int viewMode;

// Operations
public:

	void InitTimer();
	void ConfigAudio(bool forceConfig);
	void ShowSplash();
	void UpdateIBrowseDlg();
	void UpdateSBrowseDlg();
	int SongIncBpm(int x);
	void ValidateParent();
	bool isBlockCopied;	
	void UpdateConfig();

//////////////////////////////////////////////////////////////////////
// Private operations

private:
	
	// GDI Stuff
	bool DoTheVus;
	CFont seqFont;
	CopyBlock(bool cutit);
	PasteBlock(int tx,int lx);
	void DrawPatEditor(CPaintDC *devc);
	void DrawMachineVol(int x,int y,CClientDC *devc, int volu);
	void DrawMachineVumeters(CClientDC *devc);	
	void DrawMachineEditor(CPaintDC *devc);
	void DrawMachine(int x,int y, char *name, CPaintDC *devc, int panning, int type);
	void amosDraw(CPaintDC *devc, int oX,int oY,int dX,int dY);
	void TXT(CPaintDC *devc,char *txt, int x,int y,int w,int h,bool mode);
	void BOX(CPaintDC *devc,int x,int y, int w, int h);
	void Draw_BackSkin();
	StartBlock(int track,int line);
	EndBlock(int track,int line);

	int GetMachine(CPoint point);
	void ShowPatternDlg(void);
	void OutNote(CPaintDC *devc,int x,int y,int note);
	void OutData(CPaintDC *devc,int x,int y,unsigned char data,bool trflag);
	void AdvanceLine(int x,bool mode);
	void PrevLine(int x,bool mode);
	void ShiftOctave(int x);
	bool MSBPut(int nChar,unsigned char *offset,int mode,bool trflag);
	void patCopy();
	void patPaste();
	void patCut();
	void patTranspose(int trp);
	DecCurPattern();
	IncCurPattern();
	IncPosition();
	DecPosition();

	void PrevTrack(int nt);
	void AdvanceTrack(int nt);

	// Private members
	int outnote;
	bool showIBOnPatview;

	int mcd_x;
	int mcd_y;

	unsigned char blockBufferData[5*MAX_LINES*MAX_TRACKS];	
	bool blockSelected;
	int blockLineStart;
	int blockLineEnd;
	int blockTrackStart;
	int blockTrackEnd;
	int	blockNTracks;
	int	blockNLines;
	
	
	unsigned char patBufferData[5*MAX_LINES*MAX_TRACKS];
	int patBufferLines;
	bool patBufferCopy;
	
	int uiplay;
	int patStep;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	SetPatStep(int stp);
	Repaint();
	UpdateChecks();
	WriteConfig();
	ReadConfig();
	InitConfig();
	virtual ~CChildView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg void OnConfigurationAudiodriver();
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnHelpPsycleenviromentinfo();
	afx_msg void OnDestroy();
	afx_msg void OnAppExit();
	afx_msg void OnMachineview();
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnPatternView();
	afx_msg void OnBarplay();
	afx_msg void OnBarstop();
	afx_msg void OnTimer( UINT nIDEvent );
	afx_msg void OnShowied();
	afx_msg void OnRecordb();
	afx_msg void OnFileSavesong();
	afx_msg void OnFileLoadsong();
	afx_msg void OnHelpSaludos();
	afx_msg void OnConfigurationSettingsBackgroundskin();
	afx_msg void OnUpdatePatternView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMachineview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBarplay(CCmdUI* pCmdUI);
	afx_msg void OnFileSongproperties();
	afx_msg void OnViewInstrumenteditor();
	afx_msg void OnFileNew();
	afx_msg void OnNewmachine();
	afx_msg void OnConfigurationKeyboardlayoutFrench();
	afx_msg void OnConfigurationKeyboardlayoutStandard();
	afx_msg void OnUpdateConfigurationKeyboardlayoutFrench(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConfigurationKeyboardlayoutStandard(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordb(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDVIEW_H__FE0D36EA_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)

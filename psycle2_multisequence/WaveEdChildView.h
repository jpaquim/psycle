#if !defined(AFX_WAVEEDCHILDVIEW_H__266742FB_1A13_41EF_BE56_FAF1FB456280__INCLUDED_)
#define AFX_WAVEEDCHILDVIEW_H__266742FB_1A13_41EF_BE56_FAF1FB456280__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WaveEdAmplifyDialog.h"

class CMainFrame;
class Song;

class CWaveEdChildView : public CWnd
{
public:
	void SetSong(Song*);
	void SetParent(CMainFrame*);

	CWaveEdChildView();
	virtual ~CWaveEdChildView();

	void GenerateAndShow();
	void SetViewData(int ins, int wav);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveEdChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CWaveEdChildView)
	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSelectionZoom();
	afx_msg void OnSelectionZoomOut();
	afx_msg void OnSelectionFadeIn();
	afx_msg void OnSelectionFadeOut();	
	afx_msg void OnSelectionNormalize();
	afx_msg void OnSelectionRemoveDC();
	afx_msg void OnSelectionAmplify();
	afx_msg void OnSelectionReverse();
	afx_msg void OnSelectionShowall();
	afx_msg void OnUpdateSelectionAmplify(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionReverse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionFadein(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionFadeout(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionNormalize(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionRemovedc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSeleccionZoom(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSeleccionZoomout(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionShowall(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnConvertMono();
	afx_msg void OnUpdateConvertMono(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnDestroyClipboard();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	Song *_pSong;
	
	// Painting pens
	CPen cpen_lo;
	CPen cpen_me;
	CPen cpen_hi;

	//Clipboard
	char*	pClipboardData;
	HGLOBAL hClipboardData, hPasteData;

	// Wave data
	signed short* wdLeft;
	signed short* wdRight;
	bool wdStereo;
	unsigned long wdLength;
	unsigned long wdLoopS;
	unsigned long wdLoopE;
	bool wdLoop;

	// Display data
	unsigned long diStart;
	unsigned long diLength;
	unsigned long blStart;
	unsigned long blLength;
	bool blSelection;
	bool wdWave;

	unsigned long selx, selx2;

	int wsInstrument;
	int wsWave;

	bool drawwave;

	unsigned long SelStart; // Selection start (point where left clicked);

	CWaveEdAmplifyDialog AmpDialog;

	CMainFrame* pParent;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDCHILDVIEW_H__266742FB_1A13_41EF_BE56_FAF1FB456280__INCLUDED_)

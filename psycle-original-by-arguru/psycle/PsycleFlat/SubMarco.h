#if !defined(AFX_SUBMARCO_H__2CEE7C17_3D28_11D4_AB64_00C026101D9A__INCLUDED_)
#define AFX_SUBMARCO_H__2CEE7C17_3D28_11D4_AB64_00C026101D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SubMarco.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSubMarco frame

class CSubMarco : public CFrameWnd
{
	DECLARE_DYNCREATE(CSubMarco)
public:
	CSubMarco(int dum){};
	psySong *songRef;

protected:
	CSubMarco();           // protected constructor used by dynamic creation
	
	// Wave Data
	signed short* wdLeft;
	signed short* wdRight;
	bool wdStereo;
	int wdLength;

	// Display Data
	int diStart;
	int diLength;
	int blStart;
	int blLength;
	bool blSelection;
	bool dragging;
	bool wdWave;

	int wsInstrument;
	int wsWave;

	// Painting pens
	CPen cpen_lo;
	CPen cpen_me;
	CPen cpen_hi;


// Attributes
public:

// Miembros privados
private:

// Operations
public:
	FitWave();
	AdquireFromSong(int ins,int wav, bool show);
	SetWave(signed short *pleft,signed short *pright,int numsamples, bool stereo);
	GenerateAndShow();

	int obx;
	int obx2;
	bool dontdrawwave;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSubMarco)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSubMarco();

	// Generated message map functions
	//{{AFX_MSG(CSubMarco)
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSelectionZoom();
	afx_msg void OnSelectionZoomOut();
	afx_msg void OnSelectionFadeIn();
	afx_msg void OnSelectionFadeOut();	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBMARCO_H__2CEE7C17_3D28_11D4_AB64_00C026101D9A__INCLUDED_)

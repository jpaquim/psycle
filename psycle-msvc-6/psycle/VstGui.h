#if !defined(AFX_VSTGUI1_H__2C9E2D2B_5D69_11D4_A87E_00C026101D9A__INCLUDED_)
#define AFX_VSTGUI1_H__2C9E2D2B_5D69_11D4_A87E_00C026101D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VstGui.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVstGui frame
#include "Vst\Aeffect.h"
#include "childView.h"

class CVstGui : public CFrameWnd
{
	DECLARE_DYNCREATE(CVstGui)
	
	CVstGui(int n){};
	AEffect * effect;
	Machine* _pMachine;

protected:
	CVstGui();           // protected constructor used by dynamic creation
	
// Attributes
public:

// Operations
public:
	virtual ~CVstGui();
	void DoTheInit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVstGui)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVstGui)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTGUI1_H__2C9E2D2B_5D69_11D4_A87E_00C026101D9A__INCLUDED_)

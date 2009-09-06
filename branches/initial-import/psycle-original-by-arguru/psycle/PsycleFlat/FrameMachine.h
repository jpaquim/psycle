#if !defined(AFX_FRAMEMACHINE_H__D8C72DE0_5378_11D4_8913_F64D9CA20B48__INCLUDED_)
#define AFX_FRAMEMACHINE_H__D8C72DE0_5378_11D4_8913_F64D9CA20B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FrameMachine.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFrameMachine frame

class CFrameMachine : public CFrameWnd
{
	DECLARE_DYNCREATE(CFrameMachine)
protected:
	CFrameMachine();           // protected constructor used by dynamic creation

// Attributes
public:
	CFrameMachine(int dum){};
	psySong *songRef;

private:
	psyGear *machineRef;
	CBitmap b_knob;
	CFont	b_font;
// Enviroment

	int numParameters;
	bool me;

	bool istweak;
	int tweakpar;
	int tweakbase;
	int sourcepoint;
	int frame_index;
	
	int ncol;
	int parspercol;
// Operations
public:
	SelectMachine(int tmac);
	Generate();
	bool* guimatrix;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrameMachine)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFrameMachine();

	// Generated message map functions
	//{{AFX_MSG(CFrameMachine)
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnParametersRandomparameters();
	afx_msg void OnParametersResetparameters();
	afx_msg void OnParametersCommand();
	afx_msg void OnMachineAboutthismachine();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRAMEMACHINE_H__D8C72DE0_5378_11D4_8913_F64D9CA20B48__INCLUDED_)
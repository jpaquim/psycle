#if !defined(AFX_GEARSCOPE_H__48AF9103_2805_11D4_8913_C62F3248A677a__INCLUDED_)
#define AFX_GEARSCOPE_H__48AF9103_2805_11D4_8913_C62F3248A677a__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearScope.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearScope dialog

class CGearScope : public CDialog
{
// Construction
public:
	CGearScope(CChildView* pParent = NULL);   // standard constructor
	Scope* _pMachine;
	bool doit;
	BOOL Create();
	afx_msg void OnCancel();
	int div;

// Dialog Data
	//{{AFX_DATA(CGearScope)
	enum { IDD = IDD_GEAR_SCOPE };
	CSliderCtrl	m_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearScope)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	inline int GetY(float f);
	CChildView* m_pParent;
	CBitmap* bmpDC;
	CRect rc;

	// Generated message map functions
	//{{AFX_MSG(CGearScope)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARSCOPE_H__48AF9103_2805_11D4_8913_C62F3248A677a__INCLUDED_)

#if !defined(AFX_GEARGAINER_H__48AF9103_2805_11D4_8913_C62F3248A677__INCLUDED_)
#define AFX_GEARGAINER_H__48AF9103_2805_11D4_8913_C62F3248A677__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearGainer.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearGainer dialog

class CGearGainer : public CDialog
{
// Construction
public:
	CGearGainer(CChildView* pParent = NULL);   // standard constructor
	Gainer* _pMachine;
	bool doit;
	BOOL Create();
	afx_msg void OnCancel();

// Dialog Data
	//{{AFX_DATA(CGearGainer)
	enum { IDD = IDD_GEAR_GAINER };
	CStatic	m_volabel;
	CSliderCtrl	m_volsider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearGainer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearGainer)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARGAINER_H__48AF9103_2805_11D4_8913_C62F3248A677__INCLUDED_)

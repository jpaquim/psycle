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

#define MAX_SCOPE_BANDS 32
#define SCOPE_SPEC_SAMPLES	256

/////////////////////////////////////////////////////////////////////////////
// CGearScope dialog

class CGearScope : public CDialog
{
// Construction
public:
	CGearScope(CChildView* pParent = NULL);   // standard constructor
	Scope* _pMachine;

	BOOL Create();
	afx_msg void OnCancel();
	UINT this_index;

// Dialog Data
	//{{AFX_DATA(CGearScope)
	enum { IDD = IDD_GEAR_SCOPE };
	CSliderCtrl	m_slider;
	CSliderCtrl	m_slider2;
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
	void SetMode();
	CChildView* m_pParent;
	CBitmap* bmpDC;
	CRect rc;
	CFont font;
	CFont* oldFont;
	int pos;
	BOOL hold;
	int bar_heightsl[MAX_SCOPE_BANDS];
	int bar_heightsr[MAX_SCOPE_BANDS];

	// Generated message map functions
	//{{AFX_MSG(CGearScope)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMode();
	afx_msg void OnHold();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARSCOPE_H__48AF9103_2805_11D4_8913_C62F3248A677a__INCLUDED_)

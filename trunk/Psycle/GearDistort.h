#if !defined(AFX_GEARDISTORT_H__C1C0B5E0_EAC7_11D3_8913_B46842196663__INCLUDED_)
#define AFX_GEARDISTORT_H__C1C0B5E0_EAC7_11D3_8913_B46842196663__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearDistort.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearDistort dialog

class CGearDistort : public CDialog
{
// Construction
public:
	CGearDistort(CChildView* pParent = NULL);   // standard constructor
	Distortion* _pMachine;
	bool doit;
	BOOL Create();
	afx_msg void OnCancel();

// Dialog Data
	//{{AFX_DATA(CGearDistort)
	enum { IDD = IDD_GEARDISTORT };
	CSliderCtrl	m_negc;
	CSliderCtrl	m_negt;
	CSliderCtrl	m_posc;
	CSliderCtrl	m_post;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearDistort)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearDistort)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton1();
	afx_msg void OnButton3();
	afx_msg void OnButton2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARDISTORT_H__C1C0B5E0_EAC7_11D3_8913_B46842196663__INCLUDED_)

#if !defined(AFX_CGearPsychOsc_H__FE0D36FE_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_CGearPsychOsc_H__FE0D36FE_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CGearPsychOsc.h : header file
//

#include "Machine.h"
#include "constants.h"

class CChildView;

/////////////////////////////////////////////////////////////////////////////
// CGearPsychOsc dialog

class CGearPsychOsc : public CDialog
{
// Construction
public:
	CGearPsychOsc(CChildView* pParent = NULL);   // standard constructor
	Sine* _pMachine;
	bool doit;
	BOOL Create();
	afx_msg void OnCancel();

	// Dialog Data
	//{{AFX_DATA(CGearPsychOsc)
	enum { IDD = IDD_GEAR_PSYCHOSC };
	CStatic	m_label4;
	CStatic	m_label2;
	CStatic	m_label3;
	CStatic	m_label1;
	CSliderCtrl	m_lfoamplitude;
	CSliderCtrl	m_lfofrequency;
	CSliderCtrl	m_glidespeed;
	CSliderCtrl	m_oscspeed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearPsychOsc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CChildView* m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CGearPsychOsc)
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGearPsychOsc_H__FE0D36FE_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)

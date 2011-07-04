#if !defined(AFX_VSTRACK_H__871E95E1_5C98_11D4_A87E_00C026101D9A__INCLUDED_)
#define AFX_VSTRACK_H__871E95E1_5C98_11D4_A87E_00C026101D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VstRack.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVstRack dialog
#include "Vst.h"

class CVstRack : public CDialog
{
// Construction
public:
	InitializePrograms();
	UpdateOne(int npar);
	UpdateParList();
	UpdateText();
	bool canTweak;
	CVstRack(CWnd* pParent = NULL);   // standard constructor
	CVst* m_pVst;
	int nPlug;
	int nPar;
	int previousProg;
	
// Dialog Data
	//{{AFX_DATA(CVstRack)
	enum { IDD = IDD_VSTRACK };
	CComboBox	m_combo2;
	CComboBox	m_combo;
	CSliderCtrl	m_slider;
	CStatic	m_text;
	CListBox	m_parlist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVstRack)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVstRack)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeList1();
	afx_msg void OnSelchangeProgram();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTRACK_H__871E95E1_5C98_11D4_A87E_00C026101D9A__INCLUDED_)

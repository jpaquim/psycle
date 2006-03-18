#if !defined(AFX_GEARVST_H__26796AC2_5DA7_11D4_8913_C2661E167359__INCLUDED_)
#define AFX_GEARVST_H__26796AC2_5DA7_11D4_8913_C2661E167359__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearVst.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGearVst dialog
#include "Vstgui1.h"

class CGearVst : public CDialog
{
// Construction
public:
	UpdateButt();
	int SetInstCombo();
	CGearVst(CWnd* pParent = NULL);   // standard constructor
	psyGear* machineRef;
	psySong* songRef;
	int iPlug[MAX_VST_INSTANCES];
	int nFound;
	CVst* m_vst;
	CVstGui *vstgui;
	bool editor;

// Dialog Data
	//{{AFX_DATA(CGearVst)
	enum { IDD = IDD_GEARVST };
	CComboBox	m_combo2;
	CButton	m_but;
	CComboBox	m_combo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearVst)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGearVst)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnEditor();
	afx_msg void OnClose();
	afx_msg void OnEjectPlug();
	afx_msg void OnQuitEjectExit();
	virtual void OnOK();
	afx_msg void OnSelchangeCombo2();
	afx_msg void OnButton2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARVST_H__26796AC2_5DA7_11D4_8913_C2661E167359__INCLUDED_)

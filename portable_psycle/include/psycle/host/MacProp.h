#if !defined(AFX_MACPROP_H__C1C0B5E1_EAC7_11D3_8913_B46842196663__INCLUDED_)
#define AFX_MACPROP_H__C1C0B5E1_EAC7_11D3_8913_B46842196663__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MacProp.h : header file
//

#include "Machine.h"
#include "ChildView.h"

class Song;

/////////////////////////////////////////////////////////////////////////////
// CMacProp dialog

class CMacProp : public CDialog
{
// Construction
public:
	CChildView *m_view;
	CMacProp(CWnd* pParent = NULL);   // standard constructor
	Machine *pMachine;
	Song* pSong;
	int thisMac;
	char txt[32];
	bool deleted;

// Dialog Data
	//{{AFX_DATA(CMacProp)
	enum { IDD = IDD_MACPROP };
	CButton	m_soloCheck;
	CButton	m_bypassCheck;
	CButton	m_muteCheck;
	CEdit	m_macname;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMacProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMacProp)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdit1();
	afx_msg void OnButton1();
	afx_msg void OnMute();
	afx_msg void OnBypass();
	afx_msg void OnSolo();
	afx_msg void OnClone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACPROP_H__C1C0B5E1_EAC7_11D3_8913_B46842196663__INCLUDED_)

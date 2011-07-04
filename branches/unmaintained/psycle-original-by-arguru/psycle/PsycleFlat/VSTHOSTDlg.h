#if !defined(AFX_VSTHOSTDLG_H__AEA9F7C7_5BE5_11D4_A87E_00C026101D9A__INCLUDED_)
#define AFX_VSTHOSTDLG_H__AEA9F7C7_5BE5_11D4_A87E_00C026101D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVSTHOSTDlg dialog
#include "..\vst\vst.h"
#include "vstgui1.h"

class CVSTHOSTDlg : public CDialog
{
// Construction
public:
	UpdatePlugList();
	UpdateInstList();

	CVSTHOSTDlg(CWnd* pParent = NULL);	// standard constructor
	CVst* m_vst;
	CVstGui *vstgui;
	bool editor;
	psySong *songRef;

// Dialog Data
	//{{AFX_DATA(CVSTHOSTDlg)
	enum { IDD = IDD_VSTHOST_DIALOG };
	CListBox	m_list2;
	CListBox	m_list;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVSTHOSTDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CVSTHOSTDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDblclkList1();
	afx_msg void OnDblclkList2();
	afx_msg void OnInstancePlug();
	afx_msg void OnEditPlug();
	afx_msg void OnAboutHost();
	afx_msg void OnExitButton();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTHOSTDLG_H__AEA9F7C7_5BE5_11D4_A87E_00C026101D9A__INCLUDED_)

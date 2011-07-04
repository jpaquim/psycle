#if !defined(AFX_INFODLG_H__078666A2_E9D9_11D3_8913_950D7D66B163__INCLUDED_)
#define AFX_INFODLG_H__078666A2_E9D9_11D3_8913_950D7D66B163__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog

class CInfoDlg : public CDialog
{
// Construction
public:
	void UpdateInfo();
	CInfoDlg(CWnd* pParent = NULL);   // standard constructor
	Song* _pSong;
	void InitTimer();
	int itemcount;
	
// Dialog Data
	//{{AFX_DATA(CInfoDlg)
	enum { IDD = IDD_INFO };
	CStatic	m_mem_virtual;
	CStatic	m_mem_pagefile;
	CStatic	m_mem_phy;
	CStatic	m_mem_reso;
	CStatic	m_cpurout;
	CStatic	m_machscpu;
	CStatic	m_processor_label;
	CStatic	m_cpuidlelabel;
	CListCtrl	m_machlist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__078666A2_E9D9_11D3_8913_950D7D66B163__INCLUDED_)

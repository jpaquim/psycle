#if !defined(AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_)
#define AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OutputDlg.h : header file
//

#include "AudioDriver.h"

/////////////////////////////////////////////////////////////////////////////
// COutputDlg dialog

class COutputDlg : public CDialog
{
// Construction
public:
	int m_driverIndex;
	AudioDriver* m_pDrivers[2];
	COutputDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COutputDlg)
	enum { IDD = IDD_OUTPUT };
	CComboBox	m_driverComboBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_oldDriverIndex;

	// Generated message map functions
	//{{AFX_MSG(COutputDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnConfig();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTDLG_H__E5905940_8CEC_11D4_937A_CBF03FBFA838__INCLUDED_)

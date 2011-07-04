#if !defined(AFX_CONFIGDLG_H__13AB3EE8_E186_47F4_BF97_1A731EEC7F88__INCLUDED_)
#define AFX_CONFIGDLG_H__13AB3EE8_E186_47F4_BF97_1A731EEC7F88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog

class CConfigDlg : public CDialog
{
// Construction
public:
	CConfigDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigDlg)
	enum { IDD = IDD_DIALOG1 };
	CEdit	m_editnative;
	CEdit	m_editvst;
	CComboBox	m_samprate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNative();
	afx_msg void OnChangeVst();
	afx_msg void OnChangeEditNative();
	afx_msg void OnChangeEditVst();
	afx_msg void OnSelchangeSampRate();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__13AB3EE8_E186_47F4_BF97_1A731EEC7F88__INCLUDED_)

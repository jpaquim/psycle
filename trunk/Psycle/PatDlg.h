#if !defined(AFX_PATDLG_H__38327E2C_ED0C_11D3_8913_B689BEC79464__INCLUDED_)
#define AFX_PATDLG_H__38327E2C_ED0C_11D3_8913_B689BEC79464__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CPatDlg dialog

class CPatDlg : public CDialog
{
// Construction
public:
	CPatDlg(CWnd* pParent = NULL);   // standard constructor
	int patLines;
	char patName[32];

// Dialog Data
	//{{AFX_DATA(CPatDlg)
	enum { IDD = IDD_PATDLG };
	CButton	m_adaptsizeCheck;
	CEdit	m_numlines;
	CEdit	m_patname;
	CSpinButtonCtrl	m_spinlines;
	BOOL	m_adaptsize;
	CStatic m_text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPatDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheck1();
	afx_msg void OnUpdateNumLines();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATDLG_H__38327E2C_ED0C_11D3_8913_B689BEC79464__INCLUDED_)

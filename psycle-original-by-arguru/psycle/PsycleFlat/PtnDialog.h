#if !defined(AFX_PTNDIALOG_H__6583DCA0_19E6_11D4_8913_D323851E1970__INCLUDED_)
#define AFX_PTNDIALOG_H__6583DCA0_19E6_11D4_8913_D323851E1970__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PtnDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPtnDialog dialog

class CPtnDialog : public CDialog
{
// Construction
public:
	CPtnDialog(CWnd* pParent = NULL);   // standard constructor
	psySong *songRef;
	int ouPat;

// Dialog Data
	//{{AFX_DATA(CPtnDialog)
	enum { IDD = IDD_PATNAVDIALOG };
	CButton	m_okbut;
	CStatic	m_pnlabel;
	CListCtrl	m_patlist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPtnDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void UpdateBut();
	void UpdateSel();
	int PIndex[MAX_PATTERNS];
	// Generated message map functions
	//{{AFX_MSG(CPtnDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCreateb();
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PTNDIALOG_H__6583DCA0_19E6_11D4_8913_D323851E1970__INCLUDED_)

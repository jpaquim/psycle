#if !defined(AFX_GREETDIALOG_H__9EABC421_1EC3_11D4_8913_F2F1E1B2CE72__INCLUDED_)
#define AFX_GREETDIALOG_H__9EABC421_1EC3_11D4_8913_F2F1E1B2CE72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GreetDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGreetDialog dialog

class CGreetDialog : public CDialog
{
// Construction
public:
	CGreetDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGreetDialog)
	enum { IDD = IDD_GREETS };
	CListBox	m_greetz;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGreetDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGreetDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GREETDIALOG_H__9EABC421_1EC3_11D4_8913_F2F1E1B2CE72__INCLUDED_)

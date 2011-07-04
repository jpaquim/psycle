#if !defined(AFX_RECORDERDLG_H__7699CC80_1396_11D4_8913_C72AB9BD7472__INCLUDED_)
#define AFX_RECORDERDLG_H__7699CC80_1396_11D4_8913_C72AB9BD7472__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecorderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecorderDlg dialog

class CRecorderDlg : public CDialog
{
// Construction
public:
	CRecorderDlg(CWnd* pParent = NULL);   // standard constructor
	psySong *songRef;

// Dialog Data
	//{{AFX_DATA(CRecorderDlg)
	enum { IDD = IDD_RECORDER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecorderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecorderDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDERDLG_H__7699CC80_1396_11D4_8913_C72AB9BD7472__INCLUDED_)

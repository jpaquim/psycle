#if !defined(AFX_MIDIDLG_H__CC39CFA0_239F_11D4_8913_F9EA1D36FC75__INCLUDED_)
#define AFX_MIDIDLG_H__CC39CFA0_239F_11D4_8913_F9EA1D36FC75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiDlg dialog

class CMidiDlg : public CDialog
{
// Construction
public:
	CMidiDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMidiDlg)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMidiDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDIDLG_H__CC39CFA0_239F_11D4_8913_F9EA1D36FC75__INCLUDED_)

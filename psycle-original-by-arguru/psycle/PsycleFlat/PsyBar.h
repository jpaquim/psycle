#if !defined(AFX_PSYBAR_H__B1640EA0_2211_11D4_8913_F1CEBBB50A74__INCLUDED_)
#define AFX_PSYBAR_H__B1640EA0_2211_11D4_8913_F1CEBBB50A74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PsyBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPsyBar dialog

class CPsyBar : public CDialog
{
// Construction
public:
	CPsyBar(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPsyBar)
	enum { IDD = IDD_CONTROLBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPsyBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPsyBar)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PSYBAR_H__B1640EA0_2211_11D4_8913_F1CEBBB50A74__INCLUDED_)

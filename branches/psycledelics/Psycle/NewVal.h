#if !defined(AFX_NEWVAL_H__D8C72DE5_5378_11D4_8913_F64D9CA20B48__INCLUDED_)
#define AFX_NEWVAL_H__D8C72DE5_5378_11D4_8913_F64D9CA20B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewVal.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewVal dialog

class CNewVal : public CDialog
{
// Construction
public:
	CNewVal(CWnd* pParent = NULL);   // standard constructor
	int m_Value;
	char Title[1024];

// Dialog Data
	//{{AFX_DATA(CNewVal)
	enum { IDD = IDD_NEWVAL };
	CEdit	m_value;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewVal)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewVal)
	afx_msg void OnChangeEdit1();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSetfocusEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWVAL_H__D8C72DE5_5378_11D4_8913_F64D9CA20B48__INCLUDED_)

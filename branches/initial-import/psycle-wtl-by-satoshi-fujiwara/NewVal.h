#if !defined(AFX_NEWVAL_H__D8C72DE5_5378_11D4_8913_F64D9CA20B48__INCLUDED_)
#define AFX_NEWVAL_H__D8C72DE5_5378_11D4_8913_F64D9CA20B48__INCLUDED_
/** @file 
 *  @brief  header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $ */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CNewVal dialog

class CNewVal : public CDialogImpl<CNewVal> {
// Construction
public:
	CNewVal();   // standard constructor
	int m_Value;
	TCHAR Title[1024];
	int min;
	int max;
	int macindex;
	int paramindex;

// Dialog Data
	//{{AFX_DATA(CNewVal)
	enum { IDD = IDD_NEWVAL };
	CEdit	m_value;
	CStatic m_text;
	//}}AFX_DATA

/*
BEGIN_MESSAGE_MAP(CNewVal, CDialog)
	//{{AFX_MSG_MAP(CNewVal)
	ON_EN_UPDATE(IDC_EDIT1, OnUpdateEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
*/

// Implementation
protected:
/*
	// Generated message map functions
	//{{AFX_MSG(CNewVal)
	afx_msg void OnChangeEdit1();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSetfocusEdit1();
	afx_msg void OnUpdateEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
*/
public:
	BEGIN_MSG_MAP(CNewVal)
		COMMAND_HANDLER(IDC_EDIT1, EN_UPDATE, OnEnUpdateEdit1)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		COMMAND_HANDLER(IDC_EDIT1, EN_SETFOCUS, OnEnSetfocusEdit1)
	END_MSG_MAP()
	LRESULT OnEnUpdateEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnSetfocusEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWVAL_H__D8C72DE5_5378_11D4_8913_F64D9CA20B48__INCLUDED_)

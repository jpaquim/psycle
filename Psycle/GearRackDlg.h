#if !defined(AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_)
#define AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GearRackDlg.h : header file
//

#include "ChildView.h"

/////////////////////////////////////////////////////////////////////////////
// CGearRackDlg dialog

class CGearRackDlg : public CDialog
{
// Construction
public:
	CGearRackDlg(CChildView* pParent);   // standard constructor
	CChildView* m_pParent;
	void RedrawList();
	BOOL Create();
	afx_msg void OnCancel();

// Dialog Data
	//{{AFX_DATA(CGearRackDlg)
	enum { IDD = IDD_GEAR_RACK };
	CListBox	m_list;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGearRackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGearRackDlg)
	afx_msg void OnCreate();
	afx_msg void OnName();
	afx_msg void OnDelete();
	afx_msg void OnSwap();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnDblclkGearlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_)

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
	void ExchangeIns(int one,int two);
	CGearRackDlg(CChildView* pParent, CMainFrame* pMain);   // standard constructor
	CChildView* m_pParent;
	CMainFrame* pParentMain;
	void RedrawList();
	BOOL Create();
	afx_msg void OnCancel();
	static int DisplayMode;

// Dialog Data
	//{{AFX_DATA(CGearRackDlg)
	enum { IDD = IDD_GEAR_RACK };
	CButton	m_props;
	CButton	m_radio_ins;
	CButton	m_radio_gen;
	CButton	m_radio_efx;
	CButton	m_text;
	CListBox	m_list;
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
	afx_msg void OnDelete();
	afx_msg void OnDblclkGearlist();
	afx_msg void OnProperties();
	afx_msg void OnParameters();
	afx_msg void OnSelchangeGearlist();
	afx_msg void OnMachineType();
	afx_msg void OnRadioEfx();
	afx_msg void OnRadioGen();
	afx_msg void OnRadioIns();
	afx_msg void OnExchange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEARRACKDLG_H__87D6EF46_0CF5_430B_BF16_E09467410AD2__INCLUDED_)

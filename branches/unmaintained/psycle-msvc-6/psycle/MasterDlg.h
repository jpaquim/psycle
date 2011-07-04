#if !defined(AFX_MASTERDLG_H__F83FC440_4A98_11D4_8913_E55936BE8F41__INCLUDED_)
#define AFX_MASTERDLG_H__F83FC440_4A98_11D4_8913_E55936BE8F41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MasterDlg.h : header file
//

#include "Machine.h"

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog

class CMasterDlg : public CDialog
{
// Construction
public:
	CMasterDlg(CWnd* pParent = NULL);   // standard constructor
	Master* _pMachine;

// Dialog Data
	//{{AFX_DATA(CMasterDlg)
	enum { IDD = IDD_MASTERDLG };
	CButton	m_autodec;
	CStatic	m_dblevel;
	CSliderCtrl	m_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMasterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMasterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAutodec();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASTERDLG_H__F83FC440_4A98_11D4_8913_E55936BE8F41__INCLUDED_)

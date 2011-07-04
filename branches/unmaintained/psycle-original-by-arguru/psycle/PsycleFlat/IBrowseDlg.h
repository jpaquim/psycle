#if !defined(AFX_IBROWSEDLG_H__681BD120_1C4D_11D4_8913_FE844D8A7E70__INCLUDED_)
#define AFX_IBROWSEDLG_H__681BD120_1C4D_11D4_8913_FE844D8A7E70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IBrowseDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIBrowseDlg dialog

class CIBrowseDlg : public CDialog
{
// Construction
public:
	CIBrowseDlg(CWnd* pParent = NULL);   // standard constructor
	psySong* songRef;
	void UpdateIList();

// Dialog Data
	//{{AFX_DATA(CIBrowseDlg)
	enum { IDD = IDD_IBROWSE };
	CListBox	m_iblist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIBrowseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool anyInstrument;
	int ibIndex[MAX_INSTRUMENTS];
	void ShowIE();
	bool rFocus;
	// Generated message map functions
	//{{AFX_MSG(CIBrowseDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeIbList();
	afx_msg void OnDblclkList();
	afx_msg void OnEditBut();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IBROWSEDLG_H__681BD120_1C4D_11D4_8913_FE844D8A7E70__INCLUDED_)

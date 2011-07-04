#if !defined(AFX_SKINDLG_H__2135ACE0_24EA_11D4_8913_AE4655FCC076__INCLUDED_)
#define AFX_SKINDLG_H__2135ACE0_24EA_11D4_8913_AE4655FCC076__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSkinDlg dialog

class CSkinDlg : public CDialog
{
// Construction
public:
	InitChecks();
	UpdateChecks();
	CSkinDlg(CWnd* pParent = NULL);   // standard constructor

	char mv_pathbuf[_MAX_PATH];
	char pv_pathbuf[_MAX_PATH];
	bool mv_check;
	bool pv_check;
	COLORREF mv_c;
	COLORREF pv_c;
	COLORREF mas_c;
	COLORREF gen_c;
	COLORREF eff_c;
	COLORREF plg_c;
	COLORREF vub_c;
	COLORREF vug_c;
	COLORREF vuc_c;


private:
	bool BrowseTo(char *rpath);

// Dialog Data
	//{{AFX_DATA(CSkinDlg)
	enum { IDD = IDD_CONFIG };
	CButton	m_pv_check;
	CButton	m_mv_check;
	CEdit	m_pv_skin_edit;
	CEdit	m_mv_skin_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkinDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSkinDlg)
	virtual void OnOK( );
	afx_msg void OnBrowseMv();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowsePv();
	afx_msg void OnCheck1();
	afx_msg void OnCheck2();
	afx_msg void OnColourMachine();
	afx_msg void OnButtonPattern();
	afx_msg void OnGenColor();
	afx_msg void OnEffColor();
	afx_msg void OnMasterColor();
	afx_msg void OnVuBarColor();
	afx_msg void OnVuBackColor();
	afx_msg void OnVuClipBar();
	afx_msg void OnColorMisc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINDLG_H__2135ACE0_24EA_11D4_8913_AE4655FCC076__INCLUDED_)

#if !defined(AFX_DIRECTORYDLG_H__E34FBFE3_98A4_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_DIRECTORYDLG_H__E34FBFE3_98A4_11D4_937A_B3D2C98D5E39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DirectoryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDirectoryDlg dialog

class CDirectoryDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CDirectoryDlg)

// Construction
public:
	bool _instPathChanged;
	char _instPathBuf[MAX_PATH];
	bool _songPathChanged;
	char _songPathBuf[MAX_PATH];
	bool _pluginPathChanged;
	char _pluginPathBuf[MAX_PATH];
	bool _vstPathChanged;
	char _vstPathBuf[MAX_PATH];

	CDirectoryDlg();
	~CDirectoryDlg();

// Dialog Data
	//{{AFX_DATA(CDirectoryDlg)
	enum { IDD = IDD_DIRECTORIES };
	CEdit	m_vstEdit;
	CEdit	m_pluginEdit;
	CEdit	m_songEdit;
	CEdit	m_instEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDirectoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool BrowseForFolder(char *rpath);

	// Generated message map functions
	//{{AFX_MSG(CDirectoryDlg)
	afx_msg void OnBrowseInst();
	afx_msg void OnBrowseSong();
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowsePlugin();
	afx_msg void OnBrowseVst();
	afx_msg void OnChangeSongedit();
	afx_msg void OnChangeInstedit();
	afx_msg void OnChangePluginedit();
	afx_msg void OnChangeVstedit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTORYDLG_H__E34FBFE3_98A4_11D4_937A_B3D2C98D5E39__INCLUDED_)
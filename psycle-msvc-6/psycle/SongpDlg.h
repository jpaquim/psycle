#if !defined(AFX_SONGPDLG_H__FCA5A9E3_260E_11D4_8913_AD975B428477__INCLUDED_)
#define AFX_SONGPDLG_H__FCA5A9E3_260E_11D4_8913_AD975B428477__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SongpDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CSongpDlg dialog

class CSongpDlg : public CDialog
{
// Construction
public:
	CSongpDlg(CWnd* pParent = NULL);   // standard constructor
	Song* _pSong;

// Dialog Data
	//{{AFX_DATA(CSongpDlg)
	enum { IDD = IDD_SONGPROP };
	CEdit	m_songcomments;
	CEdit	m_songcredits;
	CEdit	m_songtitle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSongpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSongpDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeTitle();
	afx_msg void OnChangeAuthor();
	afx_msg void OnChangeComment();
	afx_msg void OnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONGPDLG_H__FCA5A9E3_260E_11D4_8913_AD975B428477__INCLUDED_)

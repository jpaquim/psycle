#if !defined(AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_)
#define AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveWavDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg dialog

class CSaveWavDlg : public CDialog
{
// Construction
public:
	CSaveWavDlg(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CSaveWavDlg)
	enum { IDD = IDD_SAVEWAVDLG };
	CButton	m_savewave;
	CButton	m_savewires;
	CButton	m_savesong;
	CEdit	m_rangestart;
	CEdit	m_rangeend;
	CProgressCtrl	m_progress;
	CEdit	m_patnumber;
	CEdit	m_filename;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveWavDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HANDLE thread_handle;
	int kill_thread;
	
	// Generated message map functions
	//{{AFX_MSG(CSaveWavDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFilebrowse();
	afx_msg void OnSelAllSong();
	afx_msg void OnSelRange();
	afx_msg void OnSelPattern();
	afx_msg void OnSavewave();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_)

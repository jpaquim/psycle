#if !defined(AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_)
#define AFX_SAVEWAVDLG_H__E16E4B30_31A9_4555_9087_95FBA7CCF7A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveWavDlg.h : header file
//

#include "constants.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveWavDlg dialog

class CSaveWavDlg : public CDialog
{
// Construction
public:
	void SaveTick(void);
	CSaveWavDlg(CWnd* pParent = NULL);   // standard constructor
	void SaveEnd(void);
	int kill_thread;
	
// Dialog Data
	//{{AFX_DATA(CSaveWavDlg)
	enum { IDD = IDD_SAVEWAVDLG };
	CButton	m_cancel;
	CButton	m_savewave;
	CButton	m_savewires;
	CEdit	m_rangestart;
	CEdit	m_rangeend;
	CProgressCtrl	m_progress;
	CEdit	m_patnumber;
	CEdit	m_filename;
	int		m_recmode;
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

	int lastpostick;
	int lastlinetick;
	int tickcont;

	bool autostop;
	bool playblock;
	bool sel[MAX_SONG_POSITIONS];
	bool saving;
	
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

#pragma once
#include "afxwin.h"


// CAddTrackDlg dialog

class CAddTrackDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddTrackDlg)

public:
	CAddTrackDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddTrackDlg();

// Dialog Data
	enum { IDD = IDD_ADDTRACKDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	CListBox trackList;
	afx_msg void OnBnClickedOk();
};

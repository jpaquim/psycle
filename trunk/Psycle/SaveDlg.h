#if !defined(AFX_SAVEDLG_H__4985DCC7_1B20_11D4_8913_C6A3A71F6C70__INCLUDED_)
#define AFX_SAVEDLG_H__4985DCC7_1B20_11D4_8913_C6A3A71F6C70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveDlg.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CSaveDlg dialog

class CSaveDlg : public CDialog
{
// Construction
public:
	CSaveDlg(CWnd* pParent = NULL);   // standard constructor
	Song* _pSong;
	char szFile[260];       // buffer for file name
	int OpenMode;
	void SaveSong(bool silent=false);
	void LoadSong(bool silent=false);

// Dialog Data
	//{{AFX_DATA(CSaveDlg)
	enum { IDD = IDD_SAVESONG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEDLG_H__4985DCC7_1B20_11D4_8913_C6A3A71F6C70__INCLUDED_)

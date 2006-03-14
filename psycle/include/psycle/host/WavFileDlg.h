#if !defined(AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_)
#define AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileDialogEx1.h : header file
//

class Song;

/////////////////////////////////////////////////////////////////////////////
// CWavFileDlg dialog

class CWavFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CWavFileDlg)

public:
	Song *_pSong;
	CString _lastFile;

	CWavFileDlg(
		BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	virtual void OnFileNameChange();

protected:
	//{{AFX_MSG(CWavFileDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_)

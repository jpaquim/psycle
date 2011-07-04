#if !defined(AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_)
#define AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileDialogEx1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileDialogEx dialog

class CFileDialogEx : public CFileDialog
{
	DECLARE_DYNAMIC(CFileDialogEx)

public:
	CFileDialogEx(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	psySong *songRef;
	
	virtual void OnFileNameChange();
	CString LastFile;
	
protected:
	//{{AFX_MSG(CFileDialogEx)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_)

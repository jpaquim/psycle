#if !defined(AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_)
#define AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_
/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileDialogEx1.h : header file
//


class Song;

/////////////////////////////////////////////////////////////////////////////
// CWavFileDlg dialog

class CWavFileDlg : public WTL::CFileDialogImpl<CWavFileDlg>
{
//	DECLARE_DYNAMIC(CWavFileDlg)

public:
	Song *_pSong;
	CString _lastFile;

	CWavFileDlg(
		BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL);

	void OnSelChange(LPOFNOTIFY lpon);

protected:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEDIALOGEX1_H__ABA3AC60_50FF_11D4_8913_A40B53E13A48__INCLUDED_)

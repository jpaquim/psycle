// FileDialogEx.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "FileDialogEx.h"
#include <dlgs.h>

#define IDC_MYBUTTON	2000
#define IDC_MYSTATIC	2001

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileDialogEx

IMPLEMENT_DYNAMIC(CFileDialogEx, CFileDialog)

CFileDialogEx::CFileDialogEx(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CFileDialogEx, CFileDialog)
	//{{AFX_MSG_MAP(CFileDialogEx)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CFileDialogEx::OnInitDialog() 
{
	CFileDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	// We need to enlarge standard CFileDialog to make space for our controls
	// idea from Christian Skovdal Andersen article - Customizing CFileDialog

	// This variable should be changed acording to your wishes
	const UINT iExtraSize = 10;
	// Get a pointer to the original dialog box.
	CWnd *wndDlg = GetParent();
	RECT Rect;

	wndDlg->GetWindowRect(&Rect);
	// Change the size of FileOpen dialog
	wndDlg->SetWindowPos(NULL, 0, 0, 
                        Rect.right - Rect.left, 
                        Rect.bottom - Rect.top + iExtraSize, 
                        SWP_NOMOVE);

	// Standard CFileDialog control ID's are defined in <dlgs.h>
	// Do not forget to include <dlgs.h> in implementation file

	CWnd *wndComboCtrl = wndDlg->GetDlgItem(cmb1);	// cmb1 - standard file name combo box control
	wndComboCtrl->GetWindowRect(&Rect);
	wndDlg->ScreenToClient(&Rect); // Remember it is child controls

	// Put our control(s) somewhere below HIDDEN checkbox
	// Make space for 3 different subtypes
	Rect.top += 36;
	Rect.bottom += 48;
	
	// Our control is CComboBox object
	// IMPORTANT: We must put wndDlg here as hWndParents, NOT "this" as
	// written in Microsoft documentation example
	// NOTE: IDC_MYCOMBOBOX and IDC_MYSTATIC must be defined (best in resource.h)
	m_AButton.Create("Preview Wave",WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		Rect, wndDlg, IDC_MYBUTTON);

	// save new control font according to other controls font
	m_AButton.SetFont(wndComboCtrl->GetFont(), TRUE);
	m_AButton.SetFocus();
	// We also need Static Control. Get coordinates from stc3 control
	CWnd *wndStaticCtrl = wndDlg->GetDlgItem(stc2);
	wndStaticCtrl->GetWindowRect(&Rect);
	wndDlg->ScreenToClient(&Rect);
	Rect.top += 60;
	Rect.bottom += 80;
	Rect.right += 40;

	return TRUE;
}

void CFileDialogEx::OnDestroy() 
{
	CFileDialog::OnDestroy();
	
	// Validate other data here
	// Called when finished
}

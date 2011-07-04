// FileDialogEx1.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "FileDialogEx1.h"

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
			LastFile="";
}


BEGIN_MESSAGE_MAP(CFileDialogEx, CFileDialog)
	//{{AFX_MSG_MAP(CFileDialogEx)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CFileDialogEx::OnFileNameChange()
{
	CString CurrExt=GetFileExt();

	CurrExt.MakeLower();

	if(CurrExt=="wav" && LastFile!=GetFileName())
	{
		LastFile=GetFileName();

		if(songRef->PW_Stage)
		{
			songRef->PW_Stage=0;
			Sleep(LOCK_LATENCY);
		}
		
		if(songRef->WavAlloc(PREV_WAV_INS,0,LastFile)==1)
		songRef->PW_Play();
	}

	CFileDialog::OnFileNameChange();
}

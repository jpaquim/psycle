#include "stdafx.h"

// FileDialogEx1.cpp : implementation file
//

#include "Psycle.h"
#include "Song.h"
#include "WavFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWavFileDlg

IMPLEMENT_DYNAMIC(CWavFileDlg, CFileDialog)

CWavFileDlg::CWavFileDlg(
						 BOOL bOpenFileDialog,
						 LPCTSTR lpszDefExt,
						 LPCTSTR lpszFileName,
						 DWORD dwFlags,
						 LPCTSTR lpszFilter,
						 CWnd* pParentWnd) 
						 : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	_lastFile='\0';
}


BEGIN_MESSAGE_MAP(CWavFileDlg, CFileDialog)
//{{AFX_MSG_MAP(CWavFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CWavFileDlg::OnFileNameChange()
{
	CString CurrExt=GetFileExt();
	
	CurrExt.MakeLower();
	
	if (_pSong->PW_Stage)
	{
		_pSong->PW_Stage=0;
		Sleep(LOCK_LATENCY);
	}
	
	if (CurrExt=="wav" && _lastFile != GetFileName())
	{
		_lastFile=GetFileName();
		
		if (_pSong->WavAlloc(PREV_WAV_INS,0, _lastFile) == 1)
		{
			_pSong->PW_Play();
		}
	}
	else if (CurrExt=="iff" && _lastFile != GetFileName())
	{
		_lastFile=GetFileName();
		
		if (_pSong->IffAlloc(PREV_WAV_INS,0, _lastFile) == 1)
		{
			_pSong->PW_Play();
		}
	}

	CFileDialog::OnFileNameChange();
}

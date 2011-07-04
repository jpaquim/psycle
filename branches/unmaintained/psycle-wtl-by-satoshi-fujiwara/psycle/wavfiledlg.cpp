/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "Song.h"
#include "WavFileDlg.h"




/////////////////////////////////////////////////////////////////////////////
// CWavFileDlg

//IMPLEMENT_DYNAMIC(CWavFileDlg, CFileDialog)

CWavFileDlg::CWavFileDlg(
						 BOOL bOpenFileDialog,
						 LPCTSTR lpszDefExt,
						 LPCTSTR lpszFileName,
						 DWORD dwFlags,
						 LPCTSTR lpszFilter,
						 HWND hWndParent) 
						 : CFileDialogImpl<CWavFileDlg>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
{
	_lastFile=_T('\0');
}



void CWavFileDlg::OnSelChange(LPOFNOTIFY lpon)
{
	CString CurrExt(this->m_ofn.lpstrDefExt);
	//GetFileExt();
	CurrExt.MakeLower();
	GetFilePath(m_szFileName,sizeof(m_szFileName));
	
	if (_pSong->PW_Stage())
	{
		_pSong->PW_Stage(0);
		Sleep(LOCK_LATENCY);
	}
	
	if (CurrExt == _T("wav") && _lastFile != this->m_szFileName)
	{
		_lastFile = this->m_szFileName;
		
		if (_pSong->WavAlloc(PREV_WAV_INS,0, _lastFile) == 1)
		{
			_pSong->PW_Play();
		}
	}
	else if (CurrExt==_T("iff") && _lastFile != this->m_szFileName)
	{
		_lastFile = this->m_szFileName;
		
		if (_pSong->IffAlloc(PREV_WAV_INS,0, _lastFile) == 1)
		{
			_pSong->PW_Play();
		}
	}

}

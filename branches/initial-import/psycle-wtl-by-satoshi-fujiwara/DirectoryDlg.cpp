/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.5 $
 */
// DirectoryDlg.cpp : implementation file
//


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
#include "DirectoryDlg.h"



/////////////////////////////////////////////////////////////////////////////
// CDirectoryDlg property page

//IMPLEMENT_DYNCREATE(CDirectoryDlg, CPropertyPage)

/*CDirectoryDlg::CDirectoryDlg() : 
	m_vstEdit(this),m_pluginEdit(this),m_songEdit(this),m_instEdit(this),m_skinEdit(this)*/
CDirectoryDlg::CDirectoryDlg()
{
	//{{AFX_DATA_INIT(CDirectoryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_instPathChanged = false;
	_instPathBuf[0] = '\0';
	_songPathChanged = false;
	_songPathBuf[0] = '\0';
	_pluginPathChanged = false;
	_pluginPathBuf[0] = '\0';
	_vstPathChanged = false;
	_vstPathBuf[0] = '\0';
	_skinPathChanged = false;
	_skinPathBuf[0] = '\0';
	initializingDlg = false;
}

CDirectoryDlg::~CDirectoryDlg()
{

}

/////////////////////////////////////////////////////////////////////////////
// CDirectoryDlg message handlers

const bool CDirectoryDlg::BrowseForFolder(TCHAR  * const rpath) 
{
	bool val=false;
	
	LPMALLOC pMalloc;
	// Gets the Shell's default allocator
	//
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		BROWSEINFO bi;
		TCHAR pszBuffer[MAX_PATH];
		LPITEMIDLIST pidl;
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		//
		bi.hwndOwner = m_hWnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		SF::CResourceString _title(IDS_MSG0094);
		bi.lpszTitle = _title;
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
		bi.lpfn = NULL;
		bi.lParam = 0;
		// This next call issues the dialog box.
		//
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
			{
				// At this point pszBuffer contains the selected path
				//
				val = true;
				_stprintf(rpath,pszBuffer);
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			//
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		//
		pMalloc->Release();
	}
	return val;
}

LRESULT CDirectoryDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	initializingDlg = true;
	//SubclassWindow(GetDlgItem(IDC_PROGRESS1))
/*	
	m_instEdit.SubclassWindow(GetDlgItem(IDC_INSTEDIT));
	m_songEdit.SubclassWindow(GetDlgItem(IDC_SONGEDIT));
	m_pluginEdit.SubclassWindow(GetDlgItem(IDC_PLUGINEDIT));
	m_vstEdit.SubclassWindow(GetDlgItem(IDC_VSTEDIT));
	m_skinEdit.SubclassWindow(GetDlgItem(IDC_SKINEDIT));
*/	
	m_instEdit.Attach(GetDlgItem(IDC_INSTEDIT));
	m_songEdit.Attach(GetDlgItem(IDC_SONGEDIT));
	m_pluginEdit.Attach(GetDlgItem(IDC_PLUGINEDIT));
	m_vstEdit.Attach(GetDlgItem(IDC_VSTEDIT));
	m_skinEdit.Attach(GetDlgItem(IDC_SKINEDIT));
	

	m_instEdit.SetWindowText(_instPathBuf);
	m_songEdit.SetWindowText(_songPathBuf);
	m_pluginEdit.SetWindowText(_pluginPathBuf);
	m_vstEdit.SetWindowText(_vstPathBuf);
	m_skinEdit.SetWindowText(_skinPathBuf);
	
//	DoDataExchange(FALSE);
	
	initializingDlg = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CDirectoryDlg::OnEnChangeSongedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。
	if (!initializingDlg)
	{
		_songPathChanged = true;
		m_songEdit.GetWindowText(_songPathBuf,MAX_PATH);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnBnClickedBrowsesong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (BrowseForFolder(_songPathBuf))
	{
		_songPathChanged = true;
		m_songEdit.SetWindowText(_songPathBuf);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnEnChangeInstedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。
	if (!initializingDlg)
	{
		_instPathChanged = true;
		m_instEdit.GetWindowText(_instPathBuf,MAX_PATH);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnBnClickedBrowseinst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (BrowseForFolder(_instPathBuf))
	{
		_instPathChanged = true;
		m_instEdit.SetWindowText(_instPathBuf);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnEnChangePluginedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	if (!initializingDlg)
	{
		_pluginPathChanged = true;
		m_pluginEdit.GetWindowText(_pluginPathBuf,MAX_PATH);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnBnClickedBrowseplugin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (BrowseForFolder(_pluginPathBuf))
	{
		_pluginPathChanged = true;
		m_pluginEdit.SetWindowText(_pluginPathBuf);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnBnClickedBrowsevst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (BrowseForFolder(_vstPathBuf))
	{
		_vstPathChanged = true;
		m_vstEdit.SetWindowText(_vstPathBuf);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnBnClickedBrowseskin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (BrowseForFolder(_skinPathBuf))
	{
		_skinPathChanged = true;
		m_skinEdit.SetWindowText(_skinPathBuf);
	}
	return 0;
}

LRESULT CDirectoryDlg::OnEnChangeVstedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	if (!initializingDlg)
	{
		_vstPathChanged = true;
		m_vstEdit.GetWindowText(_vstPathBuf,MAX_PATH);
	}

	return 0;
}

LRESULT CDirectoryDlg::OnEnChangeSkinedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	if (!initializingDlg)
	{
		_skinPathChanged = true;
		m_skinEdit.GetWindowText(_skinPathBuf,MAX_PATH);
	}
	return 0;
}

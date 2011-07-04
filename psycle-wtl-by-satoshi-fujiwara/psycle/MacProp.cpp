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
#include "MacProp.h"
#include "MainFrm.h"
#include ".\macprop.h"




//extern CPsycleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMacProp dialog


CMacProp::CMacProp()
{
	m_view = NULL;
}

LRESULT CMacProp::OnEnChangeEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_macname.GetWindowText(txt, 32);
	return 0;
}

LRESULT CMacProp::OnBnClickedMute(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pMachine->_mute = (m_muteCheck.GetCheck() == 1);
	pMachine->_volumeCounter=0.0f;
	pMachine->_volumeDisplay = 0;
	if ( m_view != NULL )
	{
		m_view->AddMacViewUndo();
		m_view->updatePar=thisMac;
		m_view->Repaint(DMMacRefresh);
	}
	return 0;
}

LRESULT CMacProp::OnBnClickedSolo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view->AddMacViewUndo();
	 if (m_soloCheck.GetCheck() == 1)
	 {
		for ( int i=0;i<MAX_MACHINES;i++ )
		{
			if (pSong->pMachine(i))
			{
				if ( pSong->pMachine(i)->_mode == MACHMODE_GENERATOR )
				{
					pSong->pMachine(i)->_mute = true;
					pSong->pMachine(i)->_volumeCounter=0.0f;
					pSong->pMachine(i)->_volumeDisplay =0;
				}
			}
		}
		pMachine->_mute = false;
		if ( m_muteCheck.GetCheck() ) m_muteCheck.SetCheck(0);
		pSong->MachineSoloed(thisMac);
	 }
	 else
	 {
		pSong->MachineSoloed(-1);
		for ( int i=0;i<MAX_BUSES;i++ )
		{
			if (pSong->pMachine(i))
			{
				pSong->pMachine(i)->_mute = false;
			}
		}
		if ( m_muteCheck.GetCheck() ) m_muteCheck.SetCheck(0);
	}
	if ( m_view != NULL )
	{
		m_view->Repaint(DMAllMacsRefresh);
	}
	return 0;
}

LRESULT CMacProp::OnBnClickedBypass(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pMachine->_bypass = (m_bypassCheck.GetCheck() == 1);
	if ( m_view != NULL )
	{
		m_view->AddMacViewUndo();
		m_view->updatePar=thisMac;
		m_view->Repaint(DMMacRefresh);
	}
	return 0;
}

LRESULT CMacProp::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Delete MACHINE!
	if (MessageBox(
		SF::CResourceString(IDS_MSG0052),
		SF::CResourceString(IDS_MSG0053), MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
	{
		if ( m_view != NULL )
		{
			m_view->AddMacViewUndo();
		}

		deleted = true;
		EndDialog(IDOK);
	}
	return 0;
}

LRESULT CMacProp::OnBnClickedClone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	int src = pMachine->_macIndex;
	int dst = -1;

	if ((src < MAX_BUSES) && (src >=0))
	{
		// we need to find an empty slot
		for (int i = 0; i < MAX_BUSES; i++)
		{
			if (!Global::_pSong->pMachine(i))
			{
				dst = i;
				break;
			}
		}
	}
	else if ((src < MAX_BUSES*2) && (src >= MAX_BUSES))
	{
		for (int i = MAX_BUSES; i < MAX_BUSES*2; i++)
		{
			if (!Global::_pSong->pMachine(i))
			{
				dst = i;
				break;
			}
		}
	}
	if (dst >= 0)
	{
		if (!Global::_pSong->CloneMac(src,dst))
		{
			MessageBox(SF::CResourceString(IDS_MSG0054),SF::CResourceString(IDS_MSG0054));
		}
		if ( m_view != NULL )
		{
			CMainFrame::GetInstance().UpdateComboGen();
			if (m_view->viewMode==VMMachine)
			{
				m_view->Repaint();
			}
		}
	}
	return 0;
}

LRESULT CMacProp::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	EndDialog(wID);
	return 0;
}

LRESULT CMacProp::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	EndDialog(IDOK);
	return 0;
}

LRESULT CMacProp::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_soloCheck.Attach(GetDlgItem(IDC_SOLO));
	m_bypassCheck.Attach(GetDlgItem(IDC_BYPASS));
	m_muteCheck.Attach(GetDlgItem(IDC_MUTE));
	m_macname.Attach(GetDlgItem(IDC_EDIT1));

	deleted = false;

	m_macname.SetLimitText(31);
	TCHAR buffer[64];
	_stprintf(buffer,SF::CResourceString(IDS_MSG0051),Global::_pSong->FindBusFromIndex(thisMac),pMachine->_editName);
	SetWindowText(buffer);

	m_macname.SetWindowText(pMachine->_editName);

	m_muteCheck.SetCheck(pMachine->_mute);
	m_soloCheck.SetCheck(pSong->MachineSoloed() == thisMac);
	m_bypassCheck.SetCheck(pMachine->_bypass);
	if (pMachine->_mode == MACHMODE_GENERATOR ) 
	{
		m_bypassCheck.ShowWindow(SW_HIDE);
	}
	else 
	{
		m_soloCheck.ShowWindow(SW_HIDE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

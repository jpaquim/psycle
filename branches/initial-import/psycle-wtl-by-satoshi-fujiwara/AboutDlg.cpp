/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
#include "Constants.h"
#include "resource.h"
#include "configuration.h"
#include "aboutdlg.h"
#include ".\aboutdlg.h"



LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	 m_asio.Attach(GetDlgItem(IDC_ASIO));
	 m_sourceforge.Attach(GetDlgItem(IDC_EDIT5));
	 m_psycledelics.Attach(GetDlgItem(IDC_EDIT2));
	 m_steincopyright.Attach(GetDlgItem(IDC_STEINBERGCOPY));
	 m_headerdlg.Attach(GetDlgItem(IDC_HEADERDLG));
	 m_showabout.Attach(GetDlgItem(IDC_SHOWATSTARTUP));
	 m_headercontrib.Attach(GetDlgItem(IDC_HEADER));
	 m_aboutbmp.Attach(GetDlgItem(IDC_ABOUTBMP));
	 m_contrib.Attach(GetDlgItem(IDC_EDIT1));
	 m_versioninfo.Attach(GetDlgItem(IDC_VERSION_INFO));

	 	m_contrib.SetWindowText(_T("\
Josep M Antolin. [JAZ]\tDeveloper since version 1.5\
\r\nJeremy Evers (pooplog)\tDeveloper since version 1.7\
\r\nJohan Boule\t\tDeveloper in version 1.73\
\r\nDaniel Arena\t\tDeveloper in 1.5&1.6\
\r\nMarcin Kowalski / FideLoop\tDeveloper in 1.6\
\r\nMark McCormack\t\tMIDI (in) Support\
\r\nMats Hojlund\t\tMain developer until 1.5 (Internal Recoding)\
\r\nJuan Antonio Arguelles. Arguru\tCreator and Main Developer until 1.0\
\r\nHamarr Heylen\t\tInitial Graphics\
\r\nJames Redfern [yoji]\t\tAdditional Graphics\
\r\nDavid Buist\t\tAdditional Graphics\
\r\nfrown\t\t\tAdditional Graphics\
\r\n/\\/\\ark\t\t\tAdditional Graphics\
\r\nMichael Haralabos\t\tInstaller and Debugging help\
"));
	m_showabout.SetCheck(Global::pConfig->_showAboutAtStart);

	m_psycledelics.SetWindowText(_T("http://psycle.pastnotecut.org/"));
	//m_sourceforge.SetWindowText(_T("http://sourceforge.net/projects/psycle/"));
	m_sourceforge.SetWindowText(_T("http://sourceforge.jp/projects/psyclewtl/"));
		
	CA2T _date(__DATE__);
	m_versioninfo.SetWindowText(
		(SF::tformat(_T("version %s  [%s]")) % VERSION_NUMBER % _date).str().data()	
		);

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT CAboutDlg::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( m_aboutbmp.IsWindowVisible() )
	{
		m_aboutbmp.ShowWindow(SW_HIDE);
		m_contrib.ShowWindow(SW_SHOW);
		m_headercontrib.ShowWindow(SW_SHOW);
		m_headerdlg.ShowWindow(SW_SHOW);
		m_psycledelics.ShowWindow(SW_SHOW);
		m_sourceforge.ShowWindow(SW_SHOW);
		m_asio.ShowWindow(SW_SHOW);
		m_steincopyright.ShowWindow(SW_SHOW);
	}
	else 
	{
		m_aboutbmp.ShowWindow(SW_SHOW);
		m_contrib.ShowWindow(SW_HIDE);
		m_headercontrib.ShowWindow(SW_HIDE);
		m_headerdlg.ShowWindow(SW_HIDE);
		m_psycledelics.ShowWindow(SW_HIDE);
		m_sourceforge.ShowWindow(SW_HIDE);
		m_asio.ShowWindow(SW_HIDE);
		m_steincopyright.ShowWindow(SW_HIDE);
	}
	return 0;
}

LRESULT CAboutDlg::OnBnClickedShowatstartup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	if ( m_showabout.GetCheck() )  
		Global::pConfig->_showAboutAtStart = true;
	else 
		Global::pConfig->_showAboutAtStart=false;
	return 0;
}

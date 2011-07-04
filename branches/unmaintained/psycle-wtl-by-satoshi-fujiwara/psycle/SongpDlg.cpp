/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
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
#include "Instrument.h"
#include "SongpDlg.h"
#include ".\songpdlg.h"



/////////////////////////////////////////////////////////////////////////////
// CSongpDlg dialog


CSongpDlg::CSongpDlg()
{
}


/////////////////////////////////////////////////////////////////////////////
// CSongpDlg message handlers


LRESULT CSongpDlg::OnEnChangeEdit1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CSongpDlg>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	m_songtitle.GetWindowText(const_cast<TCHAR *>(_pSong->Name().c_str()),64);
	return 0;
}

LRESULT CSongpDlg::OnEnChangeEdit3(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CSongpDlg>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	m_songcredits.GetWindowText(const_cast<TCHAR *>(_pSong->Author().c_str()),Song::MAX_AUTHOR_LEN);

	return 0;
}

LRESULT CSongpDlg::OnEnChangeEdit4(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CSongpDlg>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	m_songcomments.GetWindowText(const_cast<TCHAR *>(_pSong->Comment().c_str()),Song::MAX_COMMENT_LEN);

	return 0;
}

LRESULT CSongpDlg::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(0);
	return 0;
}

LRESULT CSongpDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_songcomments.Attach(GetDlgItem(IDC_EDIT4));
	m_songcredits.Attach(GetDlgItem(IDC_EDIT3));
	m_songtitle.Attach(GetDlgItem(IDC_EDIT1));

	m_songtitle.SetLimitText(64);
	m_songcredits.SetLimitText(64);
	m_songcomments.SetLimitText(256);

	m_songtitle.SetWindowText(_pSong->Name().c_str());
	m_songcredits.SetWindowText(_pSong->Author().c_str());
	m_songcomments.SetWindowText(_pSong->Comment().c_str());

	m_songtitle.SetFocus();
	m_songtitle.SetSel(0,-1);

	return TRUE;
}

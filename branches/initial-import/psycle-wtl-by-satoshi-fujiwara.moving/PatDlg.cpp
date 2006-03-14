/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $ */
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
#include "constants.h"
#include "PatDlg.h"
#include ".\patdlg.h"




/////////////////////////////////////////////////////////////////////////////
// CPatDlg dialog


CPatDlg::CPatDlg()
{
	m_adaptsize = FALSE;
	bInit = FALSE;
}


void CPatDlg::OnUpdateNumLines() 
{
	// TODO: Add your control notification handler code here
	TCHAR buffer[256];
	if (bInit)
	{
		m_numlines.GetWindowText(buffer,16);
		int val=_tstoi(buffer);

		if (val < 0)
		{
			val = 0;
		}
		else if(val > MAX_LINES)
		{
			val = MAX_LINES-1;
		}
		_stprintf(buffer,_T("HEX: %x"),val);
		m_text.SetWindowText(buffer);
	}
}

LRESULT CPatDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_adaptsizeCheck.Attach(GetDlgItem(IDC_CHECK1));
	m_numlines.Attach(GetDlgItem(IDC_EDIT2));
	m_patname.Attach(GetDlgItem(IDC_EDIT1));
	m_spinlines.Attach(GetDlgItem(IDC_SPIN1));
	m_text.Attach(GetDlgItem(IDC_TEXT));

	m_spinlines.SetRange(1,MAX_LINES);
	m_patname.SetWindowText(patName);
	m_patname.SetLimitText(30);

	TCHAR buffer[16];
	_itot(patLines,buffer,10);
	m_numlines.SetWindowText(buffer);

	UDACCEL acc;
	acc.nSec = 4;
	acc.nInc = 16;
	m_spinlines.SetAccel(1, &acc);

	// Pass the focus to the texbox
	m_patname.SetFocus();
	m_patname.SetSel(0,-1);
	bInit = TRUE;
	OnUpdateNumLines();
	CenterWindow();
	return TRUE;
}

LRESULT CPatDlg::OnBnClickedCheck1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_adaptsize = m_adaptsizeCheck.GetCheck();

	return 0;
}

//LRESULT CPatDlg::OnEnChangeEdit2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
//	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
//	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
//	// コントロールは、この通知を送信しません。
//
//	// TODO :  ここにコントロール通知ハンドラ コードを追加してください。
//
//	return 0;
//}

LRESULT CPatDlg::OnEnUpdateEdit2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、__super::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// OR 状態の ENM_CORRECTTEXT フラグを IParam マスクのに入れて、
	// OR 状態の ENM_UPDATE フラグを lParam マスクに入れて、

	// TODO :  ここにコントロール通知ハンドラ コードを追加してください。
	OnUpdateNumLines(); 
	return TRUE;
}

LRESULT CPatDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buffer[32];
	m_numlines.GetWindowText(buffer,16);
	
	int nlines = _tstoi(buffer);

	if (nlines < 1)
		{ nlines = 1; }
	else if (nlines > MAX_LINES)
		{ nlines = MAX_LINES; }

	patLines=nlines;

	m_patname.GetWindowText(buffer,31);
	buffer[31]=_T('\0');

	_tcscpy(patName,buffer);
	
	this->EndDialog(wID);
	return 0;
}

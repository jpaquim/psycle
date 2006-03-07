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

#include "VolumeDlg.h"
#include <math.h>
#include ".\volumedlg.h"




/////////////////////////////////////////////////////////////////////////////
// CVolumeDlg dialog


CVolumeDlg::CVolumeDlg()
{
	go = false;
}

void CVolumeDlg::DrawDb() 
{
	TCHAR bufdb[32];

	if (volume > 4.0f)
	{
		volume = 4.0f;
	}
	if (volume > 1.0f)
	{	
		_stprintf(bufdb,_T("+%.1f"),20.0f * log10(volume)); 
	}
	else if (volume == 1.0f)
	{	
		_stprintf(bufdb,_T("0.0")); 
	}
	else if (volume > 0.0f)
	{	
		_stprintf(bufdb,_T("%.1f"),20.0f * log10(volume)); 
	}
	else 
	{				
		volume = 0.0f;
		_stprintf(bufdb,_T("-999.9")); 
	}
	go = false;
	m_db.SetWindowText(bufdb);
	go = true;
}

void CVolumeDlg::DrawPer() 
{
	TCHAR bufper[32];

	if (volume > 4.0f)
	{
		volume = 4.0f;
	}
	if (volume > 1.0f)
	{	
		_stprintf(bufper,_T("%.2f%"),volume*100); 
	}
	else if (volume == 1.0f)
	{	
		_stprintf(bufper,_T("100.00%")); 
	}
	else if (volume > 0.0f)
	{	
		_stprintf(bufper,_T("%.2f"),volume*100); 
	}
	else 
	{
		volume = 0.0f;
		_stprintf(bufper,_T("0.00")); 
	}
	go = false;
	m_per.SetWindowText(bufper);
	go = true;
}




LRESULT CVolumeDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(0);
	return 0;
}

LRESULT CVolumeDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(0);
	return 0;
}

LRESULT CVolumeDlg::OnEnChangeEditDb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CVolumeDlg>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	if (go)
	{
		TCHAR buf[32];
		m_db.GetWindowText(buf,31);

		float val = float(_tstof(buf));
		volume = powf(10.0,val/20.0f);
		DrawPer();	
	}
	return 0;
}

LRESULT CVolumeDlg::OnEnChangeEditPer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CVolumeDlg>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	if (go)
	{
		TCHAR buf[32];
		m_per.GetWindowText(buf,31);

		float val = float(_tstof(buf));

		volume = val/100.0f;
		DrawDb();
	}

	return 0;
}

LRESULT CVolumeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_db.Attach(GetDlgItem(IDC_EDIT_DB));
	m_per.Attach(GetDlgItem(IDC_EDIT_PER));
	
	// TODO: Add extra initialization here

	DrawPer();
	DrawDb();

	if (edit_type)
	{
		m_per.SetFocus();
		m_per.SetSel(0,32);
	}
	else
	{
		m_db.SetFocus();
		m_db.SetSel(0,32);
	}
	go = true;
	
	//return FALSE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
	return TRUE;
}

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

#include "SwingFillDlg.h"




/////////////////////////////////////////////////////////////////////////////
// CSwingFillDlg dialog


CSwingFillDlg::CSwingFillDlg()
{
}

LRESULT CSwingFillDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_Tempo.Attach(GetDlgItem(IDC_CENTER_TEMPO));
	m_Width.Attach(GetDlgItem(IDC_WIDTH));
	m_Variance.Attach(GetDlgItem(IDC_VARIANCE));
	m_Phase.Attach(GetDlgItem(IDC_PHASE));
	m_Offset.Attach(GetDlgItem(IDC_OFFSET));

	bGo = FALSE;
	
	TCHAR buf[32];
	_stprintf(buf,_T("%d"),tempo);
	m_Tempo.SetWindowText(buf);
	m_Tempo.SetSel(-1,-1,false);

	_stprintf(buf,_T("%d"),width);
	m_Width.SetWindowText(buf);
	m_Width.SetSel(-1,-1,false);

	_stprintf(buf,_T("%.2f"),variance);
	m_Variance.SetWindowText(buf);
	m_Variance.SetSel(-1,-1,false);
	
	_stprintf(buf,_T("%.2f"),phase);
	m_Phase.SetWindowText(buf);
	m_Phase.SetSel(-1,-1,false);

	m_Offset.SetCheck(offset?1:0);

	return TRUE;
}

LRESULT CSwingFillDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bGo = TRUE;
	TCHAR buf[32];
	m_Tempo.GetWindowText(buf,32);
	tempo=_tstoi(buf);
	if (tempo < 33)
		tempo = 33;
	else if (tempo > 999)
		tempo = 999;

	m_Width.GetWindowText(buf,32);
	width=_tstoi(buf);
	if (width < 1)
		width = 1;

	m_Variance.GetWindowText(buf,32);
	variance=float(_tstof(buf));

	m_Phase.GetWindowText(buf,32);
	phase=float(_tstof(buf));

	offset = m_Offset.GetCheck()?true:false;
	EndDialog(wID);
	return 0;
}

LRESULT CSwingFillDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

//LRESULT CSwingFillDlg::OnBnClickedOffset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//
//	return 0;
//}

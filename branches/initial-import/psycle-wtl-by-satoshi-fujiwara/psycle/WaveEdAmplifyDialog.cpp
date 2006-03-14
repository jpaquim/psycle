/** @file 
 *  @brief
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

#include "WaveEdAmplifyDialog.h"
#include "Helpers.h"
#include ".\waveedamplifydialog.h"



/////////////////////////////////////////////////////////////////////////////
// CWaveEdAmplifyDialog dialog


CWaveEdAmplifyDialog::CWaveEdAmplifyDialog()
{
}


LRESULT CWaveEdAmplifyDialog::OnNMCustomdrawSlider3(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	char tmp[8];
	float db;
	db =  (float) (m_slider.GetPos() - 9600) * 0.01f;
	_gcvt(db ,4 ,tmp);
	CA2T _tmp(tmp);
	if (tmp[_tcslen(_tmp) - 1] == _T('.')) 
		tmp[_tcslen(_tmp) - 1] = _T('\0'); //Remove dot if last.
	
	m_dbedit.SetWindowText(_tmp);
	return 0;
}

LRESULT CWaveEdAmplifyDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
	m_dbedit.Detach();
	m_slider.Detach();

	m_dbedit.Attach(GetDlgItem(IDC_EDIT2));
	m_slider.Attach(GetDlgItem(IDC_SLIDER3));

	m_slider.SetRange(0, 14400); 	// Don't use (-,+) range. It fucks up with the "0"
	m_slider.SetPos(9600);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CWaveEdAmplifyDialog::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR db_t[8];
	int db_i = 0;
	m_dbedit.GetWindowText(db_t,8);
	db_i = (int)(100 * _tstof(db_t));
	if (db_i) 
		EndDialog( db_i );
	else 
		EndDialog( AMP_DIALOG_CANCEL);

	return 0;
}

LRESULT CWaveEdAmplifyDialog::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog( AMP_DIALOG_CANCEL );
	return 0;
}

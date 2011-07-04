/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */
// GearTracker.cpp : implementation file
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

#include "GearTracker.h"
#include "PsycleWTLView.h"
#include ".\geartracker.h"



/////////////////////////////////////////////////////////////////////////////
// CGearTracker dialog


CGearTracker::CGearTracker()
{
}


void CGearTracker::OnCancel()
{
	m_pParent->SamplerMachineDialog = NULL;
	DestroyWindow();
}

LRESULT CGearTracker::OnNMCustomdrawTrackslider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// Assign new value
	BOOL _bError = FALSE;
	_pMachine->_numVoices = m_polyslider.GetPos();

	for(int c = 0; c<SAMPLER_MAX_POLYPHONY; c++)
	{
		_pMachine->NoteOffFast(c);
	}

	// Label on dialog display
	TCHAR buffer[8];
	_stprintf(buffer, _T("%d"), _pMachine->_numVoices);
	m_polylabel.SetWindowText(buffer);

	return 0;
}

LRESULT CGearTracker::OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pMachine->_resampler.SetQuality((ResamplerQuality)m_interpol.GetCurSel());

	return 0;
}

LRESULT CGearTracker::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExecuteDlgInit(IDD);
	m_interpol.Attach(GetDlgItem(IDC_COMBO1));
	m_polyslider.Attach(GetDlgItem(IDC_TRACKSLIDER2));
	m_polylabel.Attach(GetDlgItem(IDC_TRACKLABEL2));

	m_interpol.AddString(SF::CResourceString(IDS_MSG0035));
	m_interpol.AddString(SF::CResourceString(IDS_MSG0036));
	m_interpol.AddString(SF::CResourceString(IDS_MSG0037));

	m_interpol.SetCurSel(_pMachine->_resampler._quality);

	SetWindowText(_pMachine->_editName);

	m_polyslider.SetRange(2, SAMPLER_MAX_POLYPHONY);
	m_polyslider.SetPos(_pMachine->_numVoices);
	return TRUE;
}

void CGearTracker::OnFinalMessage(HWND hWnd)
{
	// TODO : ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	CDialogImpl<CGearTracker>::OnFinalMessage(hWnd);
	delete this;
}

LRESULT CGearTracker::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	OnCancel();
	return 0;
}

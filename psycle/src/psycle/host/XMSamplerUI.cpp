/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
// GearTracker.cpp : implementation file
//

#include "stdafx.h"
// STL
#include <algorithm>
#include <cctype>
// #include <boost/format.hpp> //already in stdafx.h
#include <sstream>

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
#include "resource.h"
#define rdtsc __asm __emit 0fh __asm __emit 031h
#define cpuid __asm __emit 0fh __asm __emit 0a2h
#include <memory>
#include "sfhelper.h"
//#include "Psycle2.h"
#include "d3d.h"
#include "XMInstrument.h"
#include "volume.h"
#include "XMSamplerUI.h"
#include "XMSampler.h"
#include "PsycleWTLView.h"
#include ".\geartracker.h"

/////////////////////////////////////////////////////////////////////////////
// XMSamplerUI dialog

namespace SF {

XMSamplerUI::XMSamplerUI( CPsycleWTLView* const parent,XMSampler * const pMachine,ATL::_U_STRINGorID title , 
					   UINT uStartPage , HWND hWndParent) :
CPropertySheetImpl<XMSamplerUI> ( title, uStartPage, hWndParent )
{
	_pMachine = pMachine;
	m_pParent = parent;
	m_General.pMachine(pMachine);
	m_Instrument.pMachine(pMachine);
	m_Sample.pMachine(pMachine);
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	AddPage(m_General);
	AddPage(m_Instrument);
	AddPage(m_Sample);
}

//XMSamplerUI::XMSamplerUI()
//{
//
//}


void XMSamplerUI::OnCancel()
{
	m_pParent->XMSamplerMachineDialog = NULL;
//	DestroyWindow();
	DestroyWindow();
}

LRESULT XMSamplerUI::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
//	m_General.pMachine(m_pMachine);

//	ExecuteDlgInit(IDD);
	//AddPage(m_General);
	//m_interpol.Attach(GetDlgItem(IDC_COMBO1));
	//m_polyslider.Attach(GetDlgItem(IDC_TRACKSLIDER2));
	//m_polylabel.Attach(GetDlgItem(IDC_TRACKLABEL2));

	//m_interpol.AddString(_T("無し   [低品質]"));
	//m_interpol.AddString(_T("ﾘﾆｱ    [中品質]"));
	//m_interpol.AddString(_T("ｽﾌﾟﾗｲﾝ [高品質]"));

	//m_interpol.SetCurSel(_pMachine->_resampler._quality);

	SetWindowText(_pMachine->_editName);
	UpdateWindow();
	CenterWindow();
	//m_polyslider.SetRange(2, XMSampler::MAX_POLYPHONY);
	//m_polyslider.SetPos(_pMachine->_numVoices);
	return TRUE;
}

void XMSamplerUI::OnFinalMessage(HWND hWnd)
{
	// TODO : ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	CPropertySheetImpl<XMSamplerUI>::OnFinalMessage(hWnd);

	delete this;
}

LRESULT XMSamplerUI::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	OnCancel();
	return 0;
}

		}
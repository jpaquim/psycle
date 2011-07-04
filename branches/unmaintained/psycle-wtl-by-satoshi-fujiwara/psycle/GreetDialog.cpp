/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
// GreetDialog.cpp : implementation file
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
#include "GreetDialog.h"
#include ".\greetdialog.h"


/////////////////////////////////////////////////////////////////////////////
// CGreetDialog dialog


CGreetDialog::CGreetDialog()
{
	//{{AFX_DATA_INIT(CGreetDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

LRESULT CGreetDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_greetz.Attach(GetDlgItem(IDC_LIST1));
	m_greetz.AddString(_T("All the people in the Forums"));
	m_greetz.AddString(_T("All at #psycle [EFNET]"));

	m_greetz.AddString(_T("Felix Petrescu 'WakaX'"));
	m_greetz.AddString(_T("Gerwin / FreeH2o"));
	m_greetz.AddString(_T("ksn [Psycledelics WebMaster]"));
	m_greetz.AddString(_T("[SAS] SOLARiS [project-psy WebMaster]"));
	m_greetz.AddString(_T("Pikari"));
	m_greetz.AddString(_T("CyanPhase [for porting VibraSynth]"));
	m_greetz.AddString(_T("Vir|us"));
	m_greetz.AddString(_T("Bohan"));
	m_greetz.AddString(_T("Imagineer"));
	m_greetz.AddString(_T("TAo-AAS"));
	m_greetz.AddString(_T("BigTick [for his excellent VST's]"));
	m_greetz.AddString(_T("Drax_D [for asking to be here ;·D]"));
	m_greetz.AddString(_T("TimEr [Site Graphics and more]"));
	m_greetz.AddString(_T("Felix Kaplan\\Spirit Of India"));
	m_greetz.AddString(_T("Krzysztof Foltman / fsm [Coding help]"));
	m_greetz.AddString(_T("krokpitr"));
	m_greetz.AddString(_T("sampler"));
	m_greetz.AddString(_T("bdzld"));
	m_greetz.AddString(_T("LegoStar [asio]"));
	m_greetz.AddString(_T("KooPer"));
	m_greetz.AddString(_T("pooplog [Machines and Code]"));
	m_greetz.AddString(_T("dj_d [Beta testing]"));
	m_greetz.AddString(_T("Alk/yoji [Extreme testing]"));
	m_greetz.AddString(_T("_sue_ [Extreme testing]"));
	m_greetz.AddString(_T("Druttis [Machines]"));

	return TRUE;
}

LRESULT CGreetDialog::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(TRUE);
	return 0;
}

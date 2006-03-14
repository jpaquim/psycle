/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.6 $
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
#include "DefaultVstGui.h"
#include "Helpers.h"
#include "PsycleWTLView.h"
#include "configuration.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = __FILE__;
#endif
*/


/////////////////////////////////////////////////////////////////////////////
// CDefaultVstGui

//IMPLEMENT_DYNCREATE(CDefaultVstGui, CFormView)

CDefaultVstGui::CDefaultVstGui()
{
}

CDefaultVstGui::~CDefaultVstGui()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDefaultVstGui diagnostics

////////////////////////////////////////////////////////////////////////////
// CDefaultVstGui message handlers

void CDefaultVstGui::Init() 
{
	UpdateParList();
	InitializePrograms();
	//init slider range
	m_slider.SetRangeMin(0);
	m_slider.SetRangeMax(VST_QUANTIZATION);
	m_nPar = 0;
	UpdateOne();
}

void CDefaultVstGui::InitializePrograms()
{
	m_program.ResetContent();

	const int nump = m_pMachine->NumPrograms();
	for (int i=0; i < nump; i++)
	{
		TCHAR s1[256];
		char s2[256];
		strcpy(s2, "<unnamed>");

		int categories = m_pMachine->Dispatch(effGetNumProgramCategories, 0, 0, NULL, 0);
		m_pMachine->Dispatch(effGetProgramNameIndexed, i, -1, s2, 0);

		_stprintf(s1,_T("%d: %s"),i + 1,CA2T(s2));
		m_program.AddString(s1);
	}
	m_program.SetCurSel(m_pMachine->Dispatch(effGetProgram, 0, 0, NULL, 0));
}

void CDefaultVstGui::UpdateParList()
{
	m_parlist.ResetContent();

	const long params = m_pMachine->NumParameters();
	for (int i=0; i<params; i++)
	{
		char str[128], buf[128];

		memset(str,0,64);
		m_pMachine->Dispatch(effGetParamName, i, 0, str, 0);
		if (m_pMachine->Dispatch(effCanBeAutomated, i, 0, NULL, 0))
		{
			sprintf(buf, "(A)%.3X: %s", i, str);
		}
		else
		{
			sprintf(buf, "(_)%.3X: %s", i, str);
		}
		m_parlist.AddString(CA2T(buf));
	}
	
	m_nPar=0;
	m_parlist.SetCurSel(0);
}

void CDefaultVstGui::UpdateText(const float value)
{
	TCHAR str[512] = {0};
	
//	TCHAR str2[32];
//	TCHAR str3[512];
	
	m_pMachine->DescribeValue(m_nPar,str);
//	mbstowcs(str3,str,512);
//	_stprintf(str2,_T("\t[Hex: %4X]"),f2i(value * 65535.0f));
//	_tcscat(str,str2);
	m_text.SetWindowText(
		(SF::tformat(_T("%s\t[Hex: %4X]")) % str % f2i(value * 65535.0f)).str().data()
	);
}

void CDefaultVstGui::UpdateOne()
{
	//update scroll bar with initial value
	float value = m_pMachine->GetParameter(m_nPar);
	UpdateText(value);
	value *= VST_QUANTIZATION;
	m_bUpdatingValue = true;
	m_slider.SetPos(VST_QUANTIZATION - (f2i(value)));
	m_bUpdatingValue = false;
}

void CDefaultVstGui::UpdateNew(const int par,const float value)
{
	if (par != m_nPar )
	{
		m_nPar = par;
		m_parlist.SetCurSel(par);
	}
	UpdateText(value);
	float _value = value * VST_QUANTIZATION;
	m_bUpdatingValue = true;
	m_slider.SetPos(VST_QUANTIZATION - (f2i(_value)));
	m_bUpdatingValue = false;
}


LRESULT CDefaultVstGui::OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int const se=m_program.GetCurSel();
	m_pMachine->SetCurrentProgram(se);
	UpdateOne();
	return 0;
}

LRESULT CDefaultVstGui::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_program.Attach(GetDlgItem(IDC_COMBO1));
	m_slider.Attach(GetDlgItem(IDC_SLIDER1));
	m_text.Attach(GetDlgItem(IDC_TEXT1));
	m_parlist.Attach(GetDlgItem(IDC_LIST1));
	ExecuteDlgInit(IDD);
	bHandled = FALSE;
	return 0;
}

LRESULT CDefaultVstGui::OnNMCustomdrawSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	if (!m_bUpdatingValue)
	{
		int val1 = VST_QUANTIZATION - m_slider.GetPos();
		float value = (float)val1 / VST_QUANTIZATION;

		m_pMachine->SetParameter(m_nPar, value);
		UpdateText(value);
		// well, this isn't so hard... just put the twk record here
		if (Global::pConfig->_RecordTweaks)
		{
			ATLASSERT(m_pWindow != NULL);
			if (Global::pConfig->_RecordMouseTweaksSmooth)
			{
				m_pView->MousePatternTweakSlide(m_MachineIndex, m_nPar, val1);
			}
			else
			{
				m_pView->MousePatternTweak(m_MachineIndex, m_nPar, val1);
			}
		}
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CDefaultVstGui::OnDeltaposSpin1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	//NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int const se = m_program.GetCurSel() + pNMUpDown->iDelta;

	if (se >= 0)
	{
		m_program.SetCurSel(se);
		m_pMachine->SetCurrentProgram(se);
	}

	m_pWindow->SetFocus();
	bHandled = FALSE;

	return 0;
}

LRESULT CDefaultVstGui::OnNMReleasedcaptureSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	m_pWindow->SetFocus();
	bHandled = FALSE;
	return 0;
}

LRESULT CDefaultVstGui::OnCbnCloseupCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	ATLASSERT(m_pWindow != NULL);
	m_pWindow->SetFocus();
	bHandled = FALSE;
	return 0;
}

LRESULT CDefaultVstGui::OnLbnSelchangeList1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	m_nPar=m_parlist.GetCurSel();
	UpdateOne();
	ATLASSERT(m_pWindow != NULL);
	m_pWindow->SetFocus();
	bHandled = FALSE;
	return 0;
}

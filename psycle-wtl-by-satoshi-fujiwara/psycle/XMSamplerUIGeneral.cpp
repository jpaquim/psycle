/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
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

#include "d3d.h"
#include "XMSamplerUIGeneral.h"
#include "XMInstrument.h"
#include "XMSampler.h"
#include "PsycleWTLView.h"
#include ".\geartracker.h"
#include ".\xmsampleruigeneral.h"


/////////////////////////////////////////////////////////////////////////////
// XMSamplerUIGeneral dialog


namespace SF {
XMSamplerUIGeneral::XMSamplerUIGeneral()
{
	m_bInitialize = false;
}


void XMSamplerUIGeneral::OnCancel()
{
	m_pParent->XMSamplerMachineDialog = NULL;
	DestroyWindow();
}

LRESULT XMSamplerUIGeneral::OnNMCustomdrawTrackslider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// Assign new value
	BOOL _bError = FALSE;
	_pMachine->NumVoices(m_polyslider.GetPos());

	for(int c = 0; c < XMSampler::MAX_POLYPHONY; c++)
	{
		_pMachine->rVoice(c).NoteOffFast();
	}

	// Label on dialog display
	m_polylabel.SetWindowText(boost::lexical_cast<string>(_pMachine->NumVoices()).data());

	return 0;
}

LRESULT XMSamplerUIGeneral::OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pMachine->ResamplerQuality((ResamplerQuality)m_interpol.GetCurSel());

	return 0;
}

LRESULT XMSamplerUIGeneral::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExecuteDlgInit(IDD);
	m_interpol.Attach(GetDlgItem(IDC_COMBO1));
	m_polyslider.Attach(GetDlgItem(IDC_TRACKSLIDER2));
	m_polylabel.Attach(GetDlgItem(IDC_TRACKLABEL2));
	
	m_Tempo.Attach(GetDlgItem(IDC_EDIT_TEMPO));
	m_Speed.Attach(GetDlgItem(IDC_EDIT_SPEED));
	//m_GlobalVolume.Attach(GetDlgItem(IDC_EDIT_GLOBALVOL));


	m_interpol.AddString(SF::CResourceString(IDS_MSG0035));
	m_interpol.AddString(SF::CResourceString(IDS_MSG0036));
	m_interpol.AddString(SF::CResourceString(IDS_MSG0037));

	m_interpol.SetCurSel(_pMachine->ResamplerQuality());

	SetWindowText(_pMachine->_editName);

	m_polyslider.SetRange(2, XMSampler::MAX_POLYPHONY);
	m_polyslider.SetPos(_pMachine->NumVoices());
	
	
	m_Tempo.SetWindowText(boost::lexical_cast<string>(_pMachine->BPM()).c_str());
	m_Speed.SetWindowText(boost::lexical_cast<string>(_pMachine->TicksPerRow()).c_str());
	m_bInitialize = true;
//	m_Speed.SetWindowText((boo
	return TRUE;
}

//void XMSamplerUIGeneral::OnFinalMessage(HWND hWnd)
//{
//	// TODO : ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
//	//CDialogImpl<XMSamplerUIGeneral>::OnFinalMessage(hWnd);
//	//delete this;
//}

//LRESULT XMSamplerUIGeneral::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//{
//	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
//	OnCancel();
//	return 0;
//}
}

LRESULT SF::XMSamplerUIGeneral::OnEnChangeEditSpeed(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<XMSamplerUIGeneral>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	// TODO :  ここにコントロール通知ハンドラ コードを追加してください。
	if(!m_bInitialize)
	{
		return 0;
	}

	TCHAR buf[256];
	m_Speed.GetWindowText(buf,256);
	_pMachine->TicksPerRow(boost::lexical_cast<int>(buf));
	_pMachine->CalcBPMAndTick();

	return 0;
}

LRESULT SF::XMSamplerUIGeneral::OnEnChangeEditTempo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<XMSamplerUIGeneral>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。
	if(!m_bInitialize)
	{
		return 0;
	}
	// TODO :  ここにコントロール通知ハンドラ コードを追加してください。
	TCHAR buf[256];
	m_Tempo.GetWindowText(buf,256);
	_pMachine->BPM(boost::lexical_cast<int>(buf));
	_pMachine->CalcBPMAndTick();

	return 0;
}




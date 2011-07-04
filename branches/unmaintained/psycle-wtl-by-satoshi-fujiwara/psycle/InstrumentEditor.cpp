/** @file
 *  @brief implementation file
 *  $Date: 2004/10/07 21:27:21 $
 *  $Revision: 1.3 $
 */
// InstrumentEditor.cpp : implementation file
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
#include "MainFrm.h"
#include "InstrumentEditor.h"
#include "Instrument.h"
#include "EnvDialog.h"
#include "constants.h"
//#include ".\instrumenteditor.h"
//#include "io.h"
//#include "direct.h"

/////////////////////////////////////////////////////////////////////////////
// CInstrumentEditor dialog

CInstrumentEditor::CInstrumentEditor()
{
	//{{AFX_DATA_INIT(CInstrumentEditor)
	//}}AFX_DATA_INIT
}


//////////////////////////////////////////////////////////////////////
// Auxiliar members

void CInstrumentEditor::WaveUpdate()
{
	if(_pSong->WaveSelected() < 0) 
		_pSong->WaveSelected(0);
	if(_pSong->WaveSelected() >= MAX_WAVES)
		_pSong->WaveSelected(MAX_WAVES - 1);

	const int sw = _pSong->WaveSelected();
	const int si = _pSong->InstSelected();

	TCHAR buffer[64];
	// Set instrument current selected label
	_stprintf(buffer, _T("%.2X"), si);
	m_instlabel.SetWindowText(buffer);

	initializingDialog=true;
	// Set instrument current selected name
	m_instname.SetWindowText(_pSong->pInstrument(si)->_sName);
	initializingDialog=false; // This prevents that _T("OnChangeInstname()"), calls _T("UpdateComboIns()")

	UpdateCombo();

	m_panslider.SetPos(_pSong->pInstrument(si)->_pan);
	m_rpan_check.SetCheck(_pSong->pInstrument(si)->_RPAN);
	m_rcut_check.SetCheck(_pSong->pInstrument(si)->_RCUT);
	m_rres_check.SetCheck(_pSong->pInstrument(si)->_RRES);
	
	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->_pan);
	m_panlabel.SetWindowText(buffer);
	

	bool const ils = _pSong->pInstrument(si)->_loop;

	m_loopcheck.SetCheck(ils);
	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->_lines);
	m_loopedit.EnableWindow(ils);
	m_loopedit.SetWindowText(buffer);

	// Volume bar
	m_volumebar.SetPos(_pSong->pInstrument(si)->waveVolume[sw]);
	m_finetune.SetPos(_pSong->pInstrument(si)->waveFinetune[sw]+256);

	UpdateNoteLabel();	
	
	
	// Set looptype
	if(_pSong->pInstrument(si)->waveLoopType[sw])
		_stprintf(buffer,SF::CResourceString(IDS_MSG0049));
	else
		_stprintf(buffer,SF::CResourceString(IDS_MSG0050));
	
	m_looptype.SetWindowText(buffer);

	// Display Loop Points & Wave Length
	
	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->waveLoopStart[sw]);
	m_loopstart.SetWindowText(buffer);

	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->waveLoopEnd[sw]);
	m_loopend.SetWindowText(buffer);

	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->waveLength[sw]);
	m_wlen.SetWindowText(buffer);

}

//////////////////////////////////////////////////////////////////////
// Loop setting GUI Handlers



//////////////////////////////////////////////////////////////////////
// Volume bar handler



void CInstrumentEditor::UpdateCombo() 
{
	switch(_pSong->pInstrument(_pSong->InstSelected())->_NNA)
	{
	case 0:m_nna_combo.SelectString(0,SF::CResourceString(IDS_MSG0046));break;
	case 1:m_nna_combo.SelectString(0,SF::CResourceString(IDS_MSG0047));break;
	case 2:m_nna_combo.SelectString(0,SF::CResourceString(IDS_MSG0048));break;
	}
}

void CInstrumentEditor::Validate()
{
	pParentMain = CMainFrame::GetInstancePtr();
}

void CInstrumentEditor::UpdateNoteLabel()
{
	const int sw = _pSong->WaveSelected();
	const int si = _pSong->InstSelected();
	TCHAR buffer[64];
	
	const int octave= ((_pSong->pInstrument(si)->waveTune[sw]+48)/12);
	switch ((_pSong->pInstrument(si)->waveTune[sw]+48)%12)
	{
	case 0:  _stprintf(buffer,_T("C-%i"),octave);break;
	case 1:  _stprintf(buffer,_T("C#%i"),octave);break;
	case 2:  _stprintf(buffer,_T("D-%i"),octave);break;
	case 3:  _stprintf(buffer,_T("D#%i"),octave);break;
	case 4:  _stprintf(buffer,_T("E-%i"),octave);break;
	case 5:  _stprintf(buffer,_T("F-%i"),octave);break;
	case 6:  _stprintf(buffer,_T("F#%i"),octave);break;
	case 7:  _stprintf(buffer,_T("G-%i"),octave);break;
	case 8:  _stprintf(buffer,_T("G#%i"),octave);break;
	case 9:  _stprintf(buffer,_T("A-%i"),octave);break;
	case 10:  _stprintf(buffer,_T("A#%i"),octave);break;
	case 11:  _stprintf(buffer,_T("B-%i"),octave);break;
	}
	m_notelabel.SetWindowText(buffer);
}

LRESULT CInstrumentEditor::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_notelabel.Attach(GetDlgItem(IDC_NOTETUNE));
	m_panlabel.Attach(GetDlgItem(IDC_PANLABEL));
	m_finelabel.Attach(GetDlgItem(IDC_VOLABEL2));
	m_finetune.Attach(GetDlgItem(IDC_SLIDER2));
	m_loopedit.Attach(GetDlgItem(IDC_LOOPEDIT));
	m_loopcheck.Attach(GetDlgItem(IDC_CHECK4));
	m_rres_check.Attach(GetDlgItem(IDC_RRES));
	m_panslider.Attach(GetDlgItem(IDC_PANSLIDER));
	m_rpan_check.Attach(GetDlgItem(IDC_RPAN));
	m_rcut_check.Attach(GetDlgItem(IDC_RCUT));
	m_nna_combo.Attach(GetDlgItem(IDC_NNA_COMBO));
	m_instname.Attach(GetDlgItem(IDC_INSTNAME));
	m_volabel.Attach(GetDlgItem(IDC_VOLABEL));
	m_volumebar.Attach(GetDlgItem(IDC_SLIDER1));
	m_wlen.Attach(GetDlgItem(IDC_WAVELENGTH));
	m_loopstart.Attach(GetDlgItem(IDC_LOOPSTART));
	m_loopend.Attach(GetDlgItem(IDC_LOOPEND));
	m_looptype.Attach(GetDlgItem(IDC_LOOPTYPE));
	m_instlabel.Attach(GetDlgItem(IDC_INSTNUMBER));

	cando=false;
	
	m_volumebar.SetRange(0,512);
	m_finetune.SetRange(0,512);	// Don't use (-,+) range. It fucks up with the "0"

	m_instname.SetLimitText(31);
	
	m_panslider.SetRange(0,256);
	
	m_nna_combo.AddString(SF::CResourceString(IDS_MSG0046));//note cut
	m_nna_combo.AddString(SF::CResourceString(IDS_MSG0047));//note release 
	m_nna_combo.AddString(SF::CResourceString(IDS_MSG0048));//none

//	_pSong->waveSelected=0;

	CenterWindow();
	WaveUpdate();

	cando=true;

	return TRUE;
}

LRESULT CInstrumentEditor::OnBnClickedButton12(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const int si=_pSong->InstSelected();
	if(si>0)
	{
		pParentMain->ChangeIns(si-1);
	}
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedButton13(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const int si=_pSong->InstSelected();

	if(si<254)
	{
		pParentMain->ChangeIns(si+1);
	}
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedButton7(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pSong->DeleteInstrument(_pSong->InstSelected());
	WaveUpdate();
	pParentMain->UpdateComboIns();
	pParentMain->RedrawGearRackList();
	return 0;
}

LRESULT CInstrumentEditor::OnNMCustomdrawPanslider(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	TCHAR buffer[8];
	int si=_pSong->InstSelected();
	_pSong->pInstrument(si)->_pan = m_panslider.GetPos();
	
	_stprintf(buffer,_T("%d%"),_pSong->pInstrument(si)->_pan);
	m_panlabel.SetWindowText(buffer);
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedRpan(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int si = _pSong->InstSelected();
	
	if (m_rpan_check.GetCheck())
	{
		_pSong->pInstrument(si)->_RPAN = true;
	}
	else
	{
		_pSong->pInstrument(si)->_RPAN = false;
	}
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedRcut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
int si=_pSong->InstSelected();
	
	if (m_rcut_check.GetCheck())
	{
		_pSong->pInstrument(si)->_RCUT = true;
	}
	else
	{
		_pSong->pInstrument(si)->_RCUT = false;
	}
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedRres(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int si=_pSong->InstSelected();
	
	if(m_rres_check.GetCheck())
	{
	_pSong->pInstrument(si)->_RRES = true;
	}
	else
	{
	_pSong->pInstrument(si)->_RRES = false;
	}

	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedCheck4(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
int si=_pSong->InstSelected();
	
	if(m_loopcheck.GetCheck())
	{
		_pSong->pInstrument(si)->_loop = true;
		m_loopedit.EnableWindow(true);
	}
	else
	{
		_pSong->pInstrument(si)->_loop = false;
		m_loopedit.EnableWindow(false);
	}
	return 0;
}

LRESULT CInstrumentEditor::OnNMCustomdrawSlider1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int si=_pSong->InstSelected();
	int sw=_pSong->WaveSelected();
	TCHAR buffer[8];

	_pSong->pInstrument(si)->waveVolume[sw]=m_volumebar.GetPos();
	
	_stprintf(buffer,_T("%d%%"),_pSong->pInstrument(si)->waveVolume[sw]);
	m_volabel.SetWindowText(buffer);
	return 0;
}

LRESULT CInstrumentEditor::OnNMCustomdrawSlider2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int si=_pSong->InstSelected();
	int sw=_pSong->WaveSelected();
	TCHAR buffer[8];
	
	if(cando)
		_pSong->pInstrument(si)->waveFinetune[sw]=m_finetune.GetPos()-256;
	
	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->waveFinetune[sw]);
	m_finelabel.SetWindowText(buffer);
	
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedInsDecoctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const int si=_pSong->InstSelected();
	const int sw=_pSong->WaveSelected();
	if ( _pSong->pInstrument(si)->waveTune[sw]>-37)
		_pSong->pInstrument(si)->waveTune[sw]-=12;
	else _pSong->pInstrument(si)->waveTune[sw]=-48;
	UpdateNoteLabel();	
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedInsDecnote(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
const int si=_pSong->InstSelected();
	const int sw=_pSong->WaveSelected();
	if ( _pSong->pInstrument(si)->waveTune[sw]>-47)
		_pSong->pInstrument(si)->waveTune[sw]-=1;
	else _pSong->pInstrument(si)->waveTune[sw]=-48;
	UpdateNoteLabel();	
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedInsIncnote(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const int si=_pSong->InstSelected();
	const int sw=_pSong->WaveSelected();
	if ( _pSong->pInstrument(si)->waveTune[sw] < 71)
		_pSong->pInstrument(si)->waveTune[sw]+=1;
	else _pSong->pInstrument(si)->waveTune[sw]=71;
	UpdateNoteLabel();	
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedInsIncoctave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const int si=_pSong->InstSelected();
	const int sw=_pSong->WaveSelected();
	if ( _pSong->pInstrument(si)->waveTune[sw] < 60)
		_pSong->pInstrument(si)->waveTune[sw]+=12;
	else _pSong->pInstrument(si)->waveTune[sw]=71;
	UpdateNoteLabel();	
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedLoopoff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int si = _pSong->InstSelected();
	int sw = _pSong->WaveSelected();

	if(_pSong->pInstrument(si)->waveLoopType[sw])
	{
	_pSong->pInstrument(si)->waveLoopType[sw]=0;
	WaveUpdate();
	}
	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedLoopforward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int sw=_pSong->WaveSelected();	
	int si=_pSong->InstSelected();

	if(!_pSong->pInstrument(si)->waveLoopType[sw])
	{
		_pSong->pInstrument(si)->waveLoopType[sw]=1;
		WaveUpdate();
	}

	return 0;
}

LRESULT CInstrumentEditor::OnBnClickedButton8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CEnvDialog dlg;
	dlg._pSong = _pSong;
	dlg.DoModal();
	return 0;
}

LRESULT CInstrumentEditor::OnEnChangeInstname(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CInstrumentEditor>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	int si = _pSong->InstSelected();
	m_instname.GetWindowText(_pSong->pInstrument(si)->_sName, 32);
	if ( !initializingDialog ) 
	{
		pParentMain->UpdateComboIns();
		pParentMain->RedrawGearRackList();
	}
	return 0;
}

LRESULT CInstrumentEditor::OnEnChangeLoopedit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CInstrumentEditor>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	int si = _pSong->InstSelected();
	CString buffer;
	m_loopedit.GetWindowText(buffer);
	_pSong->pInstrument(si)->_lines = _tstoi(buffer);

	if (_pSong->pInstrument(si)->_lines < 1)
	{
		_pSong->pInstrument(si)->_lines = 1;
	}
/*	if (_pSong->pInstrument(si)->_lines > MAX_LINES)
	{
		_pSong->pInstrument(si)->_lines = MAX_LINES;
	}*/
	return 0;
}

LRESULT CInstrumentEditor::OnCbnSelchangeNnaCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_pSong->pInstrument(_pSong->InstSelected())->_NNA = m_nna_combo.GetCurSel();	

	return 0;
}

LRESULT CInstrumentEditor::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	ShowWindow(SW_HIDE);
	return 0;
}

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
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
#include "Song.h"
#include "Instrument.h"
#include ".\envdialog.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CEnvDialog dialog



CEnvDialog::CEnvDialog()
{
	//{{AFX_DATA_INIT(CEnvDialog)
	//}}AFX_DATA_INIT
}



LRESULT CEnvDialog::OnDrawAmpAttackSlider(NMHDR* pNMHDR) 
{
	int si = _pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_AT = m_a_attack_slider.GetPos();
	
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.2f ms."),(float)_pSong->pInstrument(si)->ENV_AT*0.0226757f);
	m_a_a_label.SetWindowText(buffer);

	// Update ADSR
	DrawADSR(
		_pSong->pInstrument(si)->ENV_AT,
		_pSong->pInstrument(si)->ENV_DT,
		_pSong->pInstrument(si)->ENV_SL,
		_pSong->pInstrument(si)->ENV_RT);

	return TRUE;

}

LRESULT CEnvDialog::OnDrawAmpDecaySlider(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_DT = m_a_decay_slider.GetPos();
	
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.2f ms."),(float)_pSong->pInstrument(si)->ENV_DT*0.0226757f);
	m_a_d_label.SetWindowText(buffer);

	// Update ADSR
	DrawADSR(
		_pSong->pInstrument(si)->ENV_AT,
		_pSong->pInstrument(si)->ENV_DT,
		_pSong->pInstrument(si)->ENV_SL,
		_pSong->pInstrument(si)->ENV_RT);

	return TRUE;
}

LRESULT CEnvDialog::OnDrawAmpSustainSlider(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_SL = m_a_sustain_slider.GetPos();
	
	TCHAR buffer[12];
	_stprintf(buffer,_T("%d%%"),_pSong->pInstrument(si)->ENV_SL);
	m_a_s_label.SetWindowText(buffer);

	// Update ADSR
	DrawADSR(
		_pSong->pInstrument(si)->ENV_AT,
		_pSong->pInstrument(si)->ENV_DT,
		_pSong->pInstrument(si)->ENV_SL,
		_pSong->pInstrument(si)->ENV_RT);

	return TRUE;
}

LRESULT CEnvDialog::OnDrawAmpReleaseSlider(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_RT = m_a_release_slider.GetPos();
	
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.2f ms."),(float)_pSong->pInstrument(si)->ENV_RT*0.0226757f);
	m_a_r_label.SetWindowText(buffer);
	
	// Update ADSR
	DrawADSR(
			_pSong->pInstrument(si)->ENV_AT,
			_pSong->pInstrument(si)->ENV_DT,
			_pSong->pInstrument(si)->ENV_SL,
			_pSong->pInstrument(si)->ENV_RT);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Filter envelope

LRESULT CEnvDialog::OnCustomdrawFSlider1(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_F_AT = m_f_attack_slider.GetPos();
	
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.2f ms."),(float)_pSong->pInstrument(si)->ENV_F_AT*0.0226757f);
	m_f_a_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(_pSong->pInstrument(si)->ENV_F_AT,
				_pSong->pInstrument(si)->ENV_F_DT,
				_pSong->pInstrument(si)->ENV_F_SL,
				_pSong->pInstrument(si)->ENV_F_RT);

	return TRUE;
}

LRESULT CEnvDialog::OnCustomdrawFSlider2(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_F_DT=m_f_decay_slider.GetPos();
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.2f ms."),(float)_pSong->pInstrument(si)->ENV_F_DT*0.0226757f);
	m_f_d_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(_pSong->pInstrument(si)->ENV_F_AT,
				_pSong->pInstrument(si)->ENV_F_DT,
				_pSong->pInstrument(si)->ENV_F_SL,
				_pSong->pInstrument(si)->ENV_F_RT);

	return TRUE;
}

LRESULT CEnvDialog::OnCustomdrawFSlider3(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();
	

	TCHAR buffer[12];
	_pSong->pInstrument(si)->ENV_F_SL = m_f_sustain_slider.GetPos();
	_stprintf(buffer,_T("%.0f%%"),_pSong->pInstrument(si)->ENV_F_SL*0.78125);
	m_f_s_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(_pSong->pInstrument(si)->ENV_F_AT,
				_pSong->pInstrument(si)->ENV_F_DT,
				_pSong->pInstrument(si)->ENV_F_SL,
				_pSong->pInstrument(si)->ENV_F_RT);

	return TRUE;
}

LRESULT CEnvDialog::OnCustomdrawFSlider4(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_F_RT = m_f_release_slider.GetPos();
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.2f ms."),(float)_pSong->pInstrument(si)->ENV_F_RT*0.0226757f);
	m_f_r_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(_pSong->pInstrument(si)->ENV_F_AT,
				_pSong->pInstrument(si)->ENV_F_DT,
				_pSong->pInstrument(si)->ENV_F_SL,
				_pSong->pInstrument(si)->ENV_F_RT);
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// GDI Envelope Drawing Members

void CEnvDialog::DrawADSR(int AX,int BX,int CX,int DX)
{
	AX /= 420;
	BX /= 420;
	DX /= 420;

	int CH=100;
	int CW=512;

	CClientDC dc(m_ampframe);
	CPoint pol[5];

	dc.FillSolidRect(0,0,494,CH+1,0xCC7788);
	pol[0].x=0;
	pol[0].y=CH;

	pol[1].x=AX;
	pol[1].y=0;
	
	pol[2].x=AX+BX;
	pol[2].y=CH-CX;
	
	pol[3].x=AX+BX;
	pol[3].y=CH-CX;
	
	pol[4].x=AX+BX+DX;
	pol[4].y=CH;
	
	dc.Polygon(&pol[0],5);

	dc.MoveTo(AX+BX,0);
	dc.LineTo(AX+BX,CH);
}

void CEnvDialog::DrawADSRFil(int AX,int BX,int CX,int DX)
{
	AX/=420;
	BX/=420;
	CX*=100;
	CX/=128;
	DX/=420;

	int CH=100;
	int CW=512;

	CClientDC dc(m_filframe);
	CPoint pol[5];

	dc.FillSolidRect(0,0,494,CH+1,0x4422CC);
	pol[0].x=0;
	pol[0].y=CH;

	pol[1].x=AX;
	pol[1].y=0;
	
	pol[2].x=AX+BX;
	pol[2].y=CH-CX;
	
	pol[3].x=AX+BX;
	pol[3].y=CH-CX;
	
	pol[4].x=AX+BX+DX;
	pol[4].y=CH;
	
	dc.Polygon(&pol[0],5);

	dc.MoveTo(AX+BX,0);
	dc.LineTo(AX+BX,CH);
}

LRESULT CEnvDialog::OnCustomdrawSliderCutoff(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_F_CO = m_cutoff_slider.GetPos();
	TCHAR buffer[12];
	_stprintf(buffer,_T("%d"),_pSong->pInstrument(si)->ENV_F_CO);
	m_cutoff_label.SetWindowText(buffer);

	return TRUE;
}

LRESULT CEnvDialog::OnCustomdrawSliderQ(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();

	_pSong->pInstrument(si)->ENV_F_RQ = m_q_slider.GetPos();

	TCHAR buffer[12];
	_stprintf(buffer,_T("%.0f%%"),_pSong->pInstrument(si)->ENV_F_RQ*0.78740);
	m_q_label.SetWindowText(buffer);

	return TRUE;
}


LRESULT CEnvDialog::OnCustomdrawEnvelope(NMHDR* pNMHDR) 
{
	int si=_pSong->InstSelected();
	_pSong->pInstrument(si)->ENV_F_EA = m_envelope_slider.GetPos()-128;
	TCHAR buffer[12];
	_stprintf(buffer,_T("%.0f"),(float)_pSong->pInstrument(si)->ENV_F_EA*0.78125f);
	m_envelope_label.SetWindowText(buffer);
	
	return TRUE;
}


LRESULT CEnvDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExecuteDlgInit(IDD_DSOUNDCONFIG);
	
	m_envelope_slider.Attach(GetDlgItem(IDC_ENVELOPE));
	m_filtercombo.Attach(GetDlgItem(IDC_COMBO1 ));
	m_cutoff_label.Attach(GetDlgItem(IDC_CUTOFF_LBL ));
	m_q_slider.Attach(GetDlgItem(IDC_SLIDER10));
	m_cutoff_slider.Attach(GetDlgItem(IDC_SLIDER9));
	m_f_s_label.Attach(GetDlgItem(IDC_F_S_LABEL));
	m_f_r_label.Attach(GetDlgItem(IDC_F_R_LABEL));
	m_f_d_label.Attach(GetDlgItem(IDC_F_D_LABEL));
	m_f_a_label.Attach(GetDlgItem(IDC_F_A_LABEL ));
	m_f_release_slider.Attach(GetDlgItem(IDC_F_SLIDER4));
	m_f_sustain_slider.Attach(GetDlgItem(IDC_F_SLIDER3));
	m_f_decay_slider.Attach(GetDlgItem(IDC_F_SLIDER2));
	m_f_attack_slider.Attach(GetDlgItem(IDC_F_SLIDER1));
	m_a_a_label.Attach(GetDlgItem(IDC_A_A_LABEL));
	m_filframe.Attach(GetDlgItem(IDC_AMPFRAME2));
	m_a_s_label.Attach(GetDlgItem(IDC_A_S_LABEL));
	m_a_r_label.Attach(GetDlgItem(IDC_A_R_LABEL));
	m_a_d_label.Attach(GetDlgItem(IDC_A_D_LABEL));
	m_ampframe.Attach(GetDlgItem(IDC_AMPFRAME));
	m_a_release_slider.Attach(GetDlgItem(IDC_SLIDER4));
	m_a_sustain_slider.Attach(GetDlgItem(IDC_SLIDER3));
	m_a_decay_slider.Attach(GetDlgItem(IDC_SLIDER2));
	m_a_attack_slider.Attach(GetDlgItem(IDC_SLIDER1));
	m_q_label.Attach(GetDlgItem(IDC_LABELQ));
	m_envelope_label.Attach(GetDlgItem(IDC_A_A_LABEL7));

	
	int si = _pSong->InstSelected();

	// Set slider ranges
	m_a_attack_slider.SetRange(1,65536);
	m_a_decay_slider.SetRange(1,65536);
	m_a_sustain_slider.SetRange(0,100);
	m_a_release_slider.SetRange(16,65536);

	m_f_attack_slider.SetRange(1,65536);
	m_f_decay_slider.SetRange(1,65536);
	m_f_sustain_slider.SetRange(0,128);
	m_f_release_slider.SetRange(16,65536);
		
	m_cutoff_slider.SetRange(0,127);
	m_q_slider.SetRange(0,127);
	
	m_envelope_slider.SetRange(0,256); // Don't use (-,+) range. It fucks up with the "0"
	
	m_filtercombo.AddString(SF::CResourceString(IDS_MSG0073));
	m_filtercombo.AddString(SF::CResourceString(IDS_MSG0074));
	m_filtercombo.AddString(SF::CResourceString(IDS_MSG0075));
	m_filtercombo.AddString(SF::CResourceString(IDS_MSG0076));
	m_filtercombo.AddString(SF::CResourceString(IDS_MSG0077));

	m_filtercombo.SetCurSel(_pSong->pInstrument(si)->ENV_F_TP);

	// Update sliders
	m_a_attack_slider.SetPos(_pSong->pInstrument(si)->ENV_AT);
	m_a_decay_slider.SetPos(_pSong->pInstrument(si)->ENV_DT);
	m_a_sustain_slider.SetPos(_pSong->pInstrument(si)->ENV_SL);
	m_a_release_slider.SetPos(_pSong->pInstrument(si)->ENV_RT);

	m_f_attack_slider.SetPos(_pSong->pInstrument(si)->ENV_F_AT);
	m_f_decay_slider.SetPos(_pSong->pInstrument(si)->ENV_F_DT);
	m_f_sustain_slider.SetPos(_pSong->pInstrument(si)->ENV_F_SL);
	m_f_release_slider.SetPos(_pSong->pInstrument(si)->ENV_F_RT);
	
	m_cutoff_slider.SetPos(_pSong->pInstrument(si)->ENV_F_CO);
	m_q_slider.SetPos(_pSong->pInstrument(si)->ENV_F_RQ);
	m_envelope_slider.SetPos(_pSong->pInstrument(si)->ENV_F_EA+128);
	
	return TRUE;
}

LRESULT CEnvDialog::OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int si=_pSong->InstSelected();
	_pSong->pInstrument(si)->ENV_F_TP = m_filtercombo.GetCurSel();	

	return 0;
}

LRESULT CEnvDialog::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	EndDialog(IDOK);
	return 0;
}

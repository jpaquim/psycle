// EnvDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "EnvDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnvDialog dialog


CEnvDialog::CEnvDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CEnvDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnvDialog)
	//}}AFX_DATA_INIT
}


void CEnvDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnvDialog)
	DDX_Control(pDX, IDC_A_A_LABEL7, m_envelope_label);
	DDX_Control(pDX, IDC_ENVELOPE, m_envelope_slider);
	DDX_Control(pDX, IDC_COMBO1, m_filtercombo);
	DDX_Control(pDX, IDC_LABELQ, m_q_label);
	DDX_Control(pDX, IDC_CUTOFF_LBL, m_cutoff_label);
	DDX_Control(pDX, IDC_SLIDER10, m_q_slider);
	DDX_Control(pDX, IDC_SLIDER9, m_cutoff_slider);
	DDX_Control(pDX, IDC_F_S_LABEL, m_f_s_label);
	DDX_Control(pDX, IDC_F_R_LABEL, m_f_r_label);
	DDX_Control(pDX, IDC_F_D_LABEL, m_f_d_label);
	DDX_Control(pDX, IDC_F_A_LABEL, m_f_a_label);
	DDX_Control(pDX, IDC_F_SLIDER4, m_f_release_slider);
	DDX_Control(pDX, IDC_F_SLIDER3, m_f_sustain_slider);
	DDX_Control(pDX, IDC_F_SLIDER2, m_f_decay_slider);
	DDX_Control(pDX, IDC_F_SLIDER1, m_f_attack_slider);
	DDX_Control(pDX, IDC_A_A_LABEL, m_a_a_label);
	DDX_Control(pDX, IDC_AMPFRAME2, m_filframe);
	DDX_Control(pDX, IDC_A_S_LABEL, m_a_s_label);
	DDX_Control(pDX, IDC_A_R_LABEL, m_a_r_label);
	DDX_Control(pDX, IDC_A_D_LABEL, m_a_d_label);
	DDX_Control(pDX, IDC_AMPFRAME, m_ampframe);
	DDX_Control(pDX, IDC_SLIDER4, m_a_release_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_a_sustain_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_a_decay_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_a_attack_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnvDialog, CDialog)
	//{{AFX_MSG_MAP(CEnvDialog)
	ON_WM_PAINT()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnDrawAmpAttackSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnDrawAmpDecaySlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnDrawAmpSustainSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnDrawAmpReleaseSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER1, OnCustomdrawFSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER2, OnCustomdrawFSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER3, OnCustomdrawFSlider3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_F_SLIDER4, OnCustomdrawFSlider4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER9, OnCustomdrawSliderCutoff)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER10, OnCustomdrawSliderQ)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ENVELOPE, OnCustomdrawEnvelope)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnvDialog message handlers

BOOL CEnvDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int si=songRef->instSelected;

	// Set slider ranges
	m_a_attack_slider.SetRange(1,65536);
	m_a_decay_slider.SetRange(1,65536);
	m_a_sustain_slider.SetRange(1,100);
	m_a_release_slider.SetRange(16,65536);

	m_f_attack_slider.SetRange(1,65536);
	m_f_decay_slider.SetRange(1,65536);
	m_f_sustain_slider.SetRange(1,128);
	m_f_release_slider.SetRange(16,65536);
		
	m_cutoff_slider.SetRange(0,127);
	m_q_slider.SetRange(0,127);
	
	m_envelope_slider.SetRange(-128,128);
	m_envelope_slider.SetPos(songRef->instENV_F_EA[si]);
	
	m_filtercombo.AddString("LowPass");
	m_filtercombo.AddString("HiPass");
	m_filtercombo.AddString("BandPass");
	m_filtercombo.AddString("NotchBand");
	m_filtercombo.AddString("None");

	m_filtercombo.SetCurSel(songRef->instENV_F_TP[si]);

	// Update sliders
	m_a_attack_slider.SetPos(songRef->instENV_AT[si]);
	m_a_decay_slider.SetPos(songRef->instENV_DT[si]);
	m_a_sustain_slider.SetPos(songRef->instENV_SL[si]);
	m_a_release_slider.SetPos(songRef->instENV_RT[si]);

	m_f_attack_slider.SetPos(songRef->instENV_F_AT[si]);
	m_f_decay_slider.SetPos(songRef->instENV_F_DT[si]);
	m_f_sustain_slider.SetPos(songRef->instENV_F_SL[si]);
	m_f_release_slider.SetPos(songRef->instENV_F_RT[si]);
	
	m_cutoff_slider.SetPos(songRef->instENV_F_CO[si]);
	m_q_slider.SetPos(songRef->instENV_F_RQ[si]);

	return TRUE;
}

void CEnvDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting	
}

void CEnvDialog::OnDrawAmpAttackSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_AT[si]=m_a_attack_slider.GetPos();
	
	char buffer[12];
	sprintf(buffer,"%.2f ms.",(float)songRef->instENV_AT[si]*0.0226757f);
	m_a_a_label.SetWindowText(buffer);

	// Update ADSR
	DrawADSR(
		songRef->instENV_AT[si],
		songRef->instENV_DT[si],
		songRef->instENV_SL[si],
		songRef->instENV_RT[si]
			);

	*pResult = 0;

}

void CEnvDialog::OnDrawAmpDecaySlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_DT[si]=m_a_decay_slider.GetPos();
	
	char buffer[12];
	sprintf(buffer,"%.2f ms.",(float)songRef->instENV_DT[si]*0.0226757f);
	m_a_d_label.SetWindowText(buffer);

	// Update ADSR
	DrawADSR(
		songRef->instENV_AT[si],
		songRef->instENV_DT[si],
		songRef->instENV_SL[si],
		songRef->instENV_RT[si]
			);

	*pResult = 0;
}

void CEnvDialog::OnDrawAmpSustainSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_SL[si]=m_a_sustain_slider.GetPos();
	
	char buffer[12];
	sprintf(buffer,"%d%%",songRef->instENV_SL[si]);
	m_a_s_label.SetWindowText(buffer);

	// Update ADSR
	DrawADSR(
		songRef->instENV_AT[si],
		songRef->instENV_DT[si],
		songRef->instENV_SL[si],
		songRef->instENV_RT[si]
			);

	*pResult = 0;
}

void CEnvDialog::OnDrawAmpReleaseSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_RT[si]=m_a_release_slider.GetPos();
	
	char buffer[12];
	sprintf(buffer,"%.2f ms.",(float)songRef->instENV_RT[si]*0.0226757f);
	m_a_r_label.SetWindowText(buffer);
	
	// Update ADSR
	DrawADSR(
			songRef->instENV_AT[si],
			songRef->instENV_DT[si],
			songRef->instENV_SL[si],
			songRef->instENV_RT[si]
			);

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////
// Filter envelope

void CEnvDialog::OnCustomdrawFSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_F_AT[si]=m_f_attack_slider.GetPos();
	
	char buffer[12];
	sprintf(buffer,"%.2f ms.",(float)songRef->instENV_F_AT[si]*0.0226757f);
	m_f_a_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(songRef->instENV_F_AT[si],songRef->instENV_F_DT[si],songRef->instENV_F_SL[si],songRef->instENV_F_RT[si]);

	*pResult = 0;
}

void CEnvDialog::OnCustomdrawFSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_F_DT[si]=m_f_decay_slider.GetPos();
	char buffer[12];
	sprintf(buffer,"%.2f ms.",(float)songRef->instENV_F_DT[si]*0.0226757f);
	m_f_d_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(songRef->instENV_F_AT[si],songRef->instENV_F_DT[si],songRef->instENV_F_SL[si],songRef->instENV_F_RT[si]);

	*pResult = 0;
}

void CEnvDialog::OnCustomdrawFSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_F_SL[si]=m_f_sustain_slider.GetPos();
	char buffer[12];
	sprintf(buffer,"%d%%",songRef->instENV_F_SL[si]);
	m_f_s_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(songRef->instENV_F_AT[si],songRef->instENV_F_DT[si],songRef->instENV_F_SL[si],songRef->instENV_F_RT[si]);

	*pResult = 0;
}

void CEnvDialog::OnCustomdrawFSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_F_RT[si]=m_f_release_slider.GetPos();
	char buffer[12];
	sprintf(buffer,"%.2f ms.",(float)songRef->instENV_F_RT[si]*0.0226757f);
	m_f_r_label.SetWindowText(buffer);

	// Update filter ADSR
	DrawADSRFil(songRef->instENV_F_AT[si],songRef->instENV_F_DT[si],songRef->instENV_F_SL[si],songRef->instENV_F_RT[si]);
	
	*pResult = 0;
}


//////////////////////////////////////////////////////////////////////
// GDI Envelope Drawing Members

void CEnvDialog::DrawADSR(int AX,int BX,int CX,int DX)
{
	AX/=420;
	BX/=420;
	DX/=420;

	int CH=100;
	int CW=512;

	CClientDC dc(&m_ampframe);
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

	CClientDC dc(&m_filframe);
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

void CEnvDialog::OnCustomdrawSliderCutoff(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_F_CO[si]=m_cutoff_slider.GetPos();
	char buffer[12];
	sprintf(buffer,"%d",songRef->instENV_F_CO[si]);
	m_cutoff_label.SetWindowText(buffer);

	*pResult = 0;
}

void CEnvDialog::OnCustomdrawSliderQ(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int si=songRef->instSelected;

	songRef->instENV_F_RQ[si]=m_q_slider.GetPos();

	char buffer[12];
	sprintf(buffer,"%d%%",songRef->instENV_F_RQ[si]);
	m_q_label.SetWindowText(buffer);

	*pResult = 0;
}

void CEnvDialog::OnSelchangeCombo1() 
{
	int si=songRef->instSelected;
	songRef->instENV_F_TP[si]=m_filtercombo.GetCurSel();	
}

void CEnvDialog::OnCustomdrawEnvelope(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int si=songRef->instSelected;
	songRef->instENV_F_EA[si]=m_envelope_slider.GetPos();
	char buffer[12];
	sprintf(buffer,"%.2f",(float)songRef->instENV_F_EA[si]*0.78125f);
	m_envelope_label.SetWindowText(buffer);
	
	*pResult = 0;
}


// GearFlanger.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearFlanger.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearFlanger dialog


CGearFlanger::CGearFlanger(CChildView* pParent /*=NULL*/)
	: CDialog(CGearFlanger::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearFlanger)
	//}}AFX_DATA_INIT
}


void CGearFlanger::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearFlanger)
	DDX_Control(pDX, IDC_PRESETCOMBO, m_presetcombo);
	DDX_Control(pDX, IDC_WETSLIDER, m_wet_slider);
	DDX_Control(pDX, IDC_DRYSLIDER, m_dry_slider);
	DDX_Control(pDX, IDC_WETLABEL, m_wet_label);
	DDX_Control(pDX, IDC_DRYLABEL, m_dry_label);
	DDX_Control(pDX, IDC_LABEL6, m_rf_label);
	DDX_Control(pDX, IDC_SLIDER6, m_rf_slider);
	DDX_Control(pDX, IDC_SLIDER5, m_phase_slider);
	DDX_Control(pDX, IDC_LABEL5, m_phase_label);
	DDX_Control(pDX, IDC_SLIDER4, m_lf_slider);
	DDX_Control(pDX, IDC_LABEL4, m_lf_label);
	DDX_Control(pDX, IDC_SLIDER3, m_speed_slider);
	DDX_Control(pDX, IDC_LABEL3, m_speed_label);
	DDX_Control(pDX, IDC_LABEL2, m_amp_label);
	DDX_Control(pDX, IDC_SLIDER2, m_amp_slider);
	DDX_Control(pDX, IDC_LABEL1, m_delay_time_label);
	DDX_Control(pDX, IDC_SLIDER1, m_flanger_delay_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearFlanger, CDialog)
	//{{AFX_MSG_MAP(CGearFlanger)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawFlangerDelaySlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawAmpSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSliderSpeed)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawLfSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, OnCustomdrawSliderPhase)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER6, OnCustomdrawSliderFr)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WETSLIDER, OnCustomdrawWetslider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DRYSLIDER, OnCustomdrawDryslider)
	ON_CBN_SELCHANGE(IDC_PRESETCOMBO, OnSelchangePresetcombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearFlanger message handlers

void CGearFlanger::OnCustomdrawFlangerDelaySlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_time = m_flanger_delay_slider.GetPos();

	char buffer[16];
	sprintf(buffer,"%d",_pMachine->_time);
	m_delay_time_label.SetWindowText(buffer);

	_pMachine->Update();

	*pResult = 0;
}

void CGearFlanger::OnCustomdrawAmpSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	char buffer[16];
	sprintf(buffer,"%.2f%%",(float)_pMachine->_lfoAmp*0.390625f);
	m_amp_label.SetWindowText(buffer);

	_pMachine->Update();

	_pMachine->_lfoAmp = m_amp_slider.GetPos();

	*pResult = 0;
}


BOOL CGearFlanger::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char buffer[64];
	sprintf(buffer, _pMachine->_editName);
	SetWindowText(buffer);

	m_presetcombo.AddString("Flanger Sweeper [Default]");
	m_presetcombo.AddString("Chorus 1");
	m_presetcombo.AddString("Chorus 2");

	m_flanger_delay_slider.SetRange(1, 1024);
	m_flanger_delay_slider.SetPos(_pMachine->_time);

	m_amp_slider.SetRange(1,256);
	m_amp_slider.SetPos(_pMachine->_lfoAmp);

	m_speed_slider.SetRange(0, 32768);
	m_speed_slider.SetPos(_pMachine->_lfoSpeed);

	m_lf_slider.SetRange(0,200);		// Don't use (-,+) range. It fucks up with the "0"
	m_lf_slider.SetPos(_pMachine->_feedbackL+100);

	m_rf_slider.SetRange(0,200);		// Don't use (-,+) range. It fucks up with the "0"
	m_rf_slider.SetPos(_pMachine->_feedbackR+100);

	m_phase_slider.SetRange(0,256);
	m_phase_slider.SetPos(_pMachine->_lfoPhase);

	m_dry_slider.SetRange(0,512);	// Don't use (-,+) range. It fucks up with the "0"
	m_dry_slider.SetPos(_pMachine->_outDry+256);

	m_wet_slider.SetRange(0,512);	// Don't use (-,+) range. It fucks up with the "0"
	m_wet_slider.SetPos(_pMachine->_outWet+256);

	return TRUE;
}


void CGearFlanger::OnCustomdrawSliderSpeed(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_lfoSpeed=m_speed_slider.GetPos();

	char buffer[16];
	sprintf(buffer,"%d",_pMachine->_lfoSpeed);
	m_speed_label.SetWindowText(buffer);

	_pMachine->Update();

	*pResult = 0;
}

void CGearFlanger::OnCustomdrawLfSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_feedbackL=m_lf_slider.GetPos()-100;
	
	char buffer[16];
	sprintf(buffer,"%d%%",_pMachine->_feedbackL);
	m_lf_label.SetWindowText(buffer);

	*pResult = 0;
}

void CGearFlanger::OnCustomdrawSliderFr(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_feedbackR=m_rf_slider.GetPos()-100;

	char buffer[16];
	sprintf(buffer,"%d%%",_pMachine->_feedbackR);
	m_rf_label.SetWindowText(buffer);

	*pResult = 0;
}

void CGearFlanger::OnCustomdrawSliderPhase(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_lfoPhase=m_phase_slider.GetPos();
	
	char buffer[16];
	sprintf(buffer,"%d",_pMachine->_lfoPhase);

	m_phase_label.SetWindowText(buffer);
	
	_pMachine->Update();

	*pResult = 0;
}

void CGearFlanger::OnCustomdrawDryslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_outDry = m_dry_slider.GetPos()-256;

	char buffer[8];
	sprintf(buffer, "%.1f%%", (float)_pMachine->_outDry*0.390625f);
	m_dry_label.SetWindowText(buffer);

	*pResult = 0;
}

void CGearFlanger::OnCustomdrawWetslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_outWet = m_wet_slider.GetPos()-256;

	char buffer[8];
	sprintf(buffer,"%.1f%%",(float)_pMachine->_outWet*0.390625f);
	m_wet_label.SetWindowText(buffer);

	*pResult = 0;
}

void CGearFlanger::OnSelchangePresetcombo() 
{
	_pMachine->SetPreset(m_presetcombo.GetCurSel());

	m_flanger_delay_slider.SetPos(_pMachine->_time);
	m_amp_slider.SetPos(_pMachine->_lfoAmp);
	m_speed_slider.SetPos(_pMachine->_lfoSpeed);
	m_lf_slider.SetPos(_pMachine->_feedbackL+100);
	m_rf_slider.SetPos(_pMachine->_feedbackR+100);
	m_phase_slider.SetPos(_pMachine->_lfoPhase);
	m_dry_slider.SetPos(_pMachine->_outDry+256);
	m_wet_slider.SetPos(_pMachine->_outWet+256);
}

BOOL CGearFlanger::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearFlanger::OnCancel()
{
	m_pParent->FlangerMachineDialog = NULL;
	DestroyWindow();
	delete this;
}

// Gearfilter.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "Gearfilter.h"
#include "ChildView.h"
#include "configuration.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearfilter dialog


CGearfilter::CGearfilter(CChildView* pParent /*=NULL*/)
	: CDialog(CGearfilter::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearfilter)
	//}}AFX_DATA_INIT
}


void CGearfilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearfilter)
	DDX_Control(pDX, IDC_PARAM_INF_6, m_ParamInf6);
	DDX_Control(pDX, IDC_PARAM_INF_5, m_ParamInf5);
	DDX_Control(pDX, IDC_PARAM_INF_4, m_ParamInf4);
	DDX_Control(pDX, IDC_PARAM_INF_3, m_ParamInf3);
	DDX_Control(pDX, IDC_PARAM_INF_2, m_ParamInf2);
	DDX_Control(pDX, IDC_COMBO1, m_filtercombo);
	DDX_Control(pDX, IDC_SLIDER5, m_lfo_phase_slider);
	DDX_Control(pDX, IDC_SLIDER4, m_lfo_amp_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_lfo_speed_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_filter_reso_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_filter_cutoff_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearfilter, CDialog)
	//{{AFX_MSG_MAP(CGearfilter)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawFCS)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawFRS)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawLSS)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawLAS)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, OnCustomdrawSliderPhase)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeFilter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearfilter message handlers

BOOL CGearfilter::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// Filter dialog initialization
	char buffer[64];
	sprintf(buffer,_pMachine->_editName);
	SetWindowText(buffer);
	
	// Set slider boundaries
	m_filter_cutoff_slider.SetRange(0, 256);
	m_filter_reso_slider.SetRange(0, 256);
	m_lfo_speed_slider.SetRange(0, 32768);
	m_lfo_amp_slider.SetRange(0, 256);
	m_lfo_phase_slider.SetRange(0, 256);

	// Init filter combo
	m_filtercombo.AddString("Lo Pass 2 Pole Filter [-12db]");
	m_filtercombo.AddString("Hi Pass 2 Pole Filter [-12db]");
	m_filtercombo.SetCurSel(_pMachine->_filterMode);

	// Init sliders
	m_filter_cutoff_slider.SetPos(_pMachine->_cutoff);
	m_filter_reso_slider.SetPos(_pMachine->_resonance);
	m_lfo_speed_slider.SetPos(_pMachine->_lfoSpeed);
	m_lfo_amp_slider.SetPos(_pMachine->_lfoAmp);
	m_lfo_phase_slider.SetPos(_pMachine->_lfoPhase);

	UpdateStatus();

	return TRUE;
}

void CGearfilter::OnCustomdrawFCS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_cutoff = m_filter_cutoff_slider.GetPos();
	UpdateStatus();

	_pMachine->Update();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawFRS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_resonance = m_filter_reso_slider.GetPos();
	UpdateStatus();

	_pMachine->Update();
	char buf[64];
	_pMachine->GetParamValue(1,buf);
	*pResult = 0;
}

void CGearfilter::OnCustomdrawLSS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_lfoSpeed = m_lfo_speed_slider.GetPos();
	UpdateStatus();
	_pMachine->Update();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawLAS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_lfoAmp = m_lfo_amp_slider.GetPos();
	UpdateStatus();

	_pMachine->Update();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawSliderPhase(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_lfoPhase = m_lfo_phase_slider.GetPos();	
	UpdateStatus();

	_pMachine->Update();
	*pResult = 0;
}

void CGearfilter::OnSelchangeFilter() 
{
	_pMachine->_filterMode = m_filtercombo.GetCurSel();

}

void CGearfilter::UpdateStatus()
{
	CString tmp;

	// get sample rate
	int rate = 44100;
	if(Global::pConfig!=NULL &&
		Global::pConfig->_pOutputDriver!=NULL)
		rate = Global::pConfig->_pOutputDriver->_samplesPerSec;

	// cutoff
	tmp.Format("%d (%dHz)",_pMachine->_cutoff, (int)((Global::pConfig->_pOutputDriver->_samplesPerSec * asin( _pMachine->_cutoff * 0.00390625f)) / 3.1415926f));
	m_ParamInf2.SetWindowText(tmp);

	// resonance
	tmp.Format("%d (%d%%)",_pMachine->_resonance,(int)(_pMachine->_resonance*0.390625f));
	m_ParamInf3.SetWindowText(tmp);
	
	// LFO
	tmp.Format("%d (%.2fHz)",_pMachine->_lfoSpeed,_pMachine->_lfoSpeed * 0.00000003f*Global::pConfig->_pOutputDriver->_samplesPerSec / 6.283185f);
	m_ParamInf4.SetWindowText(tmp);

	// LFO amp
	tmp.Format("%d (%d%%)",_pMachine->_lfoAmp,(int)(_pMachine->_lfoAmp*0.390625f));
	m_ParamInf5.SetWindowText(tmp);

	// LFO phase
	tmp.Format("%d (%d deg)",_pMachine->_lfoPhase,(int)(_pMachine->_lfoPhase*0.703125f));
	m_ParamInf6.SetWindowText(tmp);
}

BOOL CGearfilter::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearfilter::OnCancel()
{
	m_pParent->FilterMachineDialog = NULL;
	DestroyWindow();
	delete this;
}

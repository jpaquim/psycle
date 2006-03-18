// Gearfilter.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "Gearfilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearfilter dialog


CGearfilter::CGearfilter(CWnd* pParent /*=NULL*/)
	: CDialog(CGearfilter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearfilter)
	//}}AFX_DATA_INIT
doit=false;
}


void CGearfilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearfilter)
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
	sprintf(buffer,machineRef->editName);
	SetWindowText(buffer);
	
	// Set slider boundaries
	m_filter_cutoff_slider.SetRange(0,256);
	m_filter_reso_slider.SetRange(0,256);
	m_lfo_speed_slider.SetRange(0,32768);
	m_lfo_amp_slider.SetRange(0,256);
	m_lfo_phase_slider.SetRange(0,256);

	// Init filter combo
	m_filtercombo.AddString("Lo Pass 2 Pole Filter [-12db]");
	m_filtercombo.AddString("Hi Pass 2 Pole Filter [-12db]");
	m_filtercombo.SetCurSel(machineRef->filterMode);

	// Init sliders
	m_filter_cutoff_slider.SetPos(machineRef->filterCutoff);
	m_filter_reso_slider.SetPos(machineRef->filterResonance);
	m_lfo_speed_slider.SetPos(machineRef->filterLfospeed);
	m_lfo_amp_slider.SetPos(machineRef->filterLfoamp);
	m_lfo_phase_slider.SetPos(machineRef->filterLfophase);

	doit=true;

	return TRUE;
}

void CGearfilter::OnCustomdrawFCS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->filterCutoff=m_filter_cutoff_slider.GetPos();
	
	machineRef->UpdateFilter();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawFRS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->filterResonance=m_filter_reso_slider.GetPos();
	
	machineRef->UpdateFilter();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawLSS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->filterLfospeed=m_lfo_speed_slider.GetPos();
	
	machineRef->UpdateFilter();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawLAS(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->filterLfoamp=m_lfo_amp_slider.GetPos();
	
	machineRef->UpdateFilter();
	*pResult = 0;
}

void CGearfilter::OnCustomdrawSliderPhase(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->filterLfophase=m_lfo_phase_slider.GetPos();	
	
	machineRef->UpdateFilter();
	*pResult = 0;
}

void CGearfilter::OnSelchangeFilter() 
{
	if(doit)
	machineRef->filterMode=m_filtercombo.GetCurSel();
}

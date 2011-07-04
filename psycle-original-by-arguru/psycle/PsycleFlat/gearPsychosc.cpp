// gearPsychosc.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "gearPsychosc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// gearPsychosc dialog


gearPsychosc::gearPsychosc(CWnd* pParent /*=NULL*/)
	: CDialog(gearPsychosc::IDD, pParent)
{
	//{{AFX_DATA_INIT(gearPsychosc)
	//}}AFX_DATA_INIT
	doit=false;
}


void gearPsychosc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(gearPsychosc)
	DDX_Control(pDX, IDC_STATIC4, m_label4);
	DDX_Control(pDX, IDC_STATIC2, m_label2);
	DDX_Control(pDX, IDC_STATIC3, m_label3);
	DDX_Control(pDX, IDC_STATIC1, m_label1);
	DDX_Control(pDX, IDC_SLIDER4, m_lfoamplitude);
	DDX_Control(pDX, IDC_SLIDER3, m_lfofrequency);
	DDX_Control(pDX, IDC_SLIDER2, m_glidespeed);
	DDX_Control(pDX, IDC_SLIDER1, m_oscspeed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(gearPsychosc, CDialog)
	//{{AFX_MSG_MAP(gearPsychosc)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// gearPsychosc message handlers

void gearPsychosc::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->sinespeed=129-m_oscspeed.GetPos();
	
	machineRef->sineTick();
	
	char buffer[32];
	float period=6.2831853f/machineRef->sineSpeed;
	sprintf(buffer,"%.1f Hz.",44100/period);
	m_label1.SetWindowText(buffer);

	*pResult = 0;
	
}

void gearPsychosc::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->sineglide=129-m_glidespeed.GetPos();
	
	machineRef->sineTick();

	char buffer[32];
	sprintf(buffer,"%.1f%%",machineRef->sineGlide*155038.76f);
	m_label2.SetWindowText(buffer);

	*pResult = 0;
}

void gearPsychosc::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->sinelfospeed=129-m_lfofrequency.GetPos();
	
	machineRef->sineTick();

	char buffer[32];
	float period=6.2831853f/machineRef->sineLfospeed;
	sprintf(buffer,"%.1f Hz.",44100/period);
	m_label3.SetWindowText(buffer);

	*pResult = 0;
}

void gearPsychosc::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->sinelfoamp=129-m_lfoamplitude.GetPos();
	
	machineRef->sineTick();
	
	char buffer[32];
	sprintf(buffer,"%.1f%%",machineRef->sineLfoamp*1550.3876f);
	m_label4.SetWindowText(buffer);

	*pResult = 0;
}

BOOL gearPsychosc::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Initialize ranges
	m_oscspeed.SetRange(0,128,true);
	m_glidespeed.SetRange(0,128,true);
	m_lfofrequency.SetRange(0,128,true);
	m_lfoamplitude.SetRange(0,128,true);
	m_oscspeed.SetTicFreq(16);
	m_glidespeed.SetTicFreq(16);
	m_lfofrequency.SetTicFreq(16);
	m_lfoamplitude.SetTicFreq(16);
	
	// Initialize stuff
	m_oscspeed.SetPos(129-machineRef->sinespeed);
	m_glidespeed.SetPos(129-machineRef->sinelfoamp);
	m_lfofrequency.SetPos(129-machineRef->sinelfospeed);
	m_lfoamplitude.SetPos(129-machineRef->sinelfoamp);
	
	char buffer[64];
	sprintf(buffer,"%s [PsychOsc AM]",machineRef->editName);
	this->SetWindowText(buffer);

	doit=true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CGearPsychOsc.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearPsychOsc.h"
#include "Configuration.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearPsychOsc dialog


CGearPsychOsc::CGearPsychOsc(CWnd* pParent /*=NULL*/)
	: CDialog(CGearPsychOsc::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearPsychOsc)
	//}}AFX_DATA_INIT
	doit=false;
}


void CGearPsychOsc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearPsychOsc)
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


BEGIN_MESSAGE_MAP(CGearPsychOsc, CDialog)
	//{{AFX_MSG_MAP(CGearPsychOsc)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearPsychOsc message handlers

void CGearPsychOsc::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (doit)
	{
		_pMachine->_sineSpeed = 129-m_oscspeed.GetPos();
	}
	_pMachine->Update();
	
	char buffer[32];
	float period = 6.2831853f/_pMachine->_speed;
	sprintf(buffer, "%.1f Hz.", Global::pConfig->_pOutputDriver->_samplesPerSec/period);
	m_label1.SetWindowText(buffer);

	*pResult = 0;
}

void CGearPsychOsc::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (doit)
	{
		_pMachine->_sineGlide = 129-m_glidespeed.GetPos();
	}
	_pMachine->Update();

	char buffer[32];
	sprintf(buffer,"%.1f%%",_pMachine->_glide*155038.76f);
	m_label2.SetWindowText(buffer);

	*pResult = 0;
}

void CGearPsychOsc::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (doit)
	{
		_pMachine->_sineLfoSpeed = 129-m_lfofrequency.GetPos();
	}
	_pMachine->Update();

	char buffer[32];
	float period = 6.2831853f/_pMachine->_lfoSpeed;
	sprintf(buffer, "%.1f Hz.", Global::pConfig->_pOutputDriver->_samplesPerSec/period);
	m_label3.SetWindowText(buffer);

	*pResult = 0;
}

void CGearPsychOsc::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (doit)
	{
		_pMachine->_sineLfoAmp = 129-m_lfoamplitude.GetPos();
	}
	_pMachine->Update();

	char buffer[32];
	sprintf(buffer,"%.1f%%",_pMachine->_lfoAmp*1550.3876f);
	m_label4.SetWindowText(buffer);

	*pResult = 0;
}

BOOL CGearPsychOsc::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Initialize ranges
	m_oscspeed.SetRange(0, 128);
	m_glidespeed.SetRange(0, 128);
	m_lfofrequency.SetRange(0, 128);
	m_lfoamplitude.SetRange(0, 128);
	m_oscspeed.SetTicFreq(16);
	m_glidespeed.SetTicFreq(16);
	m_lfofrequency.SetTicFreq(16);
	m_lfoamplitude.SetTicFreq(16);

	// Initialize stuff
	m_oscspeed.SetPos(129-_pMachine->_sineSpeed);
	m_glidespeed.SetPos(129-_pMachine->_sineGlide);
	m_lfofrequency.SetPos(129-_pMachine->_sineLfoSpeed);
	m_lfoamplitude.SetPos(129-_pMachine->_sineLfoAmp);

	char buffer[64];
	sprintf(buffer, "%s [PsychOsc AM]", _pMachine->_editName);
	SetWindowText(buffer);

	doit = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// GearDelay.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "GearDelay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearDelay dialog


CGearDelay::CGearDelay(CWnd* pParent /*=NULL*/)
	: CDialog(CGearDelay::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearDelay)
	//}}AFX_DATA_INIT
	doit=false;

}


void CGearDelay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearDelay)
	DDX_Control(pDX, IDC_DRYLABEL, m_drylabel);
	DDX_Control(pDX, IDC_WETLABEL, m_wetlabel);
	DDX_Control(pDX, IDC_WETSLIDER, m_wetslider);
	DDX_Control(pDX, IDC_DRYSLIDER, m_dryslider);
	DDX_Control(pDX, IDC_LABEL_MSR, m_msr);
	DDX_Control(pDX, IDC_LABEL_MSL, m_msl);
	DDX_Control(pDX, IDC_F_LABEL_R, m_flabel_r);
	DDX_Control(pDX, IDC_F_LABEL_L, m_flabel_l);
	DDX_Control(pDX, IDC_SLIDER3, m_sliderfeedback_r);
	DDX_Control(pDX, IDC_SLIDER_TIME2, m_slidertime_r);
	DDX_Control(pDX, IDC_LABEL_LTIME2, m_label_timer);
	DDX_Control(pDX, IDC_SLIDER2, m_sliderfeedback_l);
	DDX_Control(pDX, IDC_LABEL_LTIME, m_label_timel);
	DDX_Control(pDX, IDC_SLIDER_TIME, m_slidertime_l);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearDelay, CDialog)
	//{{AFX_MSG_MAP(CGearDelay)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TIME, OnCustomdrawSliderTime)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TIME2, OnCustomdrawSliderTime2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON14, OnButton14)
	ON_BN_CLICKED(IDC_BUTTON15, OnButton15)
	ON_BN_CLICKED(IDC_BUTTON16, OnButton16)
	ON_BN_CLICKED(IDC_BUTTON17, OnButton17)
	ON_BN_CLICKED(IDC_BUTTON11, OnButton11)
	ON_BN_CLICKED(IDC_BUTTON18, OnButton18)
	ON_BN_CLICKED(IDC_BUTTON19, OnButton19)
	ON_BN_CLICKED(IDC_BUTTON20, OnButton20)
	ON_BN_CLICKED(IDC_BUTTON21, OnButton21)
	ON_BN_CLICKED(IDC_BUTTON22, OnButton22)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DRYSLIDER, OnCustomdrawDryslider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WETSLIDER, OnCustomdrawWetslider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearDelay message handlers

BOOL CGearDelay::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char buffer[64];
	sprintf(buffer,machineRef->editName);
	SetWindowText(buffer);
	
	m_slidertime_l.SetTicFreq(4096);
	m_slidertime_l.SetRange(1,MAX_DELAY_BUFFER-1,true);	
	m_slidertime_l.SetPos(machineRef->delayTimeL);
	
	m_sliderfeedback_l.SetTicFreq(10);
	m_sliderfeedback_l.SetRange(-100,100,true);	
	m_sliderfeedback_l.SetPos(machineRef->delayFeedbackL);
	
	m_slidertime_r.SetTicFreq(4096);
	m_slidertime_r.SetRange(1,MAX_DELAY_BUFFER-1,true);	
	m_slidertime_r.SetPos(machineRef->delayTimeR);
	
	m_sliderfeedback_r.SetTicFreq(10);
	m_sliderfeedback_r.SetRange(-100,100,true);	
	m_sliderfeedback_r.SetPos(machineRef->delayFeedbackR);
	
	
	m_dryslider.SetRange(-256,256,true);	
	m_dryslider.SetPos(machineRef->outDry);
	
	m_wetslider.SetRange(-256,256,true);
	m_wetslider.SetPos(machineRef->outWet);

	doit=true;

	return TRUE;

}

void CGearDelay::OnCustomdrawSliderTime(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->UpdateDelay(m_slidertime_l.GetPos(),NULL,0,0);

	char buffer[8];
	sprintf(buffer,"%d",machineRef->delayTimeL);
	m_label_timel.SetWindowText(buffer);
	
	sprintf(buffer,"%.2f",(float)machineRef->delayTimeL*0.0226757f);
	m_msl.SetWindowText(buffer);
	
	*pResult = 0;
}

void CGearDelay::OnCustomdrawSliderTime2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->UpdateDelay(NULL,m_slidertime_r.GetPos(),0,0);

	char buffer[8];
	sprintf(buffer,"%d",machineRef->delayTimeR);
	m_label_timer.SetWindowText(buffer);

	sprintf(buffer,"%.2f",(float)machineRef->delayTimeR*0.0226757f);
	m_msr.SetWindowText(buffer);

	*pResult = 0;
}

void CGearDelay::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->UpdateDelay(NULL,NULL,m_sliderfeedback_l.GetPos(),0);

	char buffer[8];
	sprintf(buffer,"%d%%",machineRef->delayFeedbackL);
	m_flabel_l.SetWindowText(buffer);
	*pResult = 0;
}

void CGearDelay::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->UpdateDelay(NULL,NULL,0,m_sliderfeedback_r.GetPos());

	char buffer[8];
	sprintf(buffer,"%d%%",machineRef->delayFeedbackR);
	m_flabel_r.SetWindowText(buffer);

	*pResult = 0;
}

// Tick [L] synchro buttons message handlers -------------------------

void CGearDelay::OnButton1() 
{
	machineRef->UpdateDelay(*SPT,NULL,0,0);
	m_slidertime_l.SetPos(machineRef->delayTimeL);
}

void CGearDelay::OnButton3() 
{
	machineRef->UpdateDelay(*SPT*2,NULL,0,0);
	m_slidertime_l.SetPos(machineRef->delayTimeL);
}

void CGearDelay::OnButton14() 
{
	machineRef->UpdateDelay(*SPT*3,NULL,0,0);
	m_slidertime_l.SetPos(machineRef->delayTimeL);
}

void CGearDelay::OnButton15() 
{
	machineRef->UpdateDelay(*SPT*4,NULL,0,0);
	m_slidertime_l.SetPos(machineRef->delayTimeL);	
}

void CGearDelay::OnButton16() 
{
	machineRef->UpdateDelay(*SPT*5,NULL,0,0);
	m_slidertime_l.SetPos(machineRef->delayTimeL);	
}

void CGearDelay::OnButton17() 
{
	machineRef->UpdateDelay(*SPT*6,NULL,0,0);
	m_slidertime_l.SetPos(machineRef->delayTimeL);	
}

// Tick [R] synchro buttons message handlers -------------------------

void CGearDelay::OnButton11() 
{
	machineRef->UpdateDelay(NULL,*SPT,0,0);
	m_slidertime_r.SetPos(machineRef->delayTimeR);
}

void CGearDelay::OnButton18() 
{
	machineRef->UpdateDelay(NULL,*SPT*2,0,0);
	m_slidertime_r.SetPos(machineRef->delayTimeR);
}

void CGearDelay::OnButton19() 
{
	machineRef->UpdateDelay(NULL,*SPT*3,0,0);
	m_slidertime_r.SetPos(machineRef->delayTimeR);
}

void CGearDelay::OnButton20() 
{
	machineRef->UpdateDelay(NULL,*SPT*4,0,0);
	m_slidertime_r.SetPos(machineRef->delayTimeR);
}

void CGearDelay::OnButton21() 
{
	machineRef->UpdateDelay(NULL,*SPT*5,0,0);
	m_slidertime_r.SetPos(machineRef->delayTimeR);
}

void CGearDelay::OnButton22() 
{
	machineRef->UpdateDelay(NULL,*SPT*6,0,0);
	m_slidertime_r.SetPos(machineRef->delayTimeR);
}

//
// Dry/Wet sliders control
//

void CGearDelay::OnCustomdrawDryslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->outDry=m_dryslider.GetPos();

	char buffer[8];
	sprintf(buffer,"%.1f%%",(float)machineRef->outDry*0.390625f);
	m_drylabel.SetWindowText(buffer);

	*pResult = 0;
}

void CGearDelay::OnCustomdrawWetslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(doit)
	machineRef->outWet=m_wetslider.GetPos();

	char buffer[8];
	sprintf(buffer,"%.1f%%",(float)machineRef->outWet*0.390625f);
	m_wetlabel.SetWindowText(buffer);

	*pResult = 0;
}

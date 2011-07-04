// MasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "MasterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog


CMasterDlg::CMasterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMasterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMasterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMasterDlg)
	DDX_Control(pDX, IDC_DBLEVEL, m_dblevel);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterDlg)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers

BOOL CMasterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_slider.SetRange(0,256,true);
	m_slider.SetPos(machineRef->outDry);
	
	return TRUE;
}

void CMasterDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	machineRef->outDry=m_slider.GetPos();

	float const mv=machineRef->outDry*0.00390625f;

	char buffer[16];

	if(mv>0.0f)
	sprintf(buffer,"%.1f dB",20.0f * log10(mv));
	else
	sprintf(buffer,"-Inf. dB");
	
	m_dblevel.SetWindowText(buffer);
	*pResult = 0;
}

// WireDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "WireDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWireDlg dialog


CWireDlg::CWireDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWireDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWireDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWireDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWireDlg)
	DDX_Control(pDX, IDC_STATIC1, m_volabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volslider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWireDlg, CDialog)
	//{{AFX_MSG_MAP(CWireDlg)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWireDlg message handlers

BOOL CWireDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	Inval=false;
	m_volslider.SetRange(0,128,true);
	m_volslider.SetTicFreq(16);
	m_volslider.SetPos(int(macRef->connectionVol[wireIndex]*128.0f));

	char buffer[64];
	sprintf(buffer,"%s -> %s[%d]",macRef->editName,destName,wireIndex);
	SetWindowText(buffer);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWireDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	macRef->connectionVol[wireIndex]=m_volslider.GetPos()*0.0078125f;	
	*pResult = 0;
	char buffer[32];

	if(macRef->connectionVol[wireIndex]>0.0f)
	sprintf(buffer,"%.1f dB",20.0f * log10(macRef->connectionVol[wireIndex]));
	else
	sprintf(buffer,"-Inf. dB");
	
	m_volabel.SetWindowText(buffer);
}

void CWireDlg::OnButton1() 
{
	Inval=true;
	macRef->conection[wireIndex]=false;
	macRef->numOutputs--;
	
	// Get number of wire destination machine
	int idstMac=macRef->outputDest[wireIndex];

	// Get reference to the destination machine
	psyGear* dstMac=songRef->machine[idstMac];

	for(int c=0;c<MAX_CONNECTIONS;c++)
	{
		if(dstMac->inCon[c])
		{
			if(dstMac->inputSource[c]==isrcMac)
			{
				dstMac->inCon[c]=false;
				dstMac->numInputs--;
			}
		}
	}// MAX CONNECTIONS

	OnOK();
}

// VstRack.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "VstRack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUMTICKS 500

/////////////////////////////////////////////////////////////////////////////
// CVstRack dialog


CVstRack::CVstRack(CWnd* pParent /*=NULL*/)
	: CDialog(CVstRack::IDD, pParent)
{
	canTweak=false;

	//{{AFX_DATA_INIT(CVstRack)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVstRack::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVstRack)
	DDX_Control(pDX, IDC_COMBO2, m_combo2);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	DDX_Control(pDX, IDC_TEXT1, m_text);
	DDX_Control(pDX, IDC_LIST1, m_parlist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVstRack, CDialog)
	//{{AFX_MSG_MAP(CVstRack)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeProgram)
	ON_BN_CLICKED(IDOK3, OnOk3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVstRack message handlers

BOOL CVstRack::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_pVst->GetPlugName(nPlug));

	// TODO: Add extra initialization here
	InitializePrograms();

	UpdateParList();
	
	//init slider range
	m_slider.SetRange(0,NUMTICKS);

	UpdateOne(0);

	canTweak=true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CVstRack::UpdateParList()
{
	m_parlist.ResetContent();

	char str[512];
	char buf[512];

	for(int i=0;i<m_pVst->effect[nPlug]->numParams;i++)
	{
			m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effGetParamName,i,0,str,0);
			sprintf(buf,"%.3X: %s",i,str);
			m_parlist.AddString(buf);
	}

	m_parlist.SetCurSel(0);
}

void CVstRack::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(canTweak)
	{
	float value=(float)(NUMTICKS-m_slider.GetPos())/NUMTICKS;
	m_pVst->effect[nPlug]->setParameter(m_pVst->effect[nPlug],nPar,value);
	UpdateText();
	}

	*pResult = 0;
}

CVstRack::UpdateText()
{
	//val text	
	char str[512];
	char str2[512];
	
	m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effGetParamDisplay,nPar,0,str,0.f);	
	m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effGetParamLabel,nPar,0,str2,0);
	
	strcat(str," ");
	strcat(str,str2);
			
	m_text.SetWindowText(str);
}

CVstRack::UpdateOne(int npar)
{
	nPar=npar;

	UpdateText();

	//update scroll bar with initial value
	float value=m_pVst->effect[nPlug]->getParameter(m_pVst->effect[nPlug],nPar);
	value=1.0f-value;
	m_slider.SetPos(int(value*NUMTICKS));
}

void CVstRack::OnSelchangeList1() 
{
	int const se=m_parlist.GetCurSel();

	UpdateOne(se);	
}

CVstRack::InitializePrograms()
{

	//--PROGRAMS--
	m_combo2.ResetContent();

	char str[512];
	
	previousProg=m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effGetProgram,0,0,NULL,0);

		for(int i=0;i<m_pVst->effect[nPlug]->numPrograms;i++)
		{
			char string[256];
			sprintf(string,"%d: ",i+1);

			m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effSetProgram,0,i,NULL,0);
			m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effGetProgramName,-1,i,str,0);
			strcat(string,str);
			m_combo2.AddString(string);
		}
		
		m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effSetProgram,0,previousProg,NULL,0);
		
		m_combo2.SetCurSel(0);
}

void CVstRack::OnSelchangeProgram() 
{
	int const se=m_combo2.GetCurSel();
	m_pVst->effect[nPlug]->dispatcher(m_pVst->effect[nPlug],effSetProgram,0,se,NULL,0.f);

	UpdateOne(nPar);
}

void CVstRack::OnOk3() 
{
	OnOK();	
}

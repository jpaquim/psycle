// GearPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "GearPlugin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearPlugin dialog


CGearPlugin::CGearPlugin(CWnd* pParent /*=NULL*/)
	: CDialog(CGearPlugin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGearPlugin)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGearPlugin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearPlugin)
	DDX_Control(pDX, IDC_PARLIST, m_parlist);
	DDX_Control(pDX, IDC_PLUGLABEL2, m_plugdev);
	DDX_Control(pDX, IDC_BUTTON1, m_combut1);
	DDX_Control(pDX, IDC_SHORTPAR, m_short);
	DDX_Control(pDX, IDC_DISPLAY, m_display);
	DDX_Control(pDX, IDC_SLIDER1, m_parcontrol);
	DDX_Control(pDX, IDC_PLUGLABEL, m_plugname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearPlugin, CDialog)
	//{{AFX_MSG_MAP(CGearPlugin)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowsePlug)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSliderPar)
	ON_BN_CLICKED(IDC_BUTTON3, OnReset)
	ON_BN_CLICKED(IDC_BUTTON14, OnPreset)
	ON_LBN_SELCHANGE(IDC_PARLIST, OnSelchangeParlist)
	ON_BN_CLICKED(IDC_BUTTON15, OnRandomTweak)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearPlugin message handlers

void CGearPlugin::OnBrowsePlug() 
{
	songRef->micb.hWnd=m_hWnd;

	machineRef->mi->Command();
}

void CGearPlugin::UpdateDialog() 
{
	m_parlist.ResetContent();

	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");

	if(bfxGetInfo()->Command!=NULL)
	{
	m_combut1.SetWindowText(bfxGetInfo()->Command);
	m_combut1.EnableWindow(true);
	}
	else
	{
		m_combut1.SetWindowText("");
		m_combut1.EnableWindow(false);
	}

	m_plugname.SetWindowText(bfxGetInfo()->Name);
	m_plugdev.SetWindowText(bfxGetInfo()->Author);

	for(int c=0;c<bfxGetInfo()->numParameters;c++)
	m_parlist.AddString(bfxGetInfo()->Parameters[c]->Description);

}

BOOL CGearPlugin::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	char buffer[64];
	sprintf(buffer,machineRef->editName);
	SetWindowText(buffer);
	
	UpdateDialog();

	SetCurrentParameter(machineRef->buzzpar);
	m_parlist.SetCurSel(machineRef->buzzpar);

	return TRUE;
}

void CGearPlugin::SetCurrentParameter(int p)
{
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");
	
	machineRef->buzzpar=p;

	m_short.SetWindowText(bfxGetInfo()->Parameters[p]->Name);

	int mi_v=bfxGetInfo()->Parameters[p]->MinValue;
	int ma_v=bfxGetInfo()->Parameters[p]->MaxValue;
	
	doit=false;
	
	m_parcontrol.SetRange(mi_v,ma_v,true);
	m_parcontrol.SetPos(machineRef->mi->Vals[p]);

	doit=true;
}

void CGearPlugin::OnCustomdrawSliderPar(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	x=m_parcontrol.GetPos();

	if(doit)
	machineRef->mi->ParameterTweak(machineRef->buzzpar,x);
	
	char buffer[32];

	if(machineRef->mi->DescribeValue(buffer,machineRef->buzzpar,x)==false)
	sprintf(buffer,"%d",x);

	m_display.SetWindowText(buffer);

	*pResult = 0;
}

void CGearPlugin::OnReset() 
{
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");

	for(int c=0;c<bfxGetInfo()->numParameters;c++)
	{
	int dv=bfxGetInfo()->Parameters[c]->DefValue;
	machineRef->mi->ParameterTweak(c,dv);
	}
	
	UpdateDialog();

	SetCurrentParameter(machineRef->buzzpar);
	m_parlist.SetCurSel(machineRef->buzzpar);
}

void CGearPlugin::OnPreset() 
{
MessageBox("Preset engine not implemented","Psycle");
}

void CGearPlugin::OnSelchangeParlist() 
{
	int p=m_parlist.GetCurSel();
	SetCurrentParameter(p);
}

void CGearPlugin::OnRandomTweak() 
{
	// Randomize controls
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(songRef->hBfxDll[machineRef->buzzdll],"GetInfo");

	for(int c=0;c<bfxGetInfo()->numParameters;c++)
	{
	int minran=bfxGetInfo()->Parameters[c]->MinValue;
	int maxran=bfxGetInfo()->Parameters[c]->MaxValue;
	
	int dif=maxran-minran;

	float randsem=(float)rand()*0.000030517578125f;

	float roffset=randsem*(float)dif;

	machineRef->mi->ParameterTweak(c,minran+int(roffset));
	}
	
	UpdateDialog();

	SetCurrentParameter(machineRef->buzzpar);
	m_parlist.SetCurSel(machineRef->buzzpar);
	
}

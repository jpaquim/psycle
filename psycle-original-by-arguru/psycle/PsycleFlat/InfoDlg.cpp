// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "psy.h"
#include "InfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog


CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDlg)
	DDX_Control(pDX, IDC_MEM_RESO4, m_mem_virtual);
	DDX_Control(pDX, IDC_MEM_RESO3, m_mem_pagefile);
	DDX_Control(pDX, IDC_MEM_RESO2, m_mem_phy);
	DDX_Control(pDX, IDC_MEM_RESO, m_mem_reso);
	DDX_Control(pDX, IDC_MCPULABEL2, m_cpurout);
	DDX_Control(pDX, IDC_MCPULABEL, m_machscpu);
	DDX_Control(pDX, IDC_CPUL, m_processor_label);
	DDX_Control(pDX, IDC_CPUIDLE_LABEL, m_cpuidlelabel);
	DDX_Control(pDX, IDC_MACHINELIST, m_machlist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CInfoDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers

BOOL CInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_machlist.InsertColumn(0,"Name",LVCFMT_LEFT,128,0);
	m_machlist.InsertColumn(1,"Machine",LVCFMT_LEFT,80,1);
	m_machlist.InsertColumn(2,"Type",LVCFMT_LEFT,64,1);
	m_machlist.InsertColumn(3,"WireIns",LVCFMT_LEFT,48,1);
	m_machlist.InsertColumn(4,"WireOuts",LVCFMT_LEFT,56,1);
	m_machlist.InsertColumn(5,"CPU",LVCFMT_LEFT,56,1);

	char buffer[64];
	sprintf(buffer,"%d MHZ",songRef->CPUHZ/1000000);
	m_processor_label.SetWindowText(buffer);

	UpdateInfo();

	InitTimer();

	return TRUE;
}

void CInfoDlg::InitTimer()
{
if(!SetTimer(1,500,NULL))
MessageBox("Couldnt initializate timer","Psycle Cpu Perfomance Refresh ERROR!", MB_ICONERROR);
}

void CInfoDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==1)
	{
	int n=0;
	char buffer[32];
	psyGear *tmac;

	float totalCPU=0;
	float machCPU=0;
	float machsCPU=0;
	float masterCPU=0;

	for(int c=0;c<MAX_MACHINES;c++)
	{
		if(songRef->Activemachine[c])
		{
		tmac=songRef->machine[c];
		machCPU=(float)tmac->cpuCost*0.1f;
		if(!c)masterCPU=machCPU;
		sprintf(buffer,"%.1f%%",machCPU);
		m_machlist.SetItem(n,5,LVIF_TEXT,buffer,0,0,0,NULL);
		n++;
		machsCPU+=machCPU;
		}
	}

	totalCPU=songRef->cpuIdle*0.1f+masterCPU;

	sprintf(buffer,"%.1f%%",totalCPU);
	m_cpuidlelabel.SetWindowText(buffer);
	
	sprintf(buffer,"%.1f%%",machsCPU);
	m_machscpu.SetWindowText(buffer);

	sprintf(buffer,"%.1f%%",totalCPU-machsCPU);
	m_cpurout.SetWindowText(buffer);

	// Memory status -------------------------------------------------

	MEMORYSTATUS lpBuffer;
	GlobalMemoryStatus(&lpBuffer);
	
	sprintf(buffer,"%d%%",100-lpBuffer.dwMemoryLoad);
	m_mem_reso.SetWindowText(buffer);

	sprintf(buffer,"%d / %d K",lpBuffer.dwAvailPhys/1024,lpBuffer.dwTotalPhys/1024);
	m_mem_phy.SetWindowText(buffer);

	sprintf(buffer,"%d / %d K",lpBuffer.dwAvailPageFile/1024,lpBuffer.dwTotalPageFile/1024);
	m_mem_pagefile.SetWindowText(buffer);

	sprintf(buffer,"%d / %d K",lpBuffer.dwAvailVirtual/1024,lpBuffer.dwTotalVirtual/1024);
	m_mem_virtual.SetWindowText(buffer);

	}

}

CInfoDlg::UpdateInfo()
{
	m_machlist.DeleteAllItems();

	int n=0;
	for(int c=0;c<MAX_MACHINES;c++)
	{
		if(songRef->Activemachine[c])
		{
		psyGear *tmac=songRef->machine[c];
		char buffer[32];
		
		// Name [Machine view editor custom name]
		sprintf(buffer,"%.3d: %s",n+1,tmac->editName);
		m_machlist.InsertItem(n,buffer);
		
		// Gear [Gear type]
		GetMachineName(buffer,tmac->machineMode);
		m_machlist.SetItem(n,1,LVIF_TEXT,buffer,0,0,0,NULL);
		
		// Type [Set is generator/effect/master]
		switch(tmac->type)
		{
		case 0:sprintf(buffer,"Generator");break;
		case 1:sprintf(buffer,"Effect");break;
		case 2:sprintf(buffer,"Master");break;
		}
		m_machlist.SetItem(n,2,LVIF_TEXT,buffer,0,0,0,NULL);
		
		// Input numbers
		sprintf(buffer,"%d",tmac->numInputs);
		m_machlist.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);
		
		// OutPut numbers
		sprintf(buffer,"%d",tmac->numOutputs);
		m_machlist.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
		
		
		n++;
		}
	}
}

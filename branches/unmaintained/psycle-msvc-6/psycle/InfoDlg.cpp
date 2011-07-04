// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "configuration.h"
#include "Song.h"
#include "InfoDlg.h"
#include "Machine.h"

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
	sprintf(buffer,"%d MHZ",Global::_cpuHz/1000000);
	m_processor_label.SetWindowText(buffer);
	
	UpdateInfo();
	
	InitTimer();
	
	return TRUE;
}

void CInfoDlg::InitTimer()
{
	if(!SetTimer(1,500,NULL))
		MessageBox("Error! Couldn't initialize timer","CPU Perfomance Dialog", MB_OK | MB_ICONERROR);
}

void CInfoDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==1 && !_pSong->_machineLock )
	{
		char buffer[32];
		
		float totalCPU=0;
		float machsCPU=0;
		float wiresCPU=0;

		unsigned tempSampCount = _pSong->_sampCount;
	    if( !tempSampCount ) tempSampCount=1;

		
		int n=0;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			if(_pSong->_machineActive[c])
			{
				float machCPU=0;
//				float masterCPU=0;
				Machine *tmac = _pSong->_pMachines[c];
//				machCPU = (float)tmac->_cpuCost*0.1f;
//				machCPU = ((float)tmac->_cpuCost/Global::_cpuHz) * 100;
				machCPU = ((float)tmac->_cpuCost/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
/*				if (!c)
				{
					masterCPU = machCPU;
				}*/
				sprintf(buffer,"%.1f%%",machCPU);
				m_machlist.SetItem(n,5,LVIF_TEXT,buffer,0,0,0,NULL);
				n++;
				machsCPU += machCPU;
//				wiresCPU += ((float)tmac->_wireCost/Global::_cpuHz)*100;
				wiresCPU += ((float)tmac->_wireCost/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
			}
		}
		
//		totalCPU = _pSong->cpuIdle*0.1f+masterCPU;
//		totalCPU = ((float)_pSong->cpuIdle/Global::_cpuHz)*100+machsCPU;
//		totalCPU = machsCPU + wiresCPU+((float)_pSong->cpuIdle/Global::_cpuHz)*100;
		totalCPU = machsCPU + wiresCPU+ ((float)_pSong->cpuIdle/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
		
		sprintf(buffer,"%.1f%%",totalCPU);
		m_cpuidlelabel.SetWindowText(buffer);
		
		sprintf(buffer,"%.1f%%",machsCPU);
		m_machscpu.SetWindowText(buffer);
		
//		sprintf(buffer,"%.1f%%",((float)_pSong->cpuIdle/Global::_cpuHz)*100);
//		sprintf(buffer,"%.1f%%",((float)_pSong->_pMachines[0]->_wireCost/Global::_cpuHz)*100);
		sprintf(buffer,"%.1f%%",wiresCPU);
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

void CInfoDlg::UpdateInfo()
{
	m_machlist.DeleteAllItems();
	
	int n=0;
	for(int c=0; c<MAX_MACHINES; c++)
	{
		if(_pSong->_machineActive[c])
		{
			Machine *tmac = _pSong->_pMachines[c];
			char buffer[64];
			
			// Name [Machine view editor custom name]
			sprintf(buffer,"%.3d: %s",n+1,tmac->_editName);
			m_machlist.InsertItem(n,buffer);
			
			// Gear [Gear type]
			strcpy(buffer, tmac->GetName());
			m_machlist.SetItem(n,1,LVIF_TEXT,buffer,0,0,0,NULL);
			
			// Type [Set is generator/effect/master]
			switch(tmac->_mode)
			{
			case MACHMODE_GENERATOR: strcpy(buffer,"Generator");break;
			case MACHMODE_FX: strcpy(buffer,"Effect");break;
			case MACHMODE_MASTER: strcpy(buffer,"Master");break;
			case MACHMODE_PLUGIN: strcpy(buffer, "Plug-in"); break;
			}
			m_machlist.SetItem(n,2,LVIF_TEXT,buffer,0,0,0,NULL);
			
			// Input numbers
			sprintf(buffer,"%d",tmac->_numInputs);
			m_machlist.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);
			
			// OutPut numbers
			sprintf(buffer,"%d",tmac->_numOutputs);
			m_machlist.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
			n++;
		}
	}
}

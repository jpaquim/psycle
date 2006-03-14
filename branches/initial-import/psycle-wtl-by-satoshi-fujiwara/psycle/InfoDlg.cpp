/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "configuration.h"
#include "Song.h"
#include "Instrument.h"
#include "InfoDlg.h"
#include "Machine.h"
#include ".\infodlg.h"



/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog


CInfoDlg::CInfoDlg()
{
	//{{AFX_DATA_INIT(CInfoDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


/*
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
*/


/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers


void CInfoDlg::InitTimer()
{
	if(!SetTimer(1,500,NULL))
		MessageBox(SF::CResourceString(IDS_ERR_MSG0072),
		SF::CResourceString(IDS_ERR_MSG0073), MB_OK | MB_ICONERROR);
}

void CInfoDlg::UpdateInfo()
{
	m_machlist.DeleteAllItems();
	
	int n=0;
	for(int c=0; c<MAX_MACHINES; c++)
	{
		Machine *tmac = _pSong->pMachine(c);
		if(tmac)
		{
			TCHAR buffer[128];
			
			// Name [Machine view editor custom name]
			_stprintf(buffer,_T("%.3d: %s"),n+1,tmac->_editName);
			m_machlist.InsertItem(n,buffer);
			
			// Gear [Gear type]
			_tcscpy(buffer, tmac->GetName());
			m_machlist.SetItem(n,1,LVIF_TEXT,buffer,0,0,0,NULL);
			
			// Type [Set is generator/effect/master]
			switch(tmac->_mode)
			{
			case MACHMODE_GENERATOR: _tcscpy(buffer,_T("Generator"));break;
			case MACHMODE_FX: _tcscpy(buffer,_T("Effect"));break;
			case MACHMODE_MASTER: _tcscpy(buffer,_T("Master"));break;
			}
			m_machlist.SetItem(n,2,LVIF_TEXT,buffer,0,0,0,NULL);
			
			// Input numbers
			_stprintf(buffer,_T("%d"),tmac->_numInputs);
			m_machlist.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);
			
			// OutPut numbers
			_stprintf(buffer,_T("%d"),tmac->_numOutputs);
			m_machlist.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
			n++;
		}
	}
	itemcount = n;
}

LRESULT CInfoDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
	ExecuteDlgInit(IDD_INFO);

	m_mem_virtual.Attach(GetDlgItem(IDC_MEM_RESO4));
	m_mem_pagefile.Attach(GetDlgItem(IDC_MEM_RESO3));
	m_mem_phy.Attach(GetDlgItem(IDC_MEM_RESO2));
	m_mem_reso.Attach(GetDlgItem(IDC_MEM_RESOf));
	m_cpurout.Attach(GetDlgItem(IDC_MCPULABEL2));
	m_machscpu.Attach(GetDlgItem(IDC_MCPULABEL));
	m_processor_label.Attach(GetDlgItem(IDC_CPUL));
	m_cpuidlelabel.Attach(GetDlgItem(IDC_CPUIDLE_LABEL));
	m_machlist.Attach(GetDlgItem(IDC_MACHINELIST));
	
	m_machlist.InsertColumn(0,SF::CResourceString(IDS_MSG0038),LVCFMT_LEFT,140,0);
	m_machlist.InsertColumn(1,SF::CResourceString(IDS_MSG0039),LVCFMT_LEFT,130,1);
	m_machlist.InsertColumn(2,SF::CResourceString(IDS_MSG0040),LVCFMT_LEFT,64,1);
	m_machlist.InsertColumn(3,SF::CResourceString(IDS_MSG0041),LVCFMT_RIGHT,50,1);
	m_machlist.InsertColumn(4,SF::CResourceString(IDS_MSG0042),LVCFMT_RIGHT,50,1);
	m_machlist.InsertColumn(5,SF::CResourceString(IDS_MSG0043),LVCFMT_RIGHT,48,1);
	
	TCHAR buffer[128];
	_stprintf(buffer,SF::CResourceString(IDS_MSG0044),Global::_cpuHz/1000000);
	m_processor_label.SetWindowText(buffer);
	
	UpdateInfo();
	
	InitTimer();
	
	return TRUE;
}

LRESULT CInfoDlg::OnTimer(UINT /*uMsg*/, WPARAM nIDEvent, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(nIDEvent==1 && !_pSong->IsMachineLock())
	{
		TCHAR buffer[128];
		
		float totalCPU=0;
		float machsCPU=0;
		float wiresCPU=0;

		unsigned tempSampCount = _pSong->SampCount();
	    if( !tempSampCount ) tempSampCount=1;
		
		int n=0;
		for (int c=0; c<MAX_MACHINES; c++)
		{
			Machine *tmac = _pSong->pMachine(c);
			if(tmac)
			{
				float machCPU=0;
//				float masterCPU=0;
//				machCPU = (float)tmac->_cpuCost*0.1f;
//				machCPU = ((float)tmac->_cpuCost/Global::_cpuHz) * 100;
				machCPU = ((float)tmac->_cpuCost/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
/*				if (!c)
				{
					masterCPU = machCPU;
				}*/
				_stprintf(buffer,_T("%.1f%%"),machCPU);
				m_machlist.SetItem(n,5,LVIF_TEXT,buffer,0,0,0,NULL);
				n++;
				machsCPU += machCPU;
//				wiresCPU += ((float)tmac->_wireCost/Global::_cpuHz)*100;
				wiresCPU += ((float)tmac->_wireCost/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
			}
		}
		if (itemcount != n)
		{
			UpdateInfo();
		}
		
//		totalCPU = _pSong->cpuIdle*0.1f+masterCPU;
//		totalCPU = ((float)_pSong->cpuIdle/Global::_cpuHz)*100+machsCPU;
//		totalCPU = machsCPU + wiresCPU+((float)_pSong->cpuIdle/Global::_cpuHz)*100;
		totalCPU = 
			machsCPU + wiresCPU+ ((float)_pSong->CpuIdle() / Global::_cpuHz)
			* ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
		
		_stprintf(buffer,_T("%.1f%%"),totalCPU);
		m_cpuidlelabel.SetWindowText(buffer);
		
		_stprintf(buffer,_T("%.1f%%"),machsCPU);
		m_machscpu.SetWindowText(buffer);
		
//		_stprintf(buffer,"%.1f%%",((float)_pSong->cpuIdle/Global::_cpuHz)*100);
//		_stprintf(buffer,"%.1f%%",((float)_pSong->_pMachines[MASTER_INDEX]->_wireCost/Global::_cpuHz)*100);
		_stprintf(buffer,_T("%.1f%%"),wiresCPU);
		m_cpurout.SetWindowText(buffer);
		
		// Memory status -------------------------------------------------
		
		MEMORYSTATUS lpBuffer;
		GlobalMemoryStatus(&lpBuffer);
		
		_stprintf(buffer,_T("%d%%"),100 - lpBuffer.dwMemoryLoad);
		m_mem_reso.SetWindowText(buffer);
		
		SF::CResourceString _fmt(IDS_MSG0045);

		_stprintf(buffer,_fmt,lpBuffer.dwTotalPhys/1048576.0f,lpBuffer.dwAvailPhys/1048576.0f);
		m_mem_phy.SetWindowText(buffer);
		
		_stprintf(buffer,_fmt,lpBuffer.dwTotalPageFile/1048576.0f,lpBuffer.dwAvailPageFile/1048576.0f);
		m_mem_pagefile.SetWindowText(buffer);
		
		_stprintf(buffer,_fmt,lpBuffer.dwTotalVirtual/1048576.0f,lpBuffer.dwAvailVirtual/1048576.0f);
		m_mem_virtual.SetWindowText(buffer);
	}
	return 0;
}

LRESULT CInfoDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return this->ShowWindow(SW_HIDE);
	return 0;
}

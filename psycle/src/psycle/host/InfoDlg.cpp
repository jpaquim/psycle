///\file
///\brief implementation file for psycle::host::CInfoDlg.

#include "InfoDlg.hpp"
#include "Configuration.hpp"
#include "projectdata.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
using namespace psycle::core;
#else
#include "Song.hpp"
#include "Machine.hpp"
#endif

namespace psycle {
	namespace host {
		CInfoDlg::CInfoDlg(ProjectData* projects, CWnd* pParent)
			: CDialog(CInfoDlg::IDD, pParent),
			  projects_(projects) {
		}

		void CInfoDlg::DoDataExchange(CDataExchange* pDX) {
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_MEM_RESO4, mem_virtual_);
			DDX_Control(pDX, IDC_MEM_RESO3, mem_pagefile_);
			DDX_Control(pDX, IDC_MEM_RESO2, mem_phy_);
			DDX_Control(pDX, IDC_MEM_RESO, mem_reso_);
			DDX_Control(pDX, IDC_MCPULABEL2, cpurout_);
			DDX_Control(pDX, IDC_MCPULABEL, machscpu_);
			DDX_Control(pDX, IDC_CPUL, processor_label_);
			DDX_Control(pDX, IDC_CPUIDLE_LABEL, cpuidlelabel_);
			DDX_Control(pDX, IDC_MACHINELIST, machlist_);
		}

		BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
			ON_WM_TIMER()
		END_MESSAGE_MAP()

		BOOL CInfoDlg::OnInitDialog() {
			CDialog::OnInitDialog();			
			machlist_.InsertColumn(0,"Name",LVCFMT_LEFT,120,0);
			machlist_.InsertColumn(1,"Machine",LVCFMT_LEFT,90,1);
			machlist_.InsertColumn(2,"Type",LVCFMT_LEFT,64,1);
			machlist_.InsertColumn(3,"InWire",LVCFMT_RIGHT,46,1);
			machlist_.InsertColumn(4,"Outwire",LVCFMT_RIGHT,50,1);
			machlist_.InsertColumn(5,"CPU",LVCFMT_RIGHT,48,1);
			
			char buffer[128];
			///\todo:  Using the Windows API to get clicks/CPU frequency doesn't give
			// the real frequency in some cases. This has to be worked out.
			if ( Global::_cpuHz/1000000 < 10 )
				strcpy(buffer,"Unknown");
			else 
				sprintf(buffer,"%d MHZ",Global::_cpuHz/1000000);
			processor_label_.SetWindowText(buffer);
			
			UpdateInfo();		
			InitTimer();			
			return true;
		}

		void CInfoDlg::InitTimer() {
			if(!SetTimer(1,500,NULL))
				MessageBox("Error! Couldn't initialize timer","CPU Perfomance Dialog", MB_OK | MB_ICONERROR);
		}

		void CInfoDlg::OnTimer(UINT nIDEvent) {
			if(nIDEvent==1) {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				Song::scoped_lock lock(projects_->active_project()->song());
#endif
				char buffer[128];
				
				float totalCPU=0;
				float machsCPU=0;
				float wiresCPU=0;

#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo
				unsigned tempSampCount = 1;
#else
				unsigned tempSampCount = projects_->active_project()->song()._sampCount;
				if( !tempSampCount ) tempSampCount=1;
#endif
				
				int n=0;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					Machine *tmac = projects_->active_project()->song().machine(c);
					if(tmac)
					{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						// Input numbers
						sprintf(buffer,"%d",tmac->_connectedInputs);
						machlist_.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);

						// OutPut numbers
						sprintf(buffer,"%d",tmac->_connectedOutputs);
						machlist_.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
#else
						// Input numbers
						sprintf(buffer,"%d",tmac->_numInputs);
						machlist_.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);

						// OutPut numbers
						sprintf(buffer,"%d",tmac->_numOutputs);
						machlist_.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
#endif

						float machCPU=0;
		//				float masterCPU=0;
		//				machCPU = (float)tmac->_cpuCost*0.1f;
		//				machCPU = ((float)tmac->_cpuCost/Global::_cpuHz) * 100;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						//todo
#else
						machCPU = ((float)tmac->_cpuCost/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
#endif
		/*				if (!c)
						{
							masterCPU = machCPU;
						}*/
						sprintf(buffer,"%.1f%%",machCPU);
						machlist_.SetItem(n,5,LVIF_TEXT,buffer,0,0,0,NULL);
						n++;
						machsCPU += machCPU;
		//				wiresCPU += ((float)tmac->_wireCost/Global::_cpuHz)*100;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						//todo
#else
						wiresCPU += ((float)tmac->_wireCost/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
#endif
					}
				}
				if (item_count_ != n)
				{
					UpdateInfo();
				}
				
		//		totalCPU = _pSong->cpuIdle*0.1f+masterCPU;
		//		totalCPU = ((float)_pSong->cpuIdle/Global::_cpuHz)*100+machsCPU;
		//		totalCPU = machsCPU + wiresCPU+((float)_pSong->cpuIdle/Global::_cpuHz)*100;
		//		totalCPU = machsCPU + wiresCPU+ ((float)_pSong->cpuIdle/Global::_cpuHz) * ((float)Global::pConfig->_pOutputDriver->_samplesPerSec/tempSampCount)*100;
				totalCPU = machsCPU + wiresCPU;

				sprintf(buffer,"%.1f%%",totalCPU);
				cpuidlelabel_.SetWindowText(buffer);
				
				sprintf(buffer,"%.1f%%",machsCPU);
				machscpu_.SetWindowText(buffer);
				
		//		sprintf(buffer,"%.1f%%",((float)_pSong->cpuIdle/Global::_cpuHz)*100);
		//		sprintf(buffer,"%.1f%%",((float)_pSong->_pMachines[MASTER_INDEX]->_wireCost/Global::_cpuHz)*100);
				sprintf(buffer,"%.1f%%",wiresCPU);
				cpurout_.SetWindowText(buffer);
				
				// Memory status -------------------------------------------------
				
				MEMORYSTATUS lpBuffer;
				GlobalMemoryStatus(&lpBuffer);
				
				sprintf(buffer,"%d%%",100-lpBuffer.dwMemoryLoad);
				mem_reso_.SetWindowText(buffer);
				
				sprintf(buffer,"%.1fM (of %.1fM)",lpBuffer.dwAvailPhys/1048576.0f,lpBuffer.dwTotalPhys/1048576.0f);
				mem_phy_.SetWindowText(buffer);
				
				sprintf(buffer,"%.1fM (of %.1fM)",lpBuffer.dwAvailPageFile/1048576.0f,lpBuffer.dwTotalPageFile/1048576.0f);
				mem_pagefile_.SetWindowText(buffer);
				
				sprintf(buffer,"%.1fM (of %.1fM)",lpBuffer.dwAvailVirtual/1048576.0f,lpBuffer.dwTotalVirtual/1048576.0f);
				mem_virtual_.SetWindowText(buffer);
			}
		}

		void CInfoDlg::UpdateInfo()
		{
			machlist_.DeleteAllItems();
			
			int n=0;
			for(int c=0; c<MAX_MACHINES; c++)
			{
				Machine *tmac = projects_->active_project()->song().machine(c);
				if(tmac)
				{
					char buffer[128];
					
					// Name [Machine view editor custom name]
					sprintf(buffer,"%.3d: %s",n+1,tmac->GetEditName().c_str());
					machlist_.InsertItem(n,buffer);
					
					// Gear [Gear type]
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					strcpy(buffer, tmac->GetName().c_str());
#else
					strcpy(buffer, tmac->GetName());
#endif
					machlist_.SetItem(n,1,LVIF_TEXT,buffer,0,0,0,NULL);
					
					// Type [Set is generator/effect/master]
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					if (tmac->getMachineKey() == MachineKey::master()) {
						strcpy(buffer,"Master");
					}
					else if (tmac->IsGenerator() ) {
						strcpy(buffer,"Generator");
					}
					else { strcpy(buffer,"Effect"); }
					machlist_.SetItem(n,2,LVIF_TEXT,buffer,0,0,0,NULL);

					// Input numbers
					sprintf(buffer,"%d",tmac->_connectedInputs);
					machlist_.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);
					
					// OutPut numbers
					sprintf(buffer,"%d",tmac->_connectedOutputs);
					machlist_.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
#else
					switch(tmac->_mode)
					{
					case MACHMODE_GENERATOR: strcpy(buffer,"Generator");break;
					case MACHMODE_FX: strcpy(buffer,"Effect");break;
					case MACHMODE_MASTER: strcpy(buffer,"Master");break;
					}
					machlist_.SetItem(n,2,LVIF_TEXT,buffer,0,0,0,NULL);

					// Input numbers
					sprintf(buffer,"%d",tmac->_numInputs);
					machlist_.SetItem(n,3,LVIF_TEXT,buffer,0,0,0,NULL);
					
					// OutPut numbers
					sprintf(buffer,"%d",tmac->_numOutputs);
					machlist_.SetItem(n,4,LVIF_TEXT,buffer,0,0,0,NULL);
#endif
					++n;
				}
			}
			item_count_ = n;
		}
	}   // namespace
}   // namespace

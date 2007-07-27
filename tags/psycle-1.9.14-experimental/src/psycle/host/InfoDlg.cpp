///\file
///\brief implementation file for psycle::host::CInfoDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/InfoDlg.hpp>
//#include <psycle/host/configuration.hpp>
#include <psycle/engine/Song.hpp>
#include <psycle/engine/player.hpp>
#include <psycle/engine/Machine.hpp>
#include <sstream>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		CInfoDlg::CInfoDlg(CWnd* pParent)
		: CDialog(CInfoDlg::IDD, pParent)
		{
		}

		void CInfoDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_MEM_RESO4, m_mem_virtual);
			DDX_Control(pDX, IDC_MEM_RESO3, m_mem_pagefile);
			DDX_Control(pDX, IDC_MEM_RESO2, m_mem_phy);
			DDX_Control(pDX, IDC_MEM_RESO, m_mem_reso);
			DDX_Control(pDX, IDC_MCPULABEL2, m_cpurout);
			DDX_Control(pDX, IDC_MCPULABEL, m_machscpu);
			DDX_Control(pDX, IDC_CPUL, m_processor_label);
			DDX_Control(pDX, IDC_CPUIDLE_LABEL, m_cpuidlelabel);
			DDX_Control(pDX, IDC_MACHINELIST, m_machlist);
		}

		BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
			ON_WM_TIMER()
		END_MESSAGE_MAP()

		BOOL CInfoDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			
			m_machlist.InsertColumn(0,"Name",LVCFMT_LEFT,120,0);
			m_machlist.InsertColumn(1,"Machine",LVCFMT_LEFT,90,1);
			m_machlist.InsertColumn(2,"Type",LVCFMT_LEFT,64,1);
			m_machlist.InsertColumn(3,"InWire",LVCFMT_RIGHT,46,1);
			m_machlist.InsertColumn(4,"Outwire",LVCFMT_RIGHT,50,1);
			m_machlist.InsertColumn(5,"CPU",LVCFMT_RIGHT,48,1);
			
			{
				std::ostringstream s;
				s << static_cast<cpu::cycles_type>(Global::cpu_frequency() * 1e-6) << " MHz";
				m_processor_label.SetWindowText(s.str().c_str());
			}
			
			UpdateInfo();
			
			InitTimer();
			
			return true;
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
				char buffer[128];
				
				float totalCPU=0;
				float machsCPU=0;
				float wiresCPU=0;

				unsigned tempSampCount = _pSong->_sampCount;
				if( !tempSampCount ) tempSampCount=1;
				
				int n=0;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					Machine *tmac = _pSong->_pMachine[c];
					if(tmac)
					{
						float machCPU=0;
		//				float masterCPU=0;
		//				machCPU = (float)tmac->work_cpu_cost()*0.1f;
		//				machCPU = ((float)tmac->work_cpu_cost()/Global::cpu_frequency()) * 100;
						machCPU = ((float)tmac->work_cpu_cost()/Global::cpu_frequency()) * ((float)Global::player().SampleRate()/tempSampCount)*100;
		/*				if (!c)
						{
							masterCPU = machCPU;
						}*/
						sprintf(buffer,"%.1f%%",machCPU);
						m_machlist.SetItem(n,5,LVIF_TEXT,buffer,0,0,0,NULL);
						n++;
						machsCPU += machCPU;
		//				wiresCPU += ((float)tmac->wire_cpu_cost()/Global::cpu_frequency())*100;
						wiresCPU += ((float)tmac->wire_cpu_cost()/Global::cpu_frequency()) * ((float)Global::player().SampleRate()/tempSampCount)*100;
					}
				}
				if (itemcount != n)
				{
					UpdateInfo();
				}
				
		//		totalCPU = _pSong->cpuIdle*0.1f+masterCPU;
		//		totalCPU = ((float)_pSong->cpuIdle/Global::_cpuHz)*100+machsCPU;
		//		totalCPU = machsCPU + wiresCPU + ((float)_pSong->cpu_idle()/Global::cpu_frequency())*100;
				totalCPU = machsCPU + wiresCPU + ((float)_pSong->cpu_idle()/Global::cpu_frequency()) * ((float)Global::player().SampleRate()/tempSampCount)*100;
				
				sprintf(buffer,"%.1f%%",totalCPU);
				m_cpuidlelabel.SetWindowText(buffer);
				
				sprintf(buffer,"%.1f%%",machsCPU);
				m_machscpu.SetWindowText(buffer);
				
		//		sprintf(buffer,"%.1f%%",((float)_pSong->cpu_idle()/Global::cpu_frequency())*100);
		//		sprintf(buffer,"%.1f%%",((float)_pSong->_pMachines[MASTER_INDEX]->wire_cpu_cost()/Global::cpu_frequency())*100);
				sprintf(buffer,"%.1f%%",wiresCPU);
				m_cpurout.SetWindowText(buffer);
				
				// Memory status -------------------------------------------------
				
				MEMORYSTATUS lpBuffer;
				GlobalMemoryStatus(&lpBuffer);
				
				sprintf(buffer,"%d%%",100-lpBuffer.dwMemoryLoad);
				m_mem_reso.SetWindowText(buffer);
				
				sprintf(buffer,"%.1fM (of %.1fM)",lpBuffer.dwAvailPhys/1048576.0f,lpBuffer.dwTotalPhys/1048576.0f);
				m_mem_phy.SetWindowText(buffer);
				
				sprintf(buffer,"%.1fM (of %.1fM)",lpBuffer.dwAvailPageFile/1048576.0f,lpBuffer.dwTotalPageFile/1048576.0f);
				m_mem_pagefile.SetWindowText(buffer);
				
				sprintf(buffer,"%.1fM (of %.1fM)",lpBuffer.dwAvailVirtual/1048576.0f,lpBuffer.dwTotalVirtual/1048576.0f);
				m_mem_virtual.SetWindowText(buffer);
			}
		}

		void CInfoDlg::UpdateInfo()
		{
			m_machlist.DeleteAllItems();
			
			int n=0;
			for(int c=0; c<MAX_MACHINES; c++)
			{
				Machine *tmac = _pSong->_pMachine[c];
				if(tmac)
				{
					// Name [Machine view editor custom name]
					{
						std::ostringstream s;
						s << n + 1 << ": " << tmac->GetEditName();
						m_machlist.InsertItem(n,s.str().c_str());
					}
					
					// Gear [Gear type]
					m_machlist.SetItem(n,1,LVIF_TEXT,tmac->GetBrand().c_str(),0,0,0,0);
					
					// Type [Set is generator/effect/master]
					{
						std::string s;
						switch(tmac->_mode)
						{
							case MACHMODE_GENERATOR: s = "Generator"; break;
							case MACHMODE_FX: s = "Effect"; break;
							case MACHMODE_MASTER: s = "Master"; break;
						}
						m_machlist.SetItem(n,2,LVIF_TEXT,s.c_str(),0,0,0,0);
					}

					// Input numbers
					{
						std::ostringstream s;
						s << tmac->_connectedInputs;
						m_machlist.SetItem(n,3,LVIF_TEXT,s.str().c_str(),0,0,0,0);
					}

					// OutPut numbers
					{
						std::ostringstream s;
						s << tmac->_connectedOutputs;
						m_machlist.SetItem(n,4,LVIF_TEXT,s.str().c_str(),0,0,0,0);
					}

					++n;
				}
			}
			itemcount = n;
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END


///\file
///\brief implementation file for psycle::host::CInfoDlg.

#include "InfoDlg.hpp"
#include "Configuration.hpp"
#include "projectdata.hpp"

#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/internalkeys.hpp>
#include <psycle/core/cpu_time_clock.hpp>
#include <universalis/stdlib/thread.hpp>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

using namespace core;
using namespace universalis::stdlib;

CInfoDlg::CInfoDlg(ProjectData* projects, CWnd* pParent)
: CDialog(CInfoDlg::IDD, pParent), projects_(projects) {}

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
	machlist_.InsertColumn(0, "Name"     , LVCFMT_LEFT , 118, 0);
	machlist_.InsertColumn(1, "Type"     , LVCFMT_LEFT ,  78, 1);
	machlist_.InsertColumn(2, "Kind"     , LVCFMT_LEFT ,  62, 1);
	machlist_.InsertColumn(3, "In wires" , LVCFMT_RIGHT,  52, 1);
	machlist_.InsertColumn(4, "Out wires", LVCFMT_RIGHT,  58, 1);
	machlist_.InsertColumn(5, "CPU"      , LVCFMT_RIGHT,  48, 1);

	UpdateInfo();		
	InitTimer();			
	return true;
}

void CInfoDlg::InitTimer() {
	if(!SetTimer(1, 1000, 0))
		MessageBox("Error! Could not initialize timer", "CPU Performance Dialog", MB_OK | MB_ICONERROR);
}

void CInfoDlg::OnTimer(UINT nIDEvent) {
	if(nIDEvent != 1) return;

	Song & song = projects_->active_project()->song();
	Song::scoped_lock lock(song);

	char buffer[128];
	
	nanoseconds total_machine_processing_time;
	nanoseconds const now = core::cpu_time_clock();
	nanoseconds const real_time_duration = now - last_update_time_;

	unsigned int i = 0;
	for(unsigned int m(0); m < MAX_MACHINES; ++m) if(song.machine(m)) {
		Machine & machine(*song.machine(m));

		// Input numbers
		sprintf(buffer, "%d", machine._connectedInputs);
		machlist_.SetItem(i, 3, LVIF_TEXT, buffer, 0, 0, 0, 0);

		// OutPut numbers
		sprintf(buffer, "%d", machine._connectedOutputs);
		machlist_.SetItem(i, 4, LVIF_TEXT, buffer, 0, 0, 0, 0);

		{ // processing cpu percent
			float const percent = 100.0f * machine.accumulated_processing_time().get_count() / real_time_duration.get_count();
			sprintf(buffer, "%.1f%%", percent);
			machlist_.SetItem(i, 5, LVIF_TEXT, buffer, 0, 0, 0, 0);
		}

		total_machine_processing_time += machine.accumulated_processing_time();
		machine.reset_time_measurement();

		++i;
	}

	last_update_time_ = now;

	if(item_count_ != i) UpdateInfo();
	
	{ // total cpu percent (counts everything, not just machine processing + routing)
		float const percent = 100.0f * song.accumulated_processing_time().get_count() / real_time_duration.get_count();
		sprintf(buffer, "%.1f%%", percent);
		cpuidlelabel_.SetWindowText(buffer);
	}
	
	{ // total machine processing cpu percent
		float const percent = 100.0f * total_machine_processing_time.get_count() / real_time_duration.get_count();
		sprintf(buffer, "%.1f%%", percent);
		machscpu_.SetWindowText(buffer);
	}

	{ // routing cpu percent
		float const percent = 100.0f * song.accumulated_routing_time().get_count() / real_time_duration.get_count();
		sprintf(buffer, "%.1f%%", percent);
		cpurout_.SetWindowText(buffer);
	}
	
	{ // cpu frequency
		///\todo:  Using the Windows API to get clicks/CPU frequency doesn't give
		// the real frequency in some cases. This has to be worked out.
		LARGE_INTEGER frequency;
		if(!::QueryPerformanceFrequency(&frequency)) strcpy(buffer, "unsupported");
		if(frequency.QuadPart / 1000000 < 10) strcpy(buffer, "unknown");
		else sprintf(buffer, "%dx%d MHz", thread::hardware_concurrency(), frequency.QuadPart / 1000000);
		processor_label_.SetWindowText(buffer);
	}

	song.reset_time_measurement();

	{ // memory status
		MEMORYSTATUS lpBuffer;
		GlobalMemoryStatus(&lpBuffer);

		sprintf(buffer, "%d%%", 100 - lpBuffer.dwMemoryLoad);
		mem_reso_.SetWindowText(buffer);
		
		sprintf(buffer, "%.1fM (of %.1fM)", lpBuffer.dwAvailPhys / 1048576.0f, lpBuffer.dwTotalPhys / 1048576.0f);
		mem_phy_.SetWindowText(buffer);
		
		sprintf(buffer, "%.1fM (of %.1fM)", lpBuffer.dwAvailPageFile / 1048576.0f, lpBuffer.dwTotalPageFile / 1048576.0f);
		mem_pagefile_.SetWindowText(buffer);
		
		sprintf(buffer, "%.1fM (of %.1fM)", lpBuffer.dwAvailVirtual / 1048576.0f, lpBuffer.dwTotalVirtual / 1048576.0f);
		mem_virtual_.SetWindowText(buffer);
	}
}

void CInfoDlg::UpdateInfo() {
	machlist_.DeleteAllItems();
	
	Song & song = projects_->active_project()->song();

	unsigned int i = 0;
	for(unsigned int m(0); m < MAX_MACHINES; ++m) if(song.machine(m)) {
		Machine & machine(*song.machine(m));

		char buffer[128];
			
		// Name [Machine view editor custom name]
		sprintf(buffer, "%.3d: %s", i + 1, machine.GetEditName().c_str());
		machlist_.InsertItem(i, buffer);
			
		// Gear [Gear type]
		strcpy(buffer, machine.GetName().c_str());
		machlist_.SetItem(i, 1, LVIF_TEXT, buffer, 0, 0, 0, 0);
			
		// Type [Set is generator/effect/master]
		if(machine.getMachineKey() == InternalKeys::master) strcpy(buffer, "Master");
		else if(machine.IsGenerator()) strcpy(buffer, "Generator");
		else strcpy(buffer, "Effect");

		machlist_.SetItem(i, 2, LVIF_TEXT, buffer, 0, 0, 0, 0);

		// Input numbers
		sprintf(buffer, "%d", machine._connectedInputs);
		machlist_.SetItem(i, 3, LVIF_TEXT, buffer, 0, 0, 0, 0);
			
		// OutPut numbers
		sprintf(buffer, "%d", machine._connectedOutputs);
		machlist_.SetItem(i, 4, LVIF_TEXT, buffer, 0, 0, 0, 0);

		++i;
	}
	item_count_ = i;
}

}}

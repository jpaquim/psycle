///\file
///\brief implementation file for psycle::host::CInfoDlg.

#include <psycle/host/detail/project.private.hpp>
#include "InfoDlg.hpp"
#include "Configuration.hpp"
#include "projectdata.hpp"

#include <psycle/core/player.h>
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/internalkeys.hpp>
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/chrono.hpp>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

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

void CInfoDlg::OnTimer(UINT_PTR nIDEvent) {
	if(nIDEvent != 1) return;

	Song & song = projects_->active_project()->song();

	if(!song.is_ready()) return;

	Song::scoped_lock lock(song);

	char buffer[128];
	
	chrono::nanoseconds total_machine_processing_time = 0;
	core::wall_time_clock::time_point const now = core::wall_time_clock::now();
	float const real_time_duration = chrono::nanoseconds(now - last_update_time_).count();
	std::size_t threads = Player::singleton().num_threads();
	if(!threads) threads = 1; // Beware: when not using multithreading, Player::singleton().num_threads() is zero!
	float const multicore_real_time_duration = real_time_duration * threads;

	unsigned int i = 0;
	for(unsigned int m(0); m < MAX_MACHINES; ++m) if(song.machine(m)) {
		Machine & machine(*song.machine(m));

		// Input numbers
		sprintf(buffer, "%d", machine._connectedInputs);
		machlist_.SetItem(i, 3, LVIF_TEXT, buffer, 0, 0, 0, 0);

		// Output numbers
		sprintf(buffer, "%d", machine._connectedOutputs);
		machlist_.SetItem(i, 4, LVIF_TEXT, buffer, 0, 0, 0, 0);

		{ // processing cpu percent
			float const percent = 100.0f * chrono::nanoseconds(machine.accumulated_processing_time()).count() / multicore_real_time_duration;
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
		//Accumulated processing time does not count "num_threads_running since it is acummulated in the Player thread (single threaded)
		float const percent = 100.0f * chrono::nanoseconds(song.accumulated_processing_time()).count() / real_time_duration;
		sprintf(buffer, "%.1f%%", percent);
		cpuidlelabel_.SetWindowText(buffer);
	}
	
	{ // total machine processing cpu percent
		float const percent = 100.0f * total_machine_processing_time.count() / multicore_real_time_duration;
		sprintf(buffer, "%.1f%%", percent);
		machscpu_.SetWindowText(buffer);
	}

	{ // routing cpu percent
		float const percent = 100.0f * chrono::nanoseconds(song.accumulated_routing_time()).count() / multicore_real_time_duration;
		sprintf(buffer, "%.1f%%", percent);
		cpurout_.SetWindowText(buffer);
	}
	
	{ // cpu frequency
		///\todo:  Using the Windows API to get clicks/CPU frequency doesn't give
		// the real frequency in some cases. This has to be worked out.
		LARGE_INTEGER frequency;
		if(!::QueryPerformanceFrequency(&frequency)) strcpy(buffer, "unsupported");
		if(frequency.QuadPart / 1000000 < 10) sprintf(buffer, "%d threads", threads);
		else sprintf(buffer, "%dx%d MHz", threads, frequency.QuadPart / 1000000);
		processor_label_.SetWindowText(buffer);
	}

	song.reset_time_measurement();

	{ // memory status
		MEMORYSTATUSEX lpBuffer;
		lpBuffer.dwLength = sizeof lpBuffer;
		GlobalMemoryStatusEx(&lpBuffer);

		std::sprintf(buffer, "%d%%", 100 - lpBuffer.dwMemoryLoad);
		mem_reso_.SetWindowText(buffer);
		
		std::sprintf(buffer, "%.0fM (of %.0fM)", lpBuffer.ullAvailPhys / float(1<<20), lpBuffer.ullTotalPhys / float(1<<20));
		mem_phy_.SetWindowText(buffer);
		
		std::sprintf(buffer, "%.0fM (of %.0fM)", lpBuffer.ullAvailPageFile / float(1<<20), lpBuffer.ullTotalPageFile / float(1<<20));
		mem_pagefile_.SetWindowText(buffer);
		
		#ifdef _WIN64
			std::sprintf(buffer,"%.0fG (of %.0fG)", lpBuffer.ullAvailVirtual / float(1<<30), lpBuffer.ullTotalVirtual / float(1<<30));
		#else
			std::sprintf(buffer,"%.0fM (of %.0fM)", lpBuffer.ullAvailVirtual / float(1<<20), lpBuffer.ullTotalVirtual / float(1<<20));
		#endif
		m_mem_virtual.SetWindowText(buffer);
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

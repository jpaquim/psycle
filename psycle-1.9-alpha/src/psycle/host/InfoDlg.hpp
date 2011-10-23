///\file
///\brief interface file for psycle::host::CInfoDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include <psycle/core/cpu_time_clock.hpp>

namespace psycle { namespace host {

/// info window.
class CInfoDlg : public CDialog {
	public:
		CInfoDlg(class ProjectData* projects, CWnd* pParent = 0);
		~CInfoDlg() {}

		void UpdateInfo();

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnInitDialog();
		afx_msg void OnTimer(UINT_PTR nIDEvent);		
		DECLARE_MESSAGE_MAP()

	private:
		void InitTimer();			

		enum { IDD = IDD_INFO };

		CStatic	mem_virtual_;
		CStatic	mem_pagefile_;
		CStatic	mem_phy_;
		CStatic	mem_reso_;
		CStatic	cpurout_;
		CStatic	machscpu_;
		CStatic	processor_label_;
		CStatic	cpuidlelabel_;
		CListCtrl machlist_;

		unsigned int item_count_;
		core::wall_time_clock::time_point last_update_time_;
		ProjectData* projects_;
};

}}

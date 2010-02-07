///\file
///\brief interface file for psycle::host::CInfoDlg.
#pragma once
#include "Psycle.hpp"
#include <universalis/stdlib/date_time.hpp>

namespace psycle { namespace host {

using namespace universalis::stdlib;

/// info window.
class CInfoDlg : public CDialog {
	public:
		CInfoDlg(class ProjectData* projects, CWnd* pParent = 0);
		~CInfoDlg() {}

		void UpdateInfo();

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnInitDialog();
		afx_msg void OnTimer(UINT nIDEvent);		
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
		nanoseconds last_update_time_;
		ProjectData* projects_;
};

}}

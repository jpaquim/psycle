///\file
///\brief interface file for psycle::host::CInfoDlg.
#pragma once
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		class Song;
		/// info window.
		class CInfoDlg : public CDialog
		{
			public:
				void UpdateInfo();
				CInfoDlg(CWnd* pParent = 0);
				Song* _pSong;
				void InitTimer();
				int itemcount;
				enum { IDD = IDD_INFO };
				CStatic	m_mem_virtual;
				CStatic	m_mem_pagefile;
				CStatic	m_mem_phy;
				CStatic	m_mem_reso;
				CStatic	m_cpurout;
				CStatic	m_machscpu;
				CStatic	m_processor_label;
				CStatic	m_cpuidlelabel;
				CListCtrl	m_machlist;
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
				virtual BOOL OnInitDialog();
				afx_msg void OnTimer(UINT nIDEvent);
			DECLARE_MESSAGE_MAP()
		};
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

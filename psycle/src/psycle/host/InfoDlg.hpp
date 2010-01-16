///\file
///\brief interface file for psycle::host::CInfoDlg.
#pragma once
#include "Psycle.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		/// info window.
		class CInfoDlg : public CDialog
		{
		public:
			void UpdateInfo();
			CInfoDlg(class ProjectData* projects, CWnd* pParent = 0);
			void InitTimer();
			int itemcount;
		// Dialog Data
			//{{AFX_DATA(CInfoDlg)
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
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CInfoDlg)
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
			// Generated message map functions
			//{{AFX_MSG(CInfoDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnTimer(UINT nIDEvent);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		private:
			ProjectData* projects_;
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

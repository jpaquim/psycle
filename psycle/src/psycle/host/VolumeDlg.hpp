// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::CVolumeDlg.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		/// volume window.
		class CVolumeDlg : public CDialog
		{
		public:
			CVolumeDlg(CWnd* pParent = 0);
			float volume;
			int edit_type;
		// Dialog Data
			//{{AFX_DATA(CVolumeDlg)
			enum { IDD = IDD_NEW_VOLUME };
			CEdit		m_db;
			CEdit		m_per;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CVolumeDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			void DrawDb();
			void DrawPer();
			bool go;
			// Generated message map functions
			//{{AFX_MSG(CVolumeDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnChangeEditDb();
			afx_msg void OnChangeEditPer();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

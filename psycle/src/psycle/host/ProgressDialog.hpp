///\file
///\brief interface file for psycle::host::CProgressDialog.
#pragma once
#include "Psycle.hpp"

namespace psycle { namespace host {
		/// progress meter window.
		class CProgressDialog : public CDialog
		{
		public:
			CProgressDialog(CWnd* pParent = 0);
			BOOL Create();
			afx_msg void OnCancel();
			CWnd* m_pParent;
		// Dialog Data
			enum { IDD = IDD_PROGRESS_DIALOG };
			CProgressCtrl	m_Progress;
		// Overrides
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		protected:
			// Generated message map functions
			virtual BOOL OnInitDialog();
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace host
}   // namespace psycle

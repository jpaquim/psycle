///\interface psycle::host::CProgressDialog
#pragma once
#include <psycle/host/detail/project.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		/// progress meter window.
		class CProgressDialog : public CDialog
		{
			public:
				CProgressDialog(CWnd* pParent = 0);
				BOOL Create();
				afx_msg void OnCancel();
				CWnd* m_pParent;
				enum { IDD = IDD_PROGRESS_DIALOG };
				CProgressCtrl m_Progress;
			protected:
				virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
				virtual BOOL OnInitDialog();
			DECLARE_MESSAGE_MAP()
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

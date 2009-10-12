///\file
///\brief interface file for psycle::host::CWaveEdAmplifyDialog.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		#define AMP_DIALOG_CANCEL -10000

		/// wave amplification dialog window.
		class CWaveEdAmplifyDialog : public CDialog
		{
		// Construction
		public:
			CWaveEdAmplifyDialog(CWnd* pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CWaveEdAmplifyDialog)
			enum { IDD = IDD_WAVED_AMPLIFY };
			CEdit	m_dbedit;
			CEdit	m_edit;
			CSliderCtrl	m_slider;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveEdAmplifyDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveEdAmplifyDialog)
			virtual BOOL OnInitDialog();
			afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
			virtual void OnOK();
			virtual void OnCancel();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

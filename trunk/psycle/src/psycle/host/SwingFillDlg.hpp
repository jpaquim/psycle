///\file
///\brief interface file for psycle::host::CSwingFillDlg.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		/// swing fill dialog window.
		class CSwingFillDlg : public CDialog
		{
		public:
			CSwingFillDlg(CWnd* pParent = 0);
			BOOL bGo;
			int tempo;
			int width;
			float variance;
			float phase;
			BOOL offset;
		// Dialog Data
			//{{AFX_DATA(CSwingFillDlg)
			enum { IDD = IDD_SWINGFILL };
			CEdit	m_Tempo;
			CEdit	m_Width;
			CEdit	m_Variance;
			CEdit	m_Phase;
			CButton m_Offset;
				// NOTE: the ClassWizard will add data members here
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CSwingFillDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CSwingFillDlg)
				// NOTE: the ClassWizard will add member functions here
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

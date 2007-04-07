///\file
///\brief interface file for psycle::host::CWaveEdCrossfadeDialog.
#pragma once
#include "resources/resources.hpp"
NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

		class CWaveEdCrossfadeDialog : public CDialog
		{
		// Construction
		public:
			CWaveEdCrossfadeDialog(CWnd* pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CWaveEdCrossfadeDialog)
			enum { IDD = IDD_WAVED_CROSSFADE };
			CSliderCtrl		m_srcStartVol;
			CSliderCtrl		m_srcEndVol;
			CSliderCtrl		m_destStartVol;
			CSliderCtrl		m_destEndVol;
			CStatic			m_srcStartVolText;
			CStatic			m_srcEndVolText;
			CStatic			m_destStartVolText;
			CStatic			m_destEndVolText;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveEdCrossfadeDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		public:
			float srcStartVol;
			float srcEndVol;
			float destStartVol;
			float destEndVol;
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveEdCrossfadeDialog)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnCustomDrawSliders(NMHDR *pNMHDR, LRESULT *pResult);
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	NAMESPACE__END
NAMESPACE__END

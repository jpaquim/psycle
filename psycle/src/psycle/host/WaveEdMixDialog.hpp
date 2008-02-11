/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psycle::host::CWaveEdMixDialog.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class CWaveEdMixDialog : public CDialog
		{
		// Construction
		public:
			CWaveEdMixDialog(CWnd* pParent = 0);
		// Dialog Data
			//{{AFX_DATA(CWaveEdMixDialog)
			enum { IDD = IDD_WAVED_MIX };
			CSliderCtrl		m_srcVol;
			CSliderCtrl		m_destVol;
			CButton			m_bFadeIn;
			CButton			m_bFadeOut;
			CEdit			m_fadeInTime;
			CEdit			m_fadeOutTime;
			CStatic			m_destVolText;
			CStatic			m_srcVolText;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveEdMixDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		public:
			float srcVol;
			float destVol;
			bool bFadeIn;
			bool bFadeOut;
			float fadeInTime;
			float fadeOutTime;
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveEdMixDialog)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnCustomDrawDestSlider(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnCustomDrawSrcSlider(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnBnClickedFadeoutcheck();
			afx_msg void OnBnClickedFadeincheck();
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
